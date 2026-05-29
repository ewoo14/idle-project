# PR #79 기획서 — 주간 보스 데미지 리더보드 (선행 기획)

> **선행 기획 (Codex 한도 소진 → 6/1 복구 후 디스패치)**. #77 주간 보스 누적 데미지를 #76 리더보드에 주간(ISO week) 랭킹으로 추가. client + server 5-team. 디스패치 시 현행 코드 재검증.
>
> 스펙: [`docs/superpowers/specs/2026-05-29-weekly-boss-leaderboard-design.md`](../../superpowers/specs/2026-05-29-weekly-boss-leaderboard-design.md)
> 계획: [`docs/superpowers/plans/2026-05-29-weekly-boss-leaderboard.md`](../../superpowers/plans/2026-05-29-weekly-boss-leaderboard.md)

## 1. 목표 / DoD
주간 보스(#77) 경쟁 동기 + 리더보드(#76) 연결. "이번 주 보스 데미지 순위".

### DoD
1. backend: `leaderboard_weekly_damage`(week_id/character_id/damage) + upsert/list/rank + `GET /leaderboard/weekly`·`/weekly/me` + save.service 업로드 훅(payload weeklyBossDamage/weeklyBossWeekId 기록) + vitest.
2. client: ApiClient `FetchWeeklyDamageLeaderboard`/`FetchMyWeeklyRank`, ELeaderboardKind WeeklyDamage, GameInstance RefreshLeaderboard(WeeklyDamage, GetCurrentUtcWeekString). Automation 파싱.
3. UI: 리더보드 패널 "주간 보스" 탭(현재 주·top-N·내 순위) + ko/en CsvIntegrity.
4. 세이브 변경 없음(SaveVersion 15 유지). 기존 Power/Rebirth 리더보드 불변.
5. 테스트: Automation + vitest(주간 순위/주 경계) + parity. CI GREEN + **표준 jumbo 빌드 PM 검증**.

## 2. 작업 분배
| 파트 | 작업 |
| --- | --- |
| backend | weekly_damage 테이블/repo/service/route + save.service 훅 + vitest |
| character (메인) | ApiClient/Service 주간 종류 + GameInstance Refresh + Automation |
| designer | 리더보드 "주간 보스" 탭 + ko/en |
| balance | 주간 데미지 랭킹 모델 문서 |
| qa | 제출/조회/내순위/주경계 + parity |

## 3. 범위 외
역대 주간 보관/명예의 전당, 주간 랭크 보상(#78 패턴 후속).

## 4. 워크플로우 v3
표준. [5] **표준 jumbo(unity) 빌드 PM 직접 검증**([[reference-ue-headless-verify]] §1-b).

## 5. 리스크
주 키 정합(ISO week 동일 소스) / 기존 리더보드 회귀(추가만) / 신규 테이블 마이그레이션 / jumbo ODR(동명 헬퍼 grep).
