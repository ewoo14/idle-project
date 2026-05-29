#pragma once

#include "CoreMinimal.h"
#include "GameCore/ConsumableTypes.h"
#include "UObject/Object.h"
#include "BuffService.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBuffActivated, EConsumableType, Type, int64, EndUnixSec);

UCLASS(BlueprintType)
class IDLEPROJECT_API UBuffService : public UObject
{
	GENERATED_BODY()

public:
	void Initialize();

	UFUNCTION(BlueprintPure, Category = "Idle|Consumable")
	int32 GetCount(EConsumableType Type) const;

	UFUNCTION(BlueprintCallable, Category = "Idle|Consumable")
	void AddConsumable(EConsumableType Type, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Idle|Consumable")
	bool UseConsumable(EConsumableType Type, int64 NowUnixSec);

	UFUNCTION(BlueprintPure, Category = "Idle|Consumable")
	bool IsBuffActive(EConsumableType Type, int64 NowUnixSec) const;

	UFUNCTION(BlueprintPure, Category = "Idle|Consumable")
	int64 GetBuffRemainingSec(EConsumableType Type, int64 NowUnixSec) const;

	UFUNCTION(BlueprintPure, Category = "Idle|Consumable")
	float GetBuffStatMultiplier(EConsumableType Type, int64 NowUnixSec) const;

	UFUNCTION(BlueprintPure, Category = "Idle|Consumable")
	float GetGoldBuffPct(int64 NowUnixSec) const;

	UFUNCTION(BlueprintPure, Category = "Idle|Consumable")
	float GetExpBuffPct(int64 NowUnixSec) const;

	UFUNCTION(BlueprintPure, Category = "Idle|Consumable")
	float GetDropBuffAdd(int64 NowUnixSec) const;

	TArray<FConsumableSaveEntry> ExportSave() const;
	void ImportSave(const TArray<FConsumableSaveEntry>& Entries);

	UPROPERTY(BlueprintAssignable, Category = "Idle|Consumable")
	FOnBuffActivated OnBuffActivated;

private:
	TMap<EConsumableType, int32> Counts;
	TMap<EConsumableType, int64> BuffEndUnixSecByType;
};
