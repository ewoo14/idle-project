[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)]
    [ValidateSet('designer','story','quest','character','balance','backend','qa')]
    [string]$Role,

    [string]$Task = "",

    # 작업 지시를 파일에서 읽는다. -Task 대신 사용하면 호출 명령이 짧아져 권한 규칙 매칭이 깔끔하다.
    [string]$TaskFile = "",

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

if ($TaskFile -ne "") {
    if (-not (Test-Path $TaskFile)) {
        throw "작업 파일이 없습니다: $TaskFile"
    }
    $Task = Get-Content $TaskFile -Raw -Encoding UTF8
}
if ([string]::IsNullOrWhiteSpace($Task)) {
    throw "-Task 또는 -TaskFile 중 하나는 반드시 제공해야 합니다."
}

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
# 프롬프트는 CLI 인자 대신 stdin 으로 전달한다. Windows codex.cmd(npm 셰임)가 긴 멀티라인
# 프롬프트를 인자로 받으면 cmd 가 공백/줄바꿈에서 단어 분리해 깨진다('unexpected argument').
# codex exec 는 '-' 또는 인자 미제공 시 stdin 에서 프롬프트를 읽는다.
$args += '-'

Write-Host "[invoke.ps1] codex $Role 호출 시작 (프롬프트 stdin)" -ForegroundColor Green
$fullPrompt | & codex @args
$exit = $LASTEXITCODE
Write-Host "[invoke.ps1] codex 종료 코드: $exit" -ForegroundColor Green
exit $exit
