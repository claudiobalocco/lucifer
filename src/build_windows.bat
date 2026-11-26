@echo off
rem ------------------------------------------------------------------
rem Build lucifer.exe on Windows with Microsoft Visual C++
rem (Visual Studio 2019 16.10 or newer, "Desktop development with C++").
rem
rem   build_windows.bat [path\to\python.exe]
rem
rem The Python used must be 64 bit Python >= 3.8 with numpy (python.org,
rem Anaconda/Miniforge, ...); default is the python.exe found on PATH.
rem Output: ..\bin\lucifer.exe. It uses the Python installation it was built
rem with (or PYTHONHOME if set), no need to activate it or to add it to PATH.
rem Running needs the Microsoft Visual C++ Redistributable (OpenMP runtime).
rem Set LUC_ARCH to change the instruction set (default /arch:AVX2),
rem e.g. "set LUC_ARCH=/arch:AVX512" or "set LUC_ARCH= " for any x64 CPU.
rem ------------------------------------------------------------------
setlocal
cd /d "%~dp0"

set "PYTHON=%~1"
if "%PYTHON%"=="" set "PYTHON=python"
if not defined LUC_ARCH set "LUC_ARCH=/arch:AVX2"

set "OBJ=%~dp0build_win"
set "BIN=%~dp0..\bin"
if not exist "%OBJ%" mkdir "%OBJ%"
if not exist "%BIN%" mkdir "%BIN%"

rem --- compiler: use cl.exe if already set up, otherwise find Visual Studio
where cl.exe >nul 2>nul
if not errorlevel 1 goto have_cl
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" goto no_vs
set "VSDIR="
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSDIR=%%i"
if not defined VSDIR goto no_vs
set "PATH=%PATH%;%ProgramFiles(x86)%\Microsoft Visual Studio\Installer"
call "%VSDIR%\VC\Auxiliary\Build\vcvars64.bat" >nul
where cl.exe >nul 2>nul
if errorlevel 1 goto no_vs
:have_cl

rem --- Python paths
"%PYTHON%" "%~dp0build_windows_pyconfig.py" "%OBJ%"
if errorlevel 1 goto no_python
call "%OBJ%\pyconfig.bat"

rem --- compile and link
cl /nologo /O2 /W3 %LUC_ARCH% /fp:precise /openmp:llvm /D_CRT_SECURE_NO_WARNINGS ^
   /I "%PY_INC%" /FI "%OBJ%\luc_pyhome.h" /Fo"%OBJ%\\" ^
   lucifer.c allocate_all.c par_config.c updateE.c updateH.c build_geometry.c ^
   write_header.c add_source.c abc.c pbc.c write_field_views.c ^
   /Fe"%BIN%\lucifer.exe" /link /LIBPATH:"%PY_LIBDIR%" %PY_LIB% ^
   /DELAYLOAD:%PY_DLLNAME% delayimp.lib
if errorlevel 1 goto failed
rem the Python DLL is loaded at run time from the Python installation (see lucifer.c),
rem do not copy it next to lucifer.exe: conda Pythons would then not find numpy's DLLs

echo.
echo Built %BIN%\lucifer.exe
echo Run:  "%BIN%\lucifer.exe" -t ^<threads^> -c ^<conf_file.py^>
exit /b 0

:no_vs
echo [ERROR] Visual C++ not found. Install Visual Studio with "Desktop development with C++",
echo         or run this script from a "x64 Native Tools Command Prompt".
exit /b 1

:no_python
echo [ERROR] "%PYTHON%" is not a working Python 3. Pass the path to python.exe as argument.
exit /b 1

:failed
echo [ERROR] build failed
exit /b 1
