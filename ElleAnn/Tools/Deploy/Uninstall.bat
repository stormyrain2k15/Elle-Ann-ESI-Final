@echo off
REM Uninstall.bat — retired Feb 2026.
REM
REM To uninstall: open services.msc as Administrator, right-click each
REM Elle-Ann service, Stop, then run `sc.exe delete ElleHTTPServer`
REM (etc.) from an elevated prompt.  Or run the exe with --uninstall:
REM     Elle.Service.HTTP.exe --uninstall
echo.
echo Uninstall.bat is retired.
echo.
echo To uninstall Elle services:
echo   EITHER: for each service, run (elevated):
echo             Elle.Service.HTTP.exe --uninstall
echo   OR:     in services.msc, Stop the service,
echo           then in elevated cmd:  sc delete ElleHTTPServer
echo.
pause
