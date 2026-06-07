<#
.SYNOPSIS
    Downloads Lua 5.4.6 source into Lua\lua54\src so the Elle.Lua.Behavioral
    project builds out-of-the-box with no external setup.

.DESCRIPTION
    Elle.Lua.Behavioral.vcxproj auto-detects Lua sources at
    $(LuaDir)src\*.c (defaults to ElleAnn\Lua\lua54\src) and compiles them
    inline — no prebuilt lua54.lib required. The source tree isn't checked
    into the repo (would balloon it and license-mix), so this script grabs
    it from https://www.lua.org/ftp/ on first run.

    Idempotent: skips the download if Lua\lua54\src already has files.

.EXAMPLE
    cd ElleAnn\Lua
    .\Fetch-Lua.ps1
#>
[CmdletBinding()]
param(
    [string]$Version = '5.4.6',
    [string]$Dest    = (Join-Path $PSScriptRoot 'lua54')
)

$ErrorActionPreference = 'Stop'
$srcDir = Join-Path $Dest 'src'

if (Test-Path (Join-Path $srcDir 'lapi.c')) {
    Write-Host "Lua source already present at $srcDir — skipping." -ForegroundColor Green
    exit 0
}

$url = "https://www.lua.org/ftp/lua-$Version.tar.gz"
$tmp = Join-Path $env:TEMP "lua-$Version.tar.gz"
Write-Host "Downloading $url ..."
Invoke-WebRequest -Uri $url -OutFile $tmp -UseBasicParsing

Write-Host "Extracting ..."
$extractDir = Join-Path $env:TEMP "lua-extract-$Version"
if (Test-Path $extractDir) { Remove-Item $extractDir -Recurse -Force }
New-Item -ItemType Directory -Force -Path $extractDir | Out-Null
# tar.exe ships with Win10 1803+
tar.exe -xzf $tmp -C $extractDir

$innerSrc = Join-Path $extractDir "lua-$Version\src"
if (-not (Test-Path $innerSrc)) {
    throw "Extraction didn't produce expected src folder: $innerSrc"
}

New-Item -ItemType Directory -Force -Path $Dest | Out-Null
Copy-Item $innerSrc -Destination $Dest -Recurse -Force

Remove-Item $tmp -ErrorAction SilentlyContinue
Remove-Item $extractDir -Recurse -Force -ErrorAction SilentlyContinue

$count = (Get-ChildItem $srcDir -Filter '*.c' | Measure-Object).Count
Write-Host "Lua $Version installed: $count .c files at $srcDir" -ForegroundColor Green
