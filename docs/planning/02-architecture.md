# 기술 아키텍처 — idle-project

> 본 문서는 클라이언트(UE5) + 서버(Docker) + 데이터 흐름을 정의합니다. 구체 구현(클래스/모듈/스키마)은 각 슬라이스 PR 에서 코드와 함께 추가됩니다.

---

## 1. 전체 시스템 개요

```
+----------------------+        HTTPS (REST + WSS)        +-------------------------+
|  UE5 Client (PC)     |  <----------------------------> |  API Gateway (Fastify) |
|  - GameCore (C++)    |                                  |  - Auth / Save / Rank  |
|  - UMG UI            |                                  |  - WebSocket (이벤트)  |
|  - Niagara FX        |                                  +-----------+-------------+
+----------+-----------+                                              |
           | 로컬 세이브 (오프라인 폴백)                                | TCP
           v                                                          v
   %APPDATA%/IdleProject/                              +--------------+--------------+
                                                       | PostgreSQL 16  |  Redis 7   |
                                                       | (영속)         |  (캐시/세션)|
                                                       +-----------------------------+
                                                                      |
                                                                      | (관측 1.0+)
                                                                      v
                                                       +-----------------------------+
                                                       | Prometheus + Grafana (1.0) |
                                                       +-----------------------------+
```

---

## 2. 클라이언트 (Unreal Engine 5)

### 2.1 버전 / 환경
- **UE5 5.4 (LTS)**
- 빌드 타깃: Win64 (1차), Mac (선택)
- Editor: Windows 11 + JetBrains Rider 또는 VS 2022
- 그래픽 API: DirectX 12 (PC), Metal (Mac)
- 렌더 파이프라인: Lumen 비활성 (도트 풍 → 라이트 가벼움), Niagara 활성

### 2.2 코드 정책
| 영역 | C++ | Blueprint |
| --- | --- | --- |
| 게임 코어 (능력치, 데미지 공식, 세이브, 네트워크) | **C++** | 금지 |
| 전투 시스템 (AI, 스킬 실행) | **C++ 기반** + 일부 BP 노출 | 데이터 바인딩만 |
| UI (UMG) | C++ 뷰모델 + BP 표현 | **BP 주체** |
| 이펙트 (Niagara) | — | **BP / 데이터** |
| 아트 콘텐츠 / 애님 | — | **BP / 데이터** |

원칙: **로직은 C++ , 표현은 BP**. C++ 로 작성된 결정적 로직만이 서버 검증과 호환됩니다.

### 2.3 모듈 구조 (제안)
```
client/Source/IdleProject/
├── GameCore/         # 게임 상태, 세이브, 글로벌 서비스
├── CombatSystem/     # 전투, 데미지, 스킬, 버프
├── ItemSystem/       # 인벤토리, 장비, 강화, 잠재
├── QuestSystem/      # 퀘스트, 진행, 보상
├── CharacterSystem/  # 캐릭터, 직업, 능력치, 스킬 트리, 환생
├── PetSystem/        # 펫 (1.0)
├── SaveSystem/       # 로컬 + 클라우드 세이브
├── NetworkClient/    # API 호출, WebSocket
├── UI/               # UMG ViewModel, HUD
└── DataAssets/       # 데이터 테이블, 컨텐츠 데이터
```

### 2.4 핵심 클래스 (초안)
- `UIdleGameInstance` — 글로벌 상태, 서비스 컨테이너
- `AIdleCharacter` — 캐릭터 액터 (Pawn 파생)
- `UCombatComponent` — 전투 컴포넌트
- `UInventoryComponent` — 인벤토리 컴포넌트
- `USaveService` — 세이브 직렬화/역직렬화
- `UApiClient` — REST 클라이언트
- `UWsClient` — WebSocket 클라이언트

### 2.5 데이터 자산
- 콘텐츠는 **데이터 테이블 (DataTable)** 또는 **CSV import** 로 관리.
- 핵심 테이블: `MonsterDB`, `ItemDB`, `SkillDB`, `MapDB`, `QuestDB`, `LevelCurveDB`.
- 콘텐츠 변경 시 PR 에 `[content]` 라벨 부착.

### 2.6 빌드 / 배포
- 개발: Editor PIE (Play In Editor)
- CI: UE5 헤드리스 빌드 (자체 호스팅 러너 필요, 1.0 까지는 PR 라벨로 트리거)
- 출시: Steam — `BuildCookRun` → `steamcmd`

---

