[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)]
    [ValidateSet('designer','story','quest','character','balance','backend','qa')]
    [string]$Role,

    [Parameter(Mandatory=$true)]
    [string]$Task,

    [string]$WorkingDir = "C:\game\idle game\repo",

    [string]$Model = "",

    [switch]$DryRun
)

# Codex 구현 에이전트 호출 헬퍼.
# 사용:
#   .\codex\scripts\invoke.ps1 -Role backend -Task "회원가입 핸들러 구현"
#   .\codex\scripts\invoke.ps1 -Role designer -Task "메인 HUD 와이어프레임" -DryRun

$ErrorActionPreference = 'Stop'

$repoRoot = $WorkingDir
$basePromptPath = Join-Path $repoRoot "codex\prompts\_base.md"
$rolePromptPath = Join-Path $repoRoot "codex\prompts\$Role.md"

if (-not (Test-Path $basePromptPath)) {
    throw "공통 프롬프트 파일이 없습니다: $basePromptPath"
}
if (-not (Test-Path $rolePromptPath)) {
    throw "역할 프롬프트 파일이 없습니다: $rolePromptPath"
}

$basePrompt = Get-Content $basePromptPath -Raw -Encoding UTF8
$rolePrompt = Get-Content $rolePromptPath -Raw -Encoding UTF8

$fullPrompt = @"
$basePrompt

---

$rolePrompt

---

## 이번 작업 (PM 으로부터)

$Task

## 종료 시
- 변경 사항을 git status / git diff 로 점검
- 의미 있는 단위로 커밋 (prefix: codex($Role): ...)
- 관련 문서 (docs/) 동기 갱신
- 작업 요약을 stdout 으로 출력 (PM/TM 이 PR 코멘트로 인용 예정)
"@

if ($DryRun) {
    Write-Host "[DRY-RUN] 다음 프롬프트로 호출됩니다:" -ForegroundColor Cyan
    Write-Host ("=" * 60)
    Write-Host $fullPrompt
    Write-Host ("=" * 60)
    Write-Host "[DRY-RUN] 실제 codex 호출은 생략됩니다." -ForegroundColor Cyan
    return
}

$args = @(
    'exec',
    '--dangerously-bypass-approvals-and-sandbox',
    '-C', $repoRoot,
    '--skip-git-repo-check'
)
if ($Model -ne "") {
    $args += @('-m', $Model)
}
$args += $fullPrompt

Write-Host "[invoke.ps1] codex $Role 호출 시작" -ForegroundColor Green
& codex @args
$exit = $LASTEXITCODE
Write-Host "[invoke.ps1] codex 종료 코드: $exit" -ForegroundColor Green
exit $exit
