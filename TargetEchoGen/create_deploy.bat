@echo off
setlocal enabledelayedexpansion

REM =======================
REM  CONFIGURATION
REM =======================
set "APP_NAME=TargetEchoGen"
set "REL_EXE=build\release\release\%APP_NAME%.exe"
set "WINDEPLOYQT=C:\Qt\6.7.3\mingw_64\bin\windeployqt.exe"

set "PROJ_ROOT=%~dp0"
set "BUILD_EXE=%PROJ_ROOT%%REL_EXE%"
set "DEPLOY_DIR=%PROJ_ROOT%deploy\%APP_NAME%"
set "FFTW_PATH=%PROJ_ROOT%fftwlib"
set "QWTLIB_PATH=%PROJ_ROOT%qwtlib\qwt-6.3.0\lib"
set "QT_BIN_DIR=C:\Qt\6.7.3\mingw_64\bin"  REM Qt's bin directory

REM Retry Settings
set RETRY_COUNT=5
set RETRY_DELAY=2  REM 2 seconds between retries

REM Force Copy Flag (set to true to force copy files even if they exist)
set FORCE_COPY=true

echo.
echo === Qt Deploy Script ===
echo Project root   : %PROJ_ROOT%
echo App name       : %APP_NAME%
echo Build exe      : %BUILD_EXE%
echo Deploy dir     : %DEPLOY_DIR%
echo windeployqt    : %WINDEPLOYQT%
echo.

REM =======================
REM  CHECK EXE
REM =======================
if not exist "%BUILD_EXE%" (
    echo [ERROR] Release exe missing:
    echo         %BUILD_EXE%
    pause
    exit /b 1
)

REM =======================
REM  CHECK WINDEPLOYQT
REM =======================
if not exist "%WINDEPLOYQT%" (
    echo [ERROR] windeployqt missing:
    echo         %WINDEPLOYQT%
    pause
    exit /b 1
)

REM =======================
REM  PREPARE DEPLOY DIR
REM =======================
if exist "%DEPLOY_DIR%" (
    echo [INFO] Cleaning existing deploy dir...
    rmdir /S /Q "%DEPLOY_DIR%"
)
mkdir "%DEPLOY_DIR%" >nul

echo [INFO] Copying exe...
copy "%BUILD_EXE%" "%DEPLOY_DIR%" >nul

REM =======================
REM  RUN WINDEPLOYQT
REM =======================
echo [INFO] Running windeployqt...
"%WINDEPLOYQT%" --release --no-translations "%DEPLOY_DIR%\%APP_NAME%.exe"
if errorlevel 1 (
    echo [ERROR] windeployqt failed.
    pause
    exit /b 1
)

REM =======================
REM  COPY FFTW (DLL ONLY) WITH RETRY
REM =======================
echo [INFO] Copying FFTW DLLs...
set SUCCESS=false
set ATTEMPTS=0
:RETRY_FFTW
if exist "%FFTW_PATH%\libfftw3-3.dll" (
    call :CopyFileWithCheck "%FFTW_PATH%\libfftw3-3.dll" "%DEPLOY_DIR%"
) else (
    echo [WARN] Missing FFTW DLL: libfftw3-3.dll
)

REM =======================
REM  COPY QWT DLLs WITH RETRY
REM =======================
echo [INFO] Copying QWT DLLs...
for %%f in ("%QWTLIB_PATH%\qwt*.dll") do (
    call :CopyFileWithCheck "%%f" "%DEPLOY_DIR%"
)

REM =======================
REM  COPY ALL DLLs FROM QT BIN FOLDER WITH RETRY
REM =======================
echo [INFO] Copying all DLLs from Qt's bin directory...
for %%f in ("%QT_BIN_DIR%\*.dll") do (
    call :CopyFileWithCheck "%%f" "%DEPLOY_DIR%"
)

REM =======================
REM  COPY EXTRA RUNTIME FILES
REM =======================
if exist "%PROJ_ROOT%dark_mode_style_sheet.qss" (
    echo [INFO] Copying dark_mode_style_sheet.qss...
    call :CopyFileWithCheck "%PROJ_ROOT%dark_mode_style_sheet.qss" "%DEPLOY_DIR%"
)

echo.
echo [DONE] Deployment folder ready:
echo        %DEPLOY_DIR%
echo.
pause
endlocal
exit /b 0

REM =======================
REM  COPY FILE WITH CHECK AND FORCE OPTION
REM =======================
:CopyFileWithCheck
set "SOURCE=%1"
set "DEST=%2"

if exist "%DEST%\%~nx1" (
    if /i "%FORCE_COPY%"=="true" (
        echo [INFO] Force copying: %SOURCE%
        copy /Y "%SOURCE%" "%DEST%" >nul
    ) else (
        echo [INFO] File already exists: %DEST%\%~nx1. Skipping copy.
    )
) else (
    echo [INFO] Copying: %SOURCE%
    copy "%SOURCE%" "%DEST%" >nul
)

goto :eof
