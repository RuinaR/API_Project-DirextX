# DLL 로드와 실행 흐름

이 문서는 현재 프로젝트가 실제로 어떤 순서로 실행되는지,
그리고 EXE / DLL이 어떻게 연결되는지를 코드 기준으로 쉽게 설명한 문서입니다.

## 1. 시작점

현재 실행기는 두 개입니다.

- `EditorApp`
- `GameApp`

둘 다 공통적으로 `KirbyGameDll.dll`을 실행 중에 로드합니다.

## 2. EditorApp 시작 흐름

기준 파일:

- `EditorApp/BuildLoopCls.cpp`
- `BuildCommon/AppLaunchDesc.cpp`
- `BuildCommon/AppBootstrap.cpp`
- `include/PluginManager.cpp`
- `KirbyGameDll/KirbyGame.cpp`

흐름은 대략 아래와 같습니다.

### 1) 실행기 시작

`EditorApp`이 시작되면 `BuildLoopCls::Init()`이 호출됩니다.

### 2) DLL 로드

`PluginManager::DirectLoadPlugin(...)`으로 `KirbyGameDll.dll`을 로드합니다.

이때 내부적으로:

- `LoadLibraryW(...)`
- `GetProcAddress(..., "CreatePlugin")`
- `CreatePlugin(...)`

순서로 plugin 객체를 만듭니다.

### 3) Editor용 실행 설정 만들기

`BuildEditorLaunchDesc()`를 호출해서 실행 설정을 만듭니다.

여기에는 아래 정보가 들어 있습니다.

- `renderType = Edit`
- 시작 scene 이름
- scene이 없을 때 새로 만들지 여부
- FPS

### 4) 게임별 등록 실행

그 다음 `plugin->RegisterGameContent(hInstance, launchDesc)`를 호출합니다.

현재 `KirbyGameDll`에서는 여기서 아래 작업을 합니다.

- 엔진 기본 component 등록
- 게임별 component 등록
- 게임별 UI action 등록

즉, 이 단계는 "게임별 내용을 엔진에 등록하는 단계"에 가깝습니다.

### 5) editor callback 등록

`EditorApp`에서는 editor 전용 callback도 등록합니다.

예:

- scene snapshot callback
- editor overlay callback

이건 editor에서만 필요하고,
`GameApp`에는 들어가지 않습니다.

### 6) MainFrame 시작

`BootstrapMainFrame(hInstance, launchDesc)`를 호출합니다.

이 함수 안에서는 아래를 합니다.

- `MainFrame::Create(hInstance)`
- `WindowFrame::SetRequestedSceneDataName(...)`
- startup scene 객체 생성
- `MainFrame::Initialize(...)`

### 7) editor 전용 fallback 처리

필요하면 startup scene이 없을 때 `NewScene`을 만드는 editor 쪽 처리도 합니다.

### 8) 매 프레임 업데이트

실행 중에는 `UpdateMainFrame()`이 계속 호출됩니다.
결국 `MainFrame::Update()`로 이어집니다.

### 9) 종료

종료할 때는 대략 아래 순서입니다.

- `ShutdownMainFrame()`
- `plugin->ReleaseGameContent()`
- `PluginManager::UnloadAllPlugins()`

`ReleaseGameContent()`에서는 현재
`ComponentFactory`, `UIActionRegistry` 정리도 같이 합니다.

## 3. GameApp 시작 흐름

`GameApp`도 큰 흐름은 비슷합니다.

차이점은 아래 정도입니다.

- `BuildGameLaunchDesc()` 사용
- `renderType = Game`
- editor callback 등록 없음
- editor overlay 없음
- scene이 없으면 새로 만들지 않음

즉, editor 전용 처리만 빠진 같은 흐름으로 보면 됩니다.

## 4. AppLaunchDesc는 왜 필요한가

`AppLaunchDesc`는 실행기마다 달라지는 시작 정책을 한곳에 모아 둔 구조체입니다.

예를 들면:

- editor 모드인지 game 모드인지
- 시작 scene 이름이 무엇인지
- scene이 없으면 새로 만들지, 실패할지
- FPS를 얼마로 할지

이걸 따로 모아 두니까
EditorApp과 GameApp의 차이를 설명하기 쉬워졌습니다.

## 5. 현재 plugin 표면

지금 `IPlugin`에는 핵심 함수가 많지 않습니다.

- `RegisterGameContent(...)`
- `GetName()`
- `ReleaseGameContent()`

즉, 현재 plugin은
"매 프레임 게임 루프를 직접 돌리는 역할"보다
"게임별 component/action을 등록하고 정리하는 역할"에 더 가깝습니다.

## 6. 왜 이런 흐름으로 바뀌었는가

예전에는 plugin 쪽이 더 많은 책임을 가지고 있었습니다.
하지만 정리하면서 아래처럼 방향이 바뀌었습니다.

- 실행기(App)는 실행 흐름을 더 많이 담당
- `KirbyGameDll`은 게임별 내용 등록에 집중
- 공통 엔진 생명주기는 `EngineFrameworkDll` 쪽에 더 모음

## 7. 한 줄 정리

현재 실행 흐름은
`EditorApp / GameApp -> PluginManager -> KirbyGameDll -> AppBootstrap -> MainFrame`
이 순서로 이해하면 가장 쉽습니다.