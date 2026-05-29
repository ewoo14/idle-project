# 시즌 경쟁 보상 — 설계 문서 (선행 기획, PR #78 예정)

> 작성일: 2026-05-29 · 대상: PR #78(Codex 복구 6/1 후) · PM 자율
> 상태: **구현 보류(Codex 한도)**, 기획만 선행. 디스패치 시 현행 코드 재검증.

## 0. 요약
#76 리더보드(Power/Rebirth)에 **시즌 종료 시 순위 보상**을 추가해 경쟁에 실질 동기를 부여한다. **완료된 시즌(id < 현재)** 의 최종 순위로 보상 티어를 산정, 클라가 수령.

## 1. 배경 / 핵심 난제
- #76 리더보드는 조회만 — 경쟁할 이유(보상)가 없음.
- 시즌(#22 SeasonService): `SeasonId` 보유. **시즌 종료 판정**이 핵심 난제 → 결정: **현재 시즌 id보다 작은 시즌 = 완료**로 간주, 완료 시즌 순위는 확정(불변)이므로 보상 수령 가능.

## 2. 핵심 결정
| 항목 | 결정 |
| --- | --- |
| 보상 기준 | **완료 시즌(id<현재)** 의 최종 순위(Power 기준 V1) |
| 티어 | 1위 / top10 / top100 / 참여(랭크>0) — 골드·에센스·(추후 코스메틱) |
| 수령 | 캐릭터별 **시즌당 1회**(서버 권위 중복 방지) |
| 영속 | 서버: `season_reward_claim(character_id, season_id)` 테이블(클라 세이브 아님 → 권위) |
| 현재 시즌 진행 중 | 보상 불가(미완료) — UI는 "예상 순위/보상" 미리보기만 |

## 3. 통합 지점
| 파트 | 작업 |
| --- | --- |
| backend | `seasonReward.ts`(rank→tier→reward 산정, 클라 미러) + `season_reward_claim` 테이블/마이그레이션 + repo(claimed 조회/기록) + 라우트 `GET /leaderboard/season-reward?season=&characterId=`(수령 가능 여부+티어+보상) ·`POST /leaderboard/season-reward/claim`(season<현재·미수령·rank로 보상 산정·기록, 서버 권위) + vitest |
| character | ApiClient `FetchSeasonReward`/`ClaimSeasonReward`, ULeaderboardService 시즌 보상 상태 보관, GameInstance `ClaimSeasonReward`(성공 시 AddGold/RuneEssence — **단, 서버 권위 지급이면 클라는 표시만**; V1은 서버 응답 보상값을 클라 재화에 반영하되 권위는 서버). Automation 파싱 |
| designer | 리더보드 패널 "시즌 보상" 섹션(완료 시즌 보상 수령 CTA, 현재 시즌 예상) + ko/en |
| balance | 티어 컷(1/10/100)·보상 곡선(시즌 길이·참여 기준) 문서 |
| qa | 완료/진행중 구분, 중복 수령 차단(서버), 티어 경계(rank 1/10/11/100/101), parity |

## 4. 스코프
**In:** Power 시즌 보상(완료 시즌·티어·수령 1회·서버 권위) + UI + 마이그레이션 + parity. **Out:** Rebirth 시즌 보상(후속 동일 패턴), 코스메틱 보상, 시즌 자동 롤오버 스케줄러(시즌 id 관리는 기존 #22 의존), 주간 보스 랭크 보상(#79 후속).

## 5. 리스크
| 리스크 | 완화 |
| --- | --- |
| 시즌 종료 판정 모호 | "id<현재=완료" 단순 규칙 + 진행중 시즌 수령 거부(서버) |
| 중복/조기 수령(치트) | **서버 권위**(season_reward_claim 테이블 + season<현재 + rank 서버 조회) |
| 보상 산정 클라↔서버 drift | seasonReward.ts 미러 + 티어 경계 parity |
| 현재 시즌 id 소스 불일치 | SeasonService 현재 시즌 id 일관 |
| 기존 리더보드/시즌 회귀 | 신규 테이블·엔드포인트만(추가) |

## 6. 워크플로우 v3 / 후속
표준 v3 + [5] 표준 jumbo 빌드 PM 검증. 후속: Rebirth/주간 시즌 보상, 코스메틱, 명예의 전당.
