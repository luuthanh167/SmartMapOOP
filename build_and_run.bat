@echo off
chcp 65001 >nul
echo ===================================================
echo  Bien dich va chay SmartMap C++ (Web App)
echo ===================================================
if exist "C:\msys64\ucrt64\bin\g++.exe" set "PATH=C:\msys64\ucrt64\bin;%PATH%"
if exist "C:\msys64\mingw64\bin\g++.exe" set "PATH=C:\msys64\mingw64\bin;%PATH%"

where g++ >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo Dang bien dich bang g++ (MinGW)...
    g++ -std=c++17 -O2 -I. Models/*.cpp Graph/*.cpp Service/*.cpp Server/*.cpp main.cpp -lws2_32 -lshell32 -o SmartMapOOP.exe
    if %ERRORLEVEL% EQU 0 (
        echo Bien dich thanh cong! Dang khoi dong SmartMap...
        SmartMapOOP.exe
        exit /b 0
    )
)

where cl >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo Dang bien dich bang MSVC (cl)...
    cl /std:c++17 /EHsc /O2 /I. Models\*.cpp Graph\*.cpp Service\*.cpp Server\*.cpp main.cpp ws2_32.lib shell32.lib /Fe:SmartMapOOP.exe
    if %ERRORLEVEL% EQU 0 (
        echo Bien dich thanh cong! Dang khoi dong SmartMap...
        SmartMapOOP.exe
        exit /b 0
    )
)

where cmake >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo Dang bien dich bang CMake...
    cmake -B build
    cmake --build build --config Release
    if %ERRORLEVEL% EQU 0 (
        if exist build\Release\SmartMapOOP.exe (
            build\Release\SmartMapOOP.exe
        ) else (
            build\SmartMapOOP.exe
        )
        exit /b 0
    )
)

echo Khong tim thay trinh bien dich C++ (g++, cl, hoac cmake) trong PATH.
pause

