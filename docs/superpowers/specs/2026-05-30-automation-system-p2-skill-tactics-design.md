# 통합 자동화 시스템 P2 — 스킬 자동 전술 설계 스펙

- 작성일: 2026-05-30
- 상태: 설계 승인됨 → plan 단계화
- 부모 스펙: `docs/superpowers/specs/2026-05-30-automation-system-design.md` (§5② 스킬 자동 전술)
- 선행: P1(plan `docs/superpowers/plans/2026-05-30-automation-system-p1.md`, PR #99 머지 `5f1fa38`) 위에 누적

## 1. 배경

P1으로 `AutomationPolicy` 골격 + 자동 진행이 들어왔다. 전투 스킬 자동화는 아직 **무지성**이다.

현재 `USkillComponent::TickSkills(Now, Target, AoeTargets)`:
- 궁극기 게이지가 차면 **먼저 발동 후 return**.
- 그 외 Active 스킬은 **쿨다운만 되면 선언 순서대로 전부 발동**.
- 조건(보스 보존/HP 위기 힐/버프 유지)·우선순위 통제가 없다.

P2는 여기에 **스킬별 발동 조건 + 우선순위**를 정책으로 끼운다.

## 2. 목표 / 비목표

### 목표
1. 스킬별 자동 발동 규칙(`FSkillAutoRule`)을 `AutomationPolicy`에 저장.
2. parity 결정 함수 `evaluateSkillRule`(서버 `automation.ts` + 클라 `UAutomationPolicyService` static, byte-for-byte).
3. `TickSkills`를 규칙·우선순위 게이트로 통과시키되, 미설정 스킬은 `Always`=기존 동작(완전 하위호환).
4. `SkillTactics` 해금 = 챕터 3 클리어(P1에서 이미 게이트 정의).
5. 자동화 패널 "스킬" 탭(데이터 구동, 신규 C++ 위젯 0).

### 비목표 (P2 제외)
- 규칙 슬롯 수 제한 + 효율 업그레이드 sink → **P4**. P2는 모든 스킬에 규칙 부여 가능(기본 Always).
- 자동 장비(P3)·자동 소비(P4).
- 신규 스킬/이펙트 타입(전투 콘텐츠 별개 트랙).

## 3. 데이터 모델

```
// AutomationTypes.h
enum class ESkillAutoCondition : uint8 { Always, BossEliteOnly, HpBelow, MaintainBuff };

USTRUCT FSkillAutoRule {
  FName SkillId;                       // 대상 스킬
  ESkillAutoCondition Condition = Always;
  float HpThresholdPct = 0.3f;         // HpBelow 전용 [0,1]
  int32 Priority = 0;                  // 낮을수록 먼저 평가, 동률은 선언 순서
};

// FAutomationPolicySave (P1) 확장 → SaveVer 26 → 27
TArray<FSkillAutoRule> SkillRules;     // 누락=빈, 미설정 스킬=Always
```

서버 `automation.ts`는 결정 수치/로직만 parity로 보유(규칙 토글 값은 클라 권위).

## 4. parity 결정 함수

```
type SkillAutoCondition = "Always" | "BossEliteOnly" | "HpBelow" | "MaintainBuff";
type SkillRuleContext = { selfHpPct: number; isBossElite: boolean; buffActive: boolean };

evaluateSkillRule(condition, hpThresholdPct, ctx): boolean
  Always       → true
  BossEliteOnly→ ctx.isBossElite
  HpBelow      → ctx.selfHpPct <= clamp(hpThresholdPct, 0, 1)
  MaintainBuff → !ctx.buffActive
```

- `selfHpPct`는 [0,1] 클램프. 음수/NaN 가드(회귀안전, P1 패턴).
- 클라 `UAutomationPolicyService::EvaluateSkillRule`가 1:1 미러(float 정합).

## 5. TickSkills 연동

`TickSkills` 시그니처에 컨텍스트를 추가한다(기존 호출부 갱신):

```
void TickSkills(float Now, AActor* Target, const TArray<AActor*>& AoeTargets,
                const FSkillTickContext& Ctx);

USTRUCT FSkillTickContext {
  float SelfHpPct = 1.0f;     // 소유자 CombatComponent CurrentHp/MaxHp
  bool bIsBossElite = false;  // 현재 인카운터 보스/엘리트 여부
};
```

동작:
1. 규칙 조회용 헬퍼 `GetRuleFor(SkillId) -> FSkillAutoRule`(없으면 Always 기본).
2. 스킬을 **Priority 오름차순**(동률 선언 순서)으로 평가.
3. 각 스킬: `EvaluateSkillRule(rule, ctx)` && 준비(궁극기=게이지/Active=쿨다운) → 발동.
4. 궁극기 "먼저 발동" 의미 보존: 궁극기는 Priority 기본을 가장 높게(예: -1) 두어 규칙 통과 시 우선. 단 규칙(BossEliteOnly 등)에 막히면 건너뛴다.
5. `MaintainBuff`의 `buffActive`는 SkillComponent의 기존 타임드 버프 추적(`UpdateTimedBuffs`)에서 해당 스킬 버프 활성 여부로 산출.

**컨텍스트 주입**: 호출부(전투 AI/캐릭터 틱)가 소유자 HP%와 인카운터 보스/엘리트 플래그(StageService `GetCurrentStageInfo().bBossStage/bEliteStage` 또는 대상 몬스터 IsBoss)를 채워 전달. 규칙(SkillRules)은 GameInstance `AutomationPolicy`에서 SkillComponent로 동기(장착 스킬 로드 시/정책 변경 시 캐시) — 전투 컴포넌트가 GameInstance를 직접 참조하지 않도록 주입형.

## 6. 해금 / 슬롯
- `SkillTactics` 해금 = 챕터 3 클리어(`isFeatureUnlocked`, P1 정의). 미해금 시 모든 스킬 `Always`(기존 동작), 규칙 편집 잠금.
- P2는 규칙 슬롯 무제한(기본 Always). 슬롯 제한·효율 sink는 P4.

## 7. 오류 처리 / 안전 가드
- 미설정/잘못된 SkillId 규칙은 무시(Always로 폴백).
- HpThresholdPct/selfHpPct [0,1] 클램프.
- 규칙이 모든 스킬을 막아도 안전(아무 스킬도 안 나갈 뿐, 기본 공격은 별개) — 데드락 없음.
- 미해금 캐릭터/구버전 세이브(<27)는 빈 규칙 = 전부 Always = 기존 동작.

## 8. 테스트 / 게이트
- `automation.test.ts`: `evaluateSkillRule` 조건 4종 + 클램프/가드 단위 테스트.
- 클라 `AutomationPolicyServiceTests`: `EvaluateSkillRule` parity 동일 케이스.
- `SkillComponentTests`(또는 신규): TickSkills 게이트 회귀 — 조건별 발동/억제, 우선순위 순서, 궁극기 보존, 미설정=Always.
- SaveVer 27 round-trip + <27 마이그레이션(빈 규칙).
- 표준 jumbo 빌드 + 전체 Automation(`IdleProject.UI.HUD` 포함) GREEN. SaveVer stale 일괄 점검.
- 서버 biome lint clean(P1 교훈: `**` 연산자/포맷).

## 9. 구현 단계화 (plan)
P2는 단일 충실 슬라이스로 충분하다(P1 골격 재사용).
- 서버 parity `evaluateSkillRule` + 테스트.
- 클라 타입(`ESkillAutoCondition`/`FSkillAutoRule`/`FSkillTickContext`) + `EvaluateSkillRule` static + 회귀.
- 세이브 `SkillRules` + SaveVer 27 + 마이그레이션.
- TickSkills 컨텍스트/규칙 게이트/우선순위 + 호출부 주입 + 회귀.
- HUD 스킬 탭 BP 진입점(규칙 설정/조회) + 해금 게이트.

## 10. 후속
- P3 자동 장비·매각, P4 자동 소비 + 규칙 슬롯 효율 업그레이드.
- 코드리뷰 [Minor] P1 이연분(`EfficiencyUpgradeCost` float→double Growth)은 P4 효율 배선 시 처리.
