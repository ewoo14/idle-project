# PR #67 기획서 — 유니크/초월 전용 효과 (Unique Trait)

> **PM 자율 진행**(사용자 "PM 자동 진행"). #65 레어도 7단계(유니크/초월 신규)가 아직 "더 강한 일반 등급"일 뿐 → **유니크/초월 등급 전용 고유 스탯 효과**로 차별화 + 빌드 깊이 + 콘텐츠 볼륨([[project-content-richness]]). 일반 affix(#40)보다 강력. client + server 멀티시스템(+designer/balance). [[project-rarity-overhaul]] 후속 활용.

## 1. 목표 / DoD
유니크/초월 등급 장비에만 강력한 고유 효과(Unique Trait)가 부여되어(유니크 1 / 초월 2), 최상위 등급이 일반 등급과 질적으로 차별화된다. 기존 아이템(#40 affix/#36 등급/#65 7단계)·스탯 합성·저장과 정합.

### DoD 검증
1. **데이터 모델**: `EUniqueTrait` 8종(전부 스탯류) — `AllStatSurge`/`CritDamageSurge`/`CritRateSurge`/`LifeSurge`/`SwiftSurge`/`PhysMastery`/`MagicMastery`/`GuardMastery`. `FItemInstance`에 `EUniqueTrait UniqueTrait1=None`, `UniqueTrait2=None` 추가(기존 등급 None).
2. **부여 규칙**: Unique(4) → Trait1만, Transcendent(6) → Trait1+Trait2(서로 다른 trait), 그 외 등급 None. 드롭/생성(`FDropFormula::RollUniqueTraits` 또는 ItemFactory) 시 RNG 부여(결정적).
3. **효과 공식**: `FUniqueTraitFormula::GetTraitValue(Trait, Rarity)` — Unique base, Transcendent 강화 배수(예 1.5×). 각 trait 스탯 기여:
   - AllStatSurge: 물공·마공·물방·마방 +X%
   - CritDamageSurge: 치명피해 +X / CritRateSurge: 치명률 +X
   - LifeSurge: 체력 +X% / SwiftSurge: 공속 +X
   - PhysMastery: 물공 +X% / MagicMastery: 마공 +X% / GuardMastery: 물방·마방 +X%
   - 초기 수치 balance 시뮬 튜닝(일반 affix보다 강력하되 파워크리프 가드).
4. **스탯 합성 통합**: `ComputeEquipmentBonus`(InventoryComponent)에 장착 장비의 UniqueTrait 효과 합산 → 기존 DeriveStats 경로. trait None → 0 기여(회귀안전).
5. **저장(SaveVersion 8→9)**: `FItemInstance.UniqueTrait1/2` 직렬화(기존 세이브 None 기본값, 회귀안전). 클라우드(#54) `UStructToJson` 자동 정합.
6. **서버 미러**: `server/src/core/formulas/uniqueTrait.ts` + `uniqueTrait.test.ts`(trait 값/등급 배수 parity, `Math.fround`).
7. **HUD**: 유니크/초월 아이템 표시에 고유 효과(trait명 + 값) 노출 + 로컬라이즈 ko/en(trait명 8종).
8. **테스트**: 클라 Automation(부여 규칙 등급 게이트·trait 값/등급 배수·ComputeEquipmentBonus 합산·None 회귀안전·저장 라운드트립·결정적 RNG) + 서버 vitest(uniqueTrait parity). UE Build/Automation + 서버 build/test/lint **GREEN**, server-ci 그린.

## 2. 범위 (In Scope)
### 2.1 trait 데이터/공식 (character + backend 미러)
EUniqueTrait + FItemInstance.UniqueTrait1/2 + FUniqueTraitFormula(값/부여) + 서버 uniqueTrait.ts.
### 2.2 부여 (character)
DropFormula/ItemFactory 유니크/초월 시 trait 부여(결정적 RNG, 등급 게이트).
### 2.3 스탯 합성 (character)
ComputeEquipmentBonus trait 합산.
### 2.4 저장 v9 (character)
FItemInstance 필드 + 회귀안전 + 클라우드 정합.
### 2.5 서버 미러 (backend)
uniqueTrait.ts + parity test.
### 2.6 UI (designer)
유니크/초월 고유 효과 HUD + 로컬라이즈 ko/en.
### 2.7 밸런스 (balance)
trait 값/등급 배수 시뮬 + 파워크리프 가드(affix #40 대비 강력하되 페이싱 안정).
### 2.8 테스트 — DoD 8.

## 3. 범위 외 (후속)
- 획득률/특수 trait(골드/경험치/보스피해/룬에센스) · 유니크/초월 룬 전용 trait · trait 리롤 · 고유 이름 카탈로그 확장.

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | EUniqueTrait/FItemInstance.UniqueTrait1·2/FUniqueTraitFormula + 부여(DropFormula/ItemFactory) + ComputeEquipmentBonus 합산 + 저장 v9 + 서버 uniqueTrait.ts + Automation | ✅ 메인 (`character`) |
| 디자이너 | 유니크/초월 고유 효과 HUD + 로컬라이즈 ko/en(trait명 8) | ✅ 보조 (`designer`) |
| 밸런스 | trait 값/등급 배수 시뮬 + 파워크리프 가드 | ✅ 보조 (`balance`) |
| (backend/qa) | character 흡수, [3] Claude TM parity·커버리지 점검 | — |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+designer/balance) → [3] Claude TM → [4] fix(필요시) → [5] 검증 → [N] **CI 그린 확정** + PM 종합 소견 + 머지. PM 자율([[feedback-autonomous-slices]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| trait 파워크리프(affix #40 + trait 중첩) | balance 시뮬, trait 값 보수적, 유니크/초월 희소(드롭 곡선) |
| 등급 게이트 누락(일반 등급에 trait) | RollUniqueTraits 유니크/초월만 + Automation 게이트 |
| 클라/서버 trait 값 불일치 | uniqueTrait.ts parity(Math.fround) + 앵커 |
| 저장 v8→v9 회귀 | FItemInstance enum 필드 None 기본값 + 라운드트립 Automation |
| 초월 Trait1=Trait2 중복 | 서로 다른 trait 보장 로직 + 테스트 |
| 클라우드(#54) 정합 | UStructToJson 자동 포함(UniqueTrait 필드) |

## 7. 후속
- 획득률/특수 trait, 유니크/초월 룬 trait, trait 리롤, 고유 이름.
