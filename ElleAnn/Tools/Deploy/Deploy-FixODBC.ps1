# ───────────────────────────────────────────────────────────────────────────
# Deploy-FixODBC.ps1
#
#   One-shot fix for the IM002 ODBC failures + deploy the corrected
#   elle_master_config.json to the running ElleAnn services.
#
#   Run as Administrator from the ElleAnn repo root:
#       powershell -ExecutionPolicy Bypass -File .\Deploy-FixODBC.ps1
#
#   What it does:
#     1. Checks ODBC Driver 18 for SQL Server is installed
#     2. Tries a live connection with the new connection string
#     3. If that works, copies the new config to the deployment folder
#     4. Restarts every Elle.* service so they pick up the new config
# ───────────────────────────────────────────────────────────────────────────

$ErrorActionPreference = 'Stop'

$DeployRoot   = 'C:\MoePromise\Server\ElleMemory'
$ConfigSource = Join-Path $PSScriptRoot 'elle_master_config.json'
$ConfigDest   = Join-Path $DeployRoot   '9Data\ServerInfo\elle_master_config.json'
$Server       = 'DESKTOP-IIREG0M\ELLEANN'
$Database     = 'ElleCore'

Write-Host "═══ ElleAnn ODBC Fix + Config Deploy ═══" -ForegroundColor Cyan
Write-Host ""

# ── 1. Check ODBC Driver 18 is installed ─────────────────────────────────
Write-Host "[1/4] Checking for ODBC Driver 18 for SQL Server..." -NoNewline
$driver = Get-OdbcDriver -Name 'ODBC Driver 18 for SQL Server' -ErrorAction SilentlyContinue
if (-not $driver) {
    Write-Host " MISSING" -ForegroundColor Red
    Write-Host ""
    Write-Host "  Install it from:" -ForegroundColor Yellow
    Write-Host "    https://learn.microsoft.com/sql/connect/odbc/download-odbc-driver-for-sql-server" -ForegroundColor Yellow
    Write-Host "  Then re-run this script." -ForegroundColor Yellow
    exit 1
}
Write-Host " OK" -ForegroundColor Green

# ── 2. Verify connection string works ────────────────────────────────────
Write-Host "[2/4] Testing ODBC connection to $Server\$Database..." -NoNewline
$connStr = "Driver={ODBC Driver 18 for SQL Server};Server=$Server;Database=$Database;Trusted_Connection=yes;TrustServerCertificate=yes;Encrypt=optional;"
try {
    $conn = New-Object System.Data.Odbc.OdbcConnection $connStr
    $conn.Open()
    $cmd = $conn.CreateCommand()
    $cmd.CommandText = "SELECT @@VERSION;"
    $version = $cmd.ExecuteScalar()
    $conn.Close()
    Write-Host " OK" -ForegroundColor Green
    Write-Host "    SQL Server: $($version -split "`n" | Select-Object -First 1)" -ForegroundColor DarkGray
}
catch {
    Write-Host " FAILED" -ForegroundColor Red
    Write-Host ""
    Write-Host "  ODBC error: $_" -ForegroundColor Red
    Write-Host ""
    Write-Host "  Common causes:" -ForegroundColor Yellow
    Write-Host "    a) SQL Server instance '$Server' isn't running"
    Write-Host "       → Open SQL Server Configuration Manager, start MSSQL`$ELLEANN service"
    Write-Host "    b) The current Windows account doesn't have access to '$Database'"
    Write-Host "       → In SSMS: Security → Logins → Add your Windows account → grant db_datareader/db_datawriter"
    Write-Host "    c) The instance name 'ELLEANN' is wrong"
    Write-Host "       → Check actual instance name in SQL Configuration Manager"
    exit 1
}

# ── 3. Deploy the corrected config ───────────────────────────────────────
Write-Host "[3/4] Deploying config to $ConfigDest..." -NoNewline
if (-not (Test-Path $ConfigSource)) {
    Write-Host " SOURCE MISSING" -ForegroundColor Red
    Write-Host "  Expected: $ConfigSource"
    exit 1
}
$destDir = Split-Path $ConfigDest -Parent
if (-not (Test-Path $destDir)) { New-Item -ItemType Directory -Force -Path $destDir | Out-Null }
Copy-Item $ConfigSource $ConfigDest -Force
Write-Host " OK" -ForegroundColor Green

# ── 4. Restart Elle.* services ───────────────────────────────────────────
Write-Host "[4/4] Restarting Elle.* services..."
$services = Get-Service Elle.* -ErrorAction SilentlyContinue
if (-not $services) {
    Write-Host "  (no Elle.* services found — install/register them first)" -ForegroundColor Yellow
} else {
    foreach ($svc in $services) {
        Write-Host "    $($svc.Name)... " -NoNewline
        try {
            Restart-Service $svc.Name -Force -ErrorAction Stop
            Write-Host "OK" -ForegroundColor Green
        }
        catch {
            Write-Host "FAILED ($_)" -ForegroundColor Red
        }
    }
}

Write-Host ""
Write-Host "═══ Done ═══" -ForegroundColor Cyan
Write-Host "Now reopen the Android app and try chat. Memory should work." -ForegroundColor Cyan
