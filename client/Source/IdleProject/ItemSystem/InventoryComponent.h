#pragma once

#include "CoreMinimal.h"
#include "CharacterSystem/StatFormulas.h"
#include "Components/ActorComponent.h"
#include "ItemSystem/EnhanceFormula.h"
#include "ItemSystem/ItemTypes.h"
#include "ItemSystem/UniqueTraitFormula.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquippedChanged, EItemSlot, Slot);

/** 잠재 V2: 장착 장비 잠재 줄에서 합산한 경제/전역 보너스(전투 8종 외 신규 옵션 3종). */
struct FEquippedPotentialEconomyBonus
{
	// AllStatPercent 합(예 0.05 = +5% 전역 스탯). RefreshDerivedStats 전역 배수 단일 지점에서 소비.
	float AllStatPercent = 0.0f;
	// GoldFindPercent 합. AddGold 골드 배수 단일 지점에서 소비.
	float GoldFindPercent = 0.0f;
	// DropRatePercent 합. 펫 Drop 보너스 집계 지점(ApplyEquippedPetDropBonusChance)에서 소비.
	float DropRatePercent = 0.0f;
};

UCLASS(ClassGroup = (Idle), meta = (BlueprintSpawnableComponent))
class IDLEPROJECT_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	UPROPERTY(BlueprintAssignable, Category = "Idle|Inventory")
	FOnEquippedChanged OnEquippedChanged;

	bool AddItem(const FItemInstance& NewItem);
	bool CanAddItem(const FItemInstance& NewItem) const;
	int32 GetItemCount() const { return Items.Num(); }
	void EquipItem(int32 ItemIndex);
	void UnequipSlot(EItemSlot Slot);
	FDerivedStats ComputeEquipmentBonus() const;
	FUniqueTraitCoreMultipliers ComputeUniqueTraitMultipliers() const;
	// 잠재 V2: 장착 장비 잠재 신규 옵션(AllStat/Gold/Drop) 합산. 각 필드는 단일 소비 지점에서만 사용(이중 적용 금지 #72).
	FEquippedPotentialEconomyBonus ComputeEquippedPotentialEconomyBonus() const;
	const FItemInstance* GetEquippedItem(EItemSlot Slot) const;
	bool EnhanceEquippedItem(EItemSlot Slot);
	bool ApplyEnhanceOutcome(EItemSlot Slot, const FEnhanceAttemptOutcome& Outcome);
	bool SetItemLocked(EItemSlot Slot, bool bLocked);
	bool SetEquippedPotential(EItemSlot Slot, EPotentialGrade Grade, const TArray<FPotentialLine>& Lines);
	int32 GetEquippedEnhanceLevel(EItemSlot Slot) const;
	void CaptureState(TArray<FItemInstance>& OutItems, TMap<EItemSlot, int32>& OutEquipped) const;
	void RestoreState(const TArray<FItemInstance>& InItems, const TMap<EItemSlot, int32>& InEquipped);

private:
	static constexpr int32 MaxItems = 100;

	UPROPERTY()
	TArray<FItemInstance> Items;

	UPROPERTY()
	TMap<EItemSlot, int32> EquippedIndex;
};
