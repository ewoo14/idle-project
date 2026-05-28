#include "RuneSystem/RuneService.h"

#include "RuneSystem/ClassRuneFormula.h"
#include "RuneSystem/RuneCodexFormula.h"
#include "RuneSystem/RuneFormula.h"

URuneService::URuneService()
{
	EnsureSlotCount();
	EnsureCodexGrid();
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
	UnlockCodexCell(Sanitized.RuneType, Sanitized.Rarity);
	EnsureSlotCount();
}

bool URuneService::TryEquipRune(int32 SlotIndex, int32 OwnedIndex)
{
	EnsureSlotCount();
	if (!EquippedSlots.IsValidIndex(SlotIndex) || !OwnedRunes.IsValidIndex(OwnedIndex))
	{
		return false;
	}

	const FRuneInstance& Rune = OwnedRunes[OwnedIndex];
	if (SlotIndex == FClassRuneFormula::ClassRuneSlotIndex)
	{
		if (Rune.RuneType != ERuneType::ClassMastery || Rune.ClassRestriction != OwnerClassId || OwnerClassId == EClassId::None)
		{
			return false;
		}
	}
	else if (Rune.RuneType == ERuneType::ClassMastery)
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

void URuneService::SetOwnerClassId(EClassId ClassId)
{
	OwnerClassId = ClassId >= EClassId::Warrior && ClassId <= EClassId::Summoner ? ClassId : EClassId::None;
	EnsureSlotCount();
	const int32 ClassSlotOwnedIndex = EquippedSlots[FClassRuneFormula::ClassRuneSlotIndex];
	if (OwnerClassId == EClassId::None
		|| !OwnedRunes.IsValidIndex(ClassSlotOwnedIndex)
		|| OwnedRunes[ClassSlotOwnedIndex].RuneType != ERuneType::ClassMastery
		|| OwnedRunes[ClassSlotOwnedIndex].ClassRestriction != OwnerClassId)
	{
		EquippedSlots[FClassRuneFormula::ClassRuneSlotIndex] = INDEX_NONE;
	}
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
		if (Rune.RuneType == ERuneType::ClassMastery)
		{
			if (Rune.ClassRestriction != OwnerClassId || OwnerClassId == EClassId::None)
			{
				continue;
			}

			const FRuneCoreMultipliers Bonus = FClassRuneFormula::GetClassMasteryMultipliers(Rune.ClassRestriction, Rune.Rarity, Rune.EnhanceLevel);
			Result.PhysAtk += Bonus.PhysAtk;
			Result.MagicAtk += Bonus.MagicAtk;
			Result.PhysDef += Bonus.PhysDef;
			Result.MagicDef += Bonus.MagicDef;
			Result.Hp += Bonus.Hp;
			continue;
		}

		const float Bonus = FRuneFormula::GetCoreRuneMultiplier(Rune.Rarity, Rune.EnhanceLevel);
		switch (Rune.RuneType)
		{
		case ERuneType::PhysAtk:
			Result.PhysAtk += Bonus;
			break;
		case ERuneType::MagicAtk:
			Result.MagicAtk += Bonus;
			break;
		case ERuneType::PhysDef:
			Result.PhysDef += Bonus;
			break;
		case ERuneType::MagicDef:
			Result.MagicDef += Bonus;
			break;
		case ERuneType::Hp:
			Result.Hp += Bonus;
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

	const float CapExtension = GetCodexBonus().UtilCapExtension;
	Result.CritDamage = FMath::Min(Result.CritDamage, FRuneFormula::GetUtilCap(ERuneType::CritDamage) + CapExtension);
	Result.GoldFind = FMath::Min(Result.GoldFind, FRuneFormula::GetUtilCap(ERuneType::GoldFind) + CapExtension);
	Result.ExpBoost = FMath::Min(Result.ExpBoost, FRuneFormula::GetUtilCap(ERuneType::ExpBoost) + CapExtension);
	Result.OfflineEff = FMath::Min(Result.OfflineEff, FRuneFormula::GetUtilCap(ERuneType::OfflineEff) + CapExtension);
	return Result;
}

int32 URuneService::GetEquippedOwnedIndex(int32 SlotIndex) const
{
	return EquippedSlots.IsValidIndex(SlotIndex) ? EquippedSlots[SlotIndex] : INDEX_NONE;
}

void URuneService::UnlockCodexCell(ERuneType Type, EItemRarity Rarity)
{
	if (!IsValidCodexCell(Type, Rarity))
	{
		return;
	}

	EnsureCodexGrid();
	const int32 CodexIndex = GetCodexIndex(Type, Rarity);
	if (OwnedCodex.IsValidIndex(CodexIndex))
	{
		OwnedCodex[CodexIndex].bUnlocked = true;
	}
}

FRuneCodexCompletion URuneService::GetCodexCompletion() const
{
	FRuneCodexCompletion Completion;
	Completion.TotalCells = FRuneCodexFormula::TotalCells;
	Completion.RowComplete.Init(false, 6);

	TSet<int32> UnlockedKeys;
	int32 CoreUnlocked = 0;
	int32 UtilUnlocked = 0;
	TArray<int32> RowUnlocked;
	RowUnlocked.Init(0, 6);

	for (const FRuneCodexEntry& Entry : OwnedCodex)
	{
		if (!Entry.bUnlocked || !IsValidCodexCell(Entry.RuneType, Entry.Rarity))
		{
			continue;
		}

		const int32 Key = GetCodexIndex(Entry.RuneType, Entry.Rarity);
		if (UnlockedKeys.Contains(Key))
		{
			continue;
		}

		UnlockedKeys.Add(Key);
		++Completion.UnlockedCells;
		++RowUnlocked[static_cast<int32>(Entry.Rarity) - 1];
		if (FRuneFormula::IsCoreType(Entry.RuneType))
		{
			++CoreUnlocked;
		}
		else if (FRuneFormula::IsUtilType(Entry.RuneType))
		{
			++UtilUnlocked;
		}
	}

	for (int32 RowIndex = 0; RowIndex < RowUnlocked.Num(); ++RowIndex)
	{
		Completion.RowComplete[RowIndex] = RowUnlocked[RowIndex] == 9;
	}
	Completion.bCoreCategoryComplete = CoreUnlocked == FRuneCodexFormula::CoreCategoryCells;
	Completion.bUtilCategoryComplete = UtilUnlocked == FRuneCodexFormula::UtilCategoryCells;
	return Completion;
}

FRuneCodexBonus URuneService::GetCodexBonus() const
{
	return FRuneCodexFormula::ComputeBonus(GetCodexCompletion());
}

void URuneService::CaptureState(TArray<FRuneSaveEntry>& OutRunes, TArray<int32>& OutEquippedSlots) const
{
	TArray<FRuneCodexEntry> IgnoredCodex;
	CaptureState(OutRunes, OutEquippedSlots, IgnoredCodex);
}

void URuneService::CaptureState(TArray<FRuneSaveEntry>& OutRunes, TArray<int32>& OutEquippedSlots, TArray<FRuneCodexEntry>& OutCodex) const
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
		Entry.ClassRestriction = Rune.ClassRestriction;
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

	OutCodex.Reset();
	for (const FRuneCodexEntry& Entry : OwnedCodex)
	{
		if (IsValidCodexCell(Entry.RuneType, Entry.Rarity))
		{
			OutCodex.Add(Entry);
		}
	}
}

void URuneService::RestoreState(const TArray<FRuneSaveEntry>& InRunes, const TArray<int32>& InEquippedSlots)
{
	RestoreState(InRunes, InEquippedSlots, {});
}

void URuneService::RestoreState(const TArray<FRuneSaveEntry>& InRunes, const TArray<int32>& InEquippedSlots, const TArray<FRuneCodexEntry>& InCodex)
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
		Rune.ClassRestriction = Entry.ClassRestriction;
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

	OwnedCodex = InCodex;
	EnsureCodexGrid();
}

