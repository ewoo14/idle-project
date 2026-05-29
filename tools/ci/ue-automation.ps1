<#
.SYNOPSIS
  UE5 표준 jumbo(unity) 빌드 + Automation 게이트.

.DESCRIPTION
  server-ci(GitHub-hosted)는 UE 빌드/Automation을 검증하지 않는다(러너에 UE 미설치).
  본 스크립트는 머지 전 PM(또는 self-hosted 러너)이 실행하는 단일 게이트:
    1. 표준 jumbo(unity) 빌드(-DisableUnity 없이) → ODR 충돌 검출
    2. 광범위 Automation(기본: 전체 IdleProject) → SaveVersion/오버플로/회귀 등 코어 단언 검출
    3. Result={Fail} 또는 비정상 EXIT CODE 시 비0 종료
  과거 잠재 red(#73 골드 오버플로/#75 던전패널/#81 SaveVer 단언)가 누적된 사각지대를 닫는다.

.PARAMETER Filter
  Automation 필터. 기본 "IdleProject"(전체). 빠른 점검 시 "IdleProject.GameCore" 등 좁혀도 됨.

.EXAMPLE
  ./tools/ci/ue-automation.ps1
  ./tools/ci/ue-automation.ps1 -Filter "IdleProject.Mastery+IdleProject.GameCore.SaveSystem"
#>
[CmdletBinding()]
param(
  [string]$EnginePath = "C:\Program Files\Epic Games\UE_5.7",
  [string]$Project = "$PSScriptRoot\..\..\client\IdleProject.uproject",
  [string]$Filter = "IdleProject"
)

$ErrorActionPreference = "Stop"
$buildBat = Join-Path $EnginePath "Engine\Build\BatchFiles\Build.bat"
$cmdExe = Join-Path $EnginePath "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"

if (-not (Test-Path $buildBat)) { Write-Host "[ue-automation] ERROR: Build.bat 없음: $buildBat"; exit 2 }
if (-not (Test-Path $cmdExe)) { Write-Host "[ue-automation] ERROR: UnrealEditor-Cmd 없음: $cmdExe"; exit 2 }
if (-not (Test-Path $Project)) { Write-Host "[ue-automation] ERROR: uproject 없음: $Project"; exit 2 }
$Project = (Resolve-Path $Project).Path

# 1) 표준 jumbo(unity) 빌드 — -DisableUnity 금지(ODR 검출 목적)
Write-Host "[ue-automation] (1/2) 표준 jumbo(unity) 빌드..."
& $buildBat IdleProjectEditor Win64 Development -Project="$Project" -WaitMutex -NoHotReload
if ($LASTEXITCODE -ne 0) {
  Write-Host "[ue-automation] ERROR: 빌드 실패 (exit $LASTEXITCODE). jumbo ODR/컴파일 오류 확인."
  exit 1
}
Write-Host "[ue-automation] 빌드 성공."

# 2) Automation
$stamp = Get-Date -Format "yyyyMMdd-HHmmss"
$log = Join-Path ([System.IO.Path]::GetTempPath()) "ue-automation-$stamp.log"
Write-Host "[ue-automation] (2/2) Automation 실행: $Filter"
& $cmdExe "$Project" -ExecCmds="Automation RunTests $Filter; Quit" `
  -unattended -nopause -nosplash -nullrhi -stdout -FullStdOutLogOutput *> $log

$pass = (Select-String -Path $log -Pattern "Result=\{Success\}" | Measure-Object).Count
$failLines = Select-String -Path $log -Pattern "Result=\{Fail\}"
$fail = ($failLines | Measure-Object).Count
$exitLine = Select-String -Path $log -Pattern "TEST COMPLETE\. EXIT CODE: (-?\d+)" | Select-Object -Last 1
$harnessExit = if ($exitLine) { [int]$exitLine.Matches[0].Groups[1].Value } else { 99 }

Write-Host "[ue-automation] 통과 $pass / 실패 $fail / harness exit $harnessExit"
Write-Host "[ue-automation] 로그: $log"

if ($fail -gt 0) {
  Write-Host "[ue-automation] === 실패 테스트 ==="
  $failLines | ForEach-Object { Write-Host $_.Line }
  # 실패 직후 'Expected ... but it was' 진단 라인도 출력
  Select-String -Path $log -Pattern "Expected .* but it was" | ForEach-Object { Write-Host $_.Line }
  exit 1
}
if ($harnessExit -ne 0) {
  Write-Host "[ue-automation] ERROR: harness 비정상 종료(exit $harnessExit) — 크래시/타임아웃 가능. 로그 확인."
  exit 1
}
if ($pass -le 0) {
  Write-Host "[ue-automation] ERROR: 통과 테스트 0 — 필터/실행 오류 의심."
  exit 1
}
Write-Host "[ue-automation] GREEN."
exit 0
