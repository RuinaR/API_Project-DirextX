# KirbyEngine

Win32API, DirectX9 기반의 2D/3D 게임 프레임워크와 ImGui 기반 에디터 프로젝트입니다. 현재 구조는 단순 샘플 게임을 넘어, `GameObject + Component + SceneData + Editor` 흐름을 갖춘 미니 게임 엔진/에디터 형태로 점진 확장 중입니다.

## 프로젝트 개요

- `EngineFramework`: 공통 엔진 기능, GameObject/Component, 렌더링, 입력, 리소스, UI, SceneData 처리
- `KirbyGameDll`: 게임 코드, 사용자 컴포넌트, 씬 구성, 게임 컴포넌트 등록
- `BuildEdit`: 에디터 모드 실행 프로젝트
- `BuildResultGame`: 결과 게임 모드 실행 프로젝트
- `SceneData`: 에디터에서 저장/로드하는 JSON 씬 데이터
- `Resources`: 텍스처, 모델, 애니메이션 등 게임 리소스 배치 대상

사용 기술은 C++17, Win32API, DirectX9, ImGui, Box2D, FBX SDK입니다.

## 빌드 및 실행 환경

현재 `Debug|Win32`, `Debug|x64` 빌드를 모두 지원하도록 정리되어 있습니다.

Visual Studio:

1. `Solution_Kirby/Solution_Kirby.sln`을 엽니다.
2. `Debug|Win32` 또는 `Debug|x64` 구성을 선택합니다.
3. `BuildEdit` 또는 `BuildResultGame`을 시작 프로젝트로 설정합니다.
4. 빌드 후 실행합니다.

VSCode:

1. `Solution_Kirby` 폴더를 workspace root로 엽니다.
2. `.vscode/tasks.json`의 MSBuild 작업으로 빌드합니다.
3. `.vscode/launch.json`의 디버그 구성을 사용합니다.
4. `cppvsdbg` 오류가 날 경우 VSCode C/C++ 확장을 설치해야 합니다.

스크립트 빌드 예시:

```powershell
powershell -ExecutionPolicy Bypass -File Scripts\Invoke-MSBuild.ps1 -Configuration Debug -Platform Win32
powershell -ExecutionPolicy Bypass -File Scripts\Invoke-MSBuild.ps1 -Configuration Debug -Platform x64
```

주의:

- DirectX9 기반이므로 DirectX SDK June 2010 또는 프로젝트에 포함된 DirectX SDK 경로가 필요합니다.
- x64 빌드를 위해 `Lib/x64/libfbxsdk.lib`, `libfbxsdk.dll` 경로가 정리되어 있습니다.
- 실행 기준 경로는 기본적으로 `Solution_Kirby`입니다.

## 현재 구조

```text
Solution_Kirby
├─ EngineFramework
│  ├─ Component
│  │  ├─ Collision
│  │  └─ Physics
│  ├─ Debugging
│  ├─ Input
│  ├─ Rendering
│  │  ├─ Buffer
│  │  ├─ Camera
│  │  ├─ Component
│  │  └─ UI
│  ├─ Resource
│  │  ├─ Animation
│  │  └─ Texture
│  ├─ Scene
│  └─ Utility
├─ KirbyGameDll
├─ BuildEdit
├─ BuildResultGame
├─ SceneData
├─ External
├─ Box2D
├─ Imgui
└─ fbxsdk
```

## 현재까지의 주요 작업

2026-04-28 기준으로 다음 작업이 반영되어 있습니다.

- Win32/x64 빌드 구성 정리
- VSCode MSBuild/디버그 설정 정리
- DirectX SDK 및 FBX SDK 경로 정리
- `ObjectManager` pending add/remove 흐름 정리
- `ComponentFactory` 추가
- `RegisterEngineComponents()` 추가
- `KirbyGameDll`의 `RegisterGameComponents(ComponentFactory&)` 구조 추가
- SceneData v3 로드가 ComponentFactory 기반으로 동작하도록 변경
- Inspector Add Component UI 추가
- Inspector Remove Component UI 추가
- Hierarchy GameObject 생성 기능 추가
- GameObject 생성 프리셋 추가
- Add Component 후 기본값 보정 추가
- UI 컴포넌트의 position/size를 GameObject Transform 기준으로 연동
- UI 렌더 위치와 클릭 판정 좌표 정리
- Hierarchy GameObject 삭제 기능 추가
- 씬 전환 중 iterator invalidation 및 stack overflow 문제 수정

## ObjectManager 생명주기

`ObjectManager`는 GameObject 추가/삭제를 즉시 메인 컨테이너에 반영하지 않고 pending 큐를 사용합니다.

- `AddObject()`는 `m_pendingAddObjects`에 등록합니다.
- `DestroyObject()`는 삭제 대상과 자식 트리를 `m_pendingRemoveObjects`에 등록합니다.
- `FlushPendingObjects()`에서 안전한 시점에 실제 추가/삭제를 반영합니다.
- `Update`, `Render`, `ImguiUpdate` 순회 중 직접 erase하지 않는 방향을 유지합니다.
- 삭제된 오브젝트가 Inspector에 남지 않도록 `m_selected`를 즉시 `nullptr`로 정리합니다.

부모/자식 관계가 있는 GameObject 삭제는 현재 자식까지 함께 삭제하는 정책입니다. 자식부터 pending remove에 등록하고 부모를 나중에 등록해 Release 과정의 iterator invalidation 위험을 줄였습니다.

## ComponentFactory

`EngineFramework`에 `ComponentFactory`가 추가되어 문자열 `typeName`으로 Component를 생성할 수 있습니다.

등록 정보는 다음 메타데이터를 가집니다.

