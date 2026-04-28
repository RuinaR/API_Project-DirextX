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

This workspace includes the legacy DirectX SDK headers/libs under:

```text
External/DirectXSDK
```

`Directory.Build.props` maps that folder to `$(DXSDK_DIR)` for MSBuild. If the folder is removed, set the system `DXSDK_DIR` environment variable to an installed June 2010 DirectX SDK path.
