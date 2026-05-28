# PR #63 기획서 — 직업 전용 룬 (룬 시스템 확장 2)

> **사용자 선택(룬 확장 계속)**: PR #61(장착)·#62(도감)에 이은 룬 트랙 3번째. 8직업 각각의 **전용 마스터리 룬** + 직업 전용 슬롯 1칸으로 **콘텐츠 볼륨([[project-content-richness]]) + 직업 빌드 특화 + 무한 성장([[project-infinite-growth]])** 동시 확보. 클라/서버 미러(parity). client + server 멀티시스템(+designer/balance/qa).

## 1. 목표 / DoD
캐릭터 직업과 일치하는 직업 전용 룬을 전용 슬롯에 장착해 직업 주력 스탯을 무한 강화한다. 기존 룬(#61)·도감(#62)·8직업(#57)·스탯 합성과 정합.

### DoD 검증
1. **데이터 모델**: `FRuneInstance`에 `EClassId ClassRestriction = None(0)` 추가(0=범용, 1~8=직업 전용, 기존 룬 0 직렬화 호환). `ERuneType`에 `ClassMastery = 10` 추가. 전용 슬롯 인덱스 6(`RuneSlotCount` 6→7).
2. **전용 슬롯 규칙**: 인덱스 6 = 직업 전용 슬롯. `ClassRestriction == 캐릭터 직업`인 ClassMastery 룬만 장착 가능(불일치 거부). 범용 슬롯 0~5는 ClassMastery 룬 장착 거부(전용 슬롯 전용). 기존 코어/유틸 룬은 0~5 그대로.
3. **마스터리 효과(직업별 코어 스탯 묶음, 무한 % 곱)**: `FClassRuneFormula::GetClassMasteryMultipliers(ClassId, Rarity, Lvl)` → `FRuneCoreMultipliers` 기여(기존 코어 합산 경로 합류). 직업별 정의:
   | 직업 | 마스터리 코어 스탯 |
   | --- | --- |
   | Warrior(1) | PhysAtk + PhysDef |
   | Mage(2) | MagicAtk |
   | Archer(3) | PhysAtk |
   | Thief(4) | PhysAtk |
   | Cleric(5) | MagicAtk + Hp |
   | Paladin(6) | PhysDef + Hp |
   | Berserker(7) | PhysAtk |
   | Summoner(8) | MagicAtk |
   - 단일 스탯 직업은 배율↑, 2스탯 직업은 분산(balance 시뮬 튜닝). 강화 무한(코어 step).
4. **획득**: ClassMastery 룬 드롭(낮은 확률, **현재 직업 것만** 드롭) + 룬 에센스 제작(`TryCraftClassRune` — 현재 직업 전용, 에센스 싱크).
5. **스탯 합성 통합**: 전용 슬롯 ClassMastery 룬의 마스터리 배수를 `GetEquippedCoreMultipliers`에 합산(기존 합 경로). 직업 불일치/미장착 → 0 기여(회귀안전).
6. **저장(`SaveVersion` 4→5)**: `FRuneInstance.ClassRestriction` 직렬화 + `EquippedSlots` 6→7. **v4→v5 마이그레이션**(기존 6슬롯 → 7슬롯 확장, 전용 슬롯 INDEX_NONE). 클라우드(#54) 자동 정합.
7. **서버 미러**: `server/src/core/formulas/classRune.ts` + `classRune.test.ts`(직업별 마스터리 보너스 parity, `Math.fround`, 8직업 앵커).
8. **HUD**: 룬 패널에 직업 전용 슬롯 1칸(자기 직업 룬만, 직업 표시) + 제작 액션 + 로컬라이즈 ko/en.
9. **테스트**: 클라 Automation(전용 슬롯 직업 일치/불일치 거부·범용 슬롯 ClassMastery 거부·마스터리 합산·드롭 자기 직업·제작·저장 v4→v5 마이그레이션·8직업) + 서버 vitest(`classRune.ts` 8직업 parity). UE Build/Automation + 서버 build/test/lint **GREEN**, server-ci 포함 CI 그린.

## 2. 범위 (In Scope)
### 2.1 직업 룬 데이터/공식 (character + backend 미러)
`FRuneInstance.ClassRestriction` + `ERuneType::ClassMastery` + `FClassRuneFormula`(8직업 마스터리 배수) + 서버 `classRune.ts`.
### 2.2 전용 슬롯 + 서비스 (character)
`RuneSlotCount` 7 + 전용 슬롯 직업 제약 장착 로직(`TryEquipRune` 슬롯6 규칙) + `TryCraftClassRune` + 드롭 자기 직업 제한.
### 2.3 스탯 합성 (character)
`GetEquippedCoreMultipliers`에 ClassMastery 마스터리 합산.
### 2.4 저장 v5 (character)
ClassRestriction 직렬화 + 7슬롯 + v4→v5 마이그레이션.
### 2.5 서버 미러 (backend)
`classRune.ts` + parity test.
### 2.6 UI (designer)
전용 슬롯 HUD + 제작 + 로컬라이즈.
### 2.7 밸런스 (balance)
8직업 마스터리 배율 시뮬 + 직업 밸런스(#60) 정합 + 파워크리프 가드.
### 2.8 테스트 — DoD 9.

## 3. 범위 외 (후속)
- 직업 전용 룬 도감 연계 · 다중 전용 슬롯 · 직업 변경 시 룬 재배치 UI · 룬 세트 · 룬 리롤.

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | ClassRestriction/ClassMastery/FClassRuneFormula + 전용 슬롯7 장착 규칙 + 마스터리 합산 + 제작/드롭 + 저장 v5 + 서버 classRune.ts 미러 + Automation | ✅ 메인 (`character`) |
| 백엔드 | `classRune.ts` parity 보강 + 클라우드 정합 | ✅ 보조 (`backend`, character 흡수 가능) |
| 디자이너 | 전용 슬롯 HUD + 제작 + 로컬라이즈 ko/en | ✅ 보조 (`designer`) |
| 밸런스 | 8직업 마스터리 배율 시뮬 + #60 직업 밸런스 정합 | ✅ 보조 (`balance`) |
| QA | 직업 일치/불일치·범용 슬롯 거부·마이그레이션·8직업 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+designer/balance) → [3] Claude TM → [4] Codex TM+fix(필요시) → [5] 검증 → [N] **CI 그린 확정** + PM 종합 소견 + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 전용 슬롯 직업 제약 누락(불일치 장착) | `TryEquipRune` 슬롯6 분기에서 ClassRestriction==직업 검사 + Automation 일치/불일치 |
| 범용 슬롯에 ClassMastery 장착 | 슬롯 0~5는 ClassMastery 거부 검사 |
| 마스터리 파워크리프 (코어 합산 추가) | balance 8직업 시뮬 + #60 밸런스 밴드 정합, 코어 합산(곱 폭발 아님) |
| 클라/서버 8직업 배율 불일치 | classRune.ts parity(Math.fround) + 8직업 앵커 |
| v4→v5 마이그레이션 회귀 | 6→7슬롯 확장 전용 슬롯 INDEX_NONE + 기존 룬 ClassRestriction 0 + 라운드트립 Automation |
| 직업 변경(환생/초월 무관) 시 전용 룬 무효 | 직업은 고정(DefaultClassId), 변경 경로 없음 — 단 불일치 룬은 효과 0(회귀안전) |
| 클라우드(#54) 정합 | UStructToJson 자동 포함(ClassRestriction 필드) |

## 7. 후속 (룬 확장 계속)
- 직업 전용 룬 도감 연계, 룬 세트, 강화 단계 도감, 룬 리롤/전송, 다중 전용 슬롯.
