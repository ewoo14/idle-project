#pragma once

#include "CoreMinimal.h"
#include "GameCore/AutomationTypes.h"
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

private:
	UPROPERTY()
	EProgressionMode Mode = EProgressionMode::Advance;

	UPROPERTY()
	int32 FarmLockStage = 1;

	UPROPERTY()
	bool bAutoBossChallenge = true;

	UPROPERTY()
	int32 PushDeathThreshold = 3;
};
