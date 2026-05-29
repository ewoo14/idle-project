# PR #74 기획서 — 마스터리 트랙별 로컬 보너스 (V1.5)

> **PM 자율 진행**. PR #72 통합 마스터리 후속(V1.5, 계획 Task 12). 마스터리 각 트랙이 *전역* 월드 파워뿐 아니라 **자기 시스템을 직접 심화**하는 로컬 보너스를 갖는다. 무한 성장([[project-infinite-growth]]). client + server 5-team.
>
> 스펙: [`docs/superpowers/specs/2026-05-29-mastery-local-bonuses-design.md`](../../superpowers/specs/2026-05-29-mastery-local-bonuses-design.md)
> 계획(TDD): [`docs/superpowers/plans/2026-05-29-mastery-local-bonuses.md`](../../superpowers/plans/2026-05-29-mastery-local-bonuses.md)

## 1. 목표 / DoD
"그 시스템을 쓸수록 그 시스템이 강해지는" 마스터리 설계 완성. 전역 월드 파워(#72)와 이중 계산 없이 독립.

### DoD 검증
1. **6 트랙 로컬 보너스**: 전투→처치 보상+%/장비→강화 골드 비용 절감(클램프 0.50)/심연→던전 보상+%/룬→룬 코어 가산/야성→펫 보너스+%/탐험→퀘스트 보상+%. 무한 체감 `k·ln(1+level)`.
2. **중앙화**: `FMasteryFormula::GetLocalBonus(track, level)` ↔ 서버 `localBonus` parity(Math.fround). `UMasteryService::GetLocalBonus(track)` 노출.
3. **단일 적용**: 각 보너스 1회 적용 지점(#72 EXP 이중적용/#73 교훈 — getter 중복 금지). 전역(#72)과 다른 효과·다른 지점.
4. **세이브 변경 없음**(트랙 XP #72 기존, SaveVersion 14 유지).
5. **UI**: 숙련 패널 트랙별 현재 로컬 보너스 표시 + ko/en.
6. **테스트**: 클라 Automation(단조·클램프·각 시스템 적용·회귀·전역 독립) + 서버 vitest + parity. UE Build/Automation + server-ci GREEN.

## 2. 범위 (In Scope)
2.1 공식(character+backend) — GetLocalBonus/localBonus 미러.
2.2 적용(character) — 6 소비 지점(처치보상/강화비용/던전보상/룬코어/펫보너스/퀘스트보상) 단일 적용.
2.3 UI(designer) — 숙련 패널 로컬 보너스 표시 + ko/en.
2.4 밸런스(balance) — 계수·클램프·무한 비폭주 문서.
2.5 테스트(qa) — DoD 6.

## 3. 범위 외 (후속)
- 트랙당 2종+ 로컬 보너스, 강화 성공률 보정(RNG·parity 회피 위해 V1.5는 비용 절감 채택), 던전 입장 횟수+, 로컬 보너스 연출.

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 |
| --- | --- |
| character (메인) | FMasteryFormula::GetLocalBonus + UMasteryService::GetLocalBonus, 6 소비 지점 적용, 클라 Automation |
| backend | mastery.ts localBonus 미러 + vitest parity |
| designer | 숙련 패널 로컬 보너스 표시 + ko/en + CsvIntegrity |
| balance | 계수·클램프·무한 비폭주 + balance-sim 무변동 |
| qa | 단조/클램프/각 시스템/회귀/전역 독립 + parity |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex 5-team → PM 게시 → [3] Claude TM 종합+fix → [4] Codex TM(결함 시) → [5] 검증(UE Automation 직접) → [N] CI 그린 + PM 종합 + 머지. PM 자율([[feedback-autonomous-slices]], [[feedback-ci-before-merge]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 6 시스템 적용 표면 | **중앙화 GetLocalBonus 단일 함수**(+미러), 소비 6개는 단순 가산/곱 1회 |
| 강화 절감 폭주 | `min(0.50,…)` 클램프 + balance |
| 전역(#72)과 이중 | 다른 효과·다른 지점·단일 적용 |
| 무한 폭주 | 로그 체감 곡선 + balance 문서 |
| 서버↔클라 drift | localBonus Math.fround 미러 + parity |

## 7. 후속
트랙당 2종+, 강화 성공률 보정, 던전 입장+, 연출.
