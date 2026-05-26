#include "GameCore/PetService.h"

#include "GameCore/PetLevelFormula.h"

void UPetService::InitializeDefaultPets()
{
	BuildDefaultDefinitions();
	OwnedPetIds.Empty();
	for (const FPetDefinition& Definition : Definitions)
	{
		OwnedPetIds.Add(Definition.PetId);
		PetLevels.FindOrAdd(Definition.PetId, 0);
	}

	if (EquippedPetId.IsEmpty() && Definitions.Num() > 0)
	{
		EquippedPetId = Definitions[0].PetId;
	}
}

bool UPetService::EquipPet(const FString& PetId)
{
	if (!OwnedPetIds.Contains(PetId) || !DefinitionById.Contains(PetId))
	{
		return false;
	}

	EquippedPetId = PetId;
	return true;
}

int32 UPetService::GetPetLevel(const FString& PetId) const
{
	const int32* Level = PetLevels.Find(PetId);
	return Level ? *Level : 0;
}

bool UPetService::FeedPet(const FString& PetId)
{
	if (!OwnedPetIds.Contains(PetId) || !DefinitionById.Contains(PetId))
	{
		return false;
	}

	int32& Level = PetLevels.FindOrAdd(PetId, 0);
	if (Level >= FPetLevelFormula::MaxPetLevel)
	{
		return false;
	}

	++Level;
	return true;
}

float UPetService::GetEquippedPetGoldBonusPercent() const
{
	const FPetDefinition* Pet = GetEquippedPetDefinition();
	return Pet && Pet->BonusType == EPetBonusType::Gold
		? Pet->BonusPercent * FPetLevelFormula::GetBonusMultiplier(GetPetLevel(EquippedPetId))
		: 0.0f;
}

float UPetService::GetEquippedPetDropBonusPercent() const
{
	const FPetDefinition* Pet = GetEquippedPetDefinition();
	return Pet && Pet->BonusType == EPetBonusType::Drop
		? Pet->BonusPercent * FPetLevelFormula::GetBonusMultiplier(GetPetLevel(EquippedPetId))
		: 0.0f;
}

int64 UPetService::ApplyGoldBonus(int64 BaseAmount) const
{
	if (BaseAmount <= 0)
	{
		return 0;
	}

	const double Multiplier = 1.0 + static_cast<double>(GetEquippedPetGoldBonusPercent()) / 100.0;
	return FMath::FloorToInt64(static_cast<double>(BaseAmount) * Multiplier);
}

float UPetService::ApplyDropBonusChance(float BaseChance) const
{
	if (BaseChance <= 0.0f)
	{
		return 0.0f;
	}

	const float Multiplier = 1.0f + GetEquippedPetDropBonusPercent() / 100.0f;
	return FMath::Clamp(BaseChance * Multiplier, 0.0f, 1.0f);
}

void UPetService::BuildDefaultDefinitions()
{
	if (!Definitions.IsEmpty())
	{
		return;
	}

	auto AddDefinition = [this](const TCHAR* PetId, const TCHAR* Name, EPetBonusType BonusType, float BonusPercent)
	{
		FPetDefinition Definition;
		Definition.PetId = PetId;
		Definition.Name = FText::FromString(Name);
		Definition.BonusType = BonusType;
		Definition.BonusPercent = BonusPercent;
		Definitions.Add(Definition);
		DefinitionById.Add(Definition.PetId, Definition);
	};

	AddDefinition(TEXT("dog"), TEXT("Dog"), EPetBonusType::Gold, 20.0f);
	AddDefinition(TEXT("bird"), TEXT("Bird"), EPetBonusType::Drop, 15.0f);
}

const FPetDefinition* UPetService::GetEquippedPetDefinition() const
{
	return DefinitionById.Find(EquippedPetId);
}
