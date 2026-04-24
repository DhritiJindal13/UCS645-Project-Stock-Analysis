@echo off
setlocal

:: Set paths for Vcpkg
set INCLUDE_PATH=.\vcpkg_installed\x64-windows\include
set LIB_PATH=.\vcpkg_installed\x64-windows\lib

:: Source files
set SOURCE=main.cpp 

:: Output binary
set OUTPUT=app.exe

:: Compile with libcurl support
echo Compiling %SOURCE%...
g++ %SOURCE% -o %OUTPUT% -I%INCLUDE_PATH% -L%LIB_PATH% -lcurl

if %errorlevel% neq 0 (
    echo Compilation failed!
) else (
    echo Build succeeded! Output: %OUTPUT%
)

endlocal
pause
