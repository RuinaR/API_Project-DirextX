# VSCode MSBuild Guide

## 기본 사용 경로

VSCode에서는 아래 폴더를 엽니다.

```text
Solution_Kirby
```

이 워크스페이스는 기존 `.sln`, `.vcxproj`를 유지한 채 MSBuild로 빌드하는 방식입니다.

## 출력 경로

`Directory.Build.props` 기준 출력 폴더는 아래처럼 정리됩니다.

```text
Bin/$(Configuration)_$(Platform)
```

예:

```text
Bin/Debug_Win32/BuildResultGame.exe
Bin/Debug_x64/BuildEdit.exe
Bin/Release_x64/BuildResultGame.exe
```

## D3DX 복원

이 워크스페이스는 June 2010 DirectX SDK가 없는 환경에서도 legacy D3DX를 NuGet 패키지로 복원해 사용할 수 있게 정리되어 있습니다.

패키지 경로:

```text
External/packages/Microsoft.DXSDK.D3DX.9.29.952.8
```

관련 스크립트:

- `Scripts/Invoke-MSBuild.ps1`
- `Scripts/Restore-D3DXPackage.ps1`

## 현재 빌드 구조 메모

- `BuildEdit`
  - 에디터 실행기
  - `KirbyGameDll.dll`을 로드
- `BuildResultGame`
  - 게임 실행기
  - `KirbyGameDll.dll`을 로드
- `EngineFramework`
  - 공통 엔진 코드
- `KirbyGameDll`
  - 게임별 확장 코드

## 현재 코드 상태 관련 메모

- Physics2D 네이밍은 `Collider2D`, `BoxCollider2D`, `CircleCollider2D`, `Rigidbody2D`, `Physics2D` 기준으로 정리되어 있습니다.
- 사용자 컴포넌트에서 충돌 이벤트를 override할 때는 `Collider*`가 아니라 `Collider2D*` 시그니처를 사용해야 합니다.
- `BoxCollider2D` SceneData type은 `"BoxCollider2D"`입니다.
- 구버전 SceneData 호환을 위해 `"BoxCollider"` legacy alias는 계속 로드 가능합니다.
- 현재 테스트/검증 씬은 주로 아래 파일을 사용합니다.
  - `SceneData/Physics2DTestScene.json`
  - `SceneData/Physics2DInteractionTestScene.json`
  - `SceneData/ActivePhysicsToggleTestScene.json`

## BuildSettings 메모

- 시작 씬은 `BuildSettings.json` 기준으로 결정됩니다.
- 에디터와 게임 실행 모두 완전한 하드코딩 시작 씬 구조는 아닙니다.
- 테스트 씬을 바꾸려면 `BuildSettings.json` 또는 에디터 쪽 설정 흐름을 확인해야 합니다.

## 현재 구조적 주의점

- 아직 `EngineRuntime / EngineEditor` 분리가 끝난 상태는 아닙니다.
- `MainFrame`, `WindowFrame`, `RenderManager`, `ObjectManager`, `SceneDataManager`에는 Runtime과 Editor 책임이 함께 섞여 있습니다.
- 따라서 빌드 성공 여부와 별개로 구조 리팩터링 중간 단계라는 점을 전제로 보는 것이 맞습니다.
