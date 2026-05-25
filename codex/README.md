# codex/ — Codex CLI 호출 자산

> Codex CLI 구현 에이전트를 호출하기 위한 프롬프트와 헬퍼 스크립트.

## 1. 사전 조건

- Codex CLI 설치 (`npm install -g @openai/codex-cli` 또는 ChatGPT 동봉 버전)
- 로그인: `codex login` (ChatGPT 또는 API 키)
- 작업 디렉터리: `C:\game\idle game\repo`

확인:
```powershell
codex --version           # codex-cli 0.130 이상 권장
codex login status        # "Logged in" 출력
```

## 2. 디렉터리

```
codex/
├── README.md            # 본 파일
├── prompts/
│   ├── _base.md         # 모든 호출에 첨부되는 공통 프롬프트
│   ├── pm.md            # PM 호출용 (드물게 사용; PM 은 보통 Claude)
│   ├── designer.md
│   ├── story.md
│   ├── quest.md
│   ├── character.md
│   ├── balance.md
│   ├── backend.md
│   └── qa.md
└── scripts/
    └── invoke.ps1       # 호출 헬퍼
```

## 3. 호출 예시

### 단일 역할 호출
```powershell
.\codex\scripts\invoke.ps1 -Role backend -Task "회원가입 핸들러 구현 — POST /v1/auth/register"
```

### 직접 호출 (스크립트 우회)
```powershell
$prompt = (Get-Content "codex\prompts\_base.md", "codex\prompts\backend.md" -Raw) -join "`n---`n"
$task   = "회원가입 핸들러 구현 — POST /v1/auth/register"

codex exec `
  --dangerously-bypass-approvals-and-sandbox `
  -C "C:\game\idle game\repo" `
  --skip-git-repo-check `
  "$prompt`n`n## 이번 작업`n$task"
```

## 4. 권한

사용자 명시에 따라 모든 Codex 호출은 `--dangerously-bypass-approvals-and-sandbox` 로 운영됩니다. 단:
- 시크릿 커밋 금지
- main 직접 push 금지
- 워크플로우 단계 건너뛰기 금지

## 5. 출력 / 산출

- Codex 는 PR 브랜치에 직접 커밋합니다 (`codex(<role>): ...` 메시지 prefix).
- 세션 로그는 `~/.codex/sessions/` 에 저장됩니다 (.gitignore 됨).
