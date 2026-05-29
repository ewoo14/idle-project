# 길드 기초 PR-G1 구현 계획 — CRUD/멤버십/계급

> 스펙: [`2026-05-28~`/`2026-05-29-guild-foundation-design.md`](../specs/2026-05-29-guild-foundation-design.md). v3 멀티에이전트 디스패치. 디스패치 시 현행 코드 재검증(jumbo ODR 주의).

**Goal:** 서버 권위 길드의 1차 슬라이스 — 길드 생성/조회/목록/가입(자유·승인)/탈퇴(위임·해산)/계급(인원 해금·정원) + 클라 길드 패널 기본 + SaveVer 16→17.

**Architecture:** 서버 신규 `guild` 모듈(시즌/리더보드 패턴: schema/repo/service/routes/test) + DB 3테이블. 클라 `GuildService`/`GuildFormula`/`GuildTypes` + NetworkClient `/guilds` + UI 패널. 동기화는 `GET /me` 스냅샷을 로컬 캐시.

**Tech Stack:** Node22/Fastify5/Drizzle/PostgreSQL16/vitest · UE5.7 C++/Automation.

---

## Task 1: backend (backend)
- [ ] DB 스키마(`server/src/db/schema.ts`) 추가 + 마이그레이션:
  - `guilds(id uuid pk, name varchar16 uniq, notice text, join_mode text default 'open', level int default 1, exp bigint default 0, master_character_id uuid, member_count int default 1, week_id text, created_at)`.
  - `guild_members(guild_id uuid fk, character_id uuid uniq, rank text default 'member', joined_at, weekly_contribution bigint default 0, total_contribution bigint default 0, contribution_points bigint default 0, last_attendance_date text, pk(character_id))` + index(guild_id).
  - `guild_join_requests(guild_id uuid fk, character_id uuid, requested_at, pk(guild_id,character_id))`.
- [ ] `guild.repo.ts`(PG, 트랜잭션): `createGuild`, `getGuild`, `listGuilds(limit,offset,q)`, `getMembership(characterId)`, `listMembers(guildId)`, `insertMember`/`deleteMember`(member_count 원자적 ±1), `setRank`, `updateGuild(settings)`, `insertRequest`/`deleteRequest`/`listRequests`.
- [ ] `guild.service.ts` 도메인 규칙:
  - `create(characterId, name)`: 무소속만, name 2~16/유니크, 생성자 rank='master', member_count=1.
  - `join(characterId, guildId)`: 무소속·정원<30·중복 금지. join_mode='open'→즉시 insertMember(rank='member'); 'approval'→insertRequest.
  - `approve/reject(actor, guildId, charId)`: actor rank∈{master,vice} 권한; approve=정원 검증 후 insertMember+deleteRequest.
  - `leave(characterId)`: 멤버면 deleteMember. **길드장 탈퇴**: 후보=부길드장→총기여 최다 멤버 순 위임(setRank master); 후보 0이면 길드·멤버·요청 전부 삭제(해산).
  - `setRank(actor, guildId, charId, rank)`: actor=master만, `RankUnlock(member_count)` 위반·정원(vice≤1, officer≤3) 위반 거부.
  - `updateSettings(actor, guildId, {name?,notice?,joinMode?})`: actor∈{master,vice}(name/joinMode는 master만).
  - `snapshot(characterId)`(`GET /me`): 멤버십+길드+멤버목록+요청(권한자) → 클라 캐시 소스.
- [ ] `RankUnlock(memberCount)` 순수 함수: vice 해금 ≥11, officer 해금 ≥21 (스펙 §4).
- [ ] `guild.schema.ts` Fastify JSON 스키마(생성/가입/설정/계급 body·params), `guild.routes.ts`(스펙 §3-a 표 G1행, `app.authenticate`+rateLimit, `{ok,data}`), `server.ts`에 `app.register(guildRoutes,{prefix:'/guilds'})`.
- [ ] `guild.service.test.ts`(vitest): 생성(유니크/무소속), 가입 자유/승인, 정원 초과 거부, 1캐릭1길드, 계급 해금/정원 위반, 권한 검증, 탈퇴·위임·해산. `npm run lint && test -- guild && build` GREEN(lint 필수).
- [ ] 커밋 `feat: 길드 backend — CRUD/멤버십/계급 (PR-G1)`.

