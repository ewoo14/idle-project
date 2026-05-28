#include "RuneSystem/RuneService.h"

#include "RuneSystem/RuneFormula.h"

URuneService::URuneService()
{
	EnsureSlotCount();
}

void URuneService::AddRune(const FRuneInstance& Rune)
{
	FRuneInstance Sanitized = Rune;
	Sanitized.EnhanceLevel = FMath::Max(0, Sanitized.EnhanceLevel);
	if (!IsValidRune(Sanitized))
	{
		return;
	}

	OwnedRunes.Add(Sanitized);
	EnsureSlotCount();
}

bool URuneService::TryEquipRune(int32 SlotIndex, int32 OwnedIndex)
{
	EnsureSlotCount();
	if (!EquippedSlots.IsValidIndex(SlotIndex) || !OwnedRunes.IsValidIndex(OwnedIndex))
	{
		return false;
	}

	for (int32& EquippedIndex : EquippedSlots)
	{
		if (EquippedIndex == OwnedIndex)
		{
			EquippedIndex = INDEX_NONE;
		}
	}

	EquippedSlots[SlotIndex] = OwnedIndex;
	return true;
}

bool URuneService::UnequipRune(int32 SlotIndex)
{
	EnsureSlotCount();
	if (!EquippedSlots.IsValidIndex(SlotIndex))
	{
		return false;
	}

	EquippedSlots[SlotIndex] = INDEX_NONE;
	return true;
}

bool URuneService::EnhanceRune(int32 OwnedIndex)
{
	if (!OwnedRunes.IsValidIndex(OwnedIndex))
	{
		return false;
	}

	++OwnedRunes[OwnedIndex].EnhanceLevel;
	return true;
}

bool URuneService::TryDisenchantRune(int32 OwnedIndex, int64& OutEssenceRefund)
{
	OutEssenceRefund = 0;
	if (!OwnedRunes.IsValidIndex(OwnedIndex))
	{
		return false;
	}

	EnsureSlotCount();
	if (EquippedSlots.Contains(OwnedIndex))
	{
		return false;
	}

	const FRuneInstance RemovedRune = OwnedRunes[OwnedIndex];
	OutEssenceRefund = FRuneFormula::GetDisenchantEssence(RemovedRune.Rarity, RemovedRune.EnhanceLevel);
	OwnedRunes.RemoveAt(OwnedIndex);

	for (int32& EquippedIndex : EquippedSlots)
	{
		if (EquippedIndex > OwnedIndex)
		{
			--EquippedIndex;
		}
		else if (EquippedIndex == OwnedIndex)
		{
			EquippedIndex = INDEX_NONE;
		}
	}
	return true;
}

FRuneCoreMultipliers URuneService::GetEquippedCoreMultipliers() const
{
	FRuneCoreMultipliers Result;
	for (const int32 OwnedIndex : EquippedSlots)
	{
		if (!OwnedRunes.IsValidIndex(OwnedIndex))
		{
			continue;
		}

		const FRuneInstance& Rune = OwnedRunes[OwnedIndex];
		const float Multiplier = 1.0f + FRuneFormula::GetCoreRuneMultiplier(Rune.Rarity, Rune.EnhanceLevel);
		switch (Rune.RuneType)
		{
		case ERuneType::PhysAtk:
			Result.PhysAtk *= Multiplier;
			break;
		case ERuneType::MagicAtk:
			Result.MagicAtk *= Multiplier;
			break;
		case ERuneType::PhysDef:
			Result.PhysDef *= Multiplier;
			break;
		case ERuneType::MagicDef:
			Result.MagicDef *= Multiplier;
			break;
		case ERuneType::Hp:
			Result.Hp *= Multiplier;
			break;
		default:
			break;
		}
	}
	return Result;
}

