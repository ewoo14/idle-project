# 칭호(Title) 시스템 — 설계 문서

> 작성일: 2026-05-30 · 작성: PM/Claude · 분류: 신규 메타 레이어(업적 #55 연동) · 콘텐츠 풍부화
> 대상 PR: 단일 슬라이스 · 상태: PM 자율(사용자 "PM 판단 후 진행, 질문 없음")

## 0. 한 줄 요약
업적 메트릭 달성으로 **칭호**를 해금하고 **1개 장착** 시 영구 패시브 보너스를 받는 신규 메타.
플레이 성과(처치/환생/탑/골드 등)에 정체성·보상을 부여([[project_content_richness]]).

## 1. 배경
- 업적(#55) `EAchievementMetric` 22종(MonstersKilled/BossesKilled/RebirthCount/TranscendCount/
  TowerHighestFloor/GearEnhanced/GoldEarned/HighestPetLevel/QuestsCompleted/DaysPlayed 등) + `GetMetricValue` 존재 → 칭호 해금 조건의 자연스러운 소스(신규 추적 불필요).
- 메타 보상이 수치 위주 → "칭호"라는 표시+보너스 결합으로 성취감/수집 동기.

## 2. 핵심 결정 (PM)
| 항목 | 결정 |
| --- | --- |
| 칭호 정의 | 데이터 구동 `FTitleDefinition`(TitleId, Name, 해금 metric+threshold, 보너스 type+value). 16~20종, 카테고리(전투/진행/수집/메타). |
| 해금 | 업적 메트릭(`GetMetricValue(metric) >= threshold`) 충족 시 자동 해금(영구). `RecomputeUnlockedTitles`(업적 RecordMetric 후크 또는 세이브/로그인 시). |
| 장착 | **1개만 장착**(EquippedTitleId), 보너스 적용. 미장착=보너스 없음. |
| 보너스 | 단일 차원/칭호: `ETitleBonus`(AllStatPct/GoldPct/ExpPct/CritDmgPct 등) — 기존 단일 적용 지점(RefreshDerivedStats/AddGold/AddExp/크리)에서 합산, **이중 적용 금지**(#72). |
| 세이브 | **20→21**(UnlockedTitleIds Set + EquippedTitleId, 누락=빈/없음 회귀안전) |

## 3. 공식/데이터 (초기값 — balance-note 확정)
```
해금: GetMetricValue(def.Metric) >= def.Threshold  → UnlockedTitleIds.Add(id)
장착 보너스: ApplyTitleBonus(ETitleBonus, value)  — 장착 칭호 1개만
예시 칭호:
 "몬스터 사냥꾼"   MonstersKilled>=10000  → AllStat +3%
 "보스 슬레이어"   BossesKilled>=500      → CritDmg +10%
 "환생의 대가"     RebirthCount>=10       → AllStat +5%
 "탑의 정복자"     TowerHighestFloor>=100 → Gold +15%
 "황금왕"          GoldEarned>=1e9        → Gold +20%
 "초월자"          TranscendCount>=5      → AllStat +8%
 ... (전투/진행/수집/메타 16~20종, 임계·보너스 곡선)
```

## 4. 통합 지점 (5-team)
| 파트 | 작업 |
| --- | --- |
| backend | `title.ts`(서버 미러): 칭호 카탈로그(id/metric/threshold/bonus) + `getUnlockedTitles(metricValues)` + `getTitleBonus(titleId)`. vitest(해금 경계·보너스 매핑). (해금 판정 클라 권위, 서버 parity 미러.) |
| character | `TitleService`(정의·UnlockedTitleIds·EquippedTitleId·RecomputeUnlocked(AchievementService 메트릭)·EquipTitle·GetEquippedTitleBonus) + 보너스 적용(RefreshDerivedStats/AddGold/AddExp/크리 단일 지점). SaveVer **20→21**. GameInstance 배선(업적 메트릭 갱신 시 재계산, 장착 시 RefreshDerivedStats). Automation(해금 경계·장착 보너스·세이브 v21·parity). |
| designer | 칭호 패널: 해금/미해금 목록(조건·보너스), 장착/해제, 현재 장착 칭호 표시 + ko/en + CsvIntegrity. 표준 jumbo. |
| balance | `docs/planning/title-system-balance-note.md`: 칭호 목록·임계·보너스 곡선·파워크리프/median 영향. |
| qa | 해금(경계)·장착(1개·보너스 CP 반영)·해제·세이브 v21·parity. jumbo+게이트. |

## 5. 스코프
**In:** 칭호 16~20종(업적 메트릭 해금) + 1개 장착 패시브 보너스 + UI + parity + SaveVer 21.
**Out:** 칭호별 RNG, 칭호 합성/강화, 복수 장착(슬롯 확장은 후속), 칭호 전용 신규 메트릭.

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 보너스 이중 적용(#72) | 칭호 보너스 타입별 단일 적용 지점, 장착 1개만. |
| 파워크리프 | 보너스 보수적(+3~20%), 장착 1개 제한, median 재측정. |
| 해금 판정 drift | GetMetricValue 기준 클라 권위 + 서버 title.ts parity(임계·보너스 매핑). |
| 세이브 마이그레이션 | UnlockedTitleIds/EquippedTitleId 누락=빈(회귀안전), SaveVer<21 가드. |
