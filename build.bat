@echo off
echo ========================================
echo   Gunes Sistemi Simulasyonu - Build
echo ========================================
echo.

if not exist "build" (
    echo Build klasoru olusturuluyor...
    mkdir build
)

cd build

echo CMake yapilandirmasi yapiliyor...
cmake -G "MinGW Makefiles" ..

if %errorlevel% neq 0 (
    echo.
    echo HATA: CMake yapilandirmasi basarisiz!
    echo Lutfen CMakeLists.txt icindeki kutuhane yollarini kontrol edin.
    pause
    exit /b 1
)

echo.
echo Derleme yapiliyor...
mingw32-make

if %errorlevel% neq 0 (
    echo.
    echo HATA: Derleme basarisiz!
    pause
    exit /b 1
)

echo.
echo ========================================
echo   BUILD BASARILI!
echo ========================================
echo.
echo Programi calistirmak icin:
echo   cd build
echo   SolarSystemSimulation.exe
echo.
pause