## 3. 서버 (Docker)

### 3.1 기술 스택

| 컴포넌트 | 선택 | 사유 |
| --- | --- | --- |
| 런타임 | **Node.js 22 LTS** | 빠른 프로토타이핑, 에이전트 친화, TS 생태계 |
| 프레임워크 | **Fastify 5** | 빠른 라우팅, 스키마 검증(JSON Schema/AJV) 내장 |
| 언어 | **TypeScript 5.5** | 타입 안정, 클라이언트와 스키마 공유 |
| ORM | **Drizzle ORM** | 타입 안전 마이그레이션, 가벼움 |
| DB | **PostgreSQL 16** | 정형/관계형, JSONB 지원 |
| 캐시 / 세션 | **Redis 7** | 세션, 리더보드 ZSET, rate limit |
| 인증 | **JWT (RS256)** + 이메일/패스워드 | 단순, 모바일/PC 공통 |
| 로그 | **pino** | 구조화 로그 |
| 관측 | OpenTelemetry → Prometheus + Grafana (1.0) | 점진 도입 |

### 3.2 모듈 구조 (제안)
```
server/
├── src/
│   ├── modules/
│   │   ├── auth/         # 회원가입/로그인/토큰
│   │   ├── save/         # 클라우드 세이브 업/다운
│   │   ├── leaderboard/  # 랭킹 (Redis ZSET + PG 영속)
│   │   ├── event/        # 시즌/이벤트 (1.0)
│   │   └── admin/        # 운영자 API (CSV 임포트, 정지)
│   ├── core/             # 공통: db, redis, logger, error
│   ├── plugins/          # Fastify 플러그인
│   └── server.ts         # 엔트리
├── migrations/           # Drizzle 마이그레이션
├── tests/                # vitest
├── Dockerfile
└── package.json
```

### 3.3 API 초안

> 완전한 명세는 [`docs/api/`](../api/) 에서. 여기는 윤곽만.

#### 인증
- `POST /v1/auth/register` — 이메일, 비밀번호, 닉네임
- `POST /v1/auth/login` — 이메일, 비밀번호 → JWT (access 15분, refresh 30일)
- `POST /v1/auth/refresh` — refresh 토큰 → 새 access
- `POST /v1/auth/logout`

#### 세이브
- `GET  /v1/save` — 최신 세이브 JSON
- `PUT  /v1/save` — 세이브 업로드 (서버 검증 후 저장; 의심 시 reject)
- `GET  /v1/save/history?limit=10` — 백업 목록

#### 캐릭터 / 진행
- `POST /v1/character/rebirth` — 환생 (서버 검증)
- `POST /v1/character/level-up` — 레벨업 (서버 검증 — 클라이언트는 표시만)

#### 랭킹
- `GET  /v1/leaderboard/power?range=season` — 전투력 랭킹
- `GET  /v1/leaderboard/rebirth` — 환생 랭킹

#### 이벤트 (실시간)
- `WSS /v1/events` — 서버 푸시 (이벤트 시작, 보스 출현, 길드 알림 — 1.0)

### 3.4 DB 스키마 (핵심)

> 자세한 ERD 는 [`docs/api/schema.md`](../api/schema.md) (백엔드 슬라이스 PR).

```
users
  - id (uuid, pk)
  - email (citext, unique)
  - password_hash (bcrypt)
  - nickname (varchar 16, unique)
  - created_at, last_login_at

characters
  - id (uuid, pk)
  - user_id (fk users)
  - class_id (int)
  - level (int)
  - rebirth_count (int)
  - stats (jsonb)           -- 능력치 스냅샷
  - skill_tree (jsonb)
  - inventory (jsonb)        -- M3 까지 jsonb, M5 분해 검토
  - last_save_at

saves
  - id (uuid, pk)
  - character_id (fk)
  - version (int)
  - payload (jsonb)
  - server_validated (bool)
  - created_at

leaderboard_power
  - character_id (fk, pk)
  - power_score (bigint)
  - season_id (int)
  - updated_at
  - INDEX (season_id, power_score DESC)
```

### 3.5 보안

