# 에이전트 명세 — 한눈에 보기

본 디렉터리는 17개 에이전트 (PM + TM + 7파트 × Codex/Claude) 의 시스템 프롬프트 / 책임 / 산출물을 정의합니다.

| 파일 | 에이전트 |
| --- | --- |
| [01-pm.md](01-pm.md) | **PM** (Claude) — 기획, 머지 결정 |
| [02-tm.md](02-tm.md) | **TM** (Claude) — 리뷰 종합, fix 지시 |
| [_template.md](_template.md) | 공통 시스템 프롬프트 골격 |
| [10-designer.md](10-designer.md) | 디자이너 — UI/UX, 아트 |
| [11-story.md](11-story.md) | 스토리 작가 — 시나리오, 대사 |
| [12-quest.md](12-quest.md) | 퀘스트 — 퀘스트 데이터/로직 |
| [13-character-items-stats.md](13-character-items-stats.md) | 캐릭터·아이템·능력치 |
| [14-balance.md](14-balance.md) | 밸런스 — 수치, 곡선 |
| [15-backend-db.md](15-backend-db.md) | 백엔드·DB |
| [16-qa.md](16-qa.md) | QA — 테스트, 회귀 |

각 파트 파일에는 **Codex 구현 에이전트** 와 **Claude 리뷰 에이전트** 의 책임이 함께 정의됩니다.

---

## 운영 모드

- **Codex 구현 에이전트**: `codex exec --dangerously-bypass-approvals-and-sandbox -p <role>` 로 호출. PR 브랜치에 직접 커밋.
- **Claude 리뷰 에이전트**: Claude Code 의 sub-agent (Agent 도구) 로 호출. PR 코멘트로 한글 리뷰.
- **PM / TM**: Claude Code 의 메인 세션 또는 sub-agent.

호출 헬퍼: [`codex/scripts/invoke.ps1`](../../../codex/scripts/invoke.ps1).
