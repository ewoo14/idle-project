# 보물 상자(일일 뽑기) QA 검증 노트

> 작성 2026-05-30 · 검증: PM · SaveVer 24→25

## 1. 자동 검증 게이트

| 게이트 | 범위 | 결과 |
| --- | --- | --- |
| 서버 vitest (`src/core/formulas`) | treasureBox 가중/보상 parity | **treasureBox 8/0** (전체 625/0) |
| 서버 lint/build | 전체 | **GREEN** |
| UE jumbo 빌드 | 전체(ODR) | **Succeeded** |
| UE Automation(클라) | TreasureBox(가중/1일1회/보상)/GameCore(SaveSystem v25) | **99/0** |
| UE Automation(UI) | TreasureBox/Localization(CsvIntegrity)/UI/Combat | **76/0** |

## 2. 시나리오 커버리지

- **뽑기**: 하루 1회(UTC date), 중복 거부(빈 보상), 날짜 변경 시 재가능, TotalDraws++.
- **가중 추첨**: PickReward 경계(roll 0/39/40/64/65/79/80/89/90/96/97/99 → 정확 reward), 음수/초과 클램프. RNG 클라 권위(FRandomStream 시드 결정적 테스트).
- **수량**: RandRange(min,max) 범위. **보상 지급**: 실존 재화 6종(gold/essence/consumable/protectionScroll/resetCube/rankCube) 각 반영, 단일 지급.
- **parity**: 클라 TreasureBoxService 풀 ↔ 서버 treasureBox.ts(weight/min/max/pickReward) 1:1.
- **세이브 v25**: LastTreasureDrawDate/TotalTreasureDraws 라운드트립, SaveVer<25=빈/0(회귀안전).

## 3. PM 적발·수정 (#96 잠복 회귀)

- 본 슬라이스의 UI 게이트(`IdleProject.UI` 포함)가 **#96 스킬 확장의 잠복 회귀**를 포착: `SkillDisplayModel`(CombatTests:1402)이 Warrior HUD 액티브 슬롯 4를 기대했으나, #96이 신규 액티브 스킬(earthen_cleave)을 추가해 5가 됨. HUD가 신규 액티브를 정상 표시하므로 **테스트 기대값 4→5 교정**(#96이 parity 카운트 56→64는 갱신했으나 UI.HUD 카운트는 게이트 필터 미포함으로 누락).
- **교훈**: 스킬 추가 슬라이스는 게이트 필터에 `IdleProject.UI.HUD` 포함 필요.

## 4. 후속/비고

- getter lazy-ensure(#91). 누적 뽑기 마일스톤·천장(pity)·유료 뽑기는 후속. 가중치/수량 재튜닝. PR 본문 보물 상자 패널 스크린샷.