bool URuneService::IsValidRune(const FRuneInstance& Rune)
{
	const bool bValidType = FRuneFormula::IsCoreType(Rune.RuneType)
		|| FRuneFormula::IsUtilType(Rune.RuneType)
		|| (Rune.RuneType == ERuneType::ClassMastery
			&& Rune.ClassRestriction >= EClassId::Warrior
			&& Rune.ClassRestriction <= EClassId::Summoner);
	return bValidType
		&& Rune.Rarity >= EItemRarity::Common
		&& Rune.Rarity <= EItemRarity::Mythic;
}

bool URuneService::IsValidCodexCell(ERuneType Type, EItemRarity Rarity)
{
	return (FRuneFormula::IsCoreType(Type) || FRuneFormula::IsUtilType(Type))
		&& Rarity >= EItemRarity::Common
		&& Rarity <= EItemRarity::Mythic;
}

int32 URuneService::GetCodexIndex(ERuneType Type, EItemRarity Rarity)
{
	return (static_cast<int32>(Type) - static_cast<int32>(ERuneType::PhysAtk)) * 6
		+ (static_cast<int32>(Rarity) - static_cast<int32>(EItemRarity::Common));
}

void URuneService::EnsureCodexGrid()
{
	TSet<int32> UnlockedKeys;
	for (const FRuneCodexEntry& Entry : OwnedCodex)
	{
		if (Entry.bUnlocked && IsValidCodexCell(Entry.RuneType, Entry.Rarity))
		{
			UnlockedKeys.Add(GetCodexIndex(Entry.RuneType, Entry.Rarity));
		}
	}

	OwnedCodex.Reset(FRuneCodexFormula::TotalCells);
	for (int32 TypeValue = static_cast<int32>(ERuneType::PhysAtk); TypeValue <= static_cast<int32>(ERuneType::OfflineEff); ++TypeValue)
	{
		for (int32 RarityValue = static_cast<int32>(EItemRarity::Common); RarityValue <= static_cast<int32>(EItemRarity::Mythic); ++RarityValue)
		{
			FRuneCodexEntry Entry;
			Entry.RuneType = static_cast<ERuneType>(TypeValue);
			Entry.Rarity = static_cast<EItemRarity>(RarityValue);
			Entry.bUnlocked = UnlockedKeys.Contains(GetCodexIndex(Entry.RuneType, Entry.Rarity));
			OwnedCodex.Add(Entry);
		}
	}
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
