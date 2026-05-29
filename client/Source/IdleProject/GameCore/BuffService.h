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

	/** 특정 등급의 보유 수량입니다. */
	UFUNCTION(BlueprintPure, Category = "Idle|Consumable")
	int32 GetCount(EConsumableType Type, EConsumableGrade Grade) const;

	/** 타입의 모든 등급 보유 수량 합계입니다(등급 미지정 UI 호환용). */
	UFUNCTION(BlueprintPure, Category = "Idle|Consumable")
	int32 GetTotalCount(EConsumableType Type) const;

	/** 등급 미지정 호환 오버로드: 모든 등급 보유 합계를 반환합니다. */
	int32 GetCount(EConsumableType Type) const;

	void AddConsumable(EConsumableType Type, EConsumableGrade Grade, int32 Amount);

	/** 등급 미지정 호환 오버로드: Standard 등급으로 추가합니다. */
	void AddConsumable(EConsumableType Type, int32 Amount);

	bool UseConsumable(EConsumableType Type, EConsumableGrade Grade, int64 NowUnixSec);

	/** 등급 미지정 호환 오버로드: Standard 등급을 우선 사용하고 없으면 보유 등급을 사용합니다. */
	bool UseConsumable(EConsumableType Type, int64 NowUnixSec);

	UFUNCTION(BlueprintPure, Category = "Idle|Consumable")
	bool IsBuffActive(EConsumableType Type, int64 NowUnixSec) const;

	/** 현재 활성 버프의 등급입니다(비활성 시 Standard 반환). */
	UFUNCTION(BlueprintPure, Category = "Idle|Consumable")
	EConsumableGrade GetActiveGrade(EConsumableType Type, int64 NowUnixSec) const;

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
	struct FBuffState
	{
		int64 EndUnixSec = 0;
		EConsumableGrade ActiveGrade = EConsumableGrade::Standard;
	};

	/** (Type<<8 | Grade) 키로 등급별 수량을 보관합니다. */
	static uint32 MakeCountKey(EConsumableType Type, EConsumableGrade Grade);

	TMap<uint32, int32> Counts;
	TMap<EConsumableType, FBuffState> BuffStateByType;
};
