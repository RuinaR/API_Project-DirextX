@echo off
setlocal

set "MSBUILD_ARGS=%*"

where msbuild >nul 2>nul
if %ERRORLEVEL%==0 (
    msbuild %MSBUILD_ARGS%
    exit /b %ERRORLEVEL%
)

set "MSBUILD_EXE=C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe"
if exist "%MSBUILD_EXE%" (
    "%MSBUILD_EXE%" %MSBUILD_ARGS%
    exit /b %ERRORLEVEL%
)

set "MSBUILD_EXE=C:\Program Files\Microsoft Visual Studio\18\Professional\MSBuild\Current\Bin\MSBuild.exe"
if exist "%MSBUILD_EXE%" (
    "%MSBUILD_EXE%" %MSBUILD_ARGS%
    exit /b %ERRORLEVEL%
)

set "MSBUILD_EXE=C:\Program Files\Microsoft Visual Studio\18\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
if exist "%MSBUILD_EXE%" (
    "%MSBUILD_EXE%" %MSBUILD_ARGS%
    exit /b %ERRORLEVEL%
)

set "MSBUILD_EXE=C:\Program Files\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
if exist "%MSBUILD_EXE%" (
    "%MSBUILD_EXE%" %MSBUILD_ARGS%
    exit /b %ERRORLEVEL%
)

set "MSBUILD_EXE=C:\Program Files\Microsoft Visual Studio\17\Community\MSBuild\Current\Bin\MSBuild.exe"
if exist "%MSBUILD_EXE%" (
    "%MSBUILD_EXE%" %MSBUILD_ARGS%
    exit /b %ERRORLEVEL%
)

set "MSBUILD_EXE=C:\Program Files\Microsoft Visual Studio\17\Professional\MSBuild\Current\Bin\MSBuild.exe"
if exist "%MSBUILD_EXE%" (
    "%MSBUILD_EXE%" %MSBUILD_ARGS%
    exit /b %ERRORLEVEL%
)

set "MSBUILD_EXE=C:\Program Files\Microsoft Visual Studio\17\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
if exist "%MSBUILD_EXE%" (
    "%MSBUILD_EXE%" %MSBUILD_ARGS%
    exit /b %ERRORLEVEL%
)

set "MSBUILD_EXE=C:\Program Files\Microsoft Visual Studio\17\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
if exist "%MSBUILD_EXE%" (
    "%MSBUILD_EXE%" %MSBUILD_ARGS%
    exit /b %ERRORLEVEL%
)

echo [KirbyEngine] MSBuild.exe not found.
echo [KirbyEngine] Install one of the following:
echo [KirbyEngine] - Visual Studio 2022/2025 Community, Professional, or Enterprise
echo [KirbyEngine] - Visual Studio Build Tools with MSBuild
echo [KirbyEngine] After installation, restart the editor and try Build Game again.
exit /b 9009
