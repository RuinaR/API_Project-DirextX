# VSCode MSBuild setup

Open this folder in VSCode:

```text
Solution_Kirby
```

The VSCode tasks keep the existing `.sln` and `.vcxproj` files and build through MSBuild.

Output folders are normalized by `Directory.Build.props`:

```text
Bin/$(Configuration)_$(Platform)
```

For the VSCode `Win32` build task, the debug executable path is:

```text
Bin/Debug_Win32/BuildResultGame.exe
```

Runtime asset folders are copied next to the executable during application builds so existing exe-relative resource loading keeps working.

This workspace restores the legacy D3DX headers/libs from NuGet when the
June 2010 DirectX SDK is not installed:

```text
External/packages/Microsoft.DXSDK.D3DX.9.29.952.8
```

`Scripts/Invoke-MSBuild.ps1` runs `Scripts/Restore-D3DXPackage.ps1` as needed.
`Directory.Build.props` then maps either the restored NuGet package or an
installed June 2010 DirectX SDK path into the D3DX include/library properties.

## Current Project Notes

- Physics2D 관련 네이밍은 `Collider2D`, `BoxCollider2D`, `CircleCollider2D`, `Rigidbody2D`, `Physics2D` 기준으로 정리되어 있다.
- 사용자 컴포넌트에서 충돌 이벤트를 override할 때는 `Collider*`가 아니라 `Collider2D*` 시그니처를 써야 한다.
- `BoxCollider2D` SceneData 저장 타입은 `"BoxCollider2D"`이고, 기존 `"BoxCollider"`는 legacy alias로 계속 로드 가능하다.
- 테스트 씬은 `SceneData/Physics2DTestScene.json`, `SceneData/Physics2DInteractionTestScene.json`을 사용하면 된다.
- BuildSettings의 `startScene`은 자동으로 바꾸지 않으므로, 필요한 씬은 에디터에서 직접 열거나 `BuildSettings.json`을 명시적으로 바꿔야 한다.
