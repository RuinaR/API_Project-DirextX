# KirbyEngine

C++17, Win32API, DirectX9, ImGui, Box2D, FBX SDK 기반으로 만드는 미니 게임엔진/에디터 프로젝트입니다.

이 문서는 최근 작업 내용을 문제 해결 중심으로 정리한 진행 기록입니다.

## 현재 방향

- `GameObject + Component` 구조 유지
- SceneData JSON 기반 저장/로드 확장
- 에디터(`BuildEdit`)와 게임 실행(`BuildResultGame`) 책임 분리
- 리소스는 문자열 key/path로 저장하고 런타임 포인터는 저장하지 않음
- 기존 프로젝트를 한 번에 갈아엎기보다, 동작을 유지하면서 단계적으로 엔진화

## 최근 해결한 문제

### 1. SceneData v4 최소 구조 도입

기존 v3 저장 구조를 크게 깨지 않으면서 v4 object 구조를 정리했습니다.

- 최상위: `version`, `sceneName`, `camera`, `objects`
- object: `id`, `parentId`, `name`, `tag`, `active`, `transform`, `components`
- `transform`: `position`, `rotation`, `size`
- component: `{ "type": "...", "data": { ... } }`

핵심 포인트:

- v4 저장은 `version: 4` 사용
- v3 파일도 계속 로드 가능
- parent/child는 `parentId`로 저장하고 로드 후 관계 재구성
- SceneData에는 raw pointer, DirectX 포인터, FBX runtime object, Box2D body pointer, callback/lambda를 저장하지 않음

### 2. Component type 기반 저장/복원 점검

`ComponentFactory` 기반으로 SceneData 로드 시 component를 복원하는 흐름을 점검했습니다.

확인한 주요 컴포넌트:

- `ImageRender`
- `AnimationRender`
- `FBXRender`
- `UIImage`
- `UIButton`
- `UILabel`
- `BoxCollider`

정리 결과:

- `GameObject::Serialize()`는 모든 component를 `{type, data}` 형태로 저장
- `ObjectManager::DeserializeComponents()`는 `type` 문자열로 `ComponentFactory::Create()` 호출
- 주요 기본 컴포넌트의 등록명과 `GetSerializableType()` 문자열이 일치
- 런타임 포인터나 콜백 본체는 SceneData에 직접 저장되지 않음

### 3. AnimationRender SceneData 저장/로드 정리

`.anim` 폴더 기반 애니메이션을 SceneData에 안전하게 저장/복원하도록 정리했습니다.

저장 항목:

- `animationFolderKey`
- `frameDuration`
- `loop`
- `playOnStart`
- `useColorKey`
- 호환용 `sourcePath`, `frameTime`, `play`, `oneTime`

복원 방식:

- `Deserialize()` 후 `animationFolderKey`가 있으면 `ReloadFromFolder()` 호출
- `ReloadFromFolder() -> RebuildClipFromFolder() -> RebuildRuntimeFrames()` 순서로 복원
- runtime frame cache와 `IDirect3DTexture9*`는 SceneData에 저장하지 않음

### 4. AnimationRender 마젠타 컬러키 적용

`AnimationRender`가 이미지 시퀀스를 재생할 때도 `ImageRender`처럼 마젠타 컬러키 투명 처리를 적용했습니다.

반영 내용:

- `useColorKey` 필드 추가
- 기본값 `true`
- Inspector에서 `Use Color Key` 편집 가능
- `.anim` 프레임 texture 로드시 `ResourceManager::GetTexture(textureKey, useColorKey)` 사용
- SceneData 저장/로드에 `useColorKey` 포함

### 5. AnimationRender Inspector 개선

Inspector에서 애니메이션 상태를 더 직접 확인할 수 있도록 정리했습니다.

추가된 정보:

- 현재 재생 프레임 인덱스
- 현재 타이머 값
- runtime frame 개수
- frame list (`textureKey`, `duration`, 현재 프레임 표시)

목적:

- SceneData 저장/로드 후 애니메이션이 실제로 복원됐는지 바로 확인 가능
- 썸네일 없이도 디버깅 가능한 최소 미리보기 제공

### 6. AnimationRender 로드 시 ImageRender 중복 생성 문제 수정

문제:

- SceneData 로드 중 `AnimationRender::Initialize()`가 먼저 실행되면서 내부용 `ImageRender`를 생성
- 이후 저장되어 있던 `ImageRender`가 다시 deserialize되어 같은 오브젝트에 중복 생성

해결:

- SceneData 로드 시 component를 바로 `Initialize()`하지 않고 먼저 전부 붙임
- 그 다음 일괄 `Initialize()`
- 마지막에 `Deserialize()` 실행

효과:

- `AnimationRender`가 이미 존재하는 `ImageRender`를 재사용 가능
- 저장/로드 반복 시 `ImageRender`가 늘어나는 문제 방지

### 7. UIButton actionKey 저장/복원 유지

버튼 콜백 자체는 SceneData에 저장하지 않고, `actionKey`만 저장한 뒤 로드 시 `UIActionRegistry`로 다시 바인딩하는 구조를 유지했습니다.

정리 내용:

- `UIButton::Serialize()`는 `actionKey`만 저장
- `UIButton::Deserialize()`는 `actionKey` 복원 후 `BindActionFromRegistry()` 호출
- 씬 `Init()`에서 `UIActionRegistry::RegisterAction(...)`를 먼저 등록하고, 이후 SceneData 로드 중 버튼이 재바인딩

