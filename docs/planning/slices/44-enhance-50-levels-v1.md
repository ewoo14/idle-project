# PR #44 기획서 — 강화 50단계 + 성공 확률 곡선 (무한 성장)

> 사용자 지시([[project-infinite-growth]]): "방치형은 성장이 무한해야. 50단계 강화 시스템, 강화 성공 확률 등." 현재 강화는 **max 5강**으로 방치형에 너무 낮다. **50단계**로 확장하고 레벨별 **성공 확률 곡선**(고레벨일수록 낮음)·비용 곡선을 도입해 깊은 성장 + 고레벨 재시도로 **무한 골드 싱크**(엔드게임 골드 적체 #38/#39 근본 완화)를 만든다. client+server+UI+balance+qa 5-team 충실 슬라이스.

## 1. 목표 / DoD
장비를 +50까지 강화할 수 있고, 강화 레벨이 오를수록 성공 확률이 낮아지고 비용이 커지며, 성공 시 장비 보너스가 레벨에 비례해 크게 상승한다.

### DoD 검증
1. MaxEnhanceLevel = 50. FItemInstance.EnhanceLevel ClampMax 5→50. 강화 행위(#33/#39)가 0~50 동작.
2. 성공 확률 곡선: GetEnhanceSuccessRate(level) 가 50레벨에 걸쳐 감소(저레벨 고확률 ~0.95 → 고레벨 저확률 floor ~0.05~0.10). 결정적.
3. 비용 곡선: GetEnhanceCost(level, rarity) 가 고레벨에서 급증(무한 골드 싱크). 희귀도 배수(#39) 유지.
4. 강화 보너스: ComputeEquipmentBonus 의 ×(1+EnhanceLevel×0.1) 가 Lv50 ×6.0(또는 곡선)로 큰 성장. 스탯 반영.
5. HUD 가 레벨/50·성공률%·비용 표시. 서버 EnhanceFormula 미러(50레벨 곡선) + parity. 서버 Vitest + UE 빌드/Automation GREEN. 저레벨(0~4) 기존 회귀 최소.

## 2. 범위 (In Scope)
### 2.1 강화 공식 50레벨화 (메인, C++)
- `FEnhanceFormula`(ItemSystem/EnhanceFormula.h/.cpp): `MaxEnhanceLevel = 50`.
  - `GetEnhanceSuccessRate(int32 CurrentLevel)`: 5-element 배열 → **함수 곡선**. 예 clamp(0.95 - level×0.018, 0.05, 0.95)(level 0 0.95 / 10 0.77 / 25 0.50 / 40 0.23 / 49 ~0.07). level>=Max → 0. (저레벨 0~4 값이 기존 {0.95,0.85,0.70,0.55,0.40}과 너무 어긋나면 곡선 보정 — 기존 회귀 최소 우선; 곡선 형태는 balance 와 합의 가능, 단조 감소.)
  - `GetEnhanceCost(int32 CurrentLevel, EItemRarity Rarity)`: 기존 100×(level+1)²×RarityCostMultiplier 유지(50레벨까지 자연 급증) 또는 곡선 강화. 고레벨 거대 비용 = 무한 골드 싱크.
  - GetRarityCostMultiplier(#39) 유지.
### 2.2 캡 확장/호출부 (메인, C++)
- `FItemInstance.EnhanceLevel` ClampMax 5→50(ItemTypes.h). MaxEnhanceLevel 하드코딩/가정 호출부(TryEnhanceEquipped, HUD, 테스트) 전수 점검.
- ComputeEquipmentBonus 강화 배수 ×(1+EnhanceLevel×0.1) 그대로(Lv50 ×6.0) — 무한 성장 체감. (필요 시 곡선 검토, V1 선형 유지.)
### 2.3 UI (디자이너)
- 강화 패널: 레벨 표기 "+N / 50", 성공 확률 %, 다음 비용(희귀도 반영). 최대치(+50) 표기. 로컬라이즈 ko/en.
### 2.4 서버 (백엔드)
- `enhance.ts`: MAX_ENHANCE_LEVEL=50, getEnhanceSuccessRate 곡선 미러(Math.fround float parity), getEnhanceCost(기존) + parity 테스트(레벨 0/10/25/49 성공률·비용).
### 2.5 데이터/밸런스 (리드)
- 50레벨 성공률·비용 곡선 + 희귀도별 +50 도달 기대 골드(성공률 고려 Σ cost/rate) 분석. #32 유입(654k/h) 대비 "무한 골드 싱크"로 기능하는지(고레벨 기대 비용이 시간당 유입을 크게 초과) 확인 + 문서. 곡선 균형 제안.
### 2.6 테스트
- 서버 Vitest(50레벨 성공률/비용 곡선·미러·parity) + 클라 Automation(MaxLevel 50, 성공률 단조 감소·floor, 비용 급증, ComputeEquipmentBonus Lv50 배수, 저레벨 회귀).

## 3. 범위 외
- 강화 실패 시 레벨 하락/파괴/보호권(V1 실패=레벨 유지·골드 소모; 위험 시스템 후속), 강화 재료 아이템, 100+ 단계(50 우선).
- 서버 권위 강화 정산(클라 권위 V1).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | MaxEnhanceLevel 50 + 성공률/비용 곡선 + 캡/호출부 + Automation | ✅ 메인 (`character`) |
| 백엔드 | enhance.ts 50레벨 곡선 미러 + parity | ✅ 보조 (`backend`) |
| 디자이너 | 강화 패널 +N/50·성공률%·비용 + 로컬라이즈 | ✅ 보조 (`designer`) |
| 밸런스 | 50레벨 곡선 + +50 기대 골드/무한 싱크 분석 + 문서 | ✅ 보조 (`balance`) |
| QA | 50레벨 강화/성공률/비용/회귀 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/designer/balance/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정** + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]], [[reference-ci-retrigger]]). **Codex 커밋/푸시 누락 PM 보완**([[reference-codex-invoke]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 캡 5→50 확장 누락 호출부(컴파일/표시/클램프) | MaxEnhanceLevel/ClampMax/하드코딩 전수 점검 + 빌드/Automation |
| 저레벨(0~4) 성공률 곡선 변경으로 기존 밸런스/테스트 회귀 | 곡선이 저레벨에서 기존과 근접하도록 + 기존 테스트 갱신, balance 합의 |
| 고레벨 비용/배수 폭주(스탯/골드) | 곡선 balance 점검, 강화 배수 클램프 불요(스탯은 DeriveStats 일부 클램프) — 무한 성장 의도 내 |
| 서버↔클라 곡선 parity | 성공률 함수 Math.fround float parity + 경계 테스트 |
| 강화 행위(#33/#39) 회귀 | TryEnhanceEquipped/희귀도 비용/HUD 기존 로직 보존, MaxLevel 만 확장 |

## 7. 후속
- 강화 실패 하락/파괴/보호권(위험 시스템), 강화 재료, 100+ 단계, 등급 확장(별도 슬라이스), 골드 유입 정밀 튜닝.