## Task 2: client (character)
- [ ] `GameCore/GuildTypes.h`: `EGuildRank{Master,Vice,Officer,Member}`, `EGuildJoinMode{Open,Approval}`, `FGuildMemberInfo`, `FGuildSummary`, `FGuildSnapshot`(내 rank/길드/멤버목록/요청).
- [ ] `GameCore/GuildFormula.{h,cpp}` 순수 함수: `IsRankUnlocked(EGuildRank, int32 MemberCount)`(서버 `RankUnlock` parity: vice≥11, officer≥21), `GetRankSlotCap`(vice 1/officer 3), `GUILD_CAPACITY=30`.
- [ ] `GameCore/GuildService.{h,cpp}`: 스냅샷 보관·접근자(내 길드 여부/rank/멤버), 캐시 갱신(`ApplySnapshot`). 서버 권위 — 로컬은 캐시만.
- [ ] `NetworkClient`/ApiClient: `/guilds` 호출 `CreateGuild`/`ListGuilds`/`GetGuild`/`GetMyGuild`/`JoinGuild`/`LeaveGuild`/`Approve`/`Reject`/`SetRank`/`UpdateSettings` 콜백형.
- [ ] `IdleGameInstance`: 로그인/세이브 시 `GetMyGuild`→`GuildService::ApplySnapshot`. SaveVer **16→17**: `IdleSaveGame.h`에 `CachedGuildId`(FString), `CachedGuildRank`(uint8) 추가, save/restore 배선.
- [ ] Automation(`Tests/GuildTests.cpp`, 서버 무의존): `IsRankUnlocked` parity, 스냅샷 적용/접근자, 세이브 v17 라운드트립.
- [ ] 표준 jumbo 빌드 확인(신규 익명 헬퍼 동명 grep). 커밋 `feat: 길드 클라 — Service/Formula/캐시 + SaveVer17 (PR-G1)`.

## Task 3: UI (designer)
- [ ] 길드 패널: ①무소속 화면(길드 목록·검색·생성) ②내 길드(요약·멤버 리스트·계급 배지·탈퇴) ③길드장 관리(설정 자유/승인 토글·name/notice, 승인 큐 approve/reject, 계급 승강 — 해금/정원 미충족 시 비활성).
- [ ] ko/en 로컬라이즈 + CsvIntegrity. 디자인 토큰(`docs/planning/ui-tokens.json`) 준수. 표준 jumbo 빌드.
- [ ] 커밋 `feat: 길드 패널 UI — 목록/생성/내길드/관리 (PR-G1)`.

## Task 4: balance/docs (balance)
- [ ] `docs/planning/guild-foundation-balance-note.md`: 정원 30, 계급 해금 임계(vice≥11/officer≥21, vice 1·officer 3 슬롯), 가입모드(자유/승인), 위임·해산 규칙, 1캐릭1길드. 기여/레벨/버프=G2·보스=G3 범위 밖 명시.
- [ ] [[project_pr_order]]·[[project_session_progress]] 갱신(PR-G1/SaveVer17). 커밋 `docs: 길드 기초 G1 밸런스/진행 노트 (PR-G1)`.

## Task 5: qa (qa)
- [ ] E2E: 생성→가입(자유 즉시/승인 신청→approve)→계급 해금(11/21명 경계)·승강·권한 거부→탈퇴/길드장 위임/해산→정원 초과·중복 가입 거부. cross-DB 정합 SQL(guild_members.character_id ↔ characters.id). 세이브 v17 round-trip. 표준 jumbo 빌드 + `tools/ci/ue-automation.ps1` 게이트.
- [ ] PR 본문 스크린샷 의무. 커밋 `test: 길드 기초 G1 E2E (PR-G1)`.

## Self-Review
- 스펙 §3-a G1행 라우트 전부 Task1 커버 ✓ / 클라 캐시·SaveVer17 Task2 ✓ / UI 3화면 Task3 ✓.
- 명칭 parity: 서버 `RankUnlock` ↔ 클라 `IsRankUnlocked`(vice≥11/officer≥21), `member_count` denorm 원자성 — TM cross-check 포인트.
- 범위 경계 명확: 기여/버프/상점=G2, 보스/랭킹=G3 (본 PR 미포함). 헌납·CP 버프 없음 → CP 공식 불변.
- jumbo(unity) ODR 주의([[reference_ue_headless_verify]] §1-b).

## 매핑: 1→backend, 2→character(메인), 3→designer, 4→balance, 5→qa.
