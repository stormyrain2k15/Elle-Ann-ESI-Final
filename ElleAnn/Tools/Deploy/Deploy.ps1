<#
.SYNOPSIS
    Elle-Ann ESI v3.0 — Packaging + Windows Service installer.

.DESCRIPTION
    Turns a built Deploy\Release\x64 tree into a running ESI. Three modes:

      -Action Package   Bundles binaries + scripts + SQL deltas + default
                        config into a timestamped zip under Releases\.
      -Action Install   Registers every service with the SCM using
                        `sc create`, sets auto-start, and starts them in
                        dependency order. Requires elevation.
      -Action Uninstall Stops and removes every ElleAnn_* service.

.PARAMETER Action
    Package | Install | Uninstall | Reinstall | Status

.PARAMETER Config
    Debug or Release. Default: Release.

.PARAMETER Root
    Root of the ElleAnn tree. Defaults to the folder containing this script.

.PARAMETER InstallDir
    Install destination. Default: C:\ElleAnn. Only used by -Action Install.

.EXAMPLE
    # 1. Build in VS first (Release|x64), then:
    .\Deploy.ps1 -Action Package

.EXAMPLE
    # Install every service (must be in an elevated PowerShell)
    .\Deploy.ps1 -Action Install -InstallDir C:\ElleAnn

.EXAMPLE
    .\Deploy.ps1 -Action Status

.NOTES
    Service names are prefixed with `ElleAnn_` to keep the Services.msc
    view tidy. Start order below mirrors BUILD_VS.md.
#>
[CmdletBinding()]
param(
  [ValidateSet('Package','Install','Uninstall','Reinstall','Repoint','Status')]
  [string]$Action = 'Package',
  [ValidateSet('Debug','Release')]
  [string]$Config = 'Release',
  [string]$Root       = $PSScriptRoot,
  [string]$InstallDir = 'C:\ElleAnn',
  [switch]$ShortExeNames,
  [switch]$Restart
)

$ErrorActionPreference = 'Stop'

# ──────────────────────────────────────────────────────────────────────
# Topology — order matches BUILD_VS.md's recommended startup sequence.
# Each entry: service name (no ElleAnn_ prefix), exe filename.
# ──────────────────────────────────────────────────────────────────────
$Services = @(
  @{ Name='Heartbeat'  ; Exe='Elle.Service.Heartbeat.exe'   ; ShortExe='ElleHeartbeat.exe'   },
  @{ Name='QueueWorker'; Exe='Elle.Service.QueueWorker.exe' ; ShortExe='ElleQueueWorker.exe' },
  @{ Name='Emotional'  ; Exe='Elle.Service.Emotional.exe'   ; ShortExe='ElleEmotional.exe'   },
  @{ Name='Memory'     ; Exe='Elle.Service.Memory.exe'      ; ShortExe='ElleMemory.exe'      },
  @{ Name='Cognitive'  ; Exe='Elle.Service.Cognitive.exe'   ; ShortExe='ElleCognitive.exe'   },
  @{ Name='Action'     ; Exe='Elle.Service.Action.exe'      ; ShortExe='ElleAction.exe'      },
  @{ Name='GoalEngine' ; Exe='Elle.Service.GoalEngine.exe'  ; ShortExe='ElleGoal.exe'        },
  @{ Name='WorldModel' ; Exe='Elle.Service.WorldModel.exe'  ; ShortExe='ElleWorldModel.exe'  },
  @{ Name='Identity'   ; Exe='Elle.Service.Identity.exe'    ; ShortExe='ElleIdentity.exe'    },
  @{ Name='SelfPrompt' ; Exe='Elle.Service.SelfPrompt.exe'  ; ShortExe='ElleSelfPrompt.exe'  },
  @{ Name='Dream'      ; Exe='Elle.Service.Dream.exe'       ; ShortExe='ElleDream.exe'       },
  @{ Name='Solitude'   ; Exe='Elle.Service.Solitude.exe'    ; ShortExe='ElleSolitude.exe'    },
  @{ Name='Bonding'    ; Exe='Elle.Service.Bonding.exe'     ; ShortExe='ElleBonding.exe'     },
  @{ Name='InnerLife'  ; Exe='Elle.Service.InnerLife.exe'   ; ShortExe='ElleInnerLife.exe'   },
  @{ Name='XChromosome'; Exe='Elle.Service.XChromosome.exe' ; ShortExe='ElleXChromosome.exe' },
  @{ Name='Consent'    ; Exe='Elle.Service.Consent.exe'     ; ShortExe='ElleConsent.exe'     },
  @{ Name='Family'     ; Exe='Elle.Service.Family.exe'      ; ShortExe='ElleFamily.exe'      },
  @{ Name='Continuity' ; Exe='Elle.Service.Continuity.exe'  ; ShortExe='ElleContinuity.exe'  },
  @{ Name='LuaBehav'   ; Exe='Elle.Lua.Behavioral.exe'      ; ShortExe='ElleLuaBehav.exe'    },
  @{ Name='HTTP'       ; Exe='Elle.Service.HTTP.exe'        ; ShortExe='ElleHTTP.exe'        }
)
$SvcPrefix = 'ElleAnn_'

