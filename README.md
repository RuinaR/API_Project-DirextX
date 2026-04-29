# KirbyEngine

Win32API와 DirectX9로 만든 2D/3D 게임 프레임워크를, ImGui 기반 에디터까지 갖춘 작은 게임 엔진 형태로 확장하고 있는 프로젝트입니다.

처음에는 커비 스타일의 게임을 만들기 위한 프레임워크에 가까웠지만, 작업을 진행하면서 `GameObject + Component`, JSON 기반 SceneData, Inspector/Hierarchy, 리소스 브라우저와 리소스 캐시 매니저까지 갖춘 미니 엔진/에디터 구조로 발전시키고 있습니다.

포트폴리오 관점에서는 단순히 기능을 붙인 것보다, 기존 코드에서 생기던 불안정한 지점들을 찾아서 고치고, 런타임과 에디터의 책임을 분리하고, 리소스 중복 생성 문제를 구조적으로 줄여가는 경험을 담은 프로젝트입니다.

## 프로젝트 개요

- `EngineFramework`: 엔진 공통 기능, GameObject/Component, 렌더링, 입력, UI, 리소스, SceneData 처리
- `KirbyGameDll`: 게임 코드, 사용자 컴포넌트, 씬 구성, 게임 컴포넌트 등록
- `BuildEdit`: 에디터 모드 실행 프로젝트
- `BuildResultGame`: 결과 게임 모드 실행 프로젝트
- `SceneData`: 에디터에서 저장/로드하는 JSON 씬 데이터
- `Resources`: AssetDatabase가 스캔하는 리소스 배치 폴더

사용 기술:

- C++17
- Win32API
- DirectX9
- ImGui
- Box2D
- FBX SDK
- JSON 기반 SceneData

## 빌드 및 실행

현재 주 검증 환경은 `Debug | Win32`입니다.

Visual Studio:

1. `Solution_Kirby/Solution_Kirby.sln`을 엽니다.
2. `Debug | Win32` 구성을 선택합니다.
3. `BuildEdit` 또는 `BuildResultGame`을 시작 프로젝트로 설정합니다.
4. 빌드 후 실행합니다.

스크립트 빌드:

```powershell
cd Solution_Kirby
powershell -ExecutionPolicy Bypass -File .\Scripts\Invoke-MSBuild.ps1 -Configuration Debug -Platform Win32
```

VSCode:

1. `Solution_Kirby` 폴더를 workspace root로 엽니다.
2. `.vscode/tasks.json`의 MSBuild 작업으로 빌드합니다.
3. `.vscode/launch.json`의 디버그 구성을 사용합니다.
4. include 빨간줄은 `.vscode/c_cpp_properties.json`에 엔진/리소스/DirectX SDK include 경로를 맞춰 해결했습니다.

DirectX9 관련 참고:

- NuGet 복원 기반으로 `Microsoft.DXSDK.D3DX` 패키지를 사용할 수 있게 정리했습니다.
- 기존 PC에 DirectX SDK June 2010이 설치되어 있으면 그 경로도 사용할 수 있습니다.
- `External/DirectXSDK`처럼 로컬에만 있는 SDK 폴더에 의존하지 않도록 바꿔, 다른 PC에서도 복원 가능한 방향으로 정리했습니다.

## 현재 구조

```text
Solution_Kirby
├─ EngineFramework
│  ├─ Component
│  │  ├─ Collision
│  │  └─ Physics
│  ├─ Editor
│  ├─ Debugging
│  ├─ Input
│  ├─ Rendering
│  │  ├─ Buffer
│  │  ├─ Camera
│  │  ├─ Component
│  │  └─ UI
│  ├─ Resource
│  │  ├─ Animation
│  │  ├─ Texture
│  │  ├─ AssetDatabase
│  │  ├─ ResourceBrowser
│  │  └─ ResourceManager
│  ├─ Scene
│  └─ Utility
├─ KirbyGameDll
├─ BuildEdit
├─ BuildResultGame
├─ SceneData
├─ Resources
├─ External
├─ Box2D
├─ Imgui
└─ fbxsdk
```

## 핵심 기능

### GameObject / Component 구조

`GameObject`에 여러 `Component`를 붙여 동작을 구성하는 방식입니다. 엔진 기본 컴포넌트와 게임 DLL의 사용자 컴포넌트를 같은 흐름으로 등록하고 생성할 수 있도록 `ComponentFactory`를 추가했습니다.

