# 통합 자동화 시스템 (Automation System) — 설계 스펙

- 작성일: 2026-05-30
- 상태: 설계 승인 대기 → 구현 plan 단계화 예정
- 분류: 메타/편의 기능 (자동전투·자동화 고도화)
- 관련 메모리: 무한 성장 sink, 점진 해금, 충실한 멀티시스템 슬라이스, 실존재화만, 클라 세이브 권위 + 서버 parity

## 1. 배경 / 문제

현재 자동화는 **모 아니면 도** 상태다.

- **자동전투**: `USkillComponent::TickSkills`가 쿨다운이 도는 스킬을 **준비되는 즉시 무조건 발동**한다. 플레이어의 전략적 통제(우선순위·조건)가 전혀 없다.
- **진행**: 스테이지/보스/엘리트 구조는 있으나 "어떻게 진행할지"(전진 / 특정 구간 파밍 고정 / 막히면 후퇴) 정책이 없다.
- **장비/드랍**: 강화·등급·드랍은 있으나 자동 장착·자동 정리 같은 손 덜기 기능이 없다. **장비 "판매/분해" 기본 기능 자체가 클라에 없다.**
- **오프라인 보상**: 골드/경험치 정산만(12h 캡, 효율 0.75).

방치형의 핵심 가치인 "손 덜 가는 무한 성장"을 충족하려면, 플레이어가 **자동화 동작을 설정으로 통제**하고 그 효율을 **무한히 성장**시킬 수 있어야 한다.

## 2. 목표 / 비목표

### 목표
1. 캐릭터별 **자동화 정책(AutomationPolicy)** 을 단일 저장 객체로 통합 관리.
2. 네 기둥(진행 / 스킬 / 장비 / 소비) 자동화를 그 위의 모듈로 제공.
3. 각 기능을 **진행 챕터 / 환생 횟수 / 실존재화**로 점진 해금.
4. 자동화 효율을 **기하급수 비용 sink로 무한 업그레이드**(낮은 캡 지양).
5. 기존 패턴 준수: 클라 세이브 권위 + 서버 parity 미러, 데이터 구동 HUD, Automation 회귀, SaveVer 관리.

### 비목표 (이번 스펙 제외)
- **자동 강화**: 골드 폭주/파산 위험. 효율 업그레이드 해금 + 골드 예산 상한·확률 설계를 갖춘 **후속 슬라이스**로 분리.
- 신규 전용 재화 도입(자동화 코어/티켓 등). **실존재화만** 사용한다(죽은 재화 발명 금지 가드).
- 서버 권위 멀티플레이어 동기화(길드와 달리 자동화는 싱글 메타 → 서버는 parity 수치 검증만).

## 3. 아키텍처

기존 환생특성(`rebirthPerk`)·룬 패턴을 그대로 따른다.

- **클라(UE5) 세이브 권위**: 정책·해금·업그레이드 상태는 로컬 세이브가 권위.
- **서버 parity 미러**: `server/src/core/formulas/automation.ts` (라우트/DB 없음). 효율 업그레이드 보너스 비율·해금 임계값을 1:1 미러링하여 drift 방지.
- **신규 UE 모듈** `AutomationSystem`:
  - `UAutomationPolicyComponent` — 정책 상태 보관 + 세이브 직렬화.
  - `UAutomationService` (혹은 BlueprintFunctionLibrary) — 틱 단위 행동 결정 순수 함수(테스트 가능), 해금/업그레이드 비용·효율 계산.
- **신규 세이브 구조** `FAutomationPolicySave` → **SaveVer 25 → 26**.
  - 구버전(<26) 세이브는 "전 기능 잠금 / 모든 자동화 기본 OFF / 업그레이드 레벨 0"으로 안전 마이그레이션.
- **데이터 구동 HUD**: 자동화 설정 패널 1개. 4탭(진행 / 스킬 / 장비 / 소비) + 해금·업그레이드 표시. C++ 신규 위젯 코드 최소화, 기존 데이터 구동 HUD 경로 재사용.

## 4. 데이터 모델 (`FAutomationPolicySave` 개요)

