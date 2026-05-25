# DB 스키마 (stub)

> **stub 파일입니다.** 본격 ERD / 컬럼 / 인덱스 / 마이그레이션 매핑은 **PR #7 (M3 백엔드 V1)** 에서 작성. 현 시점에는 후속 리뷰의 기준점이 되는 목차 + 윤곽만 정의합니다.

---

## 작성 책임 / 시점

- **소유자**: Codex 백엔드·DB 에이전트 (`docs/workflow/agents/15-backend-db.md`)
- **첫 작성 PR**: PR #7 (백엔드 V1)
- **승인**: PM
- **참고**: `docs/planning/02-architecture.md §3.4`

---

## 예정 목차

### 1. ERD 다이어그램
- (Mermaid 또는 dbdiagram.io 임베드)

### 2. 테이블별 상세

#### 2.1 `users`
- 컬럼 / 타입 / 제약 / 인덱스
- 마이그레이션 파일 링크

#### 2.2 `characters`
- 컬럼 / 타입 / 제약 / 인덱스
- `inventory` JSONB 의 스키마 (M3 까지) → 분해 트리거 (M5, 아이템 50개 초과 시)

#### 2.3 `saves`
- 백업 정책 (최근 10개 보관)
- 서버 검증 컬럼 (`server_validated`)

#### 2.4 `leaderboard_power`
- 계산 주기 (Redis ZSET ↔ PG 동기화)
- 시즌 컬럼 (`season_id`)

#### 2.5 `leaderboard_rebirth`

#### 2.6 `events` (1.0)

#### 2.7 `audit_log` (1.0)

### 3. 외래 키 / Cascade 정책

### 4. 인덱스 전략
- 검색 패턴별 인덱스 (예: leaderboard 의 `(season_id, power_score DESC)`)
- 부분 인덱스 / 표현 인덱스

### 5. 마이그레이션 운영
- forward / rollback 정책
- ZDD (Zero-Downtime Deployment) 패턴
- 큰 마이그레이션의 단계 분리

### 6. 보존 / 백업
- pg_dump 일일 백업 (7일 보관)
- `saves` 테이블 백업 (사용자별 10개)
- 복구 절차

---

## 본 stub 의 검증 항목 (PR #1 한정)

- [x] 후속 리뷰가 "어떤 테이블의 어떤 컬럼" 을 인용할 수 있는 목차 확보
- [x] 소유자 / 첫 작성 PR 명시
- [x] `docs/planning/02-architecture.md §3.4` 와 cross-link 가능