현재 등록 흐름:

1. 엔진 초기화 시 `RegisterEngineComponents()` 호출
2. 게임 DLL 초기화 시 `RegisterGameComponents(ComponentFactory& factory)` 호출
3. SceneData 로드 시 `ComponentFactory::Create(typeName, dataJson)` 사용
4. Inspector Add Component UI도 같은 등록 목록 사용

등록 정보에는 `typeName`, `displayName`, `category`, `canAddFromEditor`, `createFn`을 둬서 저장용 타입 문자열과 에디터 표시 이름을 분리했습니다.

### Editor

ImGui 기반으로 Hierarchy, Inspector, Resource Browser를 구현했습니다. 에디터는 `BuildEdit`에서만 사용하는 전용 UI이며, 게임 모드에서는 ImGui 기반 편집 UI가 뜨지 않도록 분리했습니다.

현재 가능한 작업:

- Scene 저장
- 저장 필요 상태 표시
- GameObject 선택
- GameObject 이름, active, position, size, angle 편집
- Component 목록 표시
- Component 추가/삭제
- GameObject 생성/삭제
- Resource Browser에서 리소스 목록 확인
- Inspector에서 AssetField로 리소스 선택

생성 프리셋:

- `Create Empty`
- `Create Sprite`
- `Create UI Image`
- `Create UI Button`
- `Create UI Text`

프리셋으로 생성한 오브젝트는 바로 화면에서 확인할 수 있도록 기본 size/color/actionKey 등을 보정합니다.

### Inspector

선택된 GameObject의 모든 Component는 공통 흐름에서 `DrawInspector()`를 호출합니다. 특정 컴포넌트만 ObjectManager 안에서 하드코딩으로 그리던 구조를 줄이고, Component가 자기 Inspector를 직접 그리는 방향으로 정리했습니다.

현재 Inspector가 지원하는 주요 필드:

- `ImageRender`: size, positionOffset, texture asset/path, color, render enabled, orderInLayer
- `UIImage`: texture asset/path, color, useTexture
- `UIButton`: actionKey, normal/hover/pressed color
- `UILabel`: text, fontSize, scale, color
- `FBXRender`: model asset/path, 로드된 모델 정보
- `BoxCollider`: bodyType, trigger, fixedRotation, size, offset

값을 수정하면 dirty flag가 켜지고, Save Scene 성공 시 dirty 상태가 해제됩니다.

### ObjectManager 책임 분리

초기에는 `ObjectManager`가 런타임 오브젝트 관리뿐 아니라 Hierarchy, Inspector, 생성 프리셋, Resource Browser 호출까지 모두 담당했습니다. 기능이 늘어날수록 수정 지점이 한 파일에 몰려서, 에디터 UI 책임을 별도 클래스로 분리했습니다.

현재 `ObjectManager`에 남긴 책임:

- GameObject 리스트 관리
- pending add/remove
- AddObject / DestroyObject / FindObject
- Update
- FlushPendingObjects
- Serialize / Deserialize
- 선택 오브젝트 getter/setter

분리한 에디터 클래스:

- `EditorObjectFactory`: GameObject 생성 프리셋과 기본값 보정
- `EditorHierarchyWindow`: Hierarchy 표시, 선택, 생성/삭제 버튼
- `EditorInspectorWindow`: GameObject/Component Inspector, Add/Remove Component
- `ResourceBrowser`: AssetDatabase 목록 표시, 타입 필터, Refresh, 상세 정보
- `EditorAssetField`: Inspector에서 타입별 리소스 선택 UI 제공

`ObjectManager::ImguiUpdate()`는 에디터 UI 호출을 위임하는 얇은 함수가 되도록 정리했습니다.

### SceneData

SceneData는 현재 v3 포맷을 유지합니다. 런타임 포인터나 DirectX 리소스 포인터는 저장하지 않고, 다시 복원 가능한 문자열과 수치 데이터만 저장합니다.

저장 대상:

- 씬 이름
- 카메라 위치/회전
- GameObject 기본 정보
- parent/child 관계
- Component 배열
- Component `type` 문자열
- Component별 JSON data
- UIButton `actionKey`
- 리소스 assetKey/path 문자열

저장하지 않는 대상:

