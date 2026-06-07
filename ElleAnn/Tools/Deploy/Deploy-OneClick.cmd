@echo off
setlocal

set DEPLOY_ROOT=%~dp0
set LOGFILE=C:\MoePromise\Server\deploy.log

echo ==== ElleAnn Deploy %DATE% %TIME% ====>>"%LOGFILE%"
echo Running from: %DEPLOY_ROOT%>>"%LOGFILE%"
echo.>>"%LOGFILE%"

powershell -NoProfile -ExecutionPolicy Bypass -File "%DEPLOY_ROOT%Deploy.ps1" -Action Repoint -InstallDir "C:\MoePromise\Server" -ShortExeNames -Restart >>"%LOGFILE%" 2>>&1
set ERR=%ERRORLEVEL%

echo.>>"%LOGFILE%"
echo ExitCode: %ERR%>>"%LOGFILE%"
echo ======================================>>"%LOGFILE%"

exit /b %ERR%