| 항목 | 정책 |
| --- | --- |
| 인증 | JWT RS256, refresh rotation, 토큰 폐기는 Redis blacklist |
| 비밀번호 | bcrypt cost 12 |
| HTTPS | 운영은 nginx + Let's Encrypt 또는 Caddy |
| CSRF | API 는 토큰 기반이므로 N/A (쿠키 미사용) |
| Rate Limit | Redis 기반, IP+User 키 — `auth/login` 5회/분 |
| 입력 검증 | Fastify schema (JSON Schema/AJV) 필수 |
| SQL Injection | Drizzle 파라미터 바인딩만 |
| 세이브 위변조 | 서버에서 핵심 진행(레벨/환생/장비 최고치) 재계산 검증, 의심 시 reject + 로그 |
| 비밀 관리 | `.env` (개발), 운영은 secret manager 또는 호스트 환경변수 |

### 3.6 Docker 운영

`infra/docker-compose.yml` 정의 서비스 (자세히는 [`docs/ops/docker.md`](../ops/docker.md)):

- `postgres:16` — 데이터 영속
- `redis:7-alpine` — 세션/캐시/랭킹
- `server` — Node.js API (multi-stage Dockerfile)
- `pgadmin:latest` — 개발 전용 (profile=dev)
- `prometheus`, `grafana` — 1.0 (profile=ops)

볼륨 마운트로 데이터 영속, `.env` 로 비밀 주입. 개발은 `docker compose --profile dev up`, 운영은 `docker compose up -d`.

---

## 4. 클라이언트 ↔ 서버 동기화

### 4.1 동기화 모델
- **하이브리드**: 로컬 우선 + 클라우드 백업.
- 로컬 세이브: `%APPDATA%/IdleProject/save.json` (PC), `~/Library/Application Support/IdleProject/save.json` (Mac).
- 5분마다 / 환생/장비 강화/보스 격파 시 클라우드 동기화.
- 충돌 시: 서버 권위적 (서버가 마지막 저장으로 결정), 로컬은 알림 후 덮어쓰기.

### 4.2 오프라인 모드
- 네트워크 단절 시 게임 진행 가능.
- 복귀 시 큐에 쌓인 변경분 일괄 동기화.
- 한도: 오프라인 누적 24시간 (그 이상은 일부 동기화 거부 가능성).

---

## 5. 환경 / 인프라

### 5.1 환경 분리
- `local` — 개발자 PC, docker compose
- `dev` — 공용 개발 서버 (1.0 이후)
- `staging` — 출시 전 검증 (1.0 이후)
- `prod` — Steam 출시 시점

### 5.2 비밀 / 환경 변수
`.env.example` 참고:
```
DATABASE_URL=postgres://idle:CHANGE_ME@postgres:5432/idle
REDIS_URL=redis://redis:6379
JWT_PRIVATE_KEY_PATH=/run/secrets/jwt_private.pem
JWT_PUBLIC_KEY_PATH=/run/secrets/jwt_public.pem
LOG_LEVEL=info
NODE_ENV=development
```

### 5.3 백업 / 복구
- PostgreSQL: `pg_dump` 일일 백업 (cron), 7일 보관.
- 세이브 히스토리: DB 내 `saves` 테이블에서 최근 10개 백업.
- 복구 절차: [`docs/ops/recovery.md`](../ops/recovery.md) (백엔드 슬라이스 PR).

---

## 6. 관측성 (Observability)

- **로그**: pino 구조화 → stdout → Docker 로그 드라이버 → (운영) Loki
- **메트릭**: prom-client → `/metrics` → Prometheus → Grafana (1.0)
- **트레이스**: OpenTelemetry → Tempo 또는 Jaeger (1.0)
- **알람**: Alertmanager → Discord/Slack 웹훅 (1.0)

MVP 단계에는 로그만, 1.0 에서 메트릭/대시보드.

---

## 7. 의존성 정책

- 외부 라이브러리는 **MIT / Apache 2.0 / BSD** 우선.
- AGPL/GPL 은 클라이언트 코드와 격리.
- 신규 의존성 추가 PR 은 본문에 라이선스/사유 명시 → TM 종합 시 확인.

---

## 8. 결정 기록 (ADR — Architecture Decision Records)

본 문서 외 주요 기술 결정은 `docs/planning/adr/NNNN-제목.md` 로 별도 기록합니다. 첫 ADR 후보:

- ADR-0001 — Node.js vs Go 서버 선택 (Node.js 채택)
- ADR-0002 — PostgreSQL vs MongoDB (PostgreSQL 채택)
- ADR-0003 — UE5 5.4 vs 5.5 (5.4 LTS 채택)
- ADR-0004 — Drizzle vs Prisma (Drizzle 채택 — 가벼움, 마이그레이션 명료)

ADR 작성은 백엔드 슬라이스 PR (#7) 에서 시작.
