# CLAUDE.md — idle-project (Claude Code 세션 자동 로드)

> 이 파일은 Claude Code 가 본 저장소에서 작업할 때 자동으로 읽습니다. 늘 보이는 컨텍스트이므로 핵심만 간결히 유지합니다.

---

## 프로젝트

- **이름**: idle-project (메이플스토리 키우기 풍 방치형 RPG)
- **엔진**: Unreal Engine 5.4
- **서버**: Node.js 22 + Fastify 5 + Drizzle + PostgreSQL 16 + Redis 7 (Docker Compose)
- **저장소**: https://github.com/ewoo14/idle-project.git

기획서: [`docs/planning/00-overview.md`](docs/planning/00-overview.md)  
기술 아키텍처: [`docs/planning/02-architecture.md`](docs/planning/02-architecture.md)  
마일스톤: [`docs/planning/03-milestones.md`](docs/planning/03-milestones.md)

---

## 멀티 에이전트 워크플로우 (모든 PR 적용 — 잊지 말 것)

```
[1] PM(Claude) 기획 → PR
[2] Codex 7파트 1차 구현 (디자이너/스토리/퀘스트/캐릭터·아이템·능력치/밸런스/백엔드·DB/QA)
[3] Claude 7파트 1차 리뷰
[4] TM(Claude) 1차 종합 + fix 지시
[5] Codex 2차 fix
[6] Claude 2차 검토
[7] TM 2차 종합 → fix 없음 → PM, fix 있음 → [5] 루프
[8] CI 통과 + PM 종합 소견 → 머지
```

상세: [`docs/workflow/01-pm-codex-claude-loop.md`](docs/workflow/01-pm-codex-claude-loop.md).

---

## 한글 원칙

- 모든 문서, PR/이슈/리뷰 코멘트, 커밋 메시지 본문은 **한글**.
- 코드 식별자, 영문 prefix (`feat:`, `codex(<role>):`), 외부 라이브러리명은 영문 허용.

---

## 권한 / 안전

- Codex / Claude 에이전트 모두 **전체 권한** (사용자 명시).
- 단, **금지**:
  - main 으로 직접 push (PR 우회)
  - 시크릿 / `.env` 커밋
  - 한글 외 리뷰
  - 워크플로우 단계 건너뛰기

---

## 자주 쓰는 도구

```powershell
# Codex 호출 (디자이너 예)
.\codex\scripts\invoke.ps1 -Role designer -Task "..."

# 인프라
cd infra; docker compose up -d

# PR 진행
gh pr create --base main --head <브랜치> --title "<제목>" --body-file <파일>
gh pr view --comments
gh pr merge --squash --delete-branch
```

---

## 머지 직전 체크 (PM 필수)

1. TM 2차 종합 = "fix 없음"
2. GitHub Actions CI = 녹색
3. **PM 종합 소견 코멘트** 게시 (템플릿: `docs/workflow/agents/01-pm.md`)
4. `gh pr merge --squash --delete-branch`
5. 다음 PR 진행 (마일스톤 순서)
