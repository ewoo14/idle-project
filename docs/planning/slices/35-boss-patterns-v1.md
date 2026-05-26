# PR #35 기획서 — 보스 패턴/페이즈 (심화)

> 스테이지 1-5 보스(#31)는 HP/Atk 만 클 뿐 일반 몬스터와 **전투 방식이 동일**하다(BattleAI 추격+평타). 보스에 **HP 페이즈(격노)** 와 **주기적 특수공격(강타)** 을 추가해 챕터 클라이맥스의 전투 하이라이트를 만든다. 페이즈는 HP 비율로 결정(HUD 와 전투가 동일 산출).

## 1. 목표 / DoD
보스는 HP 가 닳을수록 단계(페이즈)가 올라가 공격력·공속이 강화(격노)되고, 일정 주기마다 강한 특수공격을 사용한다. HUD 에 보스 HP 바와 현재 페이즈가 표시된다.

### DoD 검증
1. 보스 페이즈: HP 비율로 1/2/3 단계(예 >0.66 / >0.33 / 그 이하). 단계↑ → 공격력·공속 배수 상승(격노).
2. 특수공격: 보스가 일정 주기마다 평타 대신 강타(데미지 ×배수) + 텔레그래프 이벤트 브로드캐스트(HUD/연출용).
3. 페이즈/특수공격은 보스(AIdleMonster::IsBoss)에만 적용. 일반 몬스터/플레이어 전투 회귀 없음.
4. HUD: 보스 존재 시 상단 보스 HP 바 + 페이즈 표시.
5. 서버 BossPhaseFormula 미러 + parity. 서버 Vitest + UE 빌드/Automation GREEN.

## 2. 범위 (In Scope)
### 2.1 보스 공식 (메인, C++)
- `BossPhaseFormula.h/.cpp`(CombatSystem/, 순수 static):
  - `GetBossPhase(float HpRatio)` → int32 1/2/3 (임계 0.66/0.33).
  - `GetPhaseAtkMultiplier(int32 Phase)` → 격노 공격 배수(예 1.0/1.25/1.6).
  - `GetPhaseAtkSpeedMultiplier(int32 Phase)` → 공속 배수(예 1.0/1.15/1.3).
  - `SpecialAttackIntervalSeconds`(예 6.0) + `GetSpecialAttackDamageMultiplier()`(예 2.5).
### 2.2 보스 전투 분기 (메인, C++)
- `UBattleAIComponent::Attack`: 소유자가 보스(Cast<AIdleMonster>->IsBoss())면 현재 HP 비율 → 페이즈 → 격노 배수 적용(EffectiveAtk = Atk×PhaseAtkMul, 유효 쿨다운 = 1/(AtkSpeed×PhaseAtkSpeedMul)). 기본 Atk/AtkSpeed 는 불변(공격 시점 배수만). 비보스는 기존 경로 그대로.
- 특수공격: 보스이고 `World->Time - LastSpecialTime >= SpecialAttackIntervalSeconds` 면 이번 공격을 강타(데미지 ×SpecialMul)로 + `OnBossSpecialAttack` 브로드캐스트(텔레그래프/HUD), LastSpecialTime 갱신. (V1 windup 액터 없이 즉시 강타 + 이벤트 큐.)
### 2.3 UI (디자이너)
- HUD 보스 인디케이터: 화면 내 보스(IsBoss) 존재 시 상단 보스 HP 바(CurrentHp/MaxHp) + 페이즈(BossPhaseFormula::GetBossPhase) 표시. 특수공격(OnBossSpecialAttack) 시 짧은 경고 플래시/텍스트. 로컬라이즈 ko/en.
### 2.4 서버 (백엔드)
- `server/src/core/formulas/bossPhase.ts`: getBossPhase/getPhaseAtkMultiplier/getPhaseAtkSpeedMultiplier/SPECIAL_ATTACK_INTERVAL/getSpecialAttackDamageMultiplier 클라 미러 + parity 테스트.
### 2.5 데이터/밸런스
- 페이즈 임계/격노 배수/특수 주기·배수 + 보스 난이도 곡선(#31 보스 HP 500·#32 보상) 점검 + 문서.
### 2.6 테스트
- 서버 Vitest(공식/미러/parity) + 클라 Automation(페이즈 경계, 배수, 특수 주기 판정, 보스 한정 적용, 비보스 회귀).

## 3. 범위 외
- 특수공격 windup 텔레그래프 액터/범위 표식, 다중 보스 패턴/소환/이동기, 보스별 고유 스킬셋(후속).
- 파티클/사운드/카메라 연출(외부 의존).
- 서버 권위 보스 전투(클라 권위 V1, 서버 공식 미러만).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | BossPhaseFormula + BattleAI 보스 분기(격노/특수) + OnBossSpecialAttack + Automation | ✅ 메인 (`character`) |
| 백엔드 | bossPhase.ts 미러 + parity | ✅ 보조 (`backend`) |
| 디자이너 | 보스 HP 바/페이즈/특수 경고 HUD + 로컬라이즈 | ✅ 보조 (`designer`) |
| 밸런스 | 페이즈 임계/격노/특수 배수 + 보스 곡선 + 문서 | ✅ 보조 (`balance`) |
| QA | 페이즈 전환/격노/특수공격/보스 한정 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/designer/balance/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정** + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]]). **Codex 커밋/푸시 누락 PM 보완**([[reference-codex-invoke]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 격노가 기본 스탯 영구 변형 | 공격 시점 배수만 적용(EffectiveAtk/EffectiveCooldown), 기본 Atk/AtkSpeed 불변 |
| 특수공격 주기 비결정/난타 | 시간 임계 판정 분리 + LastSpecialTime, 결정적 테스트 |
| 보스 한정 적용 누락(일반몹 회귀) | IsBoss 게이트 + 비보스 경로 기존 유지 테스트 |
| 페이즈 산출 HUD/전투 불일치 | 둘 다 BossPhaseFormula::GetBossPhase(HP비율) 단일 사용 |
| 서버↔클라 공식 parity | BossPhaseFormula DefinitionParity 확장 |

## 7. 후속
- windup 텔레그래프/범위 표식, 보스별 고유 스킬/소환/이동기, 다중 챕터 보스, 연출(외부), 서버 권위.