- `typeName`: SceneData에 저장되는 안정적인 타입 문자열
- `displayName`: 에디터 UI 표시 이름
- `category`: Add Component 메뉴 그룹
- `canAddFromEditor`: 에디터에서 직접 추가 가능 여부
- `createFn`: 실제 Component 생성 함수

엔진 기본 컴포넌트는 `RegisterEngineComponents()`에서 등록합니다. 게임 DLL 쪽 사용자 컴포넌트는 `RegisterGameComponents(ComponentFactory& factory)`에서 등록합니다.

현재 등록 흐름:

1. 엔진 초기화 시 `RegisterEngineComponents()` 호출
2. `KirbyGameDll` 초기화 시 `RegisterGameComponents()` 호출
3. SceneData 로드 시 `ComponentFactory::Create(typeName, dataJson)` 사용
4. Inspector Add Component UI에서도 같은 등록 목록 사용

## Editor 기능

현재 에디터 Inspector/Hierarchy에서 가능한 작업:

- Scene 저장 버튼 표시
- GameObject 선택
- GameObject 이름, active, position, size, angle 편집
- Component 목록 표시
- Component 추가
- Component 삭제
- GameObject 생성
- GameObject 삭제

생성 프리셋:

- `Create Empty`: 빈 GameObject 생성
- `Create Sprite`: `ImageRender` 포함, 기본 크기/색상 설정
- `Create UI Image`: `UIImage` 포함, 기본 크기/색상 설정
- `Create UI Button`: `UIButton` 포함, 기본 크기/상태 색상/actionKey 설정
- `Create UI Text`: `UILabel` 포함, 기본 텍스트/폰트 크기/크기 설정

Component 추가 시에도 최소 기본값을 적용해, 추가 직후 화면에서 테스트하기 쉽게 구성했습니다.

## DirectX UI 시스템

UI는 DirectX 렌더링 기반이며, 에디터 조작은 ImGui를 사용합니다.

주요 클래스:

- `UIElement`: UI 공통 베이스, visible/enabled/orderInLayer 및 GameObject Transform 연동
- `UIImage`: `ImageRender`를 내부적으로 사용해 이미지 또는 단색 사각형 렌더링
- `UIButton`: hover/pressed/clicked 상태, `actionKey`, `onClick` 지원
- `UILabel`: `ID3DXFont` 기반 텍스트 렌더링
- `UIActionRegistry`: SceneData 로드 후 `actionKey`를 실제 콜백으로 재연결

현재 UI 기준:

- UI는 screen-space로 렌더링됩니다.
- 카메라 위치/회전에 영향을 받지 않습니다.
- UI 위치는 GameObject position을 기준으로 계산합니다.
- UI 크기는 GameObject size를 기준으로 계산합니다.
- 기존 SceneData v3의 UI `position`, `size`는 로드 시 GameObject Transform으로 이전 적용됩니다.
- 클릭 판정도 렌더 위치와 같은 기준을 사용합니다.

## SceneData

현재 SceneData 포맷은 v3를 유지합니다. 이번 단계에서는 SceneData 구조 자체를 변경하지 않았습니다.

저장 대상:

- 씬 이름
- 카메라 위치/회전
- GameObject 기본 정보
- parent/child 관계
- Component 배열
- Component `type` 문자열
- Component별 JSON data
- UIButton `actionKey`

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
4. 없거나 실패하면 코드 기반 `BuildInitialSceneObjects()` 실행
5. `ObjectManager::FlushPendingObjects()` 실행
6. Scene Start 호출

SceneData v3 로드는 현재 ComponentFactory 기반으로 동작합니다.

## 씬 전환 안정화

버튼 클릭 중 즉시 씬이 전환되면서 현재 프레임의 오브젝트/컴포넌트가 해제되어 iterator invalidation, use-after-free가 발생할 수 있었습니다.

현재 수정 방향:

- `UIButton` 콜백 이후 불필요한 멤버 접근을 줄임
- `ObjectManager` 삭제는 pending remove로 처리
- `GameObject::Release()`에서 자식 벡터 순회 중 erase가 발생하지 않도록 정리
- UI pending transform 적용 중 virtual 재진입으로 stack overflow가 나던 문제 수정

## 렌더링 흐름

렌더링은 월드 렌더와 UI 렌더를 분리합니다.

1. 월드 Render 객체 수집
2. 카메라 기준 depth 계산
3. 월드 렌더 정렬
4. 월드 렌더링
5. FBX 렌더링
6. 디버그 렌더링
7. UI 렌더링

UI는 항상 월드 렌더 이후 screen-space로 렌더링됩니다. UI 정렬은 `orderInLayer` 기준이며, 같은 layer에서는 등록 순서를 따릅니다.

## 향후 작업 체크포인트

다음 큰 목표는 에디터를 더 엔진답게 확장하는 것입니다.

- Component별 `DrawInspector` 정리
- 주요 기본 컴포넌트 Inspector 구현
- `AssetDatabase` 추가
- `Resources` 폴더 스캔
- Resource Browser 창 추가
- Inspector AssetField 추가
- ImageRender/UIImage/FBXRender 리소스 선택 연결
- SceneData v4 구조 설계
- Component type 기반 Serialize/Deserialize 확장
- New/Open/Save Scene 기능 정리
- BuildSettings.json 설계
- Build Game 메뉴와 패키징 기능 추가
- ResultGame에서 BuildSettings.json 기반 시작 씬 로드
- Play/Edit Mode 분리 검토

## 개발 원칙

- 기존 구조를 크게 부수지 않고 점진적으로 확장합니다.
- SceneData에는 런타임 포인터, DirectX 리소스 포인터, Box2D body, callback/lambda를 저장하지 않습니다.
