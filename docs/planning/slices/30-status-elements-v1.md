# PR #30 기획서 — 상태이상 + 속성 상성 (심화)

> 기존 슬라이스 심화. 데미지 차별화(#26)·피드백(#28) 위에 **상태이상(독/화상 지속피해, 빙결 둔화)** + **속성 상성(화/빙/번개/신성 약점 배수)** 추가 → 전투 깊이. 직업/스킬에 속성·상태 부여.

## 1. 목표 / DoD
일부 스킬이 적에게 상태이상(독·화상=지속피해, 빙결=공속 둔화)을 부여하고, 스킬 속성과 몬스터 약점에 따라 데미지가 가감된다.

### DoD 검증
1. 상태이상: ESkillStatusEffect{None,Poison,Burn,Freeze}. 독/화상 = 매 틱 지속피해(지속시간), 빙결 = 대상 공속 둔화(지속시간). 상태이상 부여 스킬이 적용.
2. 속성: ESkillElement{None,Fire,Ice,Lightning,Holy}. 스킬 Element + 몬스터 WeakElement → 약점 ×1.5 / 저항 ×0.5 / 무속성 ×1.0 배수 데미지.
3. 상태이상 틱(타이머/배틀틱), 만료 제거. 상태 아이콘 HUD 표시.
4. 서버 SkillDB 미러(status/element 필드) + parity. 서버 Vitest + UE 빌드/Automation GREEN. 기존 전투 회귀 없음.

## 2. 범위 (In Scope)
### 2.1 캐릭터·전투 (메인, C++)
- 열거형: ESkillStatusEffect, ESkillElement. FSkillDefinition 필드 추가: StatusEffect, StatusDuration, StatusMagnitude(독/화상 틱당 피해 또는 비율; 빙결 둔화율), Element.
- UCombatComponent: 활성 상태 목록(TArray<{Type,EndTime,Magnitude,NextTickTime}>) + ApplyStatus(...) + TickStatuses(Now)(독/화상 = TakeDamageTyped DoT, 빙결 = AtkSpeed 둔화 적용/해제). 틱은 반복 타이머(또는 소유자 BattleAI UpdateBattle 5Hz 에서 호출).
- 속성 상성: `ComputeElementMultiplier(SkillElement, TargetWeakElement)` 순수 함수(약점 1.5/저항 0.5/그 외 1.0). 데미지 적용 시 곱.
- IdleMonster: WeakElement(약점 속성) 필드(슬라임/보스 등 1차값).
- 스킬 적용: ApplyDamageSkill 에서 Element 배수 + 명중 시 StatusEffect 부여(스킬에 설정 시). 기존 직업 스킬에 속성/상태 부여(마법사 파이어볼=Fire, 빙결 스킬=Ice+Freeze, 도적 독칼=Poison 등).
- Automation: 상태 틱(DoT/만료), 빙결 둔화, 속성 배수(약점/저항), 스킬 적용.
### 2.2 서버 (백엔드)
- SkillDB 미러에 statusEffect/statusDuration/statusMagnitude/element 필드 추가 + parity.
### 2.3 UI (디자이너)
- 상태이상 아이콘/표시(대상 위 또는 HUD) — 활성 상태(독/화상/빙결) 표기. V1 단순.
### 2.4 데이터/밸런스
- 상태 지속/피해, 속성 배수, 몬스터 약점 1차값 + 문서.
### 2.5 테스트
- 서버 Vitest(미러/배수) + 클라 Automation(상태/속성).

## 3. 범위 외
- 상태이상 중첩/해제 스킬, 면역, 상태별 추가 효과(기절 등)(후속).
- 복잡한 속성 상성표(상극 사이클)(V1 약점/저항 단순).
- 상태 파티클/사운드(외부).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | Status/Element 시스템 + 틱 + 배수 + 스킬/몬스터 적용 + Automation | ✅ 메인 (`character`) |
| 백엔드 | SkillDB 미러 status/element + parity | ✅ 보조 (`backend`) |
| 디자이너 | 상태이상 아이콘/표시 | ✅ 보조 (`designer`) |
| 밸런스 | 상태/속성 수치 + 몬스터 약점 + 문서 | ✅ 보조 (`balance`) |
| QA | 독/화상/빙결/속성 배수 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/designer/balance/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정** + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]]). **Codex 커밋/푸시 누락 PM 보완**([[reference-codex-invoke]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 상태 틱 타이머/수명 누수 | 만료 제거 + 결정적 TickStatuses 테스트, 틱 소스 단일화 |
| 빙결 둔화 적용/해제 정합(AtkSpeed) | 적용분 추적 후 해제(버프 패턴 재사용), 테스트 |
| 서버↔클라 status/element parity | DefinitionParity 확장 |
| 기존 데미지/전투 회귀 | Element 배수=무속성 1.0 기본, 기존 테스트 보존 |

## 7. 후속
- 상태 중첩/면역/해제, 상극 사이클, 기절/넉백, 상태 파티클·사운드(외부).