FRuneUtilValues URuneService::GetEquippedUtilValues() const
{
	FRuneUtilValues Result;
	for (const int32 OwnedIndex : EquippedSlots)
	{
		if (!OwnedRunes.IsValidIndex(OwnedIndex))
		{
			continue;
		}

		const FRuneInstance& Rune = OwnedRunes[OwnedIndex];
		const float Value = FRuneFormula::GetUtilRuneValue(Rune.RuneType, Rune.Rarity, Rune.EnhanceLevel);
		switch (Rune.RuneType)
		{
		case ERuneType::CritDamage:
			Result.CritDamage += Value;
			break;
		case ERuneType::GoldFind:
			Result.GoldFind += Value;
			break;
		case ERuneType::ExpBoost:
			Result.ExpBoost += Value;
			break;
		case ERuneType::OfflineEff:
			Result.OfflineEff += Value;
			break;
		default:
			break;
		}
	}

	Result.CritDamage = FMath::Min(Result.CritDamage, FRuneFormula::GetUtilCap(ERuneType::CritDamage));
	Result.GoldFind = FMath::Min(Result.GoldFind, FRuneFormula::GetUtilCap(ERuneType::GoldFind));
	Result.ExpBoost = FMath::Min(Result.ExpBoost, FRuneFormula::GetUtilCap(ERuneType::ExpBoost));
	Result.OfflineEff = FMath::Min(Result.OfflineEff, FRuneFormula::GetUtilCap(ERuneType::OfflineEff));
	return Result;
}

int32 URuneService::GetEquippedOwnedIndex(int32 SlotIndex) const
{
	return EquippedSlots.IsValidIndex(SlotIndex) ? EquippedSlots[SlotIndex] : INDEX_NONE;
}

void URuneService::CaptureState(TArray<FRuneSaveEntry>& OutRunes, TArray<int32>& OutEquippedSlots) const
{
	OutRunes.Reset();
	for (const FRuneInstance& Rune : OwnedRunes)
	{
		if (!IsValidRune(Rune))
		{
			continue;
		}

		FRuneSaveEntry Entry;
		Entry.RuneId = Rune.RuneId;
		Entry.RuneType = Rune.RuneType;
		Entry.Rarity = Rune.Rarity;
		Entry.EnhanceLevel = FMath::Max(0, Rune.EnhanceLevel);
		OutRunes.Add(Entry);
	}

	OutEquippedSlots = EquippedSlots;
	OutEquippedSlots.SetNum(FRuneFormula::RuneSlotCount);
	for (int32& EquippedIndex : OutEquippedSlots)
	{
		if (!OutRunes.IsValidIndex(EquippedIndex))
		{
			EquippedIndex = INDEX_NONE;
		}
	}
}

void URuneService::RestoreState(const TArray<FRuneSaveEntry>& InRunes, const TArray<int32>& InEquippedSlots)
{
	OwnedRunes.Reset();
	TMap<int32, int32> OldToNewIndex;
	for (int32 OldIndex = 0; OldIndex < InRunes.Num(); ++OldIndex)
	{
		const FRuneSaveEntry& Entry = InRunes[OldIndex];
		FRuneInstance Rune;
		Rune.RuneId = Entry.RuneId;
		Rune.RuneType = Entry.RuneType;
		Rune.Rarity = Entry.Rarity;
		Rune.EnhanceLevel = FMath::Max(0, Entry.EnhanceLevel);
		if (!IsValidRune(Rune))
		{
			continue;
		}

		OldToNewIndex.Add(OldIndex, OwnedRunes.Num());
		OwnedRunes.Add(Rune);
	}

	EquippedSlots.Init(INDEX_NONE, FRuneFormula::RuneSlotCount);
	for (int32 SlotIndex = 0; SlotIndex < FRuneFormula::RuneSlotCount && SlotIndex < InEquippedSlots.Num(); ++SlotIndex)
	{
		if (const int32* NewIndex = OldToNewIndex.Find(InEquippedSlots[SlotIndex]))
		{
			EquippedSlots[SlotIndex] = *NewIndex;
		}
	}
}

bool URuneService::IsValidRune(const FRuneInstance& Rune)
{
	return (FRuneFormula::IsCoreType(Rune.RuneType) || FRuneFormula::IsUtilType(Rune.RuneType))
		&& Rune.Rarity >= EItemRarity::Common
		&& Rune.Rarity <= EItemRarity::Mythic;
}

void URuneService::EnsureSlotCount()
{
	EquippedSlots.SetNum(FRuneFormula::RuneSlotCount);
	for (int32& EquippedIndex : EquippedSlots)
	{
		if (!OwnedRunes.IsValidIndex(EquippedIndex))
		{
			EquippedIndex = INDEX_NONE;
		}
	}
}
