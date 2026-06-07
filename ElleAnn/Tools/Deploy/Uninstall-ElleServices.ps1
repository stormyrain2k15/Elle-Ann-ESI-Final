# Install-ElleServices.ps1 — RETIRED (Feb 2026)
#
# The install path is now Fiesta-style double-click-to-register:
#
#   1. Right-click each Elle.Service.*.exe (and Elle.Lua.Behavioral.exe)
#      → Run as administrator → double-click.
#   2. A MessageBox pops with "<SERVICE UPLOAD ONLY OK>" on success.
#   3. Click OK. The exe exits. From this point on you control the
#      service via services.msc exactly the way you do Zone / World /
#      Login in Fiesta itself.
#
# The services have NO SCM dependency chain — they can be started,
# stopped, or restarted in any order (see Shared/ElleServiceBase.cpp
# RunReconnectorLoop + TickReconnector). Boot order is no longer a
# deployment knob to worry about.
#
# This script exists only as a historical note and safety-exit: if
# you run it anyway, it tells you what to do instead.

Write-Host ""
Write-Host "Install-ElleServices.ps1 is retired." -ForegroundColor Yellow
Write-Host ""
Write-Host "To install Elle services now:"  -ForegroundColor Cyan
Write-Host "  1. Right-click each Elle.Service.*.exe (Release\\x64)"
Write-Host "  2. Run as administrator"
Write-Host "  3. Double-click the exe"
Write-Host "  4. Click OK on the 'SERVICE UPLOAD ONLY OK' dialog"
Write-Host ""
Write-Host "After that, use services.msc to start/stop/restart —"
Write-Host "no dependency chain, any order works."
Write-Host ""
exit 0
