# PR #76 기획서 — 리더보드 클라이언트 연동 + 랭킹 UI

> **PM 자율 진행**. 게임 **최초의 소셜/경쟁 기능**. 서버 `leaderboard` 모듈은 이미 가동(세이브 업로드 시 전투력/환생 점수 시즌별 기록 + PG/Redis + `/power`·`/rebirth` 조회)되나 클라이언트가 전혀 표시하지 않음 — 클라 연동 + 랭킹 UI + "내 순위"를 추가해 휴면 벡터를 활성화. client + server 5-team.
>
> 스펙: [`docs/superpowers/specs/2026-05-29-leaderboard-client-design.md`](../../superpowers/specs/2026-05-29-leaderboard-client-design.md)
> 계획(TDD): [`docs/superpowers/plans/2026-05-29-leaderboard-client.md`](../../superpowers/plans/2026-05-29-leaderboard-client.md)

## 1. 목표 / DoD
경쟁/사회적 동기 부여 공백 해소. 백엔드 기존 가동을 클라에 표시 + 내 순위 신설.

### DoD 검증
1. **조회**: 클라 ApiClient가 `GET /leaderboard/power|rebirth?season=`(top-N) + 신설 `/power/me`·`/rebirth/me`(내 순위) 호출. graceful 오프라인 폴백(#54 패턴).
2. **내 순위(신설)**: backend repo `getPowerRank`/`getRebirthRank`(PG rank window, 미등록 0) + service `getMyRank` + route + vitest.
3. **표시**: 리더보드 패널 Power/Rebirth 탭 + top-N 리스트(순위·점수) + 내 순위 하이라이트 + 시즌 라벨 + 로딩/오프라인 상태. ko/en + CsvIntegrity.
4. **모델**: `ULeaderboardService` JSON→`FLeaderboardEntry{CharacterId,Score,Rank}` 파싱/보관(Automation 서버 무의존 검증). 시즌 id=SeasonService 현재.
5. **세이브 변경 없음**(SaveVersion 14 유지, 읽기 전용 표시). 기존 leaderboard 제출·조회 불변(추가만).
6. **테스트**: 클라 Automation(파싱/내순위/오프라인) + 서버 vitest(내순위 정확/미등록/시즌) + parity. UE Build/Automation + server-ci GREEN.

## 2. 범위 (In Scope)
2.1 backend — 내 순위 repo/service/route + 테스트.
2.2 character — ApiClient Fetch + ULeaderboardService + GameInstance Refresh + Automation.
2.3 designer — 리더보드 패널(탭/리스트/내순위/시즌) + ko/en.
2.4 balance — 랭킹/시즌 모델 문서(공식 무변경).
2.5 qa — DoD 6.

## 3. 범위 외 (후속)
- 시즌 경쟁 보상(rank-tier claim), 길드/친구, 실시간 갱신/푸시, top-100 초과 페이지네이션, 점수 위변조 검증 강화.

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 |
| --- | --- |
| backend | getPowerRank/getRebirthRank + getMyRank + /me 라우트/스키마 + vitest |
| character (메인) | ApiClient FetchLeaderboard/FetchMyRank, ULeaderboardService(파싱/보관), GameInstance RefreshLeaderboard, LeaderboardTypes, Automation |
| designer | 리더보드 패널 Power/Rebirth 탭·top-N·내순위·시즌 + ko/en + CsvIntegrity |
| balance | 랭킹(Power=CP 파생/Rebirth=count)·시즌 모델 문서 + 시즌 보상 후속 메모 |
| qa | e2e/내순위 정확·미등록/오프라인/시즌 경계 + 내순위 parity |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex 5-team → PM 게시 → [3] Claude TM 종합+fix → [4] Codex(결함 시) → [5] 검증 → [N] CI 그린 + PM 종합 + 머지. PM 자율([[feedback-autonomous-slices]], [[feedback-ci-before-merge]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 라이브 서버 의존(클라 테스트) | Automation JSON 파싱 단위(서버 무의존) + 네트워크 graceful 폴백(#54) |
| 내 순위 쿼리 정확성 | PG rank() window + 등록/미등록/동점 vitest |
| 시즌 id 불일치 | SeasonService 현재 시즌 id 일관 사용 |
| 기존 leaderboard 회귀 | 제출·top-N 불변(추가만) + 기존 테스트 유지 |

## 7. 후속
시즌 경쟁 보상, 길드/친구, 실시간 갱신, 페이지네이션, 점수 검증 강화.
