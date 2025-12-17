@echo off
setlocal

:: Check if build directory exists, create if not
if not exist "build" mkdir build
cd build

:: Generate project with CMake
cmake ..
if %ERRORLEVEL% NEQ 0 (
echo CMake generate failed, please check the error message
pause
exit /b %ERRORLEVEL%
)

:: Build the project (Debug mode)
cmake --build . --config Debug
if %ERRORLEVEL% NEQ 0 (
echo Compile failed, please check the error messages
pause
exit /b %ERRORLEVEL%
)

echo Compile success, the executable file is in bin directory
pause
endlocal