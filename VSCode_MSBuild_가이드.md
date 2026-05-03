# VSCode / MSBuild 가이드

이 문서는 `KirbyEngine`을 VSCode에서 열고 MSBuild로 빌드할 때 필요한 기본 내용을 정리한 문서입니다.

## 1. 작업 폴더

VSCode에서는 아래 폴더를 여는 기준으로 사용합니다.

```text
Solution_Kirby
```

즉, VSCode workspace는 솔루션 루트가 아니라 실제 Visual Studio 솔루션이 들어 있는 폴더를 기준으로 잡습니다.

## 2. 현재 솔루션 구조

현재 주요 프로젝트는 아래와 같습니다.

- `EditorApp`
  - 에디터 실행기
- `GameApp`
  - 게임 실행기
- `EngineFrameworkDll`
  - 공통 엔진 DLL
- `EngineEditor`
  - editor 분리용 프로젝트
- `KirbyGameDll`
  - 게임별 사용자 코드 DLL

## 3. 빌드 출력 위치

출력 파일은 보통 아래 경로에 생성됩니다.

```text
Bin/$(Configuration)_$(Platform)
```

예:

```text
Bin/Debug_x64/EditorApp.exe
Bin/Debug_x64/GameApp.exe
Bin/Debug_x64/EngineFrameworkDll.dll
Bin/Debug_x64/KirbyGameDll.dll
```

## 4. VSCode에서 빌드할 때

이 프로젝트는 `.sln`, `.vcxproj`를 그대로 사용하는 MSBuild 방식입니다.

즉,
VSCode에서 별도 CMake를 쓰는 구조가 아니라,
Visual Studio 프로젝트를 기준으로 빌드합니다.

## 5. D3DX 관련 메모

이 프로젝트는 legacy D3DX를 사용하고 있어서,
환경에 따라 NuGet 복원 경로가 필요할 수 있습니다.

관련 스크립트:

- `Scripts/Invoke-MSBuild.ps1`
- `Scripts/Restore-D3DXPackage.ps1`

관련 패키지 경로:

```text
External/packages/Microsoft.DXSDK.D3DX.9.29.952.8
```

## 6. BuildSettings 메모

시작 Scene은 `BuildSettings.json` 기준으로 결정됩니다.

현재 정책:

- `EditorApp`
  - BuildSettings의 startup scene이 있으면 그 값을 우선 사용
  - 없으면 `NewScene`
- `GameApp`
  - BuildSettings의 startup scene이 있으면 그 값을 우선 사용
  - 없으면 `StartScene`

## 7. 알아두면 좋은 점

- 현재 구조는 `EngineRuntime / EngineEditor`가 완전히 분리된 최종 상태는 아닙니다.
- 그래서 빌드가 성공해도 내부 구조는 계속 정리 중일 수 있습니다.
- 특히 `MainFrame`, `WindowFrame`, `RenderManager`는 아직 runtime/editor 책임이 섞여 있는 부분이 있습니다.

## 8. 실행 파일 구분

### EditorApp

- 에디터 실행용
- Hierarchy, Inspector, Scene 편집 기능 포함

### GameApp

- 게임 실행용
- editor 기능 없이 게임 플레이 실행 기준

## 9. 한 줄 정리

VSCode에서는 `Solution_Kirby`를 열고,
MSBuild 기준으로 `EditorApp` 또는 `GameApp`을 빌드해서 실행하면 됩니다.