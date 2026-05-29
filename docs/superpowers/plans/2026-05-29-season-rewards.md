# 시즌 경쟁 보상 구현 계획 (선행, PR #78)

> 선행 기획. Codex 복구(6/1) 후 v3 디스패치. 디스패치 시 현행 코드 재검증. **수령은 서버 권위.**

**Goal:** #76 리더보드 완료 시즌(id<현재) 순위로 티어 보상을 서버 권위 수령(시즌당 1회).

## 계약 (양쪽 동일, fround)
```
seasonRewardTier(rank): rank==0→none, 1→tier1, <=10→tier2, <=100→tier3, else→tier4(참여)
seasonRewardGold(tier): tier1 1,000,000 / tier2 300,000 / tier3 100,000 / tier4 20,000 (참여) — balance 확정
seasonRewardEssence(tier): tier1 500 / tier2 150 / tier3 50 / tier4 10
```

## Task 1: backend (backend)
- [ ] `seasonReward.ts` + `seasonReward.test.ts` + index export: 위 계약(티어 경계 1/10/100, 보상 단조).
- [ ] 마이그레이션: `season_reward_claim(character_id uuid, season_id int, claimed_at, pk(character_id,season_id))`.
- [ ] repo: `hasClaimed(characterId, seasonId)`, `recordClaim(characterId, seasonId)`.
- [ ] service(LeaderboardService 확장 또는 신규): `getSeasonRewardStatus(characterId, seasonId, currentSeason)`(season<current 여부 + rank(listPower/getPowerRank 재사용) + tier + reward + claimed), `claimSeasonReward(characterId, seasonId, currentSeason)`(검증: season<current && !claimed && rank>0 → recordClaim + reward 반환).
- [ ] routes: `GET /leaderboard/season-reward?season=&characterId=`, `POST /leaderboard/season-reward/claim`(body season+characterId). 현재 시즌은 서버가 SeasonService/설정에서 판단(또는 요청 검증).
- [ ] vitest: 완료/진행중 거부, 중복 거부, 티어 경계, rank 0 거부. `lint && test -- leaderboard seasonReward && build` GREEN.
- [ ] 커밋 `feat: 시즌 경쟁 보상 backend (서버 권위) (PR #78)`.

## Task 2: client (character)
- [ ] ApiClient `FetchSeasonReward(season, characterId, cb)`/`ClaimSeasonReward(season, characterId, cb)`.
- [ ] ULeaderboardService: 시즌 보상 상태(FSeasonRewardStatus{bClaimable, Tier, Gold, Essence, bClaimed}) 파싱/보관. GameInstance `ClaimSeasonReward(season)`(성공 응답의 보상값 AddGold/RuneEssence 반영 — 권위는 서버). 현재 시즌/직전 시즌 id=SeasonService.
- [ ] Automation: 상태 JSON 파싱(claimable/claimed/tier), graceful.
- [ ] 커밋 `feat: 시즌 보상 클라 + 수령 (PR #78)`.

## Task 3: UI (designer)
- [ ] 리더보드 패널 "시즌 보상" 섹션: 직전(완료) 시즌 보상 수령 CTA(티어/보상 표시), 현재 시즌 예상 순위/보상(미수령). ko/en + CsvIntegrity. 표준 jumbo 빌드.
- [ ] 커밋 `feat: 시즌 보상 UI + ko/en (PR #78)`.

## Task 4: balance (balance)
- [ ] `docs/planning/season-rewards-balance-note.md`: 티어 컷(1/10/100) 근거 + 보상 곡선(시즌 길이·참여 보상) + 경제 영향(상위 보상이 코어 페이싱 비침범 — 시즌 1회성).
- [ ] 커밋 `docs: 시즌 보상 밸런스 노트 (PR #78)`.

## Task 5: qa (qa)
- [ ] 완료/진행중 구분, 중복 수령 차단(서버), 티어 경계(rank 1/10/11/100/101/0), parity. 표준 jumbo 빌드 + Automation + vitest.
- [ ] 커밋 `test: 시즌 보상 E2E/권위 (PR #78)`.

## Self-Review
- **서버 권위 수령**(클라 보상 표시 + 서버 검증/기록) — 조기/중복 수령 차단 핵심. 세이브 변경 없음(서버 테이블). parity(티어/보상). jumbo ODR 주의.
- 명칭 일관: `seasonRewardTier`/`season_reward_claim`/`FetchSeasonReward`.

## 매핑: 1→backend, 2→character(메인), 3→designer, 4→balance, 5→qa.
