# PR #78 기획서 — 시즌 경쟁 보상 (선행 기획)

> **선행 기획 (Codex 한도 소진 → 6/1 복구 후 디스패치)**. #76 리더보드에 시즌 종료 순위 보상 추가 → 경쟁 동기. **완료 시즌(id<현재)** 최종 순위로 티어 보상, 서버 권위 수령(시즌당 1회). client + server 5-team. 디스패치 시 현행 코드 재검증.
>
> 스펙: [`docs/superpowers/specs/2026-05-29-season-rewards-design.md`](../../superpowers/specs/2026-05-29-season-rewards-design.md)
> 계획: [`docs/superpowers/plans/2026-05-29-season-rewards.md`](../../superpowers/plans/2026-05-29-season-rewards.md)

## 1. 목표 / DoD
리더보드(#76)에 실질 동기 부여. 완료 시즌 순위 → 보상.

### DoD
1. backend: `seasonReward.ts`(rank→tier→reward 미러) + `season_reward_claim` 테이블 + `GET /leaderboard/season-reward`(수령 가능/티어/보상) + `POST /season-reward/claim`(season<현재·미수령·서버 rank 산정·기록) + vitest.
2. 티어: 1위/top10/top100/참여 → 골드·에센스. 시즌당 1회(서버 권위 중복 차단).
3. client: ApiClient Fetch/Claim + ULeaderboardService 시즌 보상 상태 + GameInstance ClaimSeasonReward + Automation 파싱.
4. UI: 리더보드 "시즌 보상" 섹션(완료 시즌 수령 CTA, 현재 시즌 예상) + ko/en CsvIntegrity.
5. 세이브 변경 없음(SaveVersion 15, 수령 영속은 서버 테이블). 기존 리더보드/시즌 불변.
6. 테스트: Automation + vitest(완료/진행중·중복·티어 경계) + parity. CI GREEN + **표준 jumbo 빌드 PM 검증**.

## 2. 작업 분배
| 파트 | 작업 |
| --- | --- |
| backend | seasonReward.ts + claim 테이블/repo + 조회/수령 라우트(서버 권위) + vitest |
| character (메인) | ApiClient Fetch/Claim + Service 상태 + GameInstance ClaimSeasonReward + Automation |
| designer | 시즌 보상 섹션 UI + ko/en |
| balance | 티어 컷·보상 곡선 문서 |
| qa | 완료/진행중·중복 차단·티어 경계 + parity |

## 3. 범위 외
Rebirth/주간 시즌 보상(후속), 코스메틱, 시즌 자동 롤오버 스케줄러, 주간 보스 랭크 보상(#79 후속).

## 4. 워크플로우 v3
표준. 수령은 **서버 권위**(중복/조기 수령 차단). [5] 표준 jumbo 빌드 PM 검증.

## 5. 리스크
시즌 종료 판정(id<현재) / 중복·조기 수령(서버 권위) / 보상 산정 parity / 현재 시즌 id 일관 / 기존 회귀(추가만).
