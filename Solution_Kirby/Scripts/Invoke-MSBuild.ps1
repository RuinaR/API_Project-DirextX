param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Debug",

    [ValidateSet("Win32", "x64")]
    [string]$Platform = "Win32",

    [string]$Target = "Build",

    [string]$Verbosity = "minimal",

    [string]$Project = "..\Solution_Kirby.sln"
)

$ErrorActionPreference = "Stop"

$projectPath = Join-Path $PSScriptRoot $Project
$vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"

$msbuild = $null
if (Test-Path $vswhere) {
    $msbuild = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -find "MSBuild\**\Bin\MSBuild.exe" | Select-Object -First 1
}

if (-not $msbuild) {
    $msbuild = "MSBuild.exe"
}

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$d3dxPackageNativeDir = Join-Path $repoRoot "External\packages\Microsoft.DXSDK.D3DX.9.29.952.8\build\native\"

if (-not $env:DXSDK_DIR) {
    $legacyDxSdk = Join-Path ${env:ProgramFiles(x86)} "Microsoft DirectX SDK (June 2010)\"
    if (Test-Path (Join-Path $legacyDxSdk "Include\d3dx9.h")) {
        $env:DXSDK_DIR = $legacyDxSdk
    }
    else {
        & (Join-Path $PSScriptRoot "Restore-D3DXPackage.ps1")
        $env:DXSDK_DIR = $d3dxPackageNativeDir
    }
}

& $msbuild $projectPath `
    /t:$Target `
    /p:Configuration=$Configuration `
    /p:Platform=$Platform `
    /v:$Verbosity

exit $LASTEXITCODE