- raw pointer
- DirectX texture/buffer pointer
- Box2D body pointer
- callback/lambda 본체
- RenderManager 등록 상태
- runtime cache
- ImGui editor state

로드 흐름:

1. Scene 초기화
2. `SceneData/{SceneName}.json` 존재 여부 확인
3. SceneData가 있으면 로드
4. 없거나 실패하면 코드 기반 초기 씬 생성
5. `ObjectManager::FlushPendingObjects()` 실행
6. Scene Start 호출

### DirectX UI 시스템

게임 UI는 DirectX 렌더링 기반이고, 에디터 조작 UI는 ImGui 기반입니다.

주요 클래스:

- `UIElement`: UI 공통 베이스, visible/enabled/orderInLayer, GameObject Transform 연동
- `UIImage`: 내부적으로 `ImageRender`를 사용해 이미지 또는 단색 사각형 렌더링
- `UIButton`: hover/pressed/clicked 상태, `actionKey`, onClick 지원
- `UILabel`: `ID3DXFont` 기반 텍스트 렌더링
- `UIActionRegistry`: SceneData 로드 후 `actionKey`를 실제 콜백으로 재연결

UI 기준:

- UI는 screen-space로 렌더링됩니다.
- UI 위치는 `GameObject position + localOffset` 기준입니다.
- UI 크기는 GameObject size를 기준으로 계산합니다.
- 클릭 판정도 렌더 위치와 같은 기준을 사용합니다.
- `orderInLayer`로 UI 렌더 순서를 제어합니다.

### Resource / Asset 시스템

이번 확장에서 가장 크게 정리한 부분입니다.

`AssetDatabase`는 `Resources` 폴더를 스캔해 리소스 메타데이터만 관리합니다.

`AssetInfo`:

- `key`: SceneData에 저장 가능한 상대 경로 문자열
- `path`: 실제 파일 경로
- `fileName`: 파일명
- `type`: Texture / Model / Animation / Sound / Unknown
- `extension`: 확장자

리소스 타입 분류:

- Texture: `.png`, `.jpg`, `.jpeg`, `.bmp`, `.dds`
- Model: `.fbx`
- Animation: `.anim` 또는 animation 성격의 폴더명 기반
- Sound: `.wav`, `.mp3`, `.ogg`
- Unknown: 그 외

실제 리소스 로딩과 캐시는 `ResourceManager`가 담당합니다. `ResourceManager`는 타입별 매니저를 소유하고, 외부에서는 최종적으로 ResourceManager를 통해 리소스를 요청합니다.

```text
Inspector / Component / Game Code
-> ResourceManager
-> TextureManager / FBXManager / AnimationManager / SoundManager
-> AssetDatabase에서 assetKey를 실제 path로 해석
-> 캐시에 있으면 재사용, 없으면 로드 후 캐시
```

타입별 매니저:

- `TextureManager`: `IDirect3DTexture9*` 캐시
- `FBXManager`: `FbxResource` 캐시
- `AnimationManager`: animation folder + time 기준 `Animation` 데이터 캐시
- `SoundManager`: 사운드 리소스 메타/로드 구조의 출발점

모든 타입별 매니저는 공통 인터페이스 `IResourceTypeManager`를 구현합니다.

```cpp
class IResourceTypeManager
{
public:
    virtual ~IResourceTypeManager() {}
    virtual void ReleaseAllResources() = 0;
};
```

캐시 컨테이너는 텍스처 매니저와 동일하게 `std::unordered_map`을 사용하도록 통일했습니다. 같은 assetKey로 요청되는 리소스는 중복 생성하지 않고 기존 리소스를 반환합니다.

### FBX 리소스 공유

기존에는 `FBXRender`가 직접 `FbxTool`을 들고 FBX 파일을 로드했습니다. 같은 FBX를 여러 오브젝트가 사용하면 모델 데이터와 버퍼가 중복 생성될 수 있었습니다.

현재는:

```text
FBXRender
-> ResourceManager::GetModel(assetKey)
-> FBXManager
-> FbxResource 캐시 반환
```

`FbxResource`는 FBX 로드 결과와 모델 버퍼를 보관합니다.

```cpp
struct FbxResource
{
    std::string key;
    std::string path;
    FbxTool tool;
    std::vector<Model> models;
    bool loaded = false;
};
```

