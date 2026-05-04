# DLL 로드와 실행 흐름

이 문서는 `KirbyEngine`에서 `EditorApp` 또는 `GameApp`을 실행했을 때,
어떤 순서로 DLL을 읽고 게임 내용을 등록한 뒤 실제 실행까지 이어지는지를 정리한 문서입니다.

너무 세부적인 내부 구현보다,
"실행기가 켜진 뒤 어떤 단계로 게임이 준비되는가"를 이해하는 데 초점을 맞춰 작성했습니다.

## 1. 실행기는 두 종류로 나뉩니다

현재 프로젝트의 실행기는 아래 두 가지입니다.

- `EditorApp`
- `GameApp`

둘 다 공통으로 `KirbyGameDll.dll`을 읽어 와서 게임별 등록 작업을 맡깁니다.
다만 실행 목적이 다르기 때문에 시작 설정이 조금 다릅니다.

- `EditorApp`은 editor 모드로 시작합니다.
- `GameApp`은 실제 게임 플레이 모드로 시작합니다.

## 2. 시작점은 각 앱의 `main.cpp`입니다

`EditorApp/main.cpp`, `GameApp/main.cpp`를 보면,
둘 다 `BuildLoopCls`를 만들고 아래 순서로 실행합니다.

1. `Init(hInstance)`
2. 반복문에서 `Update()`
3. 종료 시 `Release()`

즉 실제 준비 작업은 대부분 `BuildLoopCls::Init()` 안에서 시작합니다.

## 3. `BuildLoopCls::Init()`에서 먼저 DLL을 읽습니다

`EditorApp/BuildLoopCls.cpp`, `GameApp/BuildLoopCls.cpp`를 보면,
두 실행기 모두 먼저 현재 exe가 있는 폴더를 찾습니다.

그 다음 그 폴더 안의 `KirbyGameDll.dll` 경로를 만듭니다.

대략 흐름은 아래와 같습니다.

1. 실행 중인 exe 경로를 구합니다.
2. 같은 폴더에 있는 `KirbyGameDll.dll` 경로를 만듭니다.
3. `PluginManager::DirectLoadPlugin(...)`으로 DLL을 읽습니다.

이 구조 덕분에 `EditorApp`, `GameApp`은 공통 엔진 위에서 돌아가지만,
게임별 코드는 DLL 쪽에 따로 둘 수 있습니다.

## 4. `EngineFrameworkDll`과 `KirbyGameDll`의 로드 방식은 다릅니다

현재 프로젝트의 DLL은 모두 같은 방식으로 읽히지 않습니다.

### `EngineFrameworkDll`

`EngineFrameworkDll`은 `PluginManager`가 직접 읽는 DLL이 아닙니다.
`EditorApp`, `GameApp`, `KirbyGameDll`이 빌드할 때 미리 링크해 두는 공통 엔진 DLL입니다.

즉 프로그램이 시작될 때 운영체제가 자동으로 함께 로드합니다.
그래서 코드에서는 `MainFrame`, `WindowFrame`, `RenderManager` 같은 기능을
바로 호출해서 사용할 수 있습니다.

### `KirbyGameDll`

반대로 `KirbyGameDll`은 실행 중에 `PluginManager`가 직접 읽는 DLL입니다.
즉 게임별 등록 코드와 사용자 컴포넌트 쪽은
이 플러그인 DLL 안에 들어 있다고 보면 됩니다.

정리하면 아래처럼 이해하면 편합니다.

- `EngineFrameworkDll`: 공통 엔진 본체 DLL입니다.
- `KirbyGameDll`: 나중에 읽어 오는 게임별 확장 DLL입니다.

## 5. DLL은 `PluginManager`가 읽습니다

`include/PluginManager.cpp`를 보면,
`PluginManager`는 아래 순서로 DLL을 읽습니다.

1. `LoadLibraryW(...)`로 DLL 파일을 메모리에 올립니다.
2. `GetProcAddress(...)`로 `CreatePlugin` 함수를 찾습니다.
3. `CreatePlugin(...)`을 호출해서 `IPlugin` 객체를 만듭니다.
4. 만든 플러그인 객체와 DLL 핸들을 내부 목록에 저장합니다.