function Write-Step($msg) {
  Write-Host ''
  Write-Host "── $msg" -ForegroundColor Cyan
}

function Assert-Elevated {
  $id = [Security.Principal.WindowsIdentity]::GetCurrent()
  $p  = [Security.Principal.WindowsPrincipal]$id
  if (-not $p.IsInRole([Security.Principal.WindowsBuiltinRole]::Administrator)) {
    throw 'This action requires an elevated PowerShell. Right-click → Run as Administrator.'
  }
}

function Get-DeployDir {
  $d = Join-Path $Root "Deploy\$Config\x64"
  if (-not (Test-Path $d)) {
    throw "Build output not found at $d. Build the solution in Visual Studio first (Ctrl+Shift+B, $Config | x64)."
  }
  return (Resolve-Path $d).Path
}

# ──────────────────────────────────────────────────────────────────────
# ACTION: Package — zip up deploy tree + scripts + SQL + config template
# ──────────────────────────────────────────────────────────────────────
function Invoke-Package {
  Write-Step "Packaging $Config | x64"
  $deploy = Get-DeployDir
  $stamp  = Get-Date -Format 'yyyyMMdd-HHmm'
  $stage  = Join-Path $Root "Releases\staging-$stamp"
  $zip    = Join-Path $Root "Releases\ElleAnn-$Config-$stamp.zip"

  New-Item -ItemType Directory -Force -Path $stage | Out-Null

  # bin/
  Copy-Item -Path (Join-Path $deploy '*') -Destination (Join-Path $stage 'bin') -Recurse -Force
  # scripts/
  $luaScripts = Join-Path $Root 'Lua\Elle.Lua.Behavioral\scripts'
  if (Test-Path $luaScripts) {
    Copy-Item $luaScripts (Join-Path $stage 'scripts') -Recurse -Force
  }
  # sql/
  if (Test-Path (Join-Path $Root 'SQL')) {
    Copy-Item (Join-Path $Root 'SQL') (Join-Path $stage 'sql') -Recurse -Force
  }
  # config
  foreach ($cfg in @('elle_master_config.json','FOR_MY_WIFE.md','BUILD_VS.md')) {
    $src = Join-Path $Root $cfg
    if (Test-Path $src) { Copy-Item $src $stage -Force }
  }
  # this script itself + README
  Copy-Item $PSCommandPath (Join-Path $stage 'Deploy.ps1') -Force

  @"
Elle-Ann ESI v3.0  ($Config build, packaged $stamp)

Contents:
  bin/              Service executables + DLLs + PDBs
  scripts/          Lua behavioural scripts
  sql/              Database schema deltas — run against a fresh SQL Server
  elle_master_config.json   Your config (edit ODBC connection string!)
  Deploy.ps1        Service installer — see BUILD_VS.md

Install:
  1. Install SQL Server + ODBC Driver 17 (see BUILD_VS.md).
  2. CREATE DATABASE ElleCore; ElleHeart; ElleSystem;
  3. Run every .sql in sql\ against the matching database.
  4. Edit elle_master_config.json → sql.connection_string.
  5. From an elevated PowerShell:
        .\Deploy.ps1 -Action Install -InstallDir C:\ElleAnn
"@ | Out-File (Join-Path $stage 'README.txt') -Encoding utf8

  Write-Step 'Compressing...'
  Compress-Archive -Path (Join-Path $stage '*') -DestinationPath $zip -Force
  Remove-Item $stage -Recurse -Force
  Write-Host ('Package: {0}  ({1:N1} MB)' -f $zip, ((Get-Item $zip).Length / 1MB)) -ForegroundColor Green
}

