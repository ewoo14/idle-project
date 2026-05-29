# 길드 기초 (Guild Foundation) — 설계 문서

> 작성일: 2026-05-29 · 작성: PM/Claude · 분류: 신규 대형 시스템 (서버 권위 멀티플레이어 메타)
> 대상 PR: **#G1 / #G2 / #G3** (A안 — 서버 권위 수직 슬라이스 3 PR) · 백로그 로드맵 #80
> 상태: 설계 확정(브레인스토밍 7문 합의 완료), 구현 전 현행 코드 재검증 필요.

---

## 0. 한 줄 요약

서버 권위로 동작하는 **비동기 협력 길드** 시스템. 멤버는 따로 플레이해도 **4종 기여도**(일일
출석·재화 헌납·전투/던전 자동·길드 보스 데미지)가 길드에 쌓이고, **길드 레벨 영구 버프 ·
길드 상점 · 공유 HP 풀 길드 보스 · 주간 길드 랭킹**으로 보상한다. 오프라인은 **로컬 캐시 +
재접속 동기화**로 굴린다.

---

## 1. 목적 / 배경

- 현 메타는 전부 **단일 플레이어 로컬 세이브**(환생/펫/마스터리/던전/주간보스). 사회적·경쟁
  리텐션 벡터가 리더보드(#76)뿐 → **비동기 협력**이라는 강력한 장기 리텐션 축이 비어 있음.
- 기획 근거: [`docs/planning/00-overview.md`](../../planning/00-overview.md) §메타 "길드(비동기)".
- 무한 성장 원칙([[project-infinite-growth]]): 길드 레벨·보스 마일스톤·기여도 모두 무한 곡선.
- 콘텐츠 풍부화([[project-content-richness]]): 길드 상점/계급/보스/랭킹 다층 동기.
- **재사용**: #77 주간 보스(공유 HP 풀 누적), #76 리더보드(주간 랭킹), 시즌/리더보드 서버 모듈
  패턴(repo/routes/schema/service), 주간 리셋 ISO week(`GetCurrentUtcWeekString`).

### 1-a. 가장 큰 설계 전환점 — 서버 권위

지금까지 모든 슬라이스는 UE 클라 로컬 세이브였다. 길드는 **공유 상태**(멤버십·길드 레벨·보스
누적·랭킹)라 서버에 권위가 있어야 한다. 따라서 서버 신규 `guild` 모듈 + 클라 동기화 계층이
핵심이며, 오프라인 방치(게임의 본질)와 공유 상태를 **로컬 캐시 + 재접속 플러시**로 화해시킨다.

---

## 2. 핵심 결정 (브레인스토밍 합의)

| 항목 | 결정 | 근거 |
| --- | --- | --- |
| 범위 | 전체 ①CRUD/멤버십 + ②기여/출석/버프/상점 + ③길드 보스 | 풀세트 (스펙 내 3 PR 분할) |
| 가입 | **자유/승인 선택 가능**(길드장 설정) | 길드별 운영 자유도 |
| 계급 | **군대식, 인원으로 해금** (정원 30 / ≤10 길드장·멤버 / 11~20 +부길드장 / 21~30 +간부) | 소규모 관리 부담↓, 대규모 위계 |
| 기여도 발생원 | ①일일 출석 ②재화 헌납 ③전투/던전 자동 ④길드 보스 데미지 | 비동기·복합 동기 |
| 보상 | 길드 레벨 영구 버프 · 길드 상점(기여도 교환) · 보스 클리어 · 주간 랭킹 | 다층 payoff |
| 동기화 | **로컬 캐시 + 재접속 동기화** (오프라인엔 캐시 버프 적용, 기여 누적 후 플러시) | 방치형 적합·서버 부하↓ |
| 길드 보스 | **공유 HP 풀**(비동기 누적 데미지, 격파 시 전원 보상) | #77 패턴 길드화 |
| 정원 | 30 (길드 레벨로 정원·계급 확장은 후속 여지) | 비동기 규모·공유 HP 페이싱 |

### 2-a. PM 판단으로 확정한 잔여 결정

| 항목 | 결정 | 근거 |
| --- | --- | --- |
| 1캐릭터 1길드 | `guild_members.character_id` UNIQUE | 정합성·단순성 |
| 계급 정원 | 부길드장 최대 1, 간부 최대 3 (나머지 멤버) | 위계 희소성 |
| 길드장 탈퇴 | 부길드장→최고 기여 멤버 순 자동 위임, 후보 0이면 길드 해산 | 무주공산 방지 |
| 기여 화폐 2종 분리 | **길드 EXP**(레벨·랭킹용, 소비 안 됨) ↔ **개인 기여 포인트**(상점 소비) | 길드 성장과 개인 보상 분리 |
| 헌납 신뢰 경계 | 골드는 로컬 권위 → 클라가 차감 후 헌납 보고, **일일 상한**으로 어뷰즈 억제 | 기존 클라 권위 골드와 동일 신뢰 경계 |
| 주간 리셋 | ISO week 문자열(주간 기여·랭킹·보스 누적 리셋, 누적 EXP/포인트는 영속) | 기존 주간 패턴 재사용 |

---

## 3. 시스템 분해 (단위·경계)

각 단위는 하나의 책임 + 명확한 인터페이스로 독립 테스트 가능하게 설계한다.

### 3-a. 서버 — 신규 `server/src/modules/guild/`
시즌/리더보드 모듈과 동일 구조: `guild.schema.ts`(Fastify JSON 스키마) · `guild.repo.ts`(PG)
· `guild.service.ts`(도메인) · `guild.routes.ts`(라우트) · `guild.service.test.ts`(vitest).

**DB 테이블(`server/src/db/schema.ts` 추가, Drizzle):**

| 테이블 | 핵심 컬럼 | 용도 |
| --- | --- | --- |
| `guilds` | id, name(uniq,2~16), notice, join_mode('open'\|'approval'), level, exp(bigint), master_character_id, member_count(denorm), week_id, created_at | 길드 본체 |
| `guild_members` | guild_id, character_id(**uniq**), rank('master'\|'vice'\|'officer'\|'member'), joined_at, weekly_contribution, total_contribution, contribution_points, last_attendance_date | 멤버십·기여 |
| `guild_join_requests` | guild_id, character_id, requested_at, PK(guild_id,character_id) | 승인제 신청 |
| `guild_boss` | guild_id, week_id, accum_damage(bigint), defeated_count, PK(guild_id) | 공유 보스(주간) — PR-G3 |
| `guild_boss_contrib` | guild_id, week_id, character_id, damage(bigint) | 보스 데미지 기여(내부 랭킹) — PR-G3 |

> 주간 랭킹은 `guild_members.weekly_contribution` 합(또는 `guilds.exp` 주간 델타) 집계로
> 산출, 별도 영속 테이블 불필요(#76 리더보드 쿼리 패턴). 인덱스: `guild_members(guild_id)`,
> `guilds(week_id, ...)`.

**라우트(`/guilds`, 전부 `app.authenticate` + rateLimit):**

| 메서드/경로 | 권한 | 설명 | PR |
| --- | --- | --- | --- |
| `POST /` | 누구나(무소속) | 길드 생성(생성자=길드장) | G1 |
| `GET /me` | 본인 | 내 길드 스냅샷(클라 캐시 소스) | G1 |
| `GET /:id` | 누구나 | 길드 조회(공개 정보) | G1 |
| `GET /` | 누구나 | 길드 목록/검색(페이지) | G1 |
| `POST /:id/join` | 무소속 | 자유=즉시 가입 / 승인=신청 | G1 |
| `POST /:id/leave` | 멤버 | 탈퇴(길드장은 위임/해산 규칙) | G1 |
| `POST /:id/requests/:charId/{approve,reject}` | 길드장/부 | 승인제 처리 | G1 |
| `PATCH /:id` | 길드장(일부 부) | 설정(name/notice/join_mode) | G1 |
| `POST /:id/members/:charId/rank` | 길드장 | 계급 승강(인원 해금·정원 검증) | G1 |
| `POST /:id/attendance` | 멤버 | 일일 출석 기여(1일 1회) | G2 |
| `POST /:id/donate` | 멤버 | 재화 헌납 기여(일일 상한) | G2 |
| `POST /:id/contribute` | 멤버 | 전투/던전 자동 기여 델타 플러시(상한) | G2 |
| `GET /:id/shop` · `POST /:id/shop/:itemId/buy` | 멤버 | 길드 상점(개인 기여 포인트) | G2 |
| `POST /:id/boss/challenge` | 멤버 | 공유 보스 누적 데미지(주당 횟수 제한) | G3 |
| `POST /:id/boss/claim` | 멤버 | 격파 보상 수령(전원) | G3 |
| `GET /rankings` | 누구나 | 주간 길드 랭킹(#76 확장) | G3 |

### 3-b. 클라 — UE5 `client/Source/IdleProject/GameCore/`
WeeklyBossService 패턴: `GuildTypes.h`(enum/struct) · `GuildService.{h,cpp}`(상태·캐시·오프라인
적용·동기화) · `GuildFormula.{h,cpp}`(계급 해금·길드 레벨·버프·보스 HP 순수 함수).
서버 통신은 `NetworkClient` 확장(`/guilds/*` 엔드포인트). UI는 `UI/` 길드 패널.

### 3-c. 동기화 계층 (로컬 캐시 + 재접속)
- **캐시 대상(세이브 영속)**: `guildId`, 캐시된 `guildLevel`·버프 계수, 내 `rank`, 멤버 요약,
  보스 진행, `contributionPoints`, **미플러시 기여 델타**(전투/던전 자동), `lastAttendanceDate`.
- **오프라인**: 캐시 버프를 CP/전투/골드 계산에 적용. 전투/던전 자동 기여는 로컬 누적.
- **재접속/세이브 시**: `POST /contribute`로 누적 델타 플러시 → `GET /me`로 스냅샷 갱신 →
  캐시 재설정. 서버 권위로 충돌 정리(서버 값 우선).

---

## 4. 공식 / 밸런스 (balance-note로 별도 확정, 초기값 제안)

```
# 계급 해금 (인원 기반)
RankUnlock(memberCount):
  master 항상 1 ; member 항상
  memberCount >= 11 → vice 해금(최대 1)
  memberCount >= 21 → officer 해금(최대 3)
GUILD_CAPACITY = 30   # v1 고정 (길드 레벨 확장은 후속)

# 길드 레벨 (무한 기하)
GuildLevelThreshold(L) = floor(10_000 * 1.6^(L-1))   # 누적 EXP 임계
GuildBuff(L): 공격력 +0.4%*L, 골드획득 +0.4%*L  (캐시되어 오프라인 적용)

# 기여도 (EXP 와 개인 기여 포인트 동시 적립)
출석:        +50 / 일 (UTC date, 1회)
헌납:        floor(gold/1000) (+에센스 가중), 일일 상한 500
전투/던전:   던전 클리어·보스 도전당 +k, 주간 상한으로 어뷰즈 억제
보스 데미지: floor(damage / D) (PR-G3)
→ guild.exp += 기여   AND   member.contribution_points += 기여
→ member.weekly_contribution += 기여 (주간 랭킹·리셋)

# 길드 보스 (공유 HP 풀, 주간) — PR-G3, #77 재사용
WeeklyGuildBossHP(week, level) = floor(BOSS_BASE * 1.5^defeated_count)
도전: 주당 횟수 제한, accum_damage += GetChallengeDamage(CP)
격파(accum >= HP): defeated_count++, 전원 claim 가능, 다음 HP 상향
```

---

## 5. PR 분할 (A안 — 수직 슬라이스 3 PR, 각 멀티시스템)

> [[feedback_substantial_slices]] 충실 + [[feedback_ci_before_merge]] 안전. 각 PR = 서버 +
> 클라 + UI + 테스트 + 한글 문서/밸런스노트 + SaveVer 증가.

### PR-G1 · 길드 CRUD + 멤버십 + 계급 (SaveVer 16→17)
- 서버 `guild` 모듈: `guilds`/`guild_members`/`guild_join_requests` 테이블, 생성/조회/목록/
  가입(자유·승인)/탈퇴(위임·해산)/신청 처리/설정/계급 승강(인원 해금·정원). 서비스 트랜잭션
  (member_count denorm 정합).
- 클라 `GuildService`/`GuildFormula`/`GuildTypes` + NetworkClient `/guilds` + 길드 패널 UI
  (목록·검색·생성·내 길드·멤버 리스트·계급 표시·가입/탈퇴).
- SaveVer 16→17: `guildId`/내 rank 캐시(스냅샷 stub).
- 테스트: 멤버십 전이, 1캐릭1길드, 계급 해금/정원, 자유·승인 분기, 위임·해산, 세이브 v17 라운드트립.

### PR-G2 · 기여도 + 길드 레벨/버프 + 길드 상점 (SaveVer 17→18)
- 기여도 3종(출석/헌납/전투·던전 자동) 집계, 길드 EXP·길드 레벨, **영구 버프 로컬 적용**(CP/골드),
  개인 기여 포인트, 길드 상점(구매·차감). 일일/주간 상한 서버 검증. 주간 리셋(ISO week).
- 클라: 버프 캐시·오프라인 적용, 기여 델타 누적·플러시, 상점 UI, 헌납/출석 UI.
- SaveVer 17→18: 버프 캐시·기여 포인트·미플러시 델타·last_attendance.
- 테스트: 기여 상한, 길드 레벨 임계, 버프 CP 반영, 오프라인 캐시 적용, 주간 리셋, 세이브 v18.

### PR-G3 · 길드 보스(공유 HP 풀) + 주간 길드 랭킹 (SaveVer 18→19)
- `guild_boss`/`guild_boss_contrib`, 공유 HP 풀 누적·격파·전원 보상(#77 길드화), 보스 데미지→
  기여도(4번째 발생원), `/rankings` 주간 길드 랭킹(#76 확장 — 클라 랭킹 UI 재사용).
- 클라: 보스 패널(공유 HP/누적/도전), 격파 보상, 길드 랭킹 탭.
- SaveVer 18→19: 보스 진행 캐시.
- 테스트: HP/격파/마일스톤, 도전 제한, 데미지→기여, 랭킹 집계, 세이브 v19.

---

## 6. 통합 지점 / 교차 검증 포인트

- **UUID 정합**(서버 PG ↔ 클라 캐릭터): `character_id`로 길드 멤버십 조인. cross-service 정합 TM 검증.
- **API 계약**(클라 ↔ 서버): `{ok,data}` 래퍼, JSON 스키마, NetworkClient 타입 일치.
- **CP 버프 반영**: 길드 레벨 버프가 기존 CP/전투 공식([[project-infinite-growth]])과 합산되는 지점 — 중복/누락 주의.
- **주간 리셋 공유**: `GetCurrentUtcWeekString`(#56) 재사용, #77 주간보스와 리셋 타이밍 일관.
- **헌납 신뢰 경계**: 골드 로컬 권위 — 일일 상한으로 어뷰즈 제한(문서 명시).
- **jumbo(unity) 빌드 ODR**([[reference_ue_headless_verify]] §1-b): 신규 익명 헬퍼 동명 grep + PM 표준 jumbo 빌드 검증 필수.

## 7. 에러 처리 / 엣지

- 정원 초과 가입 거부, 중복 가입 거부(1캐릭1길드), 무소속만 생성/가입.
- 승인제: 신청 큐, 길드장/부 승인·거절, 가입 시 신청 정리.
- 길드장 탈퇴: 부길드장→최고 기여 멤버 위임, 후보 0이면 해산(관련 행 정리).
- 계급 승강: 인원 해금·정원 위반 시 거부, 권한 검증(길드장만 승강).
- 동시성: 가입/탈퇴/헌납/보스 도전 트랜잭션 + member_count·accum_damage 원자적 갱신.
- 주간 진입: 새 week_id면 weekly_contribution/랭킹/보스 누적 리셋.

## 8. 테스트 전략

- **서버(vitest)**: 모듈별 서비스 테스트 — 멤버십 전이·계급·가입모드·기여 상한·길드 레벨·보스 HP/격파·랭킹 집계.
- **클라(UE Automation)**: GuildService 캐시/오프라인 적용/델타 플러시, 버프 CP 반영, 세이브 v17/18/19 라운드트립. `tools/ci/ue-automation.ps1` 게이트.
- **통합(QA)**: 캐릭터→길드 생성/가입→기여→길드 레벨 버프 반영→보스 격파→랭킹 시나리오. cross-DB 정합 SQL.
- 머지 전: 전 저장소 GREEN + PM 표준 jumbo 빌드 + UE Automation 게이트.

## 9. 비고

- 본 스펙은 구현 전 기획 — 디스패치 시점 현행 코드 재검증 필요.
- v3 멀티에이전트 워크플로우([[feedback_workflow_v3]]) + Codex 한도 소진 시 Claude 서브에이전트 구현팀.
- 채팅·길드전(PvP)·길드 던전 다종화는 후속 백로그.
