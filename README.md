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

정리 결과:

- 버튼 callback/lambda 본체는 SceneData에 저장하지 않음
- `actionKey` 문자열만 저장
- 로드 시 `UIActionRegistry`에서 다시 바인딩
- 현재 action 등록은 `KirbyGameDll/UserActions/GameActionRegistry.cpp` bootstrap에서 수행

### 8. Hierarchy 드래그 앤 드롭 부모/자식 편집

Hierarchy에서 오브젝트를 드래그해서 parent/child 관계를 바꿀 수 있게 했습니다.

동작:

- A를 B 위에 드롭하면 `A.parent = B`
- `Make Root`로 parent를 `nullptr`로 변경 가능
- 자기 자신에게 드롭 금지
- 자기 자식 밑으로 드롭 금지
- 변경 즉시 dirty 처리

### 9. BuildSettings 기반 시작 씬 로드 / BuildOutput 패키징

현재 시작 씬은 코드 하드코딩이 아니라 `BuildSettings.json` 기준으로 결정합니다.

구조:

```json
{
  "version": 1,
  "startScene": "TestScene2",
  "scenes": ["TestScene1", "TestScene2"]
}
```

정리 결과:

- `KirbyGameDll` startup은 `RuntimeScene + BuildSettings + SceneData` 기준
- 에디터 fallback: `BuildSettings`가 없거나 잘못되면 빈 `NewScene`
- 게임 fallback: `BuildSettings`가 없거나 잘못되면 `StartScene`
- `Build Game`은 `Release|x64` 기준으로 자동 빌드 후 `BuildOutput` 패키징
- `BuildOutput`에는 `BuildResultGame.exe`, `KirbyGameDll.dll`, 필요한 DLL, `BuildSettings.json`, `SceneData`, `Resources`가 들어감
- 패키지 실행 시에는 `exe` 옆 `BuildSettings.json`을 우선 읽음

### 10. BuildOutput 리소스 최소 포함

이제 `BuildOutput/Resources`는 전체 리소스 복사가 아니라, 빌드 대상 씬들이 실제로 참조하는 리소스만 복사합니다.

현재 분석 대상 key:

- `texturePath`
- `imagePath`
- `fbxPath`
- `animationFolderKey`
- `soundPath`
- `audioPath`
- `bgmPath`
- `sfxPath`

정책:

- `FBX`: 본체 파일 + 같은 이름의 `.fbm` 폴더
- `Animation`: `.anim` 폴더 전체
- `Image/Texture`: 개별 파일만
- `Sound`: `.wav/.mp3/.ogg` 경로 문자열 기반 포함 틀 준비

### 11. KirbyGameDll 테스트 코드 정리

기존 테스트용 Scene/Component 대부분을 정리했습니다.

제거된 주요 항목:

- `StartScene`, `GameScene`, `EditerScene`, `SceneChanger`
- `Door`, `FBXRotateObj`
- `StageMaker`, `Edit`, `game map` 테스트 계열
- `AttackEvent`, `AttakObject`, `ChangeObject`, `MonsterAI`
- `Player`, `LandChecker`, `KirbyGameTypes`

현재 `KirbyGameDll` 방향:

- 최소 bootstrap 유지
- `RegisterGameComponents()` 유지
- `RegisterGameUIActions()` 유지
- 사용자 확장 코드는 별도 폴더로 분리

### 12. UserComponents / UserActions 구조 정리

사용자 C++ 확장 작업 영역을 `KirbyGameDll` 안으로 정리했습니다.

구조:

- `KirbyGameDll/UserComponents/GameComponentRegistry.cpp`
- `KirbyGameDll/UserComponents/UserComponentRegistration.h`
- `KirbyGameDll/UserComponents/Scripts/...`
- `KirbyGameDll/UserActions/GameActionRegistry.cpp`
- `KirbyGameDll/UserActions/Scripts/...`
- `KirbyGameDll/UserExtensionGuide.md`

정책:

- 게임별 코드는 `EngineFramework`가 아니라 `KirbyGameDll`에 둠
- 새 C++ `Component/Action` 추가 시 hot reload는 지원하지 않음
- `DLL 재빌드 + 에디터 재실행` 후 반영

샘플:

- `SampleSpinComponent`
- `SampleButtonAction`

### 13. Camera 저장/편집 확장

에디터 `Hierarchy`에서 Camera를 직접 편집할 수 있게 정리했습니다.

현재 편집 가능 항목:

- `position x/y/z`
- `rotation x/y/z`
- `projection mode`
- `fov`
- `orthographicSize`
- `nearClip`
- `farClip`

SceneData 저장 구조:

```json
"camera": {
  "position": { "x": 0, "y": 0, "z": 0 },
  "rotation": { "x": 0, "y": 0, "z": 0 },
  "projection": {
    "mode": "Orthographic",
    "fov": 0.785398,
    "orthographicSize": 720.0,
    "nearClip": 1.0,
    "farClip": 1000.0
  }
}
```

호환 정책:

- `projection` 필드가 없는 기존 SceneData도 정상 로드
- 없는 경우 Camera 기본값 사용

### 14. 에디터 Playback 제어 추가

본격 Play/Edit Mode 분리 전 단계로, 에디터에서 현재 씬의 Update 진행 여부를 제어할 수 있게 했습니다.

현재 상태:

- `Play`
- `Pause`
- `Step`
- `Playback: Playing / Paused / Step Pending` 표시

동작:

- `Playing`: `b2World::Step + ObjectManager::Update()` 실행
- `Paused`: 게임 오브젝트/컴포넌트 update 중단
- `Step`: 1프레임만 실행 후 다시 `Paused`
- ImGui/Hierarchy/Inspector/Render는 `Paused`여도 계속 동작

보강:

- `DeltaTime()`는 실제 시뮬레이션 프레임 기준으로만 반환
- `Paused` 중 장시간 대기 후 `Play`해도 delta spike를 줄임
- `Paused` 상태에서도 pending add/remove는 flush해서 Hierarchy/저장/패키징과 어긋나지 않게 정리

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
- camera projection 설정
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
- 기본 패키징 기준: `Release | x64`
- 에디터 확인 기준: `Debug | x64`

주의:

- 현재 반복되는 빌드 실패는 대체로 코드 오류보다 `Box2D obj\\Debug_x64\\*.tlog` 접근 거부 같은 환경 잠금 이슈였습니다.
- `Build Game` 패키징은 `Release|x64` 기준으로 동작합니다.

## 다음 단계 메모

다음에 이어서 할 만한 작업:

- Play/Edit Mode 복제본 분리
- 사운드 컴포넌트/데이터 구조 추가 후 리소스 최소 복사와 실제 연결
- 사용자 Component/Action 예제 확장
- SceneData 기반 워크플로우 계속 강화