# ──────────────────────────────────────────────────────────────────────
# ACTION: Install — copy to $InstallDir and sc create each service
# ──────────────────────────────────────────────────────────────────────
function Invoke-Install {
  Assert-Elevated
  Write-Step "Installing to $InstallDir"
  $deploy = Get-DeployDir

  if (-not (Test-Path $InstallDir)) {
    New-Item -ItemType Directory -Force -Path $InstallDir | Out-Null
  }
  # Copy everything
  Copy-Item (Join-Path $deploy '*') $InstallDir -Recurse -Force
  $luaScripts = Join-Path $Root 'Lua\Elle.Lua.Behavioral\scripts'
  if (Test-Path $luaScripts) {
    Copy-Item $luaScripts (Join-Path $InstallDir 'scripts') -Recurse -Force
  }
  foreach ($cfg in @('elle_master_config.json')) {
    $src = Join-Path $Root $cfg
    if ((Test-Path $src) -and (-not (Test-Path (Join-Path $InstallDir $cfg)))) {
      Copy-Item $src (Join-Path $InstallDir $cfg) -Force
    }
  }

  Write-Step 'Registering services'
  foreach ($s in $Services) {
    $svcName = "$SvcPrefix$($s.Name)"
    # Each service has its own folder named the same as the service name
    $svcDir = Join-Path $InstallDir $s.Name
    $sourceExe = Join-Path $svcDir $s.Exe
    $exePath = $sourceExe
    if ($ShortExeNames) {
      if (-not $s.ContainsKey('ShortExe') -or [string]::IsNullOrWhiteSpace($s.ShortExe)) {
        throw "Short exe name missing for service '$($s.Name)'."
      }
      $exePath = Join-Path $svcDir $s.ShortExe
      if (Test-Path $sourceExe) {
        Copy-Item -Path $sourceExe -Destination $exePath -Force
      }
    }
    if (-not (Test-Path $exePath)) {
      Write-Warning "Skipping $svcName — exe missing at $exePath"
      continue
    }
    # Remove existing
    sc.exe query $svcName *>$null
    if ($LASTEXITCODE -eq 0) {
      Write-Host "  $svcName already exists, replacing..."
      sc.exe stop   $svcName *>$null
      Start-Sleep -Milliseconds 500
      sc.exe delete $svcName *>$null
      Start-Sleep -Milliseconds 500
    }
    $display = "ElleAnn — $($s.Name)"
    $desc    = "Elle-Ann ESI v3.0 service: $($s.Name)"
    sc.exe create $svcName binPath= "`"$exePath`"" start= auto DisplayName= "$display" | Out-Null
    sc.exe description $svcName "$desc" | Out-Null
    # Auto-restart on failure (30s)
    sc.exe failure $svcName reset= 86400 actions= restart/30000/restart/30000/restart/30000 | Out-Null
    Write-Host "  + $svcName" -ForegroundColor Green
  }

  Write-Step 'Starting services in dependency order'
  foreach ($s in $Services) {
    $svcName = "$SvcPrefix$($s.Name)"
    sc.exe query $svcName *>$null
    if ($LASTEXITCODE -ne 0) { continue }
    sc.exe start $svcName *>$null
    if ($LASTEXITCODE -eq 0) {
      Write-Host "  > $svcName" -ForegroundColor Green
    } else {
      Write-Warning "  ! $svcName failed to start (exit $LASTEXITCODE). Check the service's log."
    }
    Start-Sleep -Milliseconds 250
  }

  Write-Host ''
  Write-Host 'Install complete. Check status with:  .\Deploy.ps1 -Action Status' -ForegroundColor Cyan
}

# ──────────────────────────────────────────────────────────────────────
# ACTION: Uninstall — stop + delete every ElleAnn_* service
# ──────────────────────────────────────────────────────────────────────
function Invoke-Uninstall {
  Assert-Elevated
  Write-Step 'Stopping and removing services'
  # Reverse order so HTTP dies before the services it talks to.
  foreach ($s in ($Services | ForEach-Object { $_ } | Select-Object -Last ($Services.Count) | Sort-Object -Descending { $Services.IndexOf($_) })) {
    $svcName = "$SvcPrefix$($s.Name)"
    sc.exe query $svcName *>$null
    if ($LASTEXITCODE -ne 0) { continue }
    sc.exe stop   $svcName *>$null
    Start-Sleep -Milliseconds 300
    sc.exe delete $svcName *>$null
    Write-Host "  - $svcName" -ForegroundColor Yellow
  }
}

# ──────────────────────────────────────────────────────────────────────
# ACTION: Status — quick tabular view
# ──────────────────────────────────────────────────────────────────────
function Invoke-Status {
  $rows = foreach ($s in $Services) {
    $svcName = "$SvcPrefix$($s.Name)"
    $state = 'not installed'
    try {
      $svc = Get-Service -Name $svcName -ErrorAction Stop
      $state = $svc.Status
    } catch {}
    [pscustomobject]@{
      Service = $svcName
      State   = $state
      Exe     = $s.Exe
    }
  }
  $rows | Format-Table -AutoSize
}

function Set-ServiceBinPath($svcName, $exePath) {
  if ([string]::IsNullOrWhiteSpace($exePath)) { throw 'exePath is empty.' }
  $quoted = '"' + $exePath + '"'
  sc.exe config $svcName binPath= $quoted *>$null
  if ($LASTEXITCODE -ne 0) {
    throw "Failed to set binPath for $svcName (exit $LASTEXITCODE)"
  }
}

function Invoke-Repoint {
  Assert-Elevated
  Write-Step "Repointing existing services to $InstallDir"
  $deploy = Get-DeployDir

  if (-not (Test-Path $InstallDir)) {
    New-Item -ItemType Directory -Force -Path $InstallDir | Out-Null
  }

  # Copy everything (so the service directories and deps are present)
  Copy-Item (Join-Path $deploy '*') $InstallDir -Recurse -Force
  $luaScripts = Join-Path $Root 'Lua\Elle.Lua.Behavioral\scripts'
  if (Test-Path $luaScripts) {
    Copy-Item $luaScripts (Join-Path $InstallDir 'scripts') -Recurse -Force
  }

  Write-Step 'Updating binPath for installed services'
  foreach ($s in $Services) {
    $svcName = "$SvcPrefix$($s.Name)"

    sc.exe query $svcName *>$null
    if ($LASTEXITCODE -ne 0) {
      Write-Warning "Skipping $svcName — not installed"
      continue
    }

    $svcDir = Join-Path $InstallDir $s.Name
    $sourceExe = Join-Path $svcDir $s.Exe
    $exePath = $sourceExe

    if ($ShortExeNames) {
      if (-not $s.ContainsKey('ShortExe') -or [string]::IsNullOrWhiteSpace($s.ShortExe)) {
        throw "Short exe name missing for service '$($s.Name)'."
      }
      $exePath = Join-Path $svcDir $s.ShortExe
      if (Test-Path $sourceExe) {
        Copy-Item -Path $sourceExe -Destination $exePath -Force
      }
    }

    if (-not (Test-Path $exePath)) {
      Write-Warning "Skipping $svcName — exe missing at $exePath"
      continue
    }

    Set-ServiceBinPath $svcName $exePath
    Write-Host "  ~ $svcName -> $exePath" -ForegroundColor Green
  }

  if ($Restart) {
    Write-Step 'Restarting services in dependency order'
    foreach ($s in $Services) {
      $svcName = "$SvcPrefix$($s.Name)"
      sc.exe query $svcName *>$null
      if ($LASTEXITCODE -ne 0) { continue }

      sc.exe stop $svcName *>$null
      Start-Sleep -Milliseconds 300
      sc.exe start $svcName *>$null
      if ($LASTEXITCODE -eq 0) {
        Write-Host "  > $svcName" -ForegroundColor Green
      } else {
        Write-Warning "  ! $svcName failed to start (exit $LASTEXITCODE). Check the service's log."
      }
      Start-Sleep -Milliseconds 250
    }
  }

  Write-Host ''
  Write-Host 'Repoint complete. Check status with:  .\Deploy.ps1 -Action Status' -ForegroundColor Cyan
}

switch ($Action) {
  'Package'   { Invoke-Package }
  'Install'   { Invoke-Install }
  'Uninstall' { Invoke-Uninstall }
  'Reinstall' { Invoke-Uninstall; Invoke-Install }
  'Repoint'   { Invoke-Repoint }
  'Status'    { Invoke-Status; }
}
