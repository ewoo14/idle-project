# 장비 잠재 V2 QA 검증 노트 (#71 후속)

> 작성 2026-05-30 · 검증: PM · **SaveVer 22 무변경**(enum 확장 + PotentialLine4 전방호환)

## 1. 자동 검증 게이트

| 게이트 | 범위 | 결과 |
| --- | --- | --- |
| 서버 vitest (`src/core/formulas`) | potential Transcendent/신규옵션 parity | **potential 9/0** (전체 598/0) |
| 서버 lint/build | 전체 | **GREEN** |
| UE jumbo 빌드 | 전체(ODR) | **Succeeded** |
| UE Automation(클라) | Inventory/Item/GameCore(SaveSystem v22 회귀)/Combat | **166/0** |
| UE Automation(UI) | Inventory/Item/Localization(CsvIntegrity)/UI | **75/0** |

## 2. 시나리오 커버리지

- **Transcendent 등급**: 줄 4개 롤, 값범위 Legendary×1.3, 아이템 등급 상한(저레어도 불가/고레어도 허용), Rank 큐브 Legendary→Transcendent 0.05 상승(확률 경계 1/0 결정적).
- **신규 옵션 3종**: AllStatPercent(×0.4)/GoldFindPercent(×1.5)/DropRatePercent(×1.5) 값범위. 적용: AllStat→derived 스탯 배수, Gold→AddGold, Drop→펫 Drop 집계점 합류. 각 단일 지점(전투 8종 불변, 이중 적용 없음 검증).
- **parity**: 클라 PotentialFormula ↔ 서버 potential.ts 값범위/줄수/확률 1:1, 3자리 라운딩(fround 아님), 풀 8→11 셔플.
- **세이브 v22 회귀**: Transcendent 4줄(신규 옵션 포함) 라운드트립 보존, 레거시 3줄 세이브 PotentialLine4 기본값 로드(전방호환). **SaveVer 무변경**.

## 3. 한계 / 후속

- 잠재 5번째 줄·신규 큐브·잠재 잠금/유지·신화 잠재 등급은 후속.
- 수치 초기값(값범위/옵션 배수/Rank 확률) 재튜닝. PR 본문 잠재 패널(Transcendent/신규 옵션) 스크린샷.