즉 이 프로젝트에서 `PluginManager`가 직접 읽는 대상은
`KirbyGameDll` 같은 플러그인 DLL이라고 보는 편이 맞습니다.

## 6. `CreatePlugin()`은 `KirbyGame` 객체를 돌려줍니다

`KirbyGameDll/KirbyGame.cpp`를 보면,
`CreatePlugin(PluginManager& mgr)`는 최종적으로 `new KirbyGame(mgr)`를 돌려줍니다.

즉 실행기 입장에서는
`KirbyGameDll.dll`을 읽고,
그 안에서 `KirbyGame`이라는 게임 플러그인 객체를 하나 받아 오는 셈입니다.

이 객체는 `IPlugin` 인터페이스를 따르기 때문에,
실행기 쪽은 구체적인 게임 클래스를 몰라도 공통 방식으로 다룰 수 있습니다.

## 7. 그 다음 `RegisterGameContent()`를 호출합니다

DLL을 읽은 뒤에는 `plugincls->RegisterGameContent(...)`를 호출합니다.

이 단계는
"이 게임에서 쓸 컴포넌트와 액션을 엔진에 등록하는 단계"라고 보면 됩니다.

`KirbyGame::RegisterGameContent(...)` 안에서는 주로 아래 작업을 합니다.

- 엔진 기본 컴포넌트 등록
- 사용자 컴포넌트 등록
- UI 액션 등록

즉 SceneData에 적힌 문자열만으로 컴포넌트를 만들 수 있게 준비하는 단계입니다.

이 등록이 먼저 끝나야,
그 다음 씬을 열 때 SceneData 안의 컴포넌트 타입 이름을 보고 실제 객체를 만들 수 있습니다.

## 8. 시작 씬 이름은 `AppLaunchDesc`에서 정합니다

실행기가 바로 씬을 여는 것이 아니라,
먼저 `AppLaunchDesc`를 만들어 시작 옵션을 정합니다.

`BuildCommon/AppLaunchDesc.cpp`를 보면 두 함수가 있습니다.

- `BuildEditorLaunchDesc()`
- `BuildGameLaunchDesc()`

### EditorApp의 시작 씬

Editor 쪽은 기본 시작 씬 이름을 `NewScene`으로 둡니다.

다만 `BuildSettings.json`에 시작 씬이 적혀 있고,
그 SceneData 파일도 실제로 존재하면 그 이름을 사용합니다.

반대로 아래 경우에는 다시 `NewScene`으로 돌아갑니다.

- `BuildSettings.json`을 읽지 못한 경우
- 시작 씬 이름이 비어 있는 경우
- 적혀 있는 SceneData 파일이 실제로 없는 경우

이 정책 덕분에 editor는 씬 파일이 없더라도 바로 새 씬을 열 수 있습니다.

### GameApp의 시작 씬

Game 쪽은 기본 시작 씬 이름을 `StartScene`으로 둡니다.

`BuildSettings.json`에서 시작 씬 이름을 읽어오면 그 값을 쓰고,
읽지 못하면 `StartScene`으로 시작합니다.

## 9. `BootstrapMainFrame()`에서 실제 프레임이 준비됩니다

`BuildCommon/AppBootstrap.cpp`를 보면,
등록 작업이 끝난 뒤 `BootstrapMainFrame(...)`을 호출합니다.

여기서 하는 일은 크게 세 가지입니다.

1. `MainFrame::Create(hInstance)`
2. `WindowFrame::SetRequestedSceneDataName(...)`
3. `MainFrame::Initialize(...)`

이 단계에서 엔진 쪽 메인 프레임과 윈도우가 실제로 준비됩니다.

특히 `SetRequestedSceneDataName(...)`으로
"이번에 열어야 하는 SceneData 이름"을 먼저 넘겨 주고,
그 다음 `MainFrame::Initialize(...)`가 실행되면서 첫 씬 준비가 이어집니다.

## 10. EditorApp은 editor 전용 작업이 한 번 더 들어갑니다

`EditorApp/BuildLoopCls.cpp`에는 게임 실행기와 다른 editor 전용 설정이 몇 가지 있습니다.

대표적으로 아래 두 가지가 있습니다.

- Scene snapshot callback 등록
- editor overlay callback 등록

