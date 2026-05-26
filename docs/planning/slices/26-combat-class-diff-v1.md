# PR #26 기획서 — 직업 전투 차별화 심화 (마법/크리)

> 기존 슬라이스 심화. 마법사/궁수(PR #21)가 V1 단순 계수로만 데미지를 내 직업 차이가 약함. **마법사=마법 데미지(MagicAtk vs MagicDef), 궁수=크리티컬(critRate 롤→critDmg 배수)** 로 정밀화. 능력치(magicAtk/critRate/critDmg)·CombatComponent CritRate/CritDmg 는 이미 존재 — 데미지 적용만 심화.

## 1. 목표 / DoD
스킬/공격 데미지가 직업·스킬 유형에 따라 물리/마법으로 구분되고, 크리티컬이 확률 발동해 직업이 실제로 다르게 느껴진다. 서버↔클라 공식 일치.

### DoD 검증
1. 마법 스킬(마법사 effectType=마법) → MagicAtk vs 타깃 MagicDef 로 데미지. 물리(전사/궁수 기본) → Atk vs Def.
2. 크리티컬 → critRate 확률로 발동, 발동 시 critDmg(기본 1.5x) 배수. 궁수(높은 critRate) 차별화.
3. 서버 combat.ts ↔ 클라 CombatFormulas 공식 일치(crit/magic). 결정적 테스트(crit 적용/미적용, 마법 경로).
4. 서버 Vitest + UE 빌드/Automation GREEN, 기존 전투 회귀 없음.

## 2. 범위 (In Scope)
### 2.1 전투 공식 (서버 TS + 클라 C++ 동일)
- `computeDamage(atk, def)` 유지 + **crit 적용 헬퍼**: `applyCrit(baseDamage, isCrit, critDmg)` = isCrit ? base*critDmg : base. crit 롤은 `rollCrit(critRate, rng)` 분리(결정적 테스트 가능).
- **마법 데미지**: 동일 공식에 magicAtk/magicDef 대입(또는 computeMagicDamage(magicAtk, magicDef)). 물리/마법 동일 곡선, 입력 스탯만 차이.
### 2.2 CombatComponent (클라)
- MagicAtk, MagicDef 필드 추가 + InitializeCombat 확장(DerivedStats magicAtk/magicDef 주입). CritRate/CritDmg 는 기존.
### 2.3 데미지 적용 분기 (클라)
- USkillComponent: 스킬 effectType/직업이 마법이면 마법 데미지, 아니면 물리. crit 롤 적용(공격자 critRate/critDmg).
- UCombatComponent/BattleAIComponent 기본 공격: 물리 + crit 롤.
- 마법사 스킬 = 마법 데미지, 궁수 = 물리+높은 crit 차별화 체감.
### 2.4 서버 미러 + 테스트
- combat.ts crit/magic 헬퍼 + Vitest. 클라 Automation(결정적 crit/magic). 서버↔클라 parity 테스트.
### 2.5 밸런스
- crit 곡선(이미 critRate=luk*0.002, critDmg=1.5+)·마법 계수 점검 + 문서.

## 3. 범위 외
- 속성(화/빙 등) 상성, 상태이상(중독/빙결), 회피/명중, 방어 관통(후속).
- 데미지 표시(숫자 플로팅) UI 심화(후속).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | 클라 공식 crit/magic + CombatComponent Magic 필드 + 스킬/공격 분기 + Automation | ✅ 메인 (`character`) |
| 백엔드 | combat.ts crit/magic 미러 + Vitest + parity | ✅ 보조 (`backend`) |
| 밸런스 | crit/마법 수치 점검 + 문서 | ✅ 보조 (`balance`) |
| QA | 마법/크리 데미지 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/balance/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정** + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| crit RNG 비결정성 테스트 | rollCrit/applyCrit 분리해 결정적 단위테스트, 롤은 시드/확률 경계 |
| 서버↔클라 공식 불일치 | 동일 헬퍼 스펙 + parity 테스트 |
| 기존 물리 전투 회귀 | 기본 공격 물리 유지 + 기존 테스트 보존 |

## 7. 후속
- 속성 상성/상태이상, 방어 관통/회피·명중, 데미지 플로팅 텍스트, 직업별 콤보.

## Codex character implementation note
- Client formulas now use explicit `ComputeDamage(atk, def)`, `ComputeMagicDamage(magicAtk, magicDef)`, `RollCrit(critRate, FRandomStream&)`, and `ApplyCrit(baseDamage, isCrit, critDmg)`.
- `UCombatComponent` stores `MagicAtk` and `MagicDef` while preserving the old 6-argument `InitializeCombat` overload for existing physical-only callers. The new 8-argument overload injects magic and crit stats together.
- `AIdleCharacter` initializes combat from derived `PhysAtk`, `PhysDef`, `MagicAtk`, `MagicDef`, `CritRate`, and `CritDmg`; monsters mirror physical defense into `MagicDef` for V1.
- Basic attacks remain physical and roll crit. Mage skills use `MagicAtk` vs target `MagicDef`; non-mage damage skills use `Atk` vs `Def`; both apply rolled crit.
- Server `combat.ts` mirrors the same helpers and has Vitest anchors for crit and magic damage parity.
