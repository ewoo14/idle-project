# PR #2 기획서 — 백엔드 V1 (원 슬라이스 ID: S7, M3 마일스톤)

> **순서 변경 사유**: 원 마일스톤 계획에서는 M3 의 두 번째 슬라이스 (PR #7) 였으나, 사용자 환경에 UE5 5.4 가 미설치 → 클라이언트 슬라이스 (M1) 대기 중. UE5 설치 병행하면서 백엔드를 먼저 진행 (사용자 동의 — 옵션 A). 메모리 `[[project-pr-order]]` 추적.

---

## 1. 목표 / DoD

다음 기능이 **Docker compose 만으로 로컬에서 실 동작** 해야 한다:
1. 회원가입 → 로그인 → access/refresh 토큰 발급
2. 캐릭터 생성 → 세이브 업로드 (서버 권위 검증) → 세이브 다운로드
3. 전투력 / 환생 리더보드 조회
4. JWT RS256 + bcrypt + rate limit
5. CI `server-ci` 실 통과 (lint + build + test 80%+ 커버리지)

DoD 검증 시나리오:
```powershell
cd infra
Copy-Item .env.example .env
.\scripts\generate-jwt-keys.ps1            # JWT 키 페어 생성
docker compose --profile app up -d
# 헬스체크 통과까지 ~30초 대기
.\scripts\smoke-test.ps1                    # register → login → save → leaderboard
```

## 2. 범위 (In Scope)

### 2.1 `server/` 스캐폴드
- Node.js 22 LTS + TypeScript 5.5 strict
- Fastify 5 + @fastify/jwt + @fastify/rate-limit + AJV (JSON Schema)
- Drizzle ORM + PostgreSQL 16
- ioredis (Redis 7)
- vitest 1.x (단위 + 통합)
- pino 9.x (구조화 로그)
- biome 2.x (lint/format)
- Multi-stage Dockerfile
- `tsconfig.json`, `package.json`, `.dockerignore`

### 2.2 핵심 모듈
- `src/core/`: `db.ts`, `redis.ts`, `logger.ts`, `errors.ts`, `env.ts` (zod 검증), `formulas/` (능력치 공식 TS 미러)
- `src/plugins/`: `auth.ts` (JWT 검증), `rate-limit.ts`, `request-id.ts`
- `src/modules/auth/`: register / login / refresh / logout + bcrypt + Redis blacklist
- `src/modules/save/`: load / upload (서버 검증) / history(10개)
- `src/modules/leaderboard/`: power / rebirth (Redis ZSET + PG 영속)
- `src/modules/character/`: 캐릭터 생성 (1직업 — 전사) — auth + save 의 의존성 최소 모듈

### 2.3 DB / 마이그레이션
- `server/migrations/0001_init.sql` (forward + rollback):
  - `users` (id uuid pk, email citext unique, password_hash, nickname unique, created_at, last_login_at)
  - `characters` (id, user_id fk, class_id, level, rebirth_count, stats jsonb, skill_tree jsonb, inventory jsonb, last_save_at)
  - `saves` (id, character_id fk, version, payload jsonb, server_validated bool, created_at)
  - `leaderboard_power` (character_id pk, season_id, power_score bigint, updated_at, INDEX (season_id, power_score DESC))
  - `leaderboard_rebirth` (character_id pk, season_id, rebirth_count int, updated_at)
- Drizzle config + `npm run db:migrate` / `db:rollback`

### 2.4 보안
- JWT RS256, access 15분 / refresh 30일
- bcrypt cost 12
- Redis blacklist (refresh 폐기)
- Rate limit: `auth` 5/min/IP, `save` 30/min/User, `read` 120/min/User, `mutate` 20/min/User
- `@fastify/helmet`, `@fastify/cors` (개발: 와일드카드, 운영: 화이트리스트)
- 입력 검증: 모든 라우트에 JSON Schema (AJV)
- 세이브 위변조: 핵심 진행 (level, rebirth_count, max_equipment_grade) 서버 재계산 검증, reject 시 audit log

### 2.5 Docker 통합
- `server/Dockerfile` (multi-stage: deps → build → runtime, distroless 검토)
- `infra/docker-compose.yml` 의 `server` 서비스 활성화 (profile=app)
- `infra/secrets/` 디렉터리 + JWT 키 마운트 (top-level `secrets:` 블록 추가)
- `infra/scripts/generate-jwt-keys.ps1` (PowerShell) + `.sh` (bash)
- `infra/scripts/smoke-test.ps1` (E2E 검증 스크립트)

### 2.6 API 문서
- `docs/api/auth.md` — register/login/refresh/logout (예시 curl + 에러 코드)
- `docs/api/save.md` — load/upload/history + 검증 흐름
- `docs/api/leaderboard.md` — power/rebirth + 시즌 정책
- `docs/api/character.md` — 캐릭터 생성 (전사 1직업)
- `docs/api/schema.md` — 실제 ERD + 컬럼/인덱스 채움 (PR #1 의 stub 대체)
- `docs/api/errors.md` — 공통 에러 코드 / HTTP 매핑

### 2.7 ADR
- `docs/planning/adr/0001-server-runtime-node.md` — Node.js 채택 사유
- `docs/planning/adr/0002-db-postgresql.md` — PostgreSQL 채택
- `docs/planning/adr/0003-orm-drizzle.md` — Drizzle vs Prisma 결정
- `docs/planning/adr/0004-bundler-tsx-vs-esbuild.md` — 런타임 선택 (tsx dev + esbuild build)

### 2.8 테스트
- vitest 단위 (모든 모듈) ≥ 80% 라인 커버리지
- vitest 통합 (testcontainers + 실 postgres/redis)
- rate limit 검증 (5회 초과 시 429)
- 세이브 검증 부정 케이스 (조작된 페이로드 reject)
- E2E smoke (PowerShell + curl)

### 2.9 CI
- `.github/workflows/server-ci.yml` 활성 모드 (현재의 `if scaffolded` 가 자동 활성)
- 추가: PostgreSQL + Redis 서비스 컨테이너로 통합 테스트 실행

## 3. 범위 외 (Out of Scope)

| 항목 | 시점 |
| --- | --- |
| WebSocket / 실시간 이벤트 (`/v1/events`) | 1.0 |
| 길드 / 채팅 | 1.0 |
| 시즌 패스 | PR #11 |
| 운영자 페이지 (`/v1/admin/*`) | 1.0 |
| Observability (Prometheus / Grafana / OpenTelemetry) | 1.0 |
| OAuth (Google/GitHub 로그인) | 1.0 |
| 다국어 (i18n) | 1.0 |
| 풀 inventory 정규화 (JSONB 분해) | PR #4 또는 PR #11 |
| 클라이언트 통합 (UE5 NetworkClient) | PR #5 이후 |

## 4. 7파트 작업 분배

> 본 슬라이스는 **백엔드 주체**. 6개 파트는 백엔드 관점 기여 또는 N/A.

| 파트 | 작업 | Codex 호출 필요 |
| --- | --- | --- |
| **백엔드·DB** | §2.1 ~ §2.5, §2.7 (ADR), §2.8 (테스트), §2.9 (CI) | ✅ 메인 |
| 캐릭터·아이템·능력치 | `core/formulas/stats.ts` — STR/DEX/INT 등 기본 능력치 공식 TS 미러 (PR #1 의 GDD §3.2.3 1줄 예시 구현 + 단위 테스트) | ✅ 보조 |
| 밸런스 | `core/formulas/level.ts` — 경험치 곡선 함수 (`expToNext(n)`) + 단위 테스트 (PR #1 의 balance §2.1 공식 1:1) | ✅ 보조 |
| QA | §2.8 통합 시나리오 (Given/When/Then) + `docs/qa/scenarios/M3-backend-v1.md` + `regression-checklist.md` §2 채움 | ✅ 보조 |
| 디자이너 | `docs/api/errors.md` 의 응답 JSON envelope 표준 (camelCase, `{ok, data, error}` 형식 결정), 에러 코드 컬러 매핑 (운영자 페이지 1.0 대비) | ✅ 보조 (작음) |
| 스토리 | 본 PR 에서는 **N/A**. PR #8 (스토리/퀘스트 V1) 에서 첫 기여 | ❌ |
| 퀘스트 | 본 PR 에서는 **N/A**. PR #8 에서 첫 기여 | ❌ |

→ Codex 실호출: **5회** (백엔드 메인 + 4 보조)

## 5. 7파트 Codex 호출 순서

병렬 가능하나, 의존성으로 다음 순서 권장:
1. **백엔드** — 1차 (server 스캐폴드 + 모듈 + 마이그레이션 + Dockerfile + ADR + API docs)
2. **캐릭터** + **밸런스** — 백엔드 완료 후 `core/formulas/` 추가 (동시 가능)
3. **QA** — 위 모두 완료 후 시나리오 + 통합 테스트 보강
4. **디자이너** — 마지막 (API 응답 envelope 표준 적용)

## 6. 워크플로우 v2 적용 (정식 첫 PR)

본 PR 부터 [`docs/workflow/01-pm-codex-claude-loop.md`](../../workflow/01-pm-codex-claude-loop.md) 의 8단계 모두 정식 적용:
- [1] PM 기획 (본 문서)
- [2] Codex 5회 실호출 (위 순서)
- [3a] Claude TM 종합 1개
- [3b] Codex TM 종합 1개
- [4] PM 1차 통합 1개
- [5] (필요 시) Codex 2차 fix 호출
- [6a] Claude TM 재검토 1개
- [6b] Codex TM 재검토 1개
- [7] PM 2차 통합
- [8] CI 통과 + PM 종합 소견 + Squash 머지

## 7. 일정 (잠정)

- 단계 [2]: 60~90분 (Codex 5회 호출, 백엔드 메인 30~50분)
- 단계 [3a/3b]: 각 10분
- 단계 [4]~[7]: 30분
- 단계 [8]: 10분
- **총: 약 2~3시간**

UE5 설치 (~1~2h) 와 거의 동시 완료 예상.

## 8. 리스크

| 항목 | 위험 | 대응 |
| --- | --- | --- |
| Codex 가 server/ 전체를 한 호출에 다 만들기 어려움 | 중 | 백엔드 호출 시 명세에 모듈 우선순위 명시. 부분 산출 후 추가 호출. |
| testcontainers 가 Docker-in-Docker 환경에서 안 돌 가능성 | 저 | 단위 테스트는 mock 으로, 통합은 docker compose 의존 (CI 는 서비스 컨테이너) |
| JWT 키 생성 스크립트가 PowerShell + Bash 두 종 필요 | 저 | 두 스크립트 모두 작성 |
| Drizzle 마이그레이션 rollback 작성 비용 | 중 | 0001 만 rollback 필수, 후속은 forward-only 정책으로 단순화 |
| Codex 가 본 PR 범위를 초과해 1.0 기능 (WebSocket 등) 추가 | 중 | 호출 시 "Out of Scope" 명시 강조 |

## 9. 후속 PR 예고

- **PR #3 (또는 UE5 설치 후 원래 PR #2)** — 클라이언트 코어 부트 (UE5 스캐폴드, 메인 메뉴, 전사)
- **PR #4** — 자동 전투 V1 (UE5 + 백엔드 첫 연동 — 캐릭터 조회 / 세이브 동기)
- **PR #5** — 인벤토리 + 장비 V1

---

본 PR 머지 후 메모리 `[[project-pr-order]]` 갱신.
