@echo off
REM Usage: build_x64_with_vcpkg.bat [path_to_vcpkg] [path_to_cocos2dx_root]
SETLOCAL ENABLEDELAYEDEXPANSION

:: determine vcpkg path
if "%~1"=="" (
    if defined VCPKG_ROOT (
        set "VCPKG_ROOT=%VCPKG_ROOT%"
    ) else (
        echo vcpkg path not provided as first argument and VCPKG_ROOT not set.
        echo Please provide path to vcpkg as first argument, e.g.
        echo     build_x64_with_vcpkg.bat C:\vcpkg D:\App\cocos2d-x-4.0
        exit /b 1
    )
) else (
    set "VCPKG_ROOT=%~1"
)

:: determine cocos2dx root
if "%~2"=="" (
    if defined COCOS2DX_ROOT (
        set "COCOS2DX_ROOT=%COCOS2DX_ROOT%"
    ) else (
        echo Cocos2d-x root not provided as second argument and COCOS2DX_ROOT not set.
        echo Please provide path to Cocos2d-x root as second argument, e.g.
        echo     build_x64_with_vcpkg.bat %VCPKG_ROOT% D:\App\cocos2d-x-4.0
        exit /b 1
    )
) else (
    set "COCOS2DX_ROOT=%~2"
)

:: validate vcpkg toolchain file
nif not exist "%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" (
    echo vcpkg toolchain file not found at "%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
    echo Please install vcpkg or provide correct path.
    exit /b 1
)

:: create build dir next to this script (project root assumed)
set "SCRIPT_DIR=%~dp0"
pushd "%SCRIPT_DIR%"
if not exist "build" mkdir build

:: configure with vcpkg toolchain for x64
necho Configuring CMake (x64) with vcpkg toolchain...
cmake -S "%SCRIPT_DIR%" -B "%SCRIPT_DIR%build" -G "Visual Studio 16 2019" -A x64 -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows -DCOCOS2DX_ROOT="%COCOS2DX_ROOT%"
if errorlevel 1 (
    echo CMake configuration failed.
    popd
    exit /b 1
)

:: build Debug
necho Building project (Debug)...
cmake --build "%SCRIPT_DIR%build" --config Debug
if errorlevel 1 (
    echo Build failed.
    popd
    exit /b 1
)

necho Build completed successfully.
popd
endlocal
exit /b 0
