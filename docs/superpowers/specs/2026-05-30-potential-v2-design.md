# 장비 잠재 V2 (Potential V2) — 설계 문서

> 작성일: 2026-05-30 · 작성: PM/Claude · 분류: 장비 심화(#71 후속) · 무한 성장
> 대상 PR: 단일 슬라이스 · 상태: PM 자율(사용자 "계속 진행")

## 0. 한 줄 요약
장비 잠재(#71)에 **Transcendent 등급(5번째, 4줄)** 과 **신규 옵션 3종(AllStat%/GoldFind%/
DropRate%)** 을 추가하는 심화. [[project_infinite_growth]]("낮은 캡 지양") + [[project_content_richness]].

## 1. 배경
- 잠재(#71): EPotentialGrade Rare~Legendary(4), EPotentialStat 8종(전투 %), Reset/Rank 큐브.
  **메모리 명시 후속**: "잠재 풀 확장(AllStat/Gold/Drop%)".
- 레어도 7단계(#65)·펫 Drop/Gold 보너스(#69) 집계점 존재 → 신규 옵션 적용 지점 재사용 가능.
- 잠재 줄은 기존 영속 필드(FPotentialLine.Stat enum) — enum 값 추가는 기존 세이브 전방호환 → **SaveVer 무변경**.

## 2. 핵심 결정 (PM)
| 항목 | 결정 |
| --- | --- |
| 신규 등급 | `EPotentialGrade::Transcendent = 5`(Legendary 위). **줄 수 4**(Rare1/Epic2/Unique3/Legendary3/Transcendent4). 값 범위 상향. |
| 등급 도달 | Rank 큐브가 Legendary→Transcendent **낮은 확률**로 상승(무한 chase). 아이템 등급(레어도) 연동 상한도 Transcendent까지 허용(고등급 아이템). |
| 신규 옵션 | `EPotentialStat += AllStatPercent(9), GoldFindPercent(10), DropRatePercent(11)`. |
| 적용 지점 | AllStat%→RefreshDerivedStats(전 스탯 배수, **단일 지점** #72 가드) / GoldFind%→AddGold(펫/칭호 골드 옆) / DropRate%→드롭 계산(펫 Drop 집계점 합류). 전투 8종은 기존 경로 유지. |
| 세이브 | **무변경**(enum 값 추가, 기존 줄 영향 없음, 신규 줄만 신규 값). 마이그레이션 불필요. |

## 3. 공식 (초기값 — balance-note 확정)
```
LinesForGrade: Rare1/Epic2/Unique3/Legendary3/Transcendent4
ValueRange(grade, stat): Transcendent = Legendary × ~1.3 상향(스탯별 범위)
신규 옵션 값 범위: AllStat%(소, 전 스탯이라 보수적)/GoldFind%·DropRate%(중)
RankCubeUpgradeChance: ...→Legendary 기존, Legendary→Transcendent 낮음(예 0.05)
적용: 잠재 줄 합산 → 전투 % (기존) + AllStat%(derived 배수) + GoldFind%(골드 배수) + DropRate%(드롭 배수)
```

## 4. 통합 지점 (5-team)
| 파트 | 작업 |
| --- | --- |
| backend | `potential.ts` 확장: Transcendent 등급 줄수/값범위, 신규 옵션 값범위, RankCube Transcendent 확률, 아이템등급→최대잠재등급 매핑. vitest. |
| character | EPotentialGrade Transcendent·EPotentialStat 3종 추가(ItemTypes.h) + PotentialFormula 미러(값범위/줄수/확률) + 잠재 적용 집계(AllStat/Gold/Drop 신규 옵션 각 단일 지점, 전투 기존 유지) + Rank 큐브 Transcendent 상승. **SaveVer 무변경**(enum 확장 전방호환 — 단 기존 세이브 라운드트립 회귀 테스트). Automation(Transcendent 줄4·값범위·신규옵션 집계·Rank 상승·parity·세이브 회귀). |
| designer | 잠재 UI: Transcendent 등급 색/표시 + 신규 옵션 표시명(전체스탯/골드/드롭 %) + ko/en + CsvIntegrity. 표준 jumbo. |
| balance | `docs/planning/potential-v2-balance-note.md`: Transcendent 값범위·신규 옵션 곡선·Rank 확률·파워크리프/median 영향. 세이브 무변경. |
| qa | Transcendent 롤(줄4)·신규 옵션 적용(AllStat/Gold/Drop CP·재화)·Rank 상승·아이템등급 상한·세이브 회귀·parity. jumbo+게이트. |

## 5. 스코프
**In:** Transcendent 등급 + 신규 옵션 3종 + 적용 + Rank 상승 + parity + UI. **세이브 무변경**.
**Out:** 잠재 추가 줄(4→5), 신규 큐브, 잠재 잠금/유지(다음 후속), 신화 등급(레어도 7단계 끝 정렬 후속).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 신규 옵션 이중 적용(#72) | AllStat/Gold/Drop 각 단일 집계 지점(펫/칭호 옆), 전투 기존 경로 불변. |
| 파워크리프 | Transcendent 값 상향 보수적 + Rank Transcendent 확률 낮음, median 재측정. |
| enum 확장 세이브 호환 | 기존 줄 값 0~8 불변, 신규 9~11/등급 5는 신규 롤만 → 전방호환. 기존 세이브 라운드트립 회귀 테스트 필수. |
| parity drift | potential.ts ↔ PotentialFormula 값범위/줄수/확률 1:1(#71 RNG 클라 권위). |
