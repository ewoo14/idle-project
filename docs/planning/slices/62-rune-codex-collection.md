# PR #62 기획서 — 룬 도감 + 수집 보너스 (룬 시스템 확장 1)

> **사용자 선택(룬 시스템 확장)**: PR #61(룬 장착)의 첫 확장 슬라이스. 룬 타입×레어도 54셀 도감 + 셀당/행/카테고리 수집 보너스로 **메타 성장 + 수집 갤러리 + 빌드 동기** 확보. 무한 성장은 룬 강화(#61)가 이미 제공 → 도감은 **유한 마일스톤형 메타 보상**(유의미한 메타 벡터). 클라/서버 미러(parity). client + server 멀티시스템(+designer/balance/qa).

## 1. 목표 / DoD
한 번 획득한 룬의 타입×레어도 조합이 도감에 영구 unlock되어 수집도에 따른 영구 글로벌 보너스를 제공한다. 기존 룬 시스템(#61)·스탯 합성·저장(#52~#54)·업적(#55)과 정합.

### DoD 검증
1. **데이터 모델**: `ERuneType 9 × EItemRarity 6 = 54 셀` 도감. `FRuneCodexEntry`(또는 비트셋). 한 번 unlock 영구(분해해도 유지). `#55 업적 ItemsCollected/UniqueItemsFound`과 별개(룬 전용).
2. **트리거**: `URuneService::AddRune` 시 RuneType+Rarity 조합 자동 unlock(중복 no-op). 드롭/상점 뽑기/저장 복원 시 일관 적용.
3. **수집 보너스 (절충)**: 
   - 셀당: 글로벌 코어 스탯 +0.4% 가산 (54셀 = 최대 +21.6%)
   - 레어도 행 완성(9셀): Common +1% / Uncommon +2% / Rare +3% / Epic +5% / Legendary +8% / Mythic +12% (총 +31%)
   - 카테고리 완성: 코어 30셀 완성 → 코어 +5%, 유틸 24셀 완성 → 유틸 캡 +10%
   - 초기 수치는 balance 시뮬 튜닝
4. **스탯 합성 통합**: `RefreshDerivedStats` 룬 코어 합산(`1 + Σ runeBonus`)에 **`+ codexCellSum + codexRowSum + codexCategoryBonus`** 추가(5스탯 동일). 도감 비어있으면 0 가산(회귀안전). 유틸 캡 확장은 `GetUtilCap`/`GetEquippedUtilValues`가 조회.
5. **저장(`SaveVersion` 3→4)**: `UIdleSaveGame`에 도감 필드 추가. **v<4 마이그레이션 회귀안전**(빈 도감 = 보너스 0). 클라우드(#54) `clientSave` 중첩 자동 포함.
6. **서버 미러**: `server/src/core/formulas/runeCodex.ts` + `runeCodex.test.ts`(공식 parity, `Math.fround`, 행/카테고리 임계).
7. **HUD**: 룬 패널 내부 9×6 도감 그리드(타입 행 × 레어도 열, unlock 색/체크) + 완성도(N/54) + 행/카테고리 마일스톤 표시. 로컬라이즈 `Rune.csv` ko/en 확장.
8. **테스트**: 클라 Automation(unlock 트리거/셀·행·카테고리 보너스/저장 라운드트립/v3→v4 마이그레이션/AddRune 중복 no-op/장착·분해 후 영구 유지) + 서버 vitest(`runeCodex.ts` parity + 행·카테고리 앵커). UE Build/Automation + 서버 build/test/lint **GREEN**, server-ci 포함 CI 그린 확정([[feedback-ci-before-merge]]).

## 2. 범위 (In Scope)
### 2.1 도감 데이터/공식 (character + backend 미러)
`FRuneCodexFormula` (셀/행/카테고리 보너스, 행·카테고리 임계) + 서버 `runeCodex.ts` 동기.
### 2.2 도감 서비스 통합 (character)
`URuneService` 안에 `TArray<FRuneCodexEntry>` 또는 비트셋 + Unlock(`AddRune`에서 자동) + `GetCodexCoreBonus`/`GetCodexUtilCapBonus`/`GetCodexCompletion`. (별도 서비스 분리는 YAGNI — 룬 도메인이라 RuneService 통합)
### 2.3 스탯 합성 통합 (character)
`RefreshDerivedStats` 코어 합산에 도감 가산 추가. `GetUtilCap` 조회 시 도감 확장 반영.
### 2.4 저장 v4 (character)
`SaveVersion` 3→4 + 도감 필드 + Capture/Apply + 마이그레이션 회귀안전 + 클라우드 정합.
### 2.5 서버 미러 (backend)
`runeCodex.ts` 전 공식 + `runeCodex.test.ts` parity/앵커.
### 2.6 UI (designer)
9×6 도감 그리드 + 완성도/마일스톤 + 로컬라이즈 ko/en.
### 2.7 밸런스 (balance)
셀/행/카테고리 보너스 수치 시뮬 + 파워크리프 가드 + 페이싱 영향.
### 2.8 테스트 — DoD 8.

## 3. 범위 외 (후속)
- 강화 단계 도감(타입×레어도×강화단계 = 수백 셀) · 도감 보상 일회성 픽업(자동 적용 우선) · 도감 정렬/필터 UI · 직업 전용 룬(별도 슬라이스) · 룬 세트(별도 슬라이스).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | RuneCodex 데이터/공식 + URuneService 통합 + AddRune unlock 훅 + RefreshDerivedStats 가산 + 저장 v4 + Automation | ✅ 메인 (`character`) |
| 백엔드 | `runeCodex.ts` 미러 + parity vitest + 클라우드 자동 정합 확인 | ✅ 보조 (`backend`) |
| 디자이너 | 9×6 도감 그리드 ViewModel + 룬 패널 통합 + Rune.csv 로컬라이즈 확장 | ✅ 보조 (`designer`) |
| 밸런스 | 셀/행/카테고리 보너스 수치 시뮬 + 파워크리프 가드 + 페이싱(median) 영향 | ✅ 보조 (`balance`) |
| QA | 트리거·보너스·저장·마이그레이션·중복 no-op·분해 후 유지 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/designer/balance/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정** + PM 종합 소견 + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). [[project-content-richness]].

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 도감 unlock 트리거 누락(드롭/상점/복원 경로) | AddRune 단일 진입점에 unlock 통합 + 라운드트립 Automation 전 경로 |
| #55 업적과 의미 중복 오해 | 룬 전용 카테고리 + 별도 보너스 트랙, 업적 ItemsCollected는 장비 그대로 |
| 보너스 파워크리프 (룬 강화 무한 + 도감 가산) | 도감은 유한 마일스톤(최대 +21.6%+31%+5%=+57.6% 코어), balance 시뮬 페이싱 확인 |
| 클라/서버 행·카테고리 임계 불일치 | runeCodex.ts parity + 앵커 + DefinitionParity |
| v3→v4 마이그레이션 회귀 | 빈 도감/0 보너스 기본값 + 라운드트립/마이그레이션 Automation |
| 분해 후 도감 의도 오해(영구 유지 vs 동기화) | 영구 유지 정책 명시 + Automation |
| 클라우드(#54) 정합 | UStructToJson 자동 포함, 서버 스키마 `additionalProperties:true` 유지 |

## 7. 후속 (룬 확장 계속)
- 직업 전용 룬, 룬 세트, 강화 단계 도감, 도감 정렬/필터 UI, 룬 리롤. (룬 시스템 확장 다회 슬라이스.)
