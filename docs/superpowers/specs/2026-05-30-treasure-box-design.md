# 보물 상자 (Treasure Box / 일일 뽑기) — 설계 문서

> 작성일: 2026-05-30 · 작성: PM/Claude · 분류: 리텐션 루프(신규, RNG 보상) · 콘텐츠
> 대상 PR: 단일 슬라이스 · 상태: PM 자율(사용자 "계속")

## 0. 한 줄 요약
하루 1회 **무료 보물 상자**를 열어 **가중 랜덤 보상**(골드/룬정수/소비/보호서/재설정·등급 큐브)을
받는 RNG 리텐션 루프. RNG는 클라 권위(강화 #71/룬 리롤류), 서버는 가중치 parity 미러.

## 1. 배경
- 리텐션: 미션(#91, 과제)·출석(#94, 누적 마일스톤). **랜덤 보상(뽑는 재미)** 형은 부재.
- RNG 보상 메커니즘은 마일스톤/할당형과 다른 도파민 루프([[project_content_richness]]).
- 실존 재화(gold/RuneEssence/Consumable/ProtectionScrolls/ResetCubes/RankCubes) 재사용 — 신규 재화 없음.

## 2. 핵심 결정 (PM)
| 항목 | 결정 |
| --- | --- |
| 뽑기 | **하루 1회 무료**(UTC date `LastTreasureDrawDate` 가드, 출석/미션과 동일). |
| 보상 풀 | 가중 랜덤 N종(실존 재화 + 수량 범위). 가중치로 흔함/희귀 분포. |
| RNG | **클라 권위**(FRandomStream) — 클라가 가중 추첨 + 수량 결정. 서버 `treasureBox.ts`는 가중치/보상표 parity 미러(테스트). |
| 누적 | 누적 뽑기 횟수(`TotalTreasureDraws`) 통계 + (선택) 무한 마일스톤은 후속. V1은 일일 뽑기. |
| 세이브 | **24→25**(LastTreasureDrawDate/TotalTreasureDraws, 누락=빈/0 회귀안전) |

## 3. 데이터/공식 (초기값 — balance-note 확정)
```
TREASURE_POOL: [{ reward, weight, minAmount, maxAmount }]  예:
 gold(weight 40, 10000~50000) / essence(weight 25, 3~10) / consumable(weight 15, 1~2)
 protectionScroll(weight 10, 1~3) / resetCube(weight 7, 1~2) / rankCube(weight 3, 1)
DrawTreasure(rng): 가중 추첨 → reward 결정 → RandRange(min,max) 수량 → 지급
일일: date != LastTreasureDrawDate 면 뽑기 가능, 뽑은 뒤 LastTreasureDrawDate=date, TotalTreasureDraws++
```

## 4. 통합 지점 (5-team)
| 파트 | 작업 |
| --- | --- |
| backend | `treasureBox.ts`(미러): `TREASURE_POOL`(reward/weight/min/max), `getTotalWeight()`, `pickTreasureReward(roll)`(누적 가중 인덱스 — 결정적 함수). vitest(가중 분포·경계·수량 범위). |
| character | `TreasureBoxService`(LastDrawDate/TotalDraws, `CanDrawToday(date)`, `FTreasureReward DrawTreasure(date, FRandomStream&)`(가중 추첨+수량, RNG 클라), 보상표 미러) + GameInstance(보상 지급 단일·UTC date 연동·getter lazy-ensure). SaveVer **24→25**. Automation(가중 추첨 결정적[roll 주입]·1일1회·수량 범위·보상 지급·세이브 v25·parity). |
| designer | 보물 상자 패널: 오늘 뽑기 가능 상태·뽑기 버튼·결과 표시(보상 종류/수량)·누적 횟수 + ko/en + CsvIntegrity. 표준 jumbo. |
| balance | `docs/planning/treasure-box-balance-note.md`: 가중치/수량 곡선·기대값·경제 영향. SaveVer25. |
| qa | 뽑기(1일1회·중복 거부)·가중 분포·수량·보상 지급(실존 재화)·세이브 v25·parity. jumbo+게이트. |

## 5. 스코프
**In:** 일일 무료 뽑기 + 가중 랜덤 보상(실존 재화) + UI + parity + SaveVer 25.
**Out:** 유료/광고 뽑기, 프리미엄 재화, 누적 뽑기 마일스톤(후속), 천장/pity(후속).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| RNG parity drift | 클라 권위 RNG + 서버 가중치/`pickTreasureReward(roll)` **결정적 함수** parity 테스트(#71 패턴). |
| 뽑기 중복(치트) | `LastTreasureDrawDate` UTC date 1일1회 가드. |
| 보상 인플레 | 가중치(흔한=소액·희귀=고액) + 일일 1회 제한. 기대값 일일 던전(#68) 수준 보충. |
| 세이브 마이그레이션 | 누락=빈/0(회귀안전), SaveVer<25 가드. |