추가 안전장치:

- `actionKey`가 비어 있거나 registry에 없는 경우 `SetOnClick(nullptr)`로 이전 콜백 제거
- 미등록 key에서도 크래시 없이 무시

주의:

- 현재는 씬 클래스의 `Init()`에서 action 등록을 하기 때문에, 나중에 JSON 파일만으로 씬을 여는 구조로 가면 action 등록을 전역 bootstrap 단계로 분리해야 함

### 8. Hierarchy 드래그 앤 드롭 부모/자식 편집

Hierarchy에서 오브젝트를 드래그해서 parent/child 관계를 바꿀 수 있게 했습니다.

동작:

- A를 B 위에 드롭하면 `A.parent = B`
- `Make Root`로 parent를 `nullptr`로 변경 가능
- 자기 자신에게 드롭 금지
- 자기 자식 밑으로 드롭 금지
- 변경 즉시 dirty 처리

### 9. FBXRotateObj 저장/복원 및 부모-자식 회전 누적

문제:

- `FBXRotateObj`가 SceneData에 저장되지 않아서 재실행 후 회전 동작이 사라질 수 있었음
- 부모와 자식 모두 회전 컴포넌트를 가졌을 때 자식이 같은 속도로만 도는 것처럼 보였음

해결:

- `FBXRotateObj`에 직렬화 type 추가
- SceneData에 `{ "type": "FBXRotateObj", "data": {} }`로 저장 가능
- `GameObject::SetAngleX/Y/Z()`가 절대값이 아니라 delta를 자식에게 전파하도록 수정

효과:

- 부모 회전 + 자식 자체 회전이 누적되는 구조로 정리

### 10. 시작 씬 로드 흐름 정리

현재 시작 흐름은 코드 기반 씬 객체를 먼저 만들되, 실제 오브젝트는 `SceneData/StartScene.json`이 있으면 그 파일을 우선 로드합니다.

흐름:

1. `WindowFrame::SetScene(new StartScene())`
2. `scene->Init()` 실행
3. `SceneData/StartScene.json` 존재 시 파일 로드
4. 로드 실패 또는 파일 없음일 때만 `BuildInitialSceneObjects()` fallback

이 구조 덕분에:

- 씬별 action registry 등록 코드(`Init`)는 살아 있음
- SceneData 오브젝트는 JSON에서 복원 가능

### 11. SceneData 샘플 검증

최근 저장된 `SceneData/StartScene.json`을 기준으로 확인한 결과:

- 최상위 `version: 4`
- `sceneName`, `camera`, `objects` 존재
- object는 `id`, `parentId`, `name`, `tag`, `active`, `transform`, `components` 구조 사용
- component는 `{type, data}` 구조 유지
- `UIButton.actionKey`, `FBXRender.fbxPath`, `AnimationRender.animationFolderKey`, `ImageRender.texturePath` 등 문자열 기반 저장 확인
- DirectX texture pointer, FBX runtime object, Box2D body pointer, callback/lambda, runtime frame cache는 JSON에 없음

## 창 크기/렌더링 관련 정리

### 현재 결정

실행 중 창 리사이즈는 당분간 비활성화했습니다.

이유:

- DX9 device reset, editor render target, ImGui, 내부 고정 좌표계가 모두 안정적으로 정리되기 전까지는 리사이즈가 렌더 정지/좌표 불일치 문제를 만들 가능성이 큼

현재 상태:

- 창 테두리 드래그로 크기 변경 불가
- 시작 시 창 client 크기를 `DRAWWINDOWW`, `DRAWWINDOWH` 기준으로 생성
- 내부 렌더 해상도와 시작 창 크기를 맞춤

즉, 지금은 “실행 시 정한 크기에서만 동작”하는 방향으로 안정성을 우선 확보했습니다.

## 현재 SceneData에 저장하는 것 / 저장하지 않는 것

저장하는 것:

- scene 이름
- camera position/rotation
- object id/parentId/name/tag/active/transform
- component type 문자열
- component별 직렬화 데이터
- asset key/path 문자열
- UIButton actionKey

저장하지 않는 것:

- raw pointer
- `IDirect3DTexture9*`
- FBX runtime object
- `b2Body*`
- callback/lambda 본체
- RenderManager 등록 상태
- runtime frame cache
- ImGui/editor UI state

## 현재 빌드 확인

최근 작업 기준 빌드 확인:

- `EngineFramework`: 성공
- `KirbyGameDll`: 성공
- `BuildEdit`: 성공
- `BuildResultGame`: 성공
- 구성: `Debug | x86`

기존 경고는 남아 있지만, 최근 SceneData/Animation/UIAction/창 고정 변경으로 빌드가 깨지지는 않는 상태입니다.

## 다음 단계 메모

다음에 이어서 할 만한 작업:

- Scene 클래스를 거치지 않고 JSON 파일만으로 씬을 여는 구조 정리
- 그에 맞춘 `UIActionRegistry` 전역 등록/bootstrap 단계 분리
- SceneData 기반 New/Open/Save As 흐름 강화
- 시작 씬 선택/빌드 설정 분리
- SceneData v4 구조를 기준으로 추가 컴포넌트 직렬화 확장
