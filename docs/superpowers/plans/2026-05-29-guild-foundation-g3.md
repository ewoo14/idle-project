# 길드 기초 PR-G3 구현 계획 — 길드 보스(공유 HP 풀) + 주간 길드 랭킹

> 스펙: [`2026-05-29-guild-foundation-design.md`](../specs/2026-05-29-guild-foundation-design.md) §3-a(G3행)/§4/§5. 선행 PR-G1(#85)/PR-G2(#86) 머지(SaveVer 18). v3 디스패치, 현행 재검증.

**Goal:** 길드 마지막 슬라이스 — **공유 HP 풀 길드 보스**(비동기 누적 데미지→격파 시 전원 보상, #77 패턴 길드화) + 보스 데미지→기여도(4번째 발생원) + **주간 길드 랭킹**(#76/#79 확장). SaveVer 18→19.

**Architecture:** 서버 guild 모듈 확장(guild_boss/guild_boss_contrib 테이블, 도전/격파/보상, /rankings) + 클라 GuildBossFormula/Service(WeeklyBoss 패턴 길드화) + 보스/랭킹 UI.

**Tech Stack:** G1/G2 동일. 재사용: #77 WeeklyBossFormula(공유 HP/마일스톤), #79 leaderboard_weekly_damage(주간 랭킹 쿼리).

## 공식 (스펙 §4, 초기값 — balance-note 확정)
```
WEEKLY_GUILD_BOSS_CHALLENGE_LIMIT = 7  (멤버당 주간 도전 횟수)
GetChallengeDamage(cp) = max(0, cp)    (#77 재사용)
GuildBossHP(defeatedCount) = floor(GUILD_BOSS_BASE_HP * GUILD_BOSS_HP_GROWTH^defeatedCount)
격파: accum_damage >= HP → defeated_count++, 전원 claim 가능, accum 잔여 이월 or 리셋(이월)
보스 데미지 기여: floor(damage / GUILD_BOSS_DMG_TO_CONTRIB)  → applyContribution(G2)
주간 길드 랭킹: Σ member.weekly_contribution (길드별), 상위 N
```

## Task 1: backend (backend)
- [ ] 마이그 0011: `guild_boss(guild_id pk, week_id text, accum_damage bigint default 0, defeated_count int default 0)`, `guild_boss_contrib(guild_id, week_id, character_id, damage bigint, pk(guild_id,week_id,character_id))`.
- [ ] `GuildBossFormula`(순수, 클라 parity 상수): `getGuildBossHp(defeatedCount)`(BASE/GROWTH), `getChallengeDamage(cp)`. 상수 export.
- [ ] `guild.service.ts` 확장: `challengeBoss(characterId, cp)`(주간 도전 한도 검증→accum_damage += dmg, contrib += dmg, **격파 루프**: accum>=HP면 defeated_count++·HP 갱신, 보스 데미지→`applyContribution`(G2 4번째 발생원)), `claimBossReward(characterId)`(현 주 격파분 미수령 보상 지급 — 멤버별 수령 표시), 주간 리셋(새 week_id). `guildRankings(limit)`(Σ weekly_contribution 길드별 정렬).
- [ ] 라우트: `POST /:id/boss/challenge`, `POST /:id/boss/claim`, `GET /:id/boss`(상태), `GET /rankings`. snapshot에 보스 상태(HP/accum/defeated/내 도전 잔여/미수령) 추가.
- [ ] vitest: 도전 한도·누적, 격파(단일/연속 격파 루프), 데미지→기여 반영, 주간 리셋, 랭킹 정렬·동점. lint/test/build GREEN.
- [ ] 커밋 `feat: 길드 보스/주간 랭킹 backend (PR-G3)`.

## Task 2: client (character)
- [ ] `GuildBossFormula.{h,cpp}`(parity 1:1: GetGuildBossHp/GetChallengeDamage + 상수). `GuildTypes` FGuildSnapshot에 보스 상태(int64 BossAccumDamage/int32 DefeatedCount/int32 BossChallengesRemaining/bool bHasUnclaimedBossReward) + 랭킹 FGuildRankingEntry.
- [ ] `GuildService`: 보스 상태 캐시·접근자. ApiClient `ChallengeGuildBoss`/`ClaimGuildBossReward`/`GetGuildBoss`/`GetGuildRankings`. IdleGameInstance: 도전 시 CP 전달, 보상 claim 지급(G2 ApplyGuildShopReward류 재화 지급 또는 전용 보상 적용), 랭킹 fetch.
- [ ] SaveVer **18→19**: 보스 진행 캐시(`CachedGuildBossDefeated`/`CachedBossChallengesRemaining` 등 표시용). 전 세이브 테스트 단언 19로 일괄 갱신.
- [ ] Automation: HP/격파 parity(연속 격파 경계), 도전 한도, 데미지→기여, 랭킹 파싱, 세이브 v19 라운드트립. 익명 헬퍼 Guild~ prefix.
- [ ] 커밋 `feat: 길드 보스/랭킹 클라 + SaveVer19 (PR-G3)`.

## Task 3: UI (designer)
- [ ] 길드 패널: **길드 보스 섹션**(공유 HP 바·누적 데미지·격파 횟수·도전 버튼[잔여]·격파 보상 수령) + **주간 길드 랭킹 탭**(상위 길드·내 길드 순위, #76 리더보드 UI 재사용). ko/en + CsvIntegrity. 표준 jumbo 빌드.
- [ ] 커밋 `feat: 길드 보스/랭킹 UI (PR-G3)`.

## Task 4: balance/docs (balance)
- [ ] `docs/planning/guild-boss-ranking-balance-note.md`: 보스 HP 곡선(BASE/GROWTH)·도전 한도 7·데미지→기여 비율·격파 보상·주간 랭킹 보상. #77/#79 대비 길드화 차이.
- [ ] [[project_pr_order]]/[[project_session_progress]] 갱신(PR-G3/SaveVer19, **길드 시스템 #80 완결**). 커밋 `docs: 길드 보스/랭킹 G3 밸런스 노트 (PR-G3)`.

## Task 5: qa (qa)
- [ ] E2E: 도전(한도)→누적→격파(연속)→전원 보상 수령→데미지 기여 반영→주간 랭킹 집계→주간 리셋. cross-DB 정합. 세이브 v19. jumbo + ue-automation.ps1 게이트. PR 스크린샷.
- [ ] 커밋 `test: 길드 보스/랭킹 G3 E2E (PR-G3)`.

## Self-Review
- 스펙 §3-a G3행(boss/challenge·claim·rankings) 전부 Task1 ✓. 보스 UI/랭킹 Task3 ✓.
- parity: 서버 `getGuildBossHp`/`getChallengeDamage` ↔ 클라 `GetGuildBossHp`/`GetChallengeDamage`(BASE/GROWTH) — TM cross-check.
- **보스 데미지→기여는 G2 `applyContribution` 재사용**(4번째 발생원, 이중적립 주의: contrib 1회만).
- SaveVer 18→19 전 테스트 단언 갱신(stale 방지, #83 교훈). 보스 진행은 서버 권위(클라 캐시 표시용).
- jumbo ODR 주의([[reference_ue_headless_verify]] §1-b).

## 매핑: 1→backend, 2→character(메인), 3→designer, 4→balance, 5→qa.
