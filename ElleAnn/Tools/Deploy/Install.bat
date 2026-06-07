@echo off
REM Install.bat — retired Feb 2026.
REM
REM Elle services are now Fiesta-style self-installing: double-click
REM each Release\x64\Elle.Service.*.exe (as Administrator) and click
REM OK on the "SERVICE UPLOAD ONLY OK" dialog.  After that, manage
REM them via services.msc.  No batch / ps1 / dependency chain.
REM
REM This batch is kept only so anyone relying on the old filename gets
REM a pointer instead of a silent no-op.
echo.
echo Install.bat is retired.
echo.
echo To install Elle services:
echo   1. Navigate to Release\x64 (or wherever your builds land)
echo   2. Right-click Elle.Service.HTTP.exe  -  Run as administrator
echo   3. Double-click the exe
echo   4. Click OK on the "SERVICE UPLOAD ONLY OK" dialog
echo   5. Repeat for each other Elle.Service.*.exe
echo.
echo Services can start/stop/restart in any order from services.msc.
pause
