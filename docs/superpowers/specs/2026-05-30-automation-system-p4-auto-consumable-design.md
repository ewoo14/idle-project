# 통합 자동화 시스템 P4 — 자동 버프 유지 + 무한 효율 업그레이드 설계 스펙

- 작성일: 2026-05-30
- 상태: 설계 승인됨 → plan 단계화
- 부모 스펙: `docs/superpowers/specs/2026-05-30-automation-system-design.md` (§5④, §6)
- 선행: P1(#99 `5f1fa38`)+P2(#100 `5994d98`)+P3(#101 `19e8148`). 브랜치 `feat/automation-system-p4`.
- **자동화 트랙 마지막 슬라이스 — 4기둥(진행/스킬/장비/소비) 완결 + 무한 효율 sink.**

## 1. 배경

P1~P3로 진행/스킬/장비 자동화가 들어왔다. 남은 4번째 기둥은 **자동 소비/버프**, 그리고 P1에서 이연한 **무한 효율 업그레이드**다.

소비아이템 6종(`EConsumableType`: AttackTonic/GuardTonic/AllStatElixir/FortuneScroll/GoldFeast/WisdomBooster)은 **전부 타임드 버프**다. `UBuffService`: `IsBuffActive(Type,Now)` / `GetTotalCount(Type)` / `UseConsumable(Type,Now)` / `GetBuffStatMultiplier` 보유. **HP 회복 포션 타입은 없다** → "HP 임계 자동 포션"은 본 슬라이스 범위 밖(회복 아이템 신설 시 후속).

## 2. 목표 / 비목표

### 목표
1. **자동 버프 유지**: `bAutoMaintainBuff` ON 시, 버프 만료 && 보유>0이면 자동 재사용. 처치 틱(`ScheduleRespawn`)에서 주기 점검.
2. **무한 효율 업그레이드**: P1 `efficiencyUpgradeCost`(기하급수 골드) 재사용. 단일 트랙 **자동 매각가 보너스**(매각가 ×(1+0.02×레벨), 무한).
3. `AutomationPolicy` 확장 + SaveVer 28→29(3곳). 해금 `AutoConsumable`=환생 1회(P1 게이트).
4. 자동화 패널 "소비/효율" 탭(데이터 구동).

### 비목표 (P4 제외)
- **HP 회복 자동 포션**: 회복 소비아이템 부재(신설은 별도 콘텐츠).
- 버프 타입별 선택 마스크(전체 유지로 단순화 — 후속 granularity).
- 자동 강화, 신규 효율 트랙 다수(단일 트랙만).

## 3. 데이터 모델

```
// FAutomationPolicySave (P1~P3) 확장 → SaveVer 28 → 29
bool  bAutoMaintainBuff = false;     // 자동 버프 유지
int32 SellValueUpgradeLevel = 0;     // 자동 매각가 보너스 레벨(무한)
```

## 4. parity 공식

### 매각가 보너스 (서버 `automation.ts` + 클라 `UAutomationPolicyService`)
```
sellValueMultiplier(level): number = 1 + 0.02 * max(0, trunc(level))   // 레벨당 +2%, 무한
```
### 업그레이드 비용 (P1 재사용)
```
SELL_UPGRADE_BASE = 50000, SELL_UPGRADE_GROWTH = 1.5
nextCost(level) = efficiencyUpgradeCost(SELL_UPGRADE_BASE, SELL_UPGRADE_GROWTH, level)  // 기하급수, 무한
```
- 클라 `GetSellValueMultiplier`/비용 1:1 미러(fround/round 정합). 음수 레벨 0가드.

## 5. 동작 / 통합

### ① 자동 버프 유지
- `GameInstance::MaintainBuffsIfEnabled()`: `bAutoMaintainBuff` ON일 때 6종 순회 — `!BuffService->IsBuffActive(Type, Now) && BuffService->GetTotalCount(Type) > 0`이면 `BuffService->UseConsumable(Type, Now)`(보유 등급 자동 선택 오버로드). 사용 시 스탯 갱신.
- `ScheduleRespawn`(GameMode, 처치마다)에서 `GameInstance->MaintainBuffsIfEnabled()` 호출. 보유 0이면 무동작.
- 미해금/OFF면 무동작(기존 수동 사용 불변).

### ② 무한 효율 업그레이드(자동 매각가 보너스)
- `GameInstance::UpgradeSellValue() -> bool`: `nextCost(level)` 계산, 골드 충분하면 `AddGold(-cost)` + `SellValueUpgradeLevel++`. BP 노출.
- 매각가 적용: **GameInstance 매각 지점**(수동 `SellInventoryItem`, 자동 `HandleDroppedEquipment` 자동 매각)에서 `FItemSellFormula::ComputeSellValue(...) × GetSellValueMultiplier(level)`로 골드 지급. `InventoryComponent::SellItem`은 정책 비의존 유지(베이스 반환), 곱은 GameInstance에서.
  - 주: P3 `SellInventoryItem`이 `Inv->SellItem`(베이스) 결과에 곱 적용하도록 수정. `HandleDroppedEquipment` 자동 매각도 곱 적용.

## 6. 세이브 / 해금 / HUD
- `AutomationPolicy` 2필드 + SaveVer 28→29(헤더 기본값 + `CaptureToSave` writer + 테스트 단언 **3곳**, P2 교훈). `RestoreState` 확장 + 직렬화.
- 해금 `AutoConsumable`(P1 `IsAutomationFeatureUnlocked`, 환생 1회). 효율 업그레이드는 골드 구매(해금 후).
- 자동화 패널 "소비/효율" 탭: 자동 버프 유지 토글 / 효율 레벨·다음 비용·업그레이드 버튼. 데이터 구동(신규 위젯 0).

## 7. 오류 처리 / 안전 가드
- 자동 버프 유지: 보유 0이면 무동작. 이미 활성이면 재사용 안 함(낭비 방지).
- 효율: 레벨 음수/골드 부족 가드. 매각가 곱은 [1,∞) (레벨 0=×1=기존).
- 구버전 세이브(<29)=정책 OFF/레벨 0=기존 동작.
- 실보유 소비·골드만. 신규 재화 없음.

## 8. 테스트 / 게이트
- 서버 `automation.test.ts`: `sellValueMultiplier`(레벨·가드) 단위.
- 클라 parity 회귀(`GetSellValueMultiplier`) + GameInstance: MaintainBuffsIfEnabled(만료→사용/활성→무동작/보유0→무동작), UpgradeSellValue(비용 차감·레벨++·골드부족 거부), 매각가 곱 적용.
- SaveVer 29 round-trip + **27/28→29 stale 일괄**(3곳).
- 표준 jumbo + 전체 Automation(`IdleProject.UI.HUD`/Consumable/Item 포함). 서버 biome clean.

## 9. 구현 단계화 (plan)
- 서버 `automation.ts sellValueMultiplier` + 테스트.
- 클라 `UAutomationPolicyService::GetSellValueMultiplier`/`SellUpgradeNextCost` + 정책 2필드 + 회귀.
- 세이브 SaveVer 29(3곳) + GameInstance(MaintainBuffsIfEnabled/UpgradeSellValue/매각가 곱/BP) + ScheduleRespawn 후크.
- SaveVer 27/28→29 stale 일괄 + HUD 소비/효율 탭 진입점.

## 10. 후속 (자동화 트랙 이후)
- HP 회복 소비아이템 신설 → HP 임계 자동 포션.
- 버프 타입별 유지 마스크, 효율 트랙 다수(진행 속도/필터 등).
- 자동 강화(골드 예산 상한·확률) 별도.
- 자동전투 프리셋/로드아웃(접근법 C, 4기둥 위에 얹기).
