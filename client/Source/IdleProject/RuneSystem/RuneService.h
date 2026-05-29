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

	/** 보유 룬 세트를 Offense/Bastion/Vitality/Fortune 중 균등 랜덤으로 재설정. 자원 검증/차감은 GameInstance가 수행. */
	bool RerollRuneSet(int32 OwnedIndex, FRandomStream& Rng);

	/**
	 * 보유 룬 등급 상승 시도. Mythic이거나 인덱스 무효이면 false(시도 불성립).
	 * 시도가 성립하면 true를 반환하고, SuccessChance 확률로 Rarity를 한 단계 올린다(bOutSucceeded로 결과 전달).
	 * RNG/확률은 GameInstance가 주입(테스트는 1/0 경계로 결정적 검증).
	 */
	bool TryUpgradeRuneRarity(int32 OwnedIndex, float SuccessChance, FRandomStream& Rng, bool& bOutSucceeded);

	/** Src 강화 레벨을 Dst로 전송(Dst=max(Dst,Src)) 후 Src 룬 삭제·장착 슬롯 인덱스 갱신. 자원 검증/차감은 GameInstance. */
	bool TransferEnhancement(int32 SrcIndex, int32 DstIndex);

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
