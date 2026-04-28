# API_Project-DirextX

DirectX9, Win32API 기반의 2D/3D 혼합 게임 엔진 및 에디터 프로젝트입니다.  
Visual Studio 솔루션 구조를 유지하면서 VSCode에서도 MSBuild 기반으로 빌드하고 실행할 수 있도록 정리했습니다.

## 프로젝트 개요

이 프로젝트는 게임 플레이 코드와 엔진 프레임워크를 분리하고, 에디터 모드와 결과 게임 모드를 함께 운용하는 구조를 목표로 합니다.

- `EngineFramework`: 오브젝트, 컴포넌트, 렌더링, 입력, 리소스, 씬 데이터 등 공통 엔진 기능
- `KirbyGameDll`: 실제 게임 씬, 플레이어, 몬스터, 스테이지 편집/생성 로직
- `BuildEdit`: 에디터 모드 실행 프로젝트
- `BuildResultGame`: 게임 실행 프로젝트
- `SceneData`: 에디터에서 저장한 씬 JSON 데이터

사용 기술은 C++, Win32API, DirectX9, Box2D, ImGui, FBX SDK입니다.

## 최근 리팩토링 요약

2026-04-28 기준으로 다음 구조 개선과 기능 추가를 진행했습니다.

- VSCode용 MSBuild 빌드/실행 설정 추가
- 출력 폴더를 `Bin/$(Configuration)_$(Platform)` 구조로 정리
- 소스 인코딩을 UTF-8 BOM 기준으로 통일하고 `/utf-8` 컴파일 옵션 적용
- `ObjectManager`의 pending add/remove 구조 도입
- 일부 엔진 파일을 역할별 폴더로 이동
- DirectX 기반 UI 시스템 추가
- 기존 인게임 ImGui 버튼을 `UIButton`, `UILabel`, `UIImage` 기반 UI로 교체
- UI 렌더 큐와 `orderInLayer` 정렬 추가
- 카메라 회전과 카메라 기준 렌더 정렬 추가
- 에디터 인스펙터 확장
- SceneData JSON 저장/로드 구조 추가
- 버튼 클릭 중 즉시 씬 전환으로 생기던 iterator invalidation 문제를 지연 실행 방식으로 수정
- SceneData에 카메라 위치/회전 저장 및 로드 지원 추가

## 현재 폴더 구조

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

## 오브젝트 생명주기

`ObjectManager`는 오브젝트 추가/삭제 요청을 즉시 메인 리스트에 반영하지 않고 pending 큐에 모읍니다.

- `AddObject()`는 `m_pendingAddObjects`에 등록합니다.
- `DestroyObject()`는 destroy flag를 켜고 `m_pendingRemoveObjects`에 등록합니다.
- `FlushPendingObjects()`에서 안전한 시점에 추가/삭제를 일괄 반영합니다.
- `Update`, `Render`, `ImguiUpdate` 중 destroy 상태인 오브젝트는 순회 대상에서 제외합니다.

이 구조로 Update 순회 중 컨테이너가 직접 변경되면서 발생하던 iterator invalidation과 use-after-free 위험을 줄였습니다.

## DirectX UI 시스템

인게임 UI는 ImGui 의존을 줄이고 DirectX 렌더링 기반으로 옮겼습니다. 에디터용 ImGui는 유지합니다.

주요 클래스:

- `UIElement`: 위치, 크기, 표시/활성 상태, `orderInLayer`를 가진 UI 베이스
- `UIImage`: `ImageRender`를 내부적으로 사용해 이미지 또는 단색 사각형을 렌더링
- `UIButton`: hover, pressed, clicked 상태와 `onClick` 콜백 지원
- `UILabel`: `ID3DXFont` 기반 텍스트 렌더링, 한글 출력은 wide string 경로 사용
- `UIActionRegistry`: SceneData 로드 시 `actionKey`를 실제 콜백으로 다시 연결

`UIButton`은 콜백 실행 뒤 즉시 `return`하도록 수정했습니다. 버튼 클릭으로 씬 전환이 발생하면 버튼 자신이 삭제될 수 있으므로, 콜백 이후 멤버 접근을 막아 use-after-free를 방지합니다.

## 렌더링 파이프라인

렌더링은 월드 렌더와 UI 렌더를 분리합니다.

1. 월드 `Render`객체 수집
2. 카메라 회전값을 기준으로 한 z축 depth 계산 (카메라 forward 기준)
3. 불투명/투명 월드 렌더 큐 정렬
4. 월드 렌더링
5. FBX 렌더링
6. 디버그 렌더링
7. UI 렌더 큐 렌더링

UI는 항상 월드 렌더 이후에 렌더링됩니다. UI 큐는 `orderInLayer` 오름차순으로 정렬하고, 같은 `orderInLayer`에서는 등록 순서를 유지합니다.

## 카메라와 렌더 정렬

`Camera`는 위치뿐 아니라 회전값을 가집니다.

지원 API:

- `SetRotation(float x, float y, float z)`
- `SetRotation(const D3DXVECTOR3* rotation)`
- `GetRotation()`
- `AddRotation(float x, float y, float z)`
- `AddRotation(const D3DXVECTOR3* rotation)`
- `GetForward()`
- `GetRight()`
- `GetUp()`

