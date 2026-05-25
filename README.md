# idle-project — 메이플스토리 키우기 풍 방치형 게임

> **Unreal Engine 5** 기반 횡스크롤 도트 풍 방치형 RPG. **Docker** 서버 백엔드. 모든 개발은 **PM(Claude) → Codex CLI 구현팀 → Claude 리뷰팀 → TM 종합 → PM 머지** 의 멀티 에이전트 PR 워크플로우로 진행합니다.

---

## 한눈에 보기

| 항목 | 내용 |
| --- | --- |
| 장르 | 횡스크롤 방치형 RPG (자동 전투 + 캐릭터 육성 + 오프라인 보상) |
| 엔진 | Unreal Engine **5.7.4** (Paper2D + Niagara) |
| 클라이언트 타깃 | 1차 Windows (Steam 출시 목표), 추후 macOS / 모바일 |
| 서버 | Node.js 22 (TypeScript) + Fastify · PostgreSQL 16 · Redis 7 |
| 인프라 | Docker Compose (개발) → Docker 호스트 또는 클라우드 (운영) |
| 협업 | GitHub PR · 한글 리뷰 · CI 통과 후 PM 종합 소견 → 머지 |

상세 기획·아키텍처·마일스톤은 [`docs/planning`](docs/planning/00-overview.md), 개발 워크플로우는 [`docs/workflow`](docs/workflow/00-overview.md) 를 참고하세요.

---

## 저장소 구조

```
.
├── client/                 # UE5 프로젝트 (추후 Codex가 스캐폴드)
├── server/                 # Node.js + TypeScript 백엔드 (추후 Codex가 스캐폴드)
├── infra/                  # Docker Compose, 운영 자산
├── codex/                  # Codex CLI 호출 프롬프트/스크립트
├── docs/
│   ├── planning/           # 기획서 (게임 디자인, 아키텍처, 마일스톤, 아트, 밸런스)
│   ├── workflow/           # 멀티 에이전트 워크플로우 + 에이전트 명세
│   ├── api/                # 서버 API 명세
│   └── ops/                # 운영 (Docker, CI/CD)
├── .github/                # PR/이슈 템플릿, GitHub Actions
├── CONTRIBUTING.md         # 기여 규칙 (필수)
├── CODEOWNERS              # 코드 오너십
└── LICENSE                 # MIT
```

---

## 빠른 시작

> 본 PR (PM 기획 PR) 시점에는 아직 실행 가능한 코드/서버 코드가 없습니다. 다음 PR(M1 슬라이스) 부터 동작합니다.

### 1. 저장소 클론
```powershell
git clone https://github.com/ewoo14/idle-project.git
cd idle-project
```

### 2. 인프라 기동 (서버 PR 머지 후)
```powershell
cd infra
Copy-Item .env.example .env
docker compose up -d
```

### 3. UE5 클라이언트 (클라이언트 PR 머지 후)
- `client/IdleProject.uproject` 더블 클릭 → UE5 5.7.4 에디터로 열림.

---

## 개발 워크플로우 (요약, v3 — 2026-05-25 사용자 명시)

1. **PM(Claude)** 기획 → PR 생성.
2. **Codex 개발** (메인 + 보조 합동 호출) → 커밋 + 푸시. **PM 이 Codex 산출 정리 → PR 코멘트로 게시**.
3. **Claude 리뷰 + fix** — Claude TM 종합 1개 코멘트 + Claude/PM 직접 처리 가능 항목은 fix commit.
4. **Codex 리뷰 + fix** — Codex TM 종합 1개 코멘트 (Claude TM 비교 포함) + Codex 자체 fix commit. **PM 이 Codex fix 산출 → PR 코멘트로 게시**.
5. **Claude 검증 리뷰** — Claude TM 판정. fix 없음 → 단계 [N], fix 있음 → [3] 또는 [4] 로 루프.
6. (필요 시 라운드 반복)
N. **CI 통과 + PM 종합 소견 + 머지**.

자세한 단계는 [`docs/workflow/01-pm-codex-claude-loop.md`](docs/workflow/01-pm-codex-claude-loop.md) (v3) 참고.

---

## 라이선스

[MIT](LICENSE) — 2026, ewoo14.

원작 메이플스토리 IP 와 무관한 오리지널 작품입니다. 영감만 받았으며, IP·아트·고유 명칭은 모두 자체 제작합니다.
