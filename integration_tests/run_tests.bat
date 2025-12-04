@echo off
setlocal enabledelayedexpansion

REM AWK Integration Test Runner for Windows

set "SCRIPT_DIR=%~dp0"
set "AWK_EXE=%~1"
if "%AWK_EXE%"=="" set "AWK_EXE=%SCRIPT_DIR%..\build\Release\awk.exe"

set "SCRIPTS_DIR=%SCRIPT_DIR%scripts"
set "INPUT_DIR=%SCRIPT_DIR%input"
set "EXPECTED_DIR=%SCRIPT_DIR%expected"

set PASS=0
set FAIL=0
set SKIP=0

echo ========================================
echo AWK Integration Test Suite
echo ========================================
echo AWK executable: %AWK_EXE%
echo.

REM Test each script
for %%f in ("%SCRIPTS_DIR%\*.awk") do (
    call :run_test "%%~nxf"
)

echo.
echo ========================================
echo Results: %PASS% passed, %FAIL% failed, %SKIP% skipped
echo ========================================

if %FAIL% gtr 0 exit /b 1
exit /b 0

:run_test
set "script=%~1"
set "test_name=%script:.awk=%"
set "expected=%EXPECTED_DIR%\%test_name%.txt"

REM Determine input file based on test name
set "input=none"
if "%script:~0,2%"=="01" set "input=numbers.txt"
if "%script:~0,2%"=="02" set "input=employees.txt"
if "%script:~0,2%"=="03" set "input=employees.txt"
if "%script:~0,2%"=="04" set "input=numbers.txt"
if "%script:~0,2%"=="05" set "input=numbers.txt"
if "%script:~0,2%"=="10" set "input=log.txt"
if "%script:~0,2%"=="11" set "input=log.txt"
if "%script:~0,2%"=="12" set "input=log.txt"
if "%script:~0,2%"=="13" set "input=employees.txt"
if "%script:~0,2%"=="14" set "input=employees.txt"
if "%script:~0,2%"=="20" set "input=employees.txt"
if "%script:~0,2%"=="23" set "input=log.txt"
if "%script:~0,2%"=="24" set "input=log.txt"
if "%script:~0,2%"=="40" set "input=employees.txt"
if "%script:~0,2%"=="50" set "input=csv_data.txt"
if "%script:~0,2%"=="51" set "input=log.txt"
if "%script:~0,2%"=="52" set "input=text.txt"

set /p "=Testing %test_name% ... " <nul

if not exist "%SCRIPTS_DIR%\%script%" (
    echo SKIP ^(script not found^)
    set /a SKIP+=1
    goto :eof
)

if not exist "%expected%" (
    echo SKIP ^(expected output not found^)
    set /a SKIP+=1
    goto :eof
)

REM Run the test
set "actual_file=%TEMP%\awk_test_actual.txt"
if "%input%"=="none" (
    REM No input file - pipe empty input to avoid waiting on stdin
    echo. | "%AWK_EXE%" -f "%SCRIPTS_DIR%\%script%" > "%actual_file%" 2>&1
) else (
    if not exist "%INPUT_DIR%\%input%" (
        echo SKIP ^(input file not found^)
        set /a SKIP+=1
        goto :eof
    )
    "%AWK_EXE%" -f "%SCRIPTS_DIR%\%script%" "%INPUT_DIR%\%input%" > "%actual_file%" 2>&1
)

REM Compare output
fc /b "%actual_file%" "%expected%" > nul 2>&1
if !errorlevel! equ 0 (
    echo PASS
    set /a PASS+=1
) else (
    echo FAIL
    set /a FAIL+=1
    echo   Expected:
    for /f "tokens=* usebackq" %%a in (`type "%expected%" 2^>nul`) do echo     %%a
    echo   Actual:
    for /f "tokens=* usebackq" %%a in (`type "%actual_file%" 2^>nul`) do echo     %%a
)

del "%actual_file%" 2>nul
goto :eof