이 콜백들은
씬 저장 전 백업을 잡거나,
ImGui 기반 editor 창을 그리는 데 사용합니다.

즉 `EditorApp`은 공통 엔진 위에
editor용 UI와 도구를 추가로 얹는 구조라고 볼 수 있습니다.

또한 editor는 시작 씬 파일이 없을 때
`EditorSceneWorkflow::CreateNewScene(...)`를 호출해서
새 씬을 바로 만들어 열 수 있습니다.

## 11. 실행이 시작된 뒤에는 `Update()` 반복문으로 들어갑니다

초기화가 끝나면 두 실행기 모두 반복문 안에서 `BuildLoopCls::Update()`를 계속 호출합니다.

이 함수는 내부적으로 `UpdateMainFrame()`를 부르고,
결국 `MainFrame::Update()`가 매 프레임 돌아갑니다.

실제로는 이 단계에서 아래 작업이 계속 반복됩니다.

- 입력 처리
- 씬 업데이트
- 물리 처리
- 렌더링
- UI 처리
- editor overlay 처리

즉 초기화가 끝난 뒤의 실제 게임 루프는
결국 `MainFrame::Update()`가 중심입니다.

## 12. 종료할 때는 등록을 해제하고 DLL도 내립니다

앱이 끝날 때는 `BuildLoopCls::Release()`가 호출됩니다.

이 단계에서는 보통 아래 순서로 정리합니다.

1. `ShutdownMainFrame()`
2. 플러그인의 `ReleaseGameContent()` 호출
3. `PluginManager::UnloadAllPlugins()` 호출

`UnloadAllPlugins()` 안에서는
플러그인 객체를 지우고,
마지막에 `FreeLibrary(...)`로 DLL도 내립니다.

즉 실행할 때 등록했던 게임별 내용은
종료할 때 다시 정리해 주는 구조입니다.

## 13. 이 구조를 이렇게 보는 것이 이해하기 쉽습니다

실제로는 함수와 클래스가 더 많이 얽혀 있지만,
큰 흐름만 보면 아래처럼 이해하면 됩니다.

1. 실행기(`EditorApp` 또는 `GameApp`)가 시작합니다.
2. `BuildLoopCls::Init()`이 호출됩니다.
3. `KirbyGameDll.dll`을 읽습니다.
4. `CreatePlugin()`으로 `KirbyGame` 객체를 만듭니다.
5. `RegisterGameContent()`로 컴포넌트와 액션을 등록합니다.
6. `AppLaunchDesc`로 시작 설정과 시작 씬 이름을 정합니다.
7. `BootstrapMainFrame()`으로 엔진 메인 프레임과 첫 씬을 준비합니다.
8. 반복문에서 `MainFrame::Update()`가 계속 돌며 게임이 실행됩니다.
9. 종료 시 등록을 해제하고 DLL을 내립니다.

## 14. 이 구조의 장점

현재 구조의 장점은 아래와 같습니다.

- `EditorApp`과 `GameApp`이 공통 엔진을 함께 사용할 수 있습니다.
- 게임별 코드는 `KirbyGameDll`에 모아 둘 수 있습니다.
- 실행기와 게임 등록 코드를 어느 정도 분리해서 볼 수 있습니다.
- editor 전용 기능과 runtime 기능을 조금씩 나눠 정리해 갈 수 있습니다.

완전히 최종 구조라고 보기는 어렵지만,
현재 포트폴리오 단계에서는
"엔진 공통부 / 실행기 / 게임별 DLL"이 어떻게 연결되는지를 보여 주기에 충분히 의미 있는 구조라고 생각합니다.

## 15. 같이 보면 좋은 파일

이 문서를 읽은 뒤 아래 파일을 같이 보면 흐름이 더 잘 보입니다.

- `EditorApp/main.cpp`
- `EditorApp/BuildLoopCls.cpp`
- `GameApp/main.cpp`
- `GameApp/BuildLoopCls.cpp`
- `BuildCommon/AppLaunchDesc.cpp`
- `BuildCommon/AppBootstrap.cpp`
- `include/plugin.h`
- `include/PluginManager.cpp`
- `KirbyGameDll/KirbyGame.cpp`