카메라의 forward/right/up 방향은 `D3DXMatrixRotationYawPitchRoll`로 회전 행렬을 만든 뒤 기준 벡터를 변환해 계산합니다.

월드 렌더 정렬은 단순 world z값 대신 카메라 기준 깊이를 사용합니다.

```cpp
objectToCamera = objectWorldPosition - cameraPosition;
depth = dot(objectToCamera, cameraForward);
```

이 depth를 기준으로, 투명 스프라이트는 back-to-front 정렬이 자연스럽기 때문에 멀리 있는 객체가 먼저 그려지도록 정렬합니다. 

UI 렌더는 이 정렬 대상에서 제외됩니다. UI 렌더링 직전 view/projection/world 행렬을 screen-space용 직교 투영으로 바꾸고, 렌더 후 기존 행렬을 복원합니다. 따라서 UI는 카메라 위치와 회전에 영향을 받지 않습니다.

## 입력 좌표와 에디터 Game View

게임 모드에서는 마우스 좌표를 윈도우 클라이언트 기준으로 사용합니다.  
에디터 모드에서는 게임 화면이 ImGui 창 내부 렌더 타겟으로 표시되므로, `Mouse::GetGameViewPos()`가 RenderManager에 저장된 Game View offset/size를 이용해 게임 화면 로컬 좌표로 보정합니다.

## SceneData 저장/로드

씬 저장은 에디터 모드에서만 사용자가 `Save Scene` 버튼을 눌렀을 때 수행합니다. 게임 모드에서는 파일 저장을 하지 않습니다.

게임 모드 흐름:

1. `scene->Init()`
2. `SceneData/{SceneName}.json`이 있으면 로드
3. 파일이 없거나 로드 실패 시 `BuildInitialSceneObjects()`
4. `ObjectManager::FlushPendingObjects()`
5. `scene->Start()`

에디터 모드 흐름:

1. `scene->Init()`
2. 파일이 있으면 로드, 없으면 코드 기반 초기 배치 생성
3. `ObjectManager::FlushPendingObjects()`
4. `scene->Start()`
5. 현재 상태를 메모리 스냅샷으로 보관
6. `Save Scene` 클릭 시 현재 ObjectManager 상태를 다시 직렬화해서 저장

SceneData는 version 3 포맷을 사용합니다.

현재 저장 대상:

- 씬 이름
- 카메라 위치/회전
- GameObject 기본 정보
- parent/child 관계
- ImageRender
- UIImage
- UILabel
- UIButton
- AnimationRender
- FBXRender
- Collider/BoxCollider

저장하지 않는 대상:

- raw pointer
- DirectX texture/buffer pointer
- Box2D body pointer
- callback/lambda 본체
- RenderManager 등록 상태
- runtime cache
- ImGui editor state
- Rigidbody

`UIButton`의 콜백은 JSON에 저장하지 않고 `actionKey`만 저장합니다. 로드 후 `UIActionRegistry`가 `ChangeGameScene`, `ChangeEditScene`, `ChangeStartScene` 같은 문자열 키를 실제 콜백으로 다시 연결합니다.

## 씬 전환 안정화

버튼 콜백에서 `WindowFrame::SetScene()`을 즉시 호출하면 현재 Update 순회 중인 오브젝트와 컴포넌트가 삭제될 수 있습니다. 이로 인해 invalidated vector iterator 문제가 발생했습니다.

현재는 `SceneChanger`가 씬 전환 요청을 `MainFrame::AddBtnEvent()`에 등록하고, 프레임의 안전한 지점에서 실제 `SetScene()`을 실행합니다.

## 빌드와 실행

Visual Studio:

1. `Solution_Kirby/Solution_Kirby.sln`을 엽니다.
2. 구성은 `Debug|Win32` 또는 `Release|Win32`를 사용합니다.
3. `BuildEdit` 또는 `BuildResultGame`을 시작 프로젝트로 설정합니다.
4. 빌드 후 실행합니다.

VSCode:

1. `Solution_Kirby`를 workspace root로 엽니다.
2. `.vscode/tasks.json`의 MSBuild 작업으로 빌드합니다.
3. `.vscode/launch.json`의 실행 구성을 사용합니다.
4. 실행 cwd는 `${workspaceFolder}` 기준입니다.

주의:

- June 2010 DirectX SDK 또는 프로젝트에 포함된 `External/DirectXSDK` 경로가 필요합니다.
- FBX SDK 라이브러리 경로가 맞지 않으면 `libfbxsdk.lib` 링크 오류가 발생할 수 있습니다.
- 실행 시 리소스 상대경로는 `Solution_Kirby` 기준으로 맞춰져 있습니다.

## 남은 개선점

- FBX SDK 경로를 완전히 `External` 기준으로 정리
- SceneData의 자동 마이그레이션 도구 추가
- Rigidbody 저장/로드 정책 재검토
- 렌더 큐에서 불투명/투명 객체를 더 세밀하게 분리
- 에디터 인스펙터에서 컴포넌트 추가/삭제 지원
- SceneData 저장 전 validation 기능 추가
