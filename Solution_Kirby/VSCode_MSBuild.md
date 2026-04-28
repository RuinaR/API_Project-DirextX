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
