[CmdletBinding()]
param(
    # 빌드 구성 (Shipping = 출시, Development = 디버그)
    [ValidateSet('Shipping', 'Development', 'Test')]
    [string]$Configuration = 'Shipping',

    [string]$Platform = 'Win64',

    [string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.7',

    [string]$ProjectPath = "$PSScriptRoot\..\..\client\IdleProject.uproject",

    [string]$OutputDir = "$PSScriptRoot\..\..\client\Saved\Packaged"
)

# UE5 클라이언트 패키징 스크립트 (BuildCookRun).
# GitHub 표준 러너에는 UE 5.7 이 없으므로 로컬 또는 self-hosted 러너에서 실행한다.
# 사용: .\tools\build\package-client.ps1 -Configuration Shipping

$ErrorActionPreference = 'Stop'

$version = (Get-Content (Join-Path $PSScriptRoot '..\..\VERSION') -Raw).Trim()
$runUAT = Join-Path $EngineRoot 'Engine\Build\BatchFiles\RunUAT.bat'

if (-not (Test-Path $runUAT)) {
    throw "RunUAT 를 찾을 수 없습니다: $runUAT (EngineRoot 확인)"
}
if (-not (Test-Path $ProjectPath)) {
    throw "프로젝트를 찾을 수 없습니다: $ProjectPath"
}

$archiveDir = Join-Path $OutputDir "$version-$Configuration"
Write-Host "[package-client] IdleProject $version / $Configuration / $Platform" -ForegroundColor Green
Write-Host "[package-client] 출력: $archiveDir" -ForegroundColor Green

& $runUAT BuildCookRun `
    -project="$ProjectPath" `
    -platform=$Platform `
    -clientconfig=$Configuration `
    -cook -build -stage -pak -archive `
    -archivedirectory="$archiveDir" `
    -nop4 -utf8output

$exit = $LASTEXITCODE
Write-Host "[package-client] RunUAT 종료 코드: $exit" -ForegroundColor Green
exit $exit
