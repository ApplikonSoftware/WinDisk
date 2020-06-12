@echo off
VERIFY OTHER 2>nul
SETLOCAL ENABLEEXTENSIONS
IF ERRORLEVEL 1 GOTO error
set OLDCD=%CD%
set OPTION=%~1

set QTVERSION=5.13.0
set QTSPEC=win32-msvc

IF "%OUTPUT_DIR%" == "" set OUTPUT_DIR=build_out
IF "%APP_DIR%" == "" set APP_DIR=Application
IF "%OPTION%" == "community" (
	set VSTYPE=Community
) ELSE IF "%OPTION%" == "professional" (
	set VSTYPE=Professional
) ELSE (
	ECHO Please speficy professional/community to build app with visual studio Professional version or Community version
	GOTO end
)

IF NOT "%QTHOME%" == "" GOTO qthome
set QTHOME=D:\Qt
:qthome
path %QTHOME%\%QTVERSION%\..\Tools\QtCreator\bin;%PATH%

IF NOT "%VSHOME%" == "" GOTO vshome
set VSHOME=C:\Program Files (x86)\Microsoft Visual Studio\2017\%VSTYPE%
:vshome
path %QTHOME%\%QTVERSION%\msvc2017_64\bin;%PATH%

echo Prepare output directory...
IF NOT EXIST %OUTPUT_DIR% GOTO createout
rd /q /s %OUTPUT_DIR%
:createout
mkdir %OUTPUT_DIR%
cd %OUTPUT_DIR%

echo Setup build environment...
call "%VSHOME%\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64

echo Build windisk
echo Generate makefiles...
qmake.exe ..\src\windisk.pro -r -spec %QTSPEC%
IF ERRORLEVEL 1 GOTO error

echo Build executable...
jom.exe /J 4
IF ERRORLEVEL 1 GOTO error
echo Build succeeded
cd %OLDCD%

cd installer
IF NOT EXIST %APP_DIR% GOTO createapp
rd /q /s %APP_DIR%
:createapp
mkdir %APP_DIR%
copy /y ..\%OUTPUT_DIR%\release\windisk.exe %APP_DIR%
IF ERRORLEVEL 1 GOTO error

echo Deploy application ...
cd %APP_DIR%
windeployqt --qmldir ..\..\src windisk.exe --release --no-translations --no-system-d3d-compiler --no-compiler-runtime --no-webkit2 --no-angle --no-opengl-sw
IF ERRORLEVEL 1 GOTO error
echo Deploy succeeded

cd ..
echo Build installer ...

IF NOT "%AIHOME%" == "" GOTO aihome
REM set AIHOME=C:\Program Files (x86)\Caphyon\Advanced Installer 14.3
call getaihome.bat
IF ERRORLEVEL 1 GOTO error

:aihome
path %AIHOME%\bin\x86;%PATH%
advancedinstaller.com /execute "windisk.aip" "windisk.aic"
IF ERRORLEVEL 1 GOTO error

echo Installer build successfully
echo Find the installer in installer\SetupFiles
cd %OLDCD%
GOTO end

:error
echo Installer Build failed!
cd %OLDCD%
REM force errorlevel 1
set = 2>nul

:end
