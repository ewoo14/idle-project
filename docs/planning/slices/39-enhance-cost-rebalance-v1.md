# PR #39 기획서 — 강화 비용 희귀도 스케일링 (경제 리밸런스)

> #38 밸런스 분석에서 **엔드게임 골드 유입(~654k/h)이 모든 싱크를 압도**해 골드가 무의미하게 적체됨이 드러났다. 원인 중 하나: **강화 비용(#33)이 희귀도/기어 품질을 무시**한다(Common +5 = Legendary +5 = 5500골드 동일). 강화 비용을 **희귀도에 비례**시켜, 좋은 장비(고희귀도)를 강화하는 데 진행도에 걸맞은 골드가 들도록 → 골드가 의미를 갖는 지속 싱크로 만든다.

## 1. 목표 / DoD
강화 비용이 대상 장비의 희귀도가 높을수록 커지며, 엔드게임(고희귀도) 강화가 골드의 주요 소비처가 된다.

### DoD 검증
1. 강화 비용 = 기존 레벨 곡선 × 희귀도 배수(Common 1 / Uncommon 2 / Rare 4 / Epic 8 / Legendary 16). 같은 강화 레벨이라도 희귀도↑ → 비용↑.
2. TryEnhanceEquipped 가 장착 장비 희귀도를 비용에 반영(골드 게이트/차감 정합).
3. HUD 강화 패널 비용 표시가 희귀도 반영 비용으로 갱신.
4. balance-sim/문서로 #32 골드 유입 대비 강화 싱크가 의미 있는 규모(예 Legendary 풀세트 강화 ≈ 수십 분~시간 단위 유입)임을 확인.
5. 서버 EnhanceFormula 미러(희귀도 인자) + parity. 서버 Vitest + UE 빌드/Automation GREEN. 기존 강화/경제 회귀(저희귀도=거의 동일) 최소.

## 2. 범위 (In Scope)
### 2.1 강화 비용 공식 (메인, C++)
- `FEnhanceFormula::GetEnhanceCost(int32 CurrentLevel, EItemRarity Rarity)` (시그니처 확장):
  - 기존 레벨 곡선 base×(level+1)² 유지 × `GetRarityCostMultiplier(Rarity)`.
  - `GetRarityCostMultiplier(EItemRarity)`: None 0(또는 무의미) / Common 1 / Uncommon 2 / Rare 4 / Epic 8 / Legendary 16. (지수형 — 고희귀도 강화가 큰 골드 싱크.)
  - 기존 단일 인자 호출부(HUD/테스트)는 새 시그니처로 갱신(장착 아이템 희귀도 전달). 하위호환이 필요하면 (level) 오버로드 = Common 기준.
### 2.2 강화 행위/HUD 정합 (메인, C++)
- `UIdleGameInstance::TryEnhanceEquipped`: 장착 아이템 Rarity 를 GetEnhanceCost 에 전달(비용/골드 게이트). 기존 가드(미장착/최대치/골드부족/1회 차감) 유지.
- HUD 강화 패널: GetEnhanceCost(level, rarity) 로 비용 표시(슬롯별 장착 아이템 희귀도 반영).
### 2.3 서버 (백엔드)
- `server/src/core/formulas/enhance.ts`: getEnhanceCost(currentLevel, rarity) + getRarityCostMultiplier 미러 + parity 테스트. ItemRarity 타입은 drop.ts 재사용.
### 2.4 데이터/밸런스 (리드)
- 희귀도 배수 곡선 근거 + #32 골드 유입(스테이지/시간당) 대비 강화 싱크 규모 분석(balance-sim 또는 계산 표): 저희귀도 초반 부담 최소 + 고희귀도 엔드게임 의미. 필요 시 배수 조정. #38 골드 상점과 합산 싱크 결론.
### 2.5 테스트
- 서버 Vitest(희귀도별 비용/미러/parity) + 클라 Automation(희귀도 배수 비용, TryEnhanceEquipped 희귀도 반영 골드 게이트, 저희귀도 회귀 최소).

## 3. 범위 외
- 골드 유입(#32) 자체 하향(이번은 싱크 측 교정), 강화 재료/소비 아이템, 강화 실패 파괴(후속).
- 상점 비용 재조정(#38 유지) — 별도 필요 시 후속.

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | GetEnhanceCost(level,rarity) + RarityCostMultiplier + TryEnhanceEquipped/HUD 비용 반영 + Automation | ✅ 메인 (`character`) |
| 백엔드 | enhance.ts 희귀도 인자 미러 + parity | ✅ 보조 (`backend`) |
| 밸런스 | 희귀도 배수 곡선 + 유입 대비 싱크 분석 + 문서 | ✅ 보조 (`balance`) |
| QA | 희귀도별 강화 비용/골드 게이트 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/balance/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정** + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]]). **Codex 커밋/푸시 누락 PM 보완**([[reference-codex-invoke]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 시그니처 변경으로 호출부 누락(컴파일/표시 깨짐) | GetEnhanceCost 호출부(HUD/TryEnhanceEquipped/테스트) 전수 갱신 + 빌드/Automation 검증 |
| 초반 저희귀도 강화 부담 급증 | Common 배수 1(기존과 동일) → 저희귀도 회귀 최소, 고희귀도만 상승 |
| 배수 과/소(싱크 여전히 불균형) | balance-sim/계산으로 유입 대비 검증 후 배수 조정 |
| 서버↔클라 parity | EnhanceFormula DefinitionParity 확장(희귀도 인자) |

## 7. 후속
- 골드 유입(#32) 정밀 튜닝, 상점 비용 재조정, 강화 재료/소비 아이템, 다른 골드 싱크(재화 다양화).
