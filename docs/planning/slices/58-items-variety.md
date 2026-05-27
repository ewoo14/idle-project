# PR #58 기획서 — 아이템 종류 확대 (콘텐츠 볼륨)

> **사용자 지시([[project-content-richness]]): 아이템 종류를 많이.** 현재 장비 다양성 부족 — 베이스 아이템 이름이 generic("{희귀도} {슬롯}", 모든 무기가 "Legendary Weapon"), 세트 3종(Warrior/Guardian/Arcane), affix 3종(CritRate/AtkSpeed/MagicAtk)뿐. **명명된 베이스 아이템 카탈로그(슬롯별 다양한 이름) + 세트 확대 + affix 종류 확대**로 수집·빌드 다양성 강화. 클라/서버 미러(parity). server + client 멀티시스템(+designer/qa).

## 1. 목표 / DoD
드롭 아이템이 슬롯별 다양한 고유 이름(검/대검/지팡이/활/단검/방패…)을 가지고, 세트·affix 종류가 풍부해 수집·빌드 다양성이 커진다. 기존 강화(#44)/등급(#45)/세트(#43)/affix(#40) 시스템과 정합.

### DoD 검증
1. **명명된 베이스 아이템 카탈로그(많고 다양하게)**: 슬롯별 베이스 아이템 다수(예 Weapon: 롱소드/대검/마법봉/장궁/단검/너클…, Helmet: 투구/모자/서클릿…, 각 슬롯 3~6종). 드롭 시 레벨/희귀도 + 베이스 선택(결정적 RNG) → DisplayName = 희귀도 접두 + 베이스명(예 "전설 대검"). ItemId 에 베이스 반영. 슬롯별 베이스가 스탯 편향(예 대검=물공↑/지팡이=마공↑) 가지면 더 다양(선택).
2. **세트 확대**: `EItemSet` 3 → **+3~4**(예 Assassin/Hunter/Holy/Berserker 테마, 8직업 플레이스타일 연계). 각 2/4세트 보너스(SetBonusFormula + 서버 setBonus.ts 미러). 드롭 시 세트 부여(희귀도 비례, #43 유지).
3. **affix 종류 확대**: `EAffixKind` 3(CritRate/AtkSpeed/MagicAtk) → **+PhysDef/MagicDef/Hp/CritDmg** 등. RollAffixes 풀 확대 + ComputeEquipmentBonus 전파 + PowerScore 가중. 희귀도별 affix 개수 기존.
4. **클라/서버 미러**: FDropFormula(베이스 카탈로그/affix/세트 롤) ↔ 서버 drop.ts, SetBonusFormula ↔ setBonus.ts, 전부 parity(Math.fround). 베이스 카탈로그 정의 클라↔서버 동기.
5. **HUD**: 아이템 표시에 베이스명/세트/affix 반영(기존 affix HUD #40·세트 HUD #43 재사용 + 베이스명). 로컬라이즈 ko/en(베이스 아이템명/세트명/affix명).
6. **테스트**: 클라 Automation(베이스 카탈로그 드롭·세트 부여/보너스·affix 풀·PowerScore·결정적 RNG) + 서버 vitest(drop/setBonus parity·신규 세트/affix) + CsvIntegrity. UE 빌드/Automation + 서버 build/test/lint GREEN.

## 2. 범위 (In Scope)
### 2.1 베이스 아이템 카탈로그 (character + backend 미러)
- 슬롯별 베이스 아이템 테이블(ID/표시명키/슬롯/스탯편향). FItemFactory/FDropFormula 가 베이스 선택→DisplayName/ItemId. 서버 drop.ts 동기.
### 2.2 세트 확대 (character + backend)
- EItemSet +3~4 + SetBonusFormula 2/4세트 보너스 + 서버 setBonus.ts 미러 + RollSetForRarity 확대.
### 2.3 affix 확대 (character + backend)
- EAffixKind +4(PhysDef/MagicDef/Hp/CritDmg) + RollAffixes 풀 + FItemInstance 보너스 필드(있으면 재사용, 없으면 추가) + ComputeEquipmentBonus 전파 + 서버 drop.ts.
### 2.4 UI (디자이너)
- 베이스명/세트명/affix명 로컬라이즈 ko/en + HUD 표시 정합(기존 패널 재사용).
### 2.5 테스트 — 위 DoD 6.

## 3. 범위 외 (후속)
- 소비 아이템/재료/제작, 아이템 도감 상세 UI, 직업 전용 장비, 룬/소켓 시스템, 아이템 재련(리롤).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | 베이스 카탈로그 + EItemSet+ + EAffixKind+ + FDropFormula/ItemFactory/SetBonusFormula + ComputeEquipmentBonus + Automation | ✅ 메인 (`character`) |
| 백엔드 | 서버 drop.ts/setBonus.ts 미러(베이스/세트/affix) + parity vitest | ✅ 보조 (`backend`) |
| 디자이너 | 베이스명/세트명/affix명 로컬라이즈 ko/en + HUD 표시 정합 | ✅ 보조 (`designer`) |
| QA | 베이스 다양성·세트 보너스·affix·PowerScore·드롭 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/designer/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증(UE+서버) → [N] **CI 그린 확정**(server-ci 포함) + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). [[project-content-richness]].

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 클라/서버 드롭/세트/affix 불일치 | drop.ts/setBonus.ts parity(Math.fround) + 베이스 카탈로그 정의 동기 + DefinitionParity |
| 베이스 스탯 편향 밸런스 | 편향 소폭(또는 표시명만) + balance 영향 점검, PowerScore 정합 |
| 기존 아이템/세트/affix 회귀 | 기존 3세트/3affix/드롭 동작 보존, 신규만 추가, Common/None 회귀안전 |
| 결정적 RNG 깨짐(베이스 선택) | RandomStream 시드 순서 보존, 라운드트립/결정성 Automation |
| 로컬라이즈 영문 잔존 | 베이스/세트/affix명 ko/en + CsvIntegrity |
| 저장/클라우드(#53/#54) 영향 | FItemInstance 필드 추가 시 직렬화/clientSave 정합, 라운드트립 |

## 7. 후속 (콘텐츠 확장 계속)
- 소비/재료/제작 아이템, 아이템 도감 UI, 직업 전용 장비, 룬/소켓, 재련/리롤. (사용자 4부작 지시 — 업적#55/퀘스트#56/직업#57/아이템#58 완료 후 추가 콘텐츠.)