```
FAutomationPolicySave
  SaveVersion 가드: >= 26 에서만 로드, 미만이면 기본값

  // 해금/업그레이드 공통
  UnlockedFeatures : bitflags (Progression/SkillTactics/AutoGear/AutoConsumable)
  EfficiencyLevels : Map<EAutomationUpgrade, int32>   // 무한, 기본 0

  // ① 진행 정책
  ProgressionMode   : enum { Advance, FarmLock, AutoRetreat }
  FarmLockStageIdx  : int32        // FarmLock 시 고정 글로벌 스테이지
  bAutoBossChallenge: bool
  PushDeathThreshold: int32        // 연속 사망 N회 → 파밍 전환/후퇴

  // ② 스킬 자동 전술
  SkillRules : Array<FSkillAutoRule>
    { SkillId, Priority, Condition (Always/BossEliteOnly/HpBelow/MaintainBuff), HpThresholdPct }

  // ③ 자동 장비
  bAutoEquipByPower : bool
  AutoSellFilters   : Array<FAutoSellFilter>   // { MaxRarity, SlotMask } 슬롯 수는 업그레이드로 확장

  // ④ 자동 소비/버프
  bAutoPotion       : bool
  PotionHpThreshold : float
  bAutoMaintainBuff : bool
```

서버 `automation.ts`는 위 중 **수치(효율 보너스·해금 임계·비용 곡선)** 만 parity로 보유한다. 정책 토글 값은 클라 권위라 서버가 알 필요 없다.

## 5. 네 기둥 상세

### ① 자동 진행 정책
- **모드**
  - `Advance(전진)`: 클리어 시 다음 스테이지로.
  - `FarmLock(파밍 고정)`: 지정 스테이지 반복.
  - `AutoRetreat(자동 후퇴)`: `PushDeathThreshold`회 연속 사망 시 직전 클리어 스테이지로 후퇴 후 파밍.
- `bAutoBossChallenge`: 보스 스테이지 자동 도전 on/off (off면 보스 앞에서 직전 일반 스테이지 파밍).
- **푸시 루프**: Advance 중 사망 N회 → 자동 FarmLock 전환(스펙상 AutoRetreat의 변형으로 통합).
- 잘못된 `FarmLockStageIdx`(미해금/존재X)는 **현재 도달 최대 스테이지로 클램프**(회귀안전).

### ② 스킬 자동 전술
`USkillComponent::TickSkills`의 무지성 발동 직전에 **조건 게이트**를 삽입한다.
- 스킬별 규칙(`FSkillAutoRule`):
  - `Always`: 기존 동작(쿨다운만).
  - `BossEliteOnly`: 보스/엘리트 전투에서만 발동(궁극기 보존).
  - `HpBelow`: 자가 HP ≤ `HpThresholdPct`일 때만(힐/생존기).
  - `MaintainBuff`: 버프 미적용 시에만 재발동(상시 유지, 낭비 방지).
- `Priority`: 같은 틱에 여러 스킬 준비 시 우선순위 순으로 평가.
- 규칙 미설정 스킬은 `Always`(기존 동작) → **하위 호환**.
- 규칙 슬롯 수는 효율 업그레이드로 확장.

### ③ 자동 장비 관리
- **자동 장착** `bAutoEquipByPower`: 드랍/인벤 장비 중 부위별 전투력(`CombatPowerFormula` 재사용)이 더 높으면 자동 교체. 강화/잠재/룬 부여 장비는 보호(자동 교체 대상 제외 옵션).
- **최소 판매 기본 기능(신규)**: 드랍/인벤 장비를 **기존 드랍 골드 가치 곡선으로 환산 매각**. 수동 매각 + 자동 매각 둘 다 동작하는 1차 경제 루프.
- **자동 매각** `AutoSellFilters`: 설정 `MaxRarity` 이하 & `SlotMask` 일치 드랍을 자동 매각. 필터 슬롯 수는 업그레이드로 확장. 강화/룬/잠재 부여분·잠금 표시 아이템은 매각 제외(안전 가드).
- **자동 강화 제외**(비목표).

### ④ 자동 소비/버프
- `bAutoPotion` + `PotionHpThreshold`: HP ≤ 임계% 시 보유 회복 소비아이템 자동 사용(기존 consumable 시스템 재사용). 보유 0이면 무동작.
- `bAutoMaintainBuff`: 전투 진입/버프 만료 시 보유 버프 소비아이템 자동 재사용.
- 소비는 실보유 재화만 소모 → 죽은 재화 없음.

