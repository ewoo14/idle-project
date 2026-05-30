# 환생 특성 (Rebirth Perks) — 설계 문서

> 작성일: 2026-05-30 · 작성: PM/Claude · 분류: 프레스티지 심화(환생 #19 후속) · 무한 성장
> 대상 PR: 단일 슬라이스 · 상태: PM 자율(사용자 "계속")

## 0. 한 줄 요약
환생 횟수만큼 얻는 **환생 특성 포인트**를 6종 특성(골드/드롭/크리뎀/전체스탯/경험치/오프라인 %)에
**자유 분배**하는 프레스티지 빌드 심화. 무한 성장(환생 누적), 기존 환생 포인트(HP/공격) 비파괴.

## 1. 배경
- 환생(#19): `RebirthCount` + `RebirthBonusPoints`(점당 고정 +10 HP/+2 공격, **자동 적용·분배 없음**).
- 프레스티지에 **빌드 선택**(자유 분배) 부재 → 특성 트리로 깊이·재플레이성 부여([[project_content_richness]]).
- 무한 성장([[project_infinite_growth]]): 환생할수록 특성 포인트 누적 → 무한 분배.

## 2. 핵심 결정 (PM)
| 항목 | 결정 |
| --- | --- |
| 포인트 출처 | **신규** 환생 특성 포인트 = `GetTotalRebirthPerkPoints(rebirthCount)`(예 rebirthCount, 1/환생). 기존 RebirthBonusPoints(HP/공격) **불변**(비파괴). |
| 특성 | 6종 `ERebirthPerk`: GoldPct/DropPct/CritDmgPct/AllStatPct/ExpPct/OfflineEffPct. 각 레벨당 % 보너스(무한). |
| 분배 | `Allocations: TMap<ERebirthPerk,int32>`(레벨), Σ ≤ total. **자유 재분배(리셋 무료)**. |
| 적용 | 각 특성 단일 지점(Gold→AddGold·Drop→드롭·CritDmg→크리뎀·AllStat→derived·Exp→AddExp·Offline→오프라인). 기존 마스터리/펫/칭호/잠재 옆 합산(#72 가드). |
| 세이브 | **23→24**(Allocations 맵, 누락=빈 회귀안전) |

## 3. 공식 (초기값 — balance-note 확정)
```
GetTotalRebirthPerkPoints(rebirthCount) = rebirthCount          # 1/환생 (무한)
PerkStep: GoldPct 0.02 / DropPct 0.02 / CritDmgPct 0.03 / AllStatPct 0.01 / ExpPct 0.02 / OfflineEffPct 0.03 (레벨당)
PerkBonus(perk, level) = PerkStep[perk] * level                # 선형 무한
SpentPoints = Σ Allocations[perk] ; Available = Total - Spent (>=0)
AllocatePerk(perk): Available>0 면 Allocations[perk]++
DeallocatePerk(perk) / ResetPerks(): 무료 재분배
```

## 4. 통합 지점 (5-team)
| 파트 | 작업 |
| --- | --- |
| backend | `rebirthPerk.ts`(미러): `getTotalRebirthPerkPoints(rebirthCount)`, `getPerkBonus(perk, level)`(PerkStep 상수). vitest. |
| character | `RebirthPerkTypes`(ERebirthPerk) + `RebirthPerkService`(Allocations·Total(rebirthCount 주입)·Available·Allocate/Deallocate/Reset·GetPerkLevel·GetPerkBonus 미러) + 적용(6종 각 단일 지점) + GameInstance(rebirthCount 연동·진입점). SaveVer **23→24**. Automation(분배/한도/리셋·보너스 적용·세이브 v24·parity). |
| designer | 환생 특성 패널: 가용/총 포인트·6종 특성(레벨·보너스·+/− 버튼)·리셋 + ko/en + CsvIntegrity. 표준 jumbo. |
| balance | `docs/planning/rebirth-perks-balance-note.md`: PerkStep 곡선·포인트 출처·파워크리프/median. SaveVer24. |
| qa | 분배(가용 한도·중복)·리셋·보너스 적용(6종 CP/재화)·세이브 v24·parity. jumbo+게이트. |

## 5. 스코프
**In:** 환생 특성 포인트(rebirthCount) + 6종 특성 자유 분배 + 적용 + 리셋 + UI + parity + SaveVer 24.
**Out:** 특성 노드 트리(선후행), 특성 RNG, 기존 RebirthBonusPoints 재설계(비파괴 유지), 초월 특성(후속).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 기존 환생 포인트 회귀 | RebirthBonusPoints(HP/공격) **불변**(비파괴), 신규 풀 별도. |
| 보너스 이중 적용(#72) | 6종 각 단일 집계 지점(마스터리/펫/칭호/잠재 옆), 장착/소비처 1곳. |
| 분배 무결성 | Available = Total - Spent ≥ 0, Allocate는 Available>0만. 리셋=재분배. |
| 세이브 마이그레이션 | Allocations 누락=빈(회귀안전), SaveVer<24 가드. |
| parity | rebirthPerk.ts ↔ RebirthPerkService(PerkStep/Total) 1:1. |
