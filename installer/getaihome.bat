@echo off
SET AIHOME=
call :GetAiHomeDirHelper HKLM\SOFTWARE > nul 2>&1
IF errorlevel 1 call :GetAiHomeDirHelper HKCU\SOFTWARE > nul 2>&1
IF errorlevel 1 call :GetAiHomeDirHelper HKLM\SOFTWARE\Wow6432Node > nul 2>&1
IF errorlevel 1 call :GetAiHomeDirHelper HKCU\SOFTWARE\Wow6432Node > nul 2>&1
echo "AIHOME=%AIHOME%"
exit /B 0

:GetAiHomeDirHelper
FOR /F "tokens=1,2,3,4*" %%i IN ('reg query "%1\Caphyon\Advanced Installer" /v "Advanced Installer Path"') DO (
	IF "%%i %%j %%k"=="Advanced Installer Path" (
		SET "AIHOME=%%m"
	)
)
IF "%AIHOME%"=="" exit /B 1
exit /B 0
