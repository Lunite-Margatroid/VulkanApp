@echo off
rem ============================================================
rem  install_sol2.bat
rem  Download sol2 sources into vendor/sol2 and copy the public headers
rem  into vendor/dist/include (use it as: #include <sol/sol.hpp>).
rem  Requires: curl (built into Windows 10+), 7-Zip.
rem
rem  Usage:
rem    install_sol2.bat                                install default tag v3.5.0
rem    install_sol2.bat v3.3.0                         install a specific tag
rem    install_sol2.bat v3.5.0 -f                      overwrite existing vendor/sol2
rem    install_sol2.bat -proxy http://127.0.0.1:7890   download through an HTTP/HTTPS proxy
rem
rem  Proxy can also be set with the SOL2_PROXY environment variable.
rem ============================================================
setlocal EnableExtensions

set "SOL2_VERSION="
set "SOL2_PROXY="
set "FORCE=0"

:parse_args
if "%~1"=="" goto :args_parsed
if /i "%~1"=="-f" (
    set "FORCE=1"
    shift
    goto :parse_args
)
if /i "%~1"=="--force" (
    set "FORCE=1"
    shift
    goto :parse_args
)
if /i "%~1"=="-proxy" goto :parse_proxy
if /i "%~1"=="--proxy" goto :parse_proxy
set "SOL2_VERSION=%~1"
shift
goto :parse_args

:parse_proxy
shift
if "%~1"=="" (
    echo [sol2] ERROR: -proxy requires an address, e.g. -proxy http://127.0.0.1:7890
    exit /b 1
)
set "SOL2_PROXY=%~1"
shift
goto :parse_args

:args_parsed
if "%SOL2_VERSION%"=="" set "SOL2_VERSION=v3.5.0"

rem Repo root (this script lives in <repo>/script)
for %%I in ("%~dp0..") do set "REPO_ROOT=%%~fI"

set "VENDOR_DIR=%REPO_ROOT%\vendor"
set "SOL2_DIR=%VENDOR_DIR%\sol2"
set "ARCHIVE_URL=https://github.com/ThePhD/sol2/archive/refs/tags/%SOL2_VERSION%.zip"

echo [sol2] version : %SOL2_VERSION%
if defined SOL2_PROXY (
    echo [sol2] proxy   : %SOL2_PROXY%
) else (
    echo [sol2] proxy   : not set - use -proxy or the HTTPS_PROXY env var
)
echo [sol2] url     : %ARCHIVE_URL%
echo [sol2] target  : %SOL2_DIR%

rem Check required tools
where curl >nul 2>nul
if errorlevel 1 (
    echo [sol2] ERROR: curl not found
    exit /b 1
)

rem Locate 7-Zip (PATH or common install location)
set "SEVENZIP="
where 7z.exe >nul 2>nul
if not errorlevel 1 set "SEVENZIP=7z.exe"
if not defined SEVENZIP if exist "%ProgramFiles%\7-Zip\7z.exe" set "SEVENZIP=%ProgramFiles%\7-Zip\7z.exe"
if not defined SEVENZIP if exist "%ProgramW6432%\7-Zip\7z.exe" set "SEVENZIP=%ProgramW6432%\7-Zip\7z.exe"
if not defined SEVENZIP (
    echo [sol2] ERROR: 7z not found - install 7-Zip or add it to PATH
    exit /b 1
)

rem Handle an existing target directory
if exist "%SOL2_DIR%" (
    if "%FORCE%"=="1" (
        echo [sol2] removing existing directory...
        rmdir /s /q "%SOL2_DIR%"
        if exist "%SOL2_DIR%" (
            echo [sol2] ERROR: failed to remove %SOL2_DIR%
            exit /b 1
        )
    ) else (
        echo [sol2] directory already exists: %SOL2_DIR%
        echo [sol2] use -f to reinstall, or delete it manually
        exit /b 1
    )
)

rem Temp dir sits next to vendor/ so the final move is a same-volume rename
if not exist "%VENDOR_DIR%" mkdir "%VENDOR_DIR%"
set "TMP_DIR=%VENDOR_DIR%\.install_sol2_%RANDOM%%RANDOM%"
mkdir "%TMP_DIR%"
if errorlevel 1 (
    echo [sol2] ERROR: cannot create temp dir %TMP_DIR%
    exit /b 1
)

echo [sol2] downloading...
if defined SOL2_PROXY (
    curl -L --fail --retry 3 --retry-all-errors --connect-timeout 15 -x "%SOL2_PROXY%" -# -o "%TMP_DIR%\sol2.zip" "%ARCHIVE_URL%"
) else (
    curl -L --fail --retry 3 --retry-all-errors --connect-timeout 15 -# -o "%TMP_DIR%\sol2.zip" "%ARCHIVE_URL%"
)
if errorlevel 1 goto :fail_download

echo [sol2] extracting...
mkdir "%TMP_DIR%\extract"
"%SEVENZIP%" x -y -bso0 -bsp0 -o"%TMP_DIR%\extract" "%TMP_DIR%\sol2.zip"
rem 7z exit codes: 0 = OK, 1 = warning (still usable), 2 and above = failure
if errorlevel 2 goto :fail_extract

rem The GitHub archive contains a single top-level dir (sol2-<version>/)
set "SRC_DIR="
for /d %%D in ("%TMP_DIR%\extract\*") do if not defined SRC_DIR set "SRC_DIR=%%~fD"
if not defined SRC_DIR goto :fail_extract

rem Move into place
move "%SRC_DIR%" "%SOL2_DIR%" >nul
if errorlevel 1 goto :fail_move

if not exist "%SOL2_DIR%\include\sol\sol.hpp" goto :fail_verify

rem Copy public headers to vendor/dist/include (include path: vendor/dist/include)
set "DIST_INCLUDE_DIR=%VENDOR_DIR%\dist\include"
echo [sol2] copying headers to %DIST_INCLUDE_DIR%...
if not exist "%DIST_INCLUDE_DIR%" mkdir "%DIST_INCLUDE_DIR%"
xcopy "%SOL2_DIR%\include" "%DIST_INCLUDE_DIR%" /E /I /Y >nul
if errorlevel 1 goto :fail_copy

echo.
echo [sol2] installed
echo [sol2] sources : %SOL2_DIR%
echo [sol2] header  : include\sol\sol.hpp
echo [sol2] dist    : %DIST_INCLUDE_DIR%\sol
set "RESULT=0"
goto :cleanup

:fail_download
echo [sol2] ERROR: download failed - check network or tag "%SOL2_VERSION%"
echo [sol2] hint: behind a proxy? try -proxy http://127.0.0.1:7890
set "RESULT=1"
goto :cleanup

:fail_extract
echo [sol2] ERROR: extraction failed
set "RESULT=1"
goto :cleanup

:fail_move
echo [sol2] ERROR: could not move files to %SOL2_DIR%
set "RESULT=1"
goto :cleanup

:fail_verify
echo [sol2] ERROR: incomplete install - include\sol\sol.hpp missing
rmdir /s /q "%SOL2_DIR%"
set "RESULT=1"
goto :cleanup

:fail_copy
echo [sol2] ERROR: failed to copy headers to %DIST_INCLUDE_DIR%
set "RESULT=1"
goto :cleanup

:cleanup
if exist "%TMP_DIR%" rmdir /s /q "%TMP_DIR%"
endlocal & exit /b %RESULT%
