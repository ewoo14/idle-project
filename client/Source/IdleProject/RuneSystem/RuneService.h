#pragma once

#include "CoreMinimal.h"
#include "RuneSystem/RuneCodexTypes.h"
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
	void SetOwnerClassId(EClassId ClassId);

	UFUNCTION(BlueprintCallable, Category = "Idle|Rune")
	bool UnequipRune(int32 SlotIndex);

	bool EnhanceRune(int32 OwnedIndex);
	bool TryDisenchantRune(int32 OwnedIndex, int64& OutEssenceRefund);

	FRuneCoreMultipliers GetEquippedCoreMultipliers() const;
	FRuneUtilValues GetEquippedUtilValues() const;
	const TArray<FRuneInstance>& GetOwnedRunes() const { return OwnedRunes; }
	const TArray<FRuneCodexEntry>& GetOwnedCodex() const { return OwnedCodex; }
	EClassId GetOwnerClassId() const { return OwnerClassId; }
	int32 GetEquippedOwnedIndex(int32 SlotIndex) const;
	void UnlockCodexCell(ERuneType Type, EItemRarity Rarity);
	FRuneCodexCompletion GetCodexCompletion() const;
	FRuneCodexBonus GetCodexBonus() const;

	void CaptureState(TArray<FRuneSaveEntry>& OutRunes, TArray<int32>& OutEquippedSlots) const;
	void CaptureState(TArray<FRuneSaveEntry>& OutRunes, TArray<int32>& OutEquippedSlots, TArray<FRuneCodexEntry>& OutCodex) const;
	void RestoreState(const TArray<FRuneSaveEntry>& InRunes, const TArray<int32>& InEquippedSlots);
	void RestoreState(const TArray<FRuneSaveEntry>& InRunes, const TArray<int32>& InEquippedSlots, const TArray<FRuneCodexEntry>& InCodex);

private:
	UPROPERTY()
	TArray<FRuneInstance> OwnedRunes;

	UPROPERTY()
	TArray<FRuneCodexEntry> OwnedCodex;

	UPROPERTY()
	TArray<int32> EquippedSlots;

	UPROPERTY()
	EClassId OwnerClassId = EClassId::None;

	static bool IsValidRune(const FRuneInstance& Rune);
	static bool IsValidCodexCell(ERuneType Type, EItemRarity Rarity);
	static int32 GetCodexIndex(ERuneType Type, EItemRarity Rarity);
	void EnsureCodexGrid();
	void EnsureSlotCount();
};
