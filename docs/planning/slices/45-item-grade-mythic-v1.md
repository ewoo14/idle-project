# PR #45 기획서 — 아이템 등급 확장 Mythic (무한 성장)

> 사용자 지시([[project-infinite-growth]]): "아이템 등급" 확장. 현재 등급은 Common~Legendary(5종)로 상한이 낮다. **Mythic 등급**을 추가(UIThemeTokens 에 RarityMythic 색 이미 존재)해 loot 상한을 끌어올린다. 등급에 의존하는 전 시스템(드롭 스탯 배수·확률·affix·세트, 강화 비용 배수, HUD, 서버 미러)을 Mythic 까지 확장. 아이템 파워는 스테이지 레벨×등급 배수(#36)·50강(#44)으로 무한 스케일, Mythic 은 그 위의 최상위 명명 등급. client+server+UI+balance+qa 5-team.

## 1. 목표 / DoD
드롭/상점 장비에 Mythic 등급이 추가돼 더 강한 스탯·affix·세트·강화를 가지며, 고스테이지에서 매우 드물게 등장한다.

### DoD 검증
1. EItemRarity 에 Mythic 추가(기존 None~Legendary 보존). 전 참조 사이트(HUD 색/이름, factory, PowerScore, 드롭, 강화) Mythic 처리.
2. 스탯 배수: GetRarityStatMultiplier(Mythic) > Legendary(3.2) — 예 4.5.
3. 드롭 확률: 고레벨에서 Mythic 극희귀 등장(Legendary 보다 낮게). 저레벨 Mythic 0.
4. 강화 비용 배수(#39): RarityCostMultiplier(Mythic) > Legendary(16) — 예 32. affix 개수(Mythic) ≥ Legendary. 세트 부여 Mythic 포함.
5. HUD 가 Mythic 색(RarityMythicStart)·이름 표시. 서버 미러(drop/enhance/equipment) + parity. 서버 Vitest + UE 빌드/Automation GREEN. 기존 등급(Common~Legendary) 회귀 없음.

## 2. 범위 (In Scope)
### 2.1 등급 확장 (메인, C++)
- `EItemRarity`(ItemTypes.h): ... Legendary=5 + **Mythic=6**(UMETA DisplayName). 전 switch/참조(RarityToString, RarityToColor(#36), GetRarityAdjective, EItemRarity 비교, 테스트) Mythic 처리.
### 2.2 등급 의존 공식 (메인, C++)
- `FDropFormula::GetRarityStatMultiplier`: Mythic 4.5(Legendary 3.2 위).
- `FDropFormula::RollRarityForLevel`: Mythic 확률(LevelScale 비례, Legendary 보다 낮게 예 0.005×scale). Common 잔여 재계산.
- `FDropFormula::RollAffixes`: Mythic affix 개수(3, Legendary 이상).
- `FDropFormula::RollItemSet`: Mythic 세트 부여(고희귀도 취급).
- `FEnhanceFormula::GetRarityCostMultiplier`(#39): Mythic 32(Legendary 16 위).
### 2.3 UI (디자이너)
- RarityToColor(Mythic → Theme::RarityMythicStart), 희귀도 이름(Mythic) ko/en. 장착/드롭/affix/세트 표시에 Mythic 반영.
### 2.4 서버 (백엔드)
- drop.ts(ItemRarity "Mythic" + getRarityStatMultiplier/rollRarityForLevel/affix/set), enhance.ts(getRarityCostMultiplier Mythic), equipment.ts ItemRarity 확장. parity 테스트(Mythic 값).
### 2.5 데이터/밸런스
- Mythic 스탯/비용/확률 값 + 기존 등급 곡선 정합 + 무한 성장 맥락 문서.
### 2.6 테스트
- 서버 Vitest(Mythic 배수/비용/확률/미러/parity) + 클라 Automation(Mythic 스탯 배수·affix·세트·강화 비용·드롭 확률·HUD 색, 기존 등급 회귀).

## 3. 범위 외
- Mythic 초과 등급/무한 등급 명명(아이템 파워 무한은 스테이지×배수·50강으로 충족), 등급 승급/재련(후속).
- 보상/골드 변경(드롭 분포만). 서버 권위 드롭(클라 권위 V1).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | EItemRarity Mythic + 등급 의존 공식(드롭/affix/세트/강화) 확장 + Automation | ✅ 메인 (`character`) |
| 백엔드 | drop/enhance/equipment Mythic 미러 + parity | ✅ 보조 (`backend`) |
| 디자이너 | Mythic 색/이름 HUD + 로컬라이즈 | ✅ 보조 (`designer`) |
| 밸런스 | Mythic 값/확률 + 등급 곡선 + 문서 | ✅ 보조 (`balance`) |
| QA | Mythic 드롭/스탯/강화/세트/회귀 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/designer/balance/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정** + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]], [[reference-ci-retrigger]]). **Codex 커밋/푸시 누락 PM 보완**([[reference-codex-invoke]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| EItemRarity 확장 누락 사이트(컴파일/표시) | grep "EItemRarity"/"Legendary" 전수 + switch default 안전 + 빌드/Automation |
| Mythic 확률/배수 폭주 | Legendary 위 보수적 값 + balance 점검, 고레벨 극희귀 |
| 서버↔클라 parity(배수/확률/비용) | drop/enhance/equipment DefinitionParity 확장(Math.fround float) |
| 기존 등급 회귀 | Mythic 추가만, Common~Legendary 값/로직 불변, 회귀 테스트 |

## 7. 후속
- 등급 승급/재련, Mythic 초과 등급, 등급별 전용 외형, 서버 권위 드롭.
