@echo off
REM ============================================
REM  DUCK FACE Synth - easy build script
REM  Requires: CMake + Visual Studio (or MinGW)
REM ============================================
setlocal
cd /d "%~dp0"

if not exist build mkdir build
cd build

echo.
echo [1/2] Configuring (CMake will download raylib automatically)...
if exist _deps\raylib-src rmdir /s /q _deps\raylib-src
if exist _deps\raylib-subbuild rmdir /s /q _deps\raylib-subbuild
if exist _deps\raylib-build rmdir /s /q _deps\raylib-build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.5
if errorlevel 1 goto :err

echo.
echo [2/2] Building Release...
cmake --build . --config Release
if errorlevel 1 goto :err

echo.
echo ============================================
echo  BUILD OK!
echo  Executable: build\Release\DuckFaceSynth.exe
echo  (or build\DuckFaceSynth.exe with MinGW)
echo ============================================
pause
exit /b 0

:err
echo.
echo *** BUILD FAILED - check messages above ***
pause
exit /b 1
