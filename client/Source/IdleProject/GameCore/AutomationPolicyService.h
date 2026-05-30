#pragma once

#include "CoreMinimal.h"
#include "GameCore/AutomationTypes.h"
#include "ItemSystem/ItemTypes.h"
#include "UObject/Object.h"
#include "AutomationPolicyService.generated.h"

/**
 * 통합 자동화 정책 서비스(P1: 자동 진행). 정책 상태(모드/파밍고정/보스자동/후퇴임계)를
 * 보관하고, 클리어/사망 시 다음 행동을 결정한다(서버 automation.ts 결정 함수 1:1 미러).
 * 정책은 클라 로컬 세이브 권위. 해금 게이팅/효율 비용 곡선도 서버 parity static 으로 제공.
 * 음수/범위초과 입력은 안전 클램프(회귀안전, 환생특성 패턴).
 */
UCLASS(BlueprintType)
class IDLEPROJECT_API UAutomationPolicyService : public UObject
{
	GENERATED_BODY()

public:
	// ── 서버 parity static (automation.ts 1:1) ──

	// 클리어 직후 결정(decideOnClear).
	static FProgressionDecision DecideOnClear(
		EProgressionMode Mode,
		int32 ClearedGlobalStage,
		int32 HighestClearedGlobalStage,
		int32 FarmLockStage,
		bool bAutoBossChallenge,
		bool bNextIsBoss);

	// 사망 직후 결정(decideOnDeath).
	static FProgressionDecision DecideOnDeath(
		EProgressionMode Mode,
		int32 CurrentGlobalStage,
		int32 ConsecutiveDeaths,
		int32 DeathThreshold);

	// 기능 해금 여부(isFeatureUnlocked).
	static bool IsFeatureUnlocked(
		EAutomationFeature Feature,
		int32 HighestClearedChapter,
		int32 RebirthCount);

	// 효율 업그레이드 비용(efficiencyUpgradeCost). base*growth^level, 음수 level 0가드.
	static int64 EfficiencyUpgradeCost(int64 Base, float Growth, int32 Level);

	// 자동 매각가 효율 보너스(서버 sellValueMultiplier 1:1). 1 + 0.02×max(0,level).
	static float GetSellValueMultiplier(int32 Level);

	// 매각가 업그레이드 다음 비용(P1 EfficiencyUpgradeCost 재사용, base 50000 × 1.5^level).
	static int64 SellUpgradeNextCost(int32 Level);

	// 스킬 발동 규칙 평가(서버 evaluateSkillRule 1:1). hpThreshold/selfHpPct [0,1] 클램프.
	static bool EvaluateSkillRule(
		ESkillAutoCondition Condition,
		float HpThresholdPct,
		float SelfHpPct,
		bool bIsBossElite,
		bool bBuffActive);

	// ── 정책 상태(클라 세이브 권위) ──

	UFUNCTION(BlueprintPure, Category = "Idle|Automation")
	EProgressionMode GetMode() const { return Mode; }

	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void SetMode(EProgressionMode InMode) { Mode = InMode; }

	UFUNCTION(BlueprintPure, Category = "Idle|Automation")
	int32 GetFarmLockStage() const { return FarmLockStage; }

	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void SetFarmLockStage(int32 InStage) { FarmLockStage = FMath::Max(1, InStage); }

	UFUNCTION(BlueprintPure, Category = "Idle|Automation")
	bool GetAutoBossChallenge() const { return bAutoBossChallenge; }

	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void SetAutoBossChallenge(bool bValue) { bAutoBossChallenge = bValue; }

	UFUNCTION(BlueprintPure, Category = "Idle|Automation")
	int32 GetPushDeathThreshold() const { return PushDeathThreshold; }

	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void SetPushDeathThreshold(int32 InThreshold) { PushDeathThreshold = FMath::Max(1, InThreshold); }

	// 세이브 복원(클램프 적용). 호출 측이 SaveVer>=26 가드 후 호출.
	void RestoreState(EProgressionMode InMode, int32 InFarmLockStage, bool bInAutoBoss, int32 InThreshold);

	// ── 스킬 자동 전술 규칙(P2, 클라 세이브 권위) ──

	const TArray<FSkillAutoRule>& GetSkillRules() const { return SkillRules; }

	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void SetSkillRule(const FSkillAutoRule& Rule); // 같은 SkillId 있으면 교체, 없으면 추가

	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void ClearSkillRule(FName SkillId);

	void RestoreSkillRules(const TArray<FSkillAutoRule>& InRules);

	// ── 자동 장비 정책(P3, 클라 세이브 권위) ──

	UFUNCTION(BlueprintPure, Category = "Idle|Automation")
	bool GetAutoEquipByPower() const { return bAutoEquipByPower; }
	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void SetAutoEquipByPower(bool b) { bAutoEquipByPower = b; }

	UFUNCTION(BlueprintPure, Category = "Idle|Automation")
	bool GetAutoSell() const { return bAutoSell; }
	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void SetAutoSell(bool b) { bAutoSell = b; }

	UFUNCTION(BlueprintPure, Category = "Idle|Automation")
	EItemRarity GetAutoSellMaxRarity() const { return AutoSellMaxRarity; }
	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void SetAutoSellMaxRarity(EItemRarity R) { AutoSellMaxRarity = R; }

	void RestoreGearPolicy(bool bInAutoEquip, bool bInAutoSell, EItemRarity InMaxRarity);

	// ── 자동 소비/효율 정책(P4, 클라 세이브 권위) ──

	UFUNCTION(BlueprintPure, Category = "Idle|Automation")
	bool GetAutoMaintainBuff() const { return bAutoMaintainBuff; }
	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void SetAutoMaintainBuff(bool b) { bAutoMaintainBuff = b; }

	UFUNCTION(BlueprintPure, Category = "Idle|Automation")
	int32 GetSellValueUpgradeLevel() const { return SellValueUpgradeLevel; }
	void SetSellValueUpgradeLevel(int32 L) { SellValueUpgradeLevel = FMath::Max(0, L); }

	void RestoreConsumablePolicy(bool bInMaintain, int32 InSellLevel);

private:
	UPROPERTY()
	EProgressionMode Mode = EProgressionMode::Advance;

	UPROPERTY()
	int32 FarmLockStage = 1;

	UPROPERTY()
	bool bAutoBossChallenge = true;

	UPROPERTY()
	int32 PushDeathThreshold = 3;

	UPROPERTY()
	TArray<FSkillAutoRule> SkillRules;

	// 자동 장비 정책(P3). SaveVer 28+.
	UPROPERTY()
	bool bAutoEquipByPower = false;

	UPROPERTY()
	bool bAutoSell = false;

	UPROPERTY()
	EItemRarity AutoSellMaxRarity = EItemRarity::Common;

	// 자동 소비/효율 정책(P4). SaveVer 29+.
	UPROPERTY()
	bool bAutoMaintainBuff = false;

	UPROPERTY()
	int32 SellValueUpgradeLevel = 0;
};