FBX 내부 material texture도 `ResourceManager::GetTexture()`를 통해 받아, 모델 텍스처 역시 TextureManager 캐시를 재사용합니다.

## 문제 해결 경험

### 씬 전환 중 use-after-free / iterator invalidation

버튼 클릭 콜백에서 씬 전환이 바로 일어나면, 현재 프레임에서 순회 중이던 오브젝트나 컴포넌트가 해제되어 문제가 발생할 수 있었습니다.

해결 방향:

- GameObject 삭제를 pending remove로 처리
- 순회 중 직접 erase하지 않도록 정리
- 삭제 대상의 자식 트리까지 안전한 순서로 제거
- 선택된 오브젝트가 삭제되면 Inspector 선택 상태를 즉시 해제
- UIButton 콜백 이후 불필요한 멤버 접근을 줄임

### UI 렌더 위치와 클릭 판정 불일치

UI가 보이는 위치와 실제 클릭 판정 위치가 어긋나는 문제가 있었습니다. UI 컴포넌트 위치 기준이 컴포넌트 내부 position과 GameObject position 사이에서 섞여 있었기 때문입니다.

해결 방향:

- UI 위치를 `GameObject position + localOffset` 기준으로 통일
- UIImage/UIButton/UILabel 모두 같은 기준 적용
- 클릭 판정도 렌더 위치와 같은 기준 사용
- 기존 SceneData v3의 UI position/size는 로드 시 GameObject Transform으로 이전 적용

### 리소스 중복 생성

Sprite, UI Image, FBX 모델을 여러 곳에서 사용할 때 같은 파일이 반복 로드될 수 있었습니다. 특히 FBX는 모델 데이터와 vertex/index buffer까지 중복될 가능성이 있어 구조적으로 정리가 필요했습니다.

해결 방향:

- `AssetDatabase`: 리소스 메타데이터만 관리
- `ResourceManager`: 모든 리소스 요청의 단일 진입점
- 타입별 매니저: 실제 로드와 캐시 담당
- 같은 assetKey 요청 시 기존 리소스 재사용
- FBX material texture도 TextureManager 캐시 경유

### ObjectManager 비대화

ObjectManager가 런타임 관리와 에디터 UI까지 모두 담당하면서 수정 위험이 커졌습니다.

해결 방향:

- 런타임 오브젝트 관리 책임은 ObjectManager에 유지
- 에디터 전용 UI와 생성 프리셋은 Editor 클래스로 분리
- 선택 상태는 ObjectManager의 getter/setter로 최소 공유
- SceneData 포맷과 pending add/remove 흐름은 유지

## 현재 빌드 확인

최근 확인:

```text
Configuration: Debug
Platform: Win32

EngineFramework   Success
KirbyGameDll      Success
BuildEdit         Success
BuildResultGame   Success
```

기존 경고는 남아 있지만, 현재 리소스 매니저 및 에디터 구조 변경 후 `Debug | Win32` 빌드는 통과했습니다.

## 향후 작업 체크포인트

다음 단계로 생각 중인 작업들입니다.

- assetKey/path 정규화 강화
- 동일 파일이 다른 문자열로 요청되어도 하나의 캐시로 묶기
- SoundManager 실제 사운드 로딩 연결
- Animation 리소스를 폴더 단위 asset으로 더 명확하게 선택하는 UI
- Resource Browser 드래그 앤 드롭
- Inspector AssetField와 Resource Browser 선택 상태 연동
- SceneData v4 설계
- New/Open/Save As Scene 기능
- BuildSettings.json 설계
- Build Game 메뉴와 패키징 기능
- Play/Edit Mode 분리

## 개발 원칙

- 기존 구조를 한 번에 갈아엎기보다, 동작하는 상태를 유지하면서 점진적으로 확장합니다.
- SceneData에는 런타임 포인터, DirectX 리소스 포인터, Box2D body, callback/lambda를 저장하지 않습니다.
- 게임 모드와 에디터 모드의 책임을 분리합니다.
- 리소스는 assetKey/path 문자열로 참조하고, 실제 로드는 ResourceManager를 통해 처리합니다.
- ObjectManager의 pending add/remove 흐름은 우회하지 않습니다.
- raw pointer 기반의 기존 구조를 무리하게 스마트 포인터로 바꾸지 않고, 현재 코드베이스와 맞는 범위에서 안정성을 높입니다.

