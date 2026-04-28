param(
    [string]$Version = "9.29.952.8"
)

$ErrorActionPreference = "Stop"
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12

$root = Resolve-Path (Join-Path $PSScriptRoot "..")
$packagesDir = Join-Path $root "External\packages"
$packageId = "Microsoft.DXSDK.D3DX"
$packageDir = Join-Path $packagesDir "$packageId.$Version"
$nativeDir = Join-Path $packageDir "build\native"
$headerPath = Join-Path $nativeDir "include\d3dx9.h"

if (Test-Path $headerPath) {
    return
}

New-Item -ItemType Directory -Force -Path $packagesDir | Out-Null

$nupkgPath = Join-Path $packagesDir "$packageId.$Version.nupkg"
$zipPath = Join-Path $packagesDir "$packageId.$Version.zip"
$packageUrl = "https://www.nuget.org/api/v2/package/$packageId/$Version"

Write-Host "Restoring $packageId $Version..."
Invoke-WebRequest -Uri $packageUrl -OutFile $nupkgPath

Copy-Item -LiteralPath $nupkgPath -Destination $zipPath -Force
if (Test-Path $packageDir) {
    Remove-Item -LiteralPath $packageDir -Recurse -Force
}

Expand-Archive -LiteralPath $zipPath -DestinationPath $packageDir -Force
Remove-Item -LiteralPath $zipPath -Force

if (-not (Test-Path $headerPath)) {
    throw "Failed to restore $packageId ${Version}: d3dx9.h was not found."
}