## 6. 해금 + 무한 효율 업그레이드

### 점진 해금 게이트 (예시, plan에서 수치 확정)
| 기능 | 해금 조건(예시) |
|---|---|
| ① 자동 진행 | 초반 기본 제공(튜토리얼 직후) |
| ② 스킬 자동 전술 | 챕터 3 클리어 |
| ③ 자동 장착 + 판매 | 챕터 5 클리어 |
| ③ 자동 매각 필터 | 자동 장착 해금 후 골드 구매 |
| ④ 자동 소비/버프 | 환생 1회 |

### 무한 효율 업그레이드 (기하급수 골드/실존재화 sink)
- 자동 진행 판정 가속(틱 주기 단축), 자동 매각 **필터 슬롯 +1**, 스킬 규칙 **슬롯 +1**, 자동 후퇴 판정 정밀도 등.
- 비용 곡선: `base * growth^level` (환생특성·펫 진화 sink 패턴 준용). **상한 없음**.
- 효율 보너스 비율은 환생특성처럼 `fround` 선형/기하로 서버 parity와 정합.

## 7. 데이터 흐름

```
[전투 틱]
  → AutomationPolicy 조회(로컬 세이브)
  → ① 진행 결정(전진/파밍/후퇴) + ② 스킬 게이트 + ④ 소비 트리거
[클리어/드랍 이벤트]
  → ③ 자동 장착 비교 + 자동 매각 필터 → 인벤/골드 반영
[설정 변경/업그레이드 구매]
  → AutomationPolicy 갱신 → 세이브 → (서버는 parity 수치만 독립 계산)
```

## 8. 오류 처리 / 안전 가드
- 모든 정책 값은 로드 시 **안전 기본값으로 클램프**(미해금 기능 OFF, 음수/범위초과 0가드 — 환생특성 패턴).
- 자동 매각은 강화/룬/잠재/잠금 아이템 **제외 화이트리스트 가드**(사고 매각 방지).
- 자동 장착은 더 낮은 전투력으로 교체 금지.
- **실존재화만** 사용. 보상/비용에 게임에 없는 재화 등장 금지(서브에이전트 죽은 재화 발명 경향 차단).

## 9. 테스트 / 게이트
- `server/src/core/formulas/automation.test.ts` — 해금 임계·업그레이드 비용/효율 곡선 단위 테스트.
- `automation.parity.test.ts` — 클라 미러 값과 서버 parity 일치.
- UE `AutomationTests` — 틱 행동 결정(모드별/스킬규칙별), 해금 게이트, 자동 매각 가드, **세이브 round-trip(SaveVer 26)**.
- `tools/ci/ue-automation.ps1` 표준 jumbo 빌드 + 전체 `IdleProject` Automation 게이트(HUD 영향 → `IdleProject.UI.HUD` 포함) GREEN.
- SaveVer stale 일괄 점검(누적 stale 방지).

## 10. 구현 단계화 (plan에서 PR로 순서화)

"전체"라 골격이 크므로 멀티시스템 충실 슬라이스 4개로 단계화한다.

- **P1 — 골격 + 자동 진행**: `FAutomationPolicySave`, SaveVer 26, 서버 parity 모듈, 해금/업그레이드 프레임, HUD 패널 골격, ① 자동 진행 정책.
- **P2 — 스킬 자동 전술**: `TickSkills` 조건 게이트, `FSkillAutoRule`, HUD 스킬 탭.
- **P3 — 자동 장비 + 최소 판매**: 판매 기본 기능 신규, 자동 장착, 자동 매각 필터, HUD 장비 탭.
- **P4 — 자동 소비/버프 + 무한 효율 업그레이드 패널**: ④ 소비 자동화, 업그레이드 구매 UI/곡선 완성.

각 P는 세이브/서버 parity/HUD/테스트를 포함하는 독립 충실 슬라이스이며, 직전 P 위에 누적된다.

## 11. 미해결/후속
- **자동 강화**(비목표): 효율 업그레이드 해금 후, 골드 예산 상한·성공확률 곡선 갖춘 별도 스펙.
- 자동전투 **프리셋/로드아웃**(접근법 C): P1~P4 위에 "이름 붙은 정책 묶음 전환"으로 얹기 쉬움 — 백로그.
- 해금 임계값·비용 곡선 구체 수치는 plan/구현에서 밸런싱.
