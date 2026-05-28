#pragma once

#include "CoreMinimal.h"
#include "RuneSystem/RuneTypes.h"
#include "RuneService.generated.h"

UCLASS()
class IDLEPROJECT_API URuneService : public UObject
{
	GENERATED_BODY()

public:
	URuneService();

	UFUNCTION(BlueprintCallable, Category = "Idle|Rune")
	void AddRune(const FRuneInstance& Rune);

	UFUNCTION(BlueprintCallable, Category = "Idle|Rune")
	bool TryEquipRune(int32 SlotIndex, int32 OwnedIndex);

	UFUNCTION(BlueprintCallable, Category = "Idle|Rune")
	bool UnequipRune(int32 SlotIndex);

	bool EnhanceRune(int32 OwnedIndex);
	bool TryDisenchantRune(int32 OwnedIndex, int64& OutEssenceRefund);

	FRuneCoreMultipliers GetEquippedCoreMultipliers() const;
	FRuneUtilValues GetEquippedUtilValues() const;
	const TArray<FRuneInstance>& GetOwnedRunes() const { return OwnedRunes; }
	int32 GetEquippedOwnedIndex(int32 SlotIndex) const;

	void CaptureState(TArray<FRuneSaveEntry>& OutRunes, TArray<int32>& OutEquippedSlots) const;
	void RestoreState(const TArray<FRuneSaveEntry>& InRunes, const TArray<int32>& InEquippedSlots);

private:
	UPROPERTY()
	TArray<FRuneInstance> OwnedRunes;

	UPROPERTY()
	TArray<int32> EquippedSlots;

	static bool IsValidRune(const FRuneInstance& Rune);
	void EnsureSlotCount();
};
