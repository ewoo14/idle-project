# PR #29 기획서 — 도적 + 성직자 직업 (심화, 5직업 완성)

> 기존 슬라이스 심화. 마법사/궁수(PR #21) 패턴으로 **도적·성직자** 추가 → GDD 1.0 목표 5직업 완성. 도적(DEX·LUK 회피/후방), 성직자(WIS·INT 자가회복+버프). 스탯 성장곡선(StatFormulas Thief/Cleric)·EClassId 이미 존재. 성직자는 **Heal 효과 타입(자가회복)** 신규 도입.

## 1. 목표 / DoD
도적/성직자를 선택하면 각 직업 스킬 7종이 자동 전투에 적용되며, 성직자는 HP를 자가 회복한다. 5직업 모두 선택 가능.

### DoD 검증
1. ClassId(Thief/Cleric)에 따라 해당 스킬셋 로드 + 스탯 성장.
2. 도적 7스킬(DEX·크리/회피 컨셉), 성직자 7스킬(WIS 기반 + **자가 회복 스킬**) 자동 발동.
3. 성직자 Heal 스킬 → 자신 HP 회복(MaxHp 상한). 신규 ESkillEffectType::Heal.
4. 직업 선택 UI 5직업(전사/마법사/궁수/도적/성직자).
5. 서버 SkillDB 미러 5직업 + DefinitionParity. 서버 Vitest + UE 빌드/Automation GREEN.

## 2. 범위 (In Scope)
### 2.1 캐릭터·전투 (메인, C++)
- ESkillEffectType 에 **Heal** 추가 → ApplyHeal(자신 HP += MaxHp×magnitude 또는 고정, 상한).
- `LoadDefaultThiefSkills()`: 도적 7종 — 액티브4(독칼/그림자베기/회피태세(자버프 회피·공속)/기습(후방 강타)), 패시브2(크리율+/회피+), 궁극기1(암살). DEX·LUK·크리 기반.
- `LoadDefaultClericSkills()`: 성직자 7종 — 액티브4(성스러운 일격(마법)/치유(자가회복 Heal)/축복(자버프 공격력)/정화(자버프 방어)), 패시브2(WIS/회복량+/최대HP+), 궁극기1(신성 폭발 또는 대치유). WIS·INT 기반.
- `LoadSkillsForClass(EClassId)` 분기에 Thief/Cleric 추가(기존 Warrior/Mage/Archer 패턴).
- Automation: 직업별 스킬 로드, Heal 효과(HP 회복/상한), ClassId 분기.
### 2.2 서버 (백엔드)
- SkillDB 미러에 도적/성직자 스킬 추가(클라와 동일 id/type/effectType/수치). Heal effectType 반영.
### 2.3 직업 선택 UI (디자이너)
- 기존 직업 선택(#21)을 5직업으로 확장(도적/성직자 추가 + 설명).
### 2.4 데이터/밸런스
- 도적/성직자 스킬 수치 + 회복량 1차값 + 문서.
### 2.5 테스트
- 서버 Vitest(미러 parity) + 클라 Automation(스킬/Heal/분기).

## 3. 범위 외
- 회피(Dodge) 완전 구현(명중/회피 판정 시스템) — V1 은 회피=자버프 수치로 근사, 판정 시스템 후속.
- 성직자 아군 힐(파티) — V1 자가 회복만(파티 시스템 후속).
- 직업별 고유 애니/아트(후속).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | Heal 효과 + Thief/Cleric 스킬셋 + 분기 + Automation | ✅ 메인 (`character`) |
| 백엔드 | SkillDB 미러 도적/성직자 | ✅ 보조 (`backend`) |
| 디자이너 | 직업 선택 5직업 확장 | ✅ 보조 (`designer`) |
| 밸런스 | 스킬 수치/회복량 + 문서 | ✅ 보조 (`balance`) |
| QA | 직업별 스킬/Heal 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/designer/balance/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정** + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]]). **Codex 커밋/푸시 누락 PM 보완**([[reference-codex-invoke]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| Heal 효과 신규 → 기존 데미지 경로 영향 | ApplyHeal 별도 분기, 데미지 로직 무변경 + 테스트 |
| 회피 판정 시스템 부재 | V1 회피=수치 버프 근사, 판정 후속 명시 |
| 서버↔클라 5직업 parity | DefinitionParity 5직업 확장 |

## 7. 후속
- 회피/명중 판정 시스템, 파티/아군 힐, 직업별 아트, 6직업+.
