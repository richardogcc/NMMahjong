# Comprueba que este PC con Windows tiene todo lo necesario para compilar
# NM Mahjong y lo compila. Pensado para un equipo recién clonado.
#
#   powershell -ExecutionPolicy Bypass -File scripts\setup.ps1            comprueba y compila
#   powershell -ExecutionPolicy Bypass -File scripts\setup.ps1 -Install   además instala lo que falte con winget
#   powershell -ExecutionPolicy Bypass -File scripts\setup.ps1 -Check     solo comprueba
param([switch]$Install, [switch]$Check)

$ErrorActionPreference = 'Stop'
Set-Location (Join-Path $PSScriptRoot '..')
$script:missing = $false

function Ok($text)   { Write-Host "  [ok] $text" -ForegroundColor Green }
function Bad($text)  { Write-Host "  [!!] $text" -ForegroundColor Red; $script:missing = $true }
function Has($name)  { [bool](Get-Command $name -ErrorAction SilentlyContinue) }

function Need($command, $wingetId, $purpose) {
  if (Has $command) { Ok "$command - $purpose"; return }
  if ($Install -and (Has 'winget')) {
    Write-Host "  Instalando $wingetId..."
    winget install --id $wingetId -e --silent --accept-package-agreements --accept-source-agreements | Out-Null
    # winget cambia el PATH del sistema; se relee para esta sesión.
    $env:Path = [Environment]::GetEnvironmentVariable('Path', 'Machine') + ';' +
                [Environment]::GetEnvironmentVariable('Path', 'User')
    if (Has $command) { Ok "$command - instalado"; return }
  }
  Bad "$command - falta ($purpose). Instalalo con: winget install --id $wingetId -e"
}

Write-Host 'Comprobando el equipo'
if (-not (Has 'winget')) { Write-Host '  [..] winget no esta disponible: habra que instalar a mano lo que falte' -ForegroundColor Yellow }
Need 'git'   'Git.Git'                          'descargar el codigo'
Need 'cmake' 'Kitware.CMake'                    'preparar la compilacion'
Need 'ninja' 'Ninja-build.Ninja'                'compilar'
Need 'g++'   'BrechtSanders.WinLibs.POSIX.UCRT' 'compilador de C++ (MinGW-w64)'

if ($script:missing) {
  Write-Host ''
  Write-Host 'Falta algo de lo anterior. Corrigelo o vuelve a lanzar con -Install.'
  exit 1
}
Write-Host 'Todo listo.'
if ($Check) { exit 0 }

Write-Host ''
Write-Host 'Compilando para Windows'
cmake -S . -B build/windows -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
if ($LASTEXITCODE -ne 0) { Write-Host 'La preparacion ha fallado.' -ForegroundColor Red; exit 1 }
cmake --build build/windows
if ($LASTEXITCODE -ne 0) { Write-Host 'La compilacion ha fallado.' -ForegroundColor Red; exit 1 }
ctest --test-dir build/windows --output-on-failure
if ($LASTEXITCODE -ne 0) { Write-Host 'Las pruebas han fallado.' -ForegroundColor Red; exit 1 }

Write-Host ''
Write-Host 'Hecho. Para jugar: build\windows\NMMahjong.exe'
