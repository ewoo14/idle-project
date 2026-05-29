#include "GameCore/PetService.h"

#include "GameCore/PetLevelFormula.h"

void UPetService::InitializeDefaultPets()
{
	BuildDefaultDefinitions();
	OwnedPetIds.Empty();
	PetLevels.Empty();
	PetStars.Empty();
	OwnedPetIds.Add(TEXT("dog"));
	OwnedPetIds.Add(TEXT("bird"));
	PetLevels.FindOrAdd(TEXT("dog"), 0);
	PetLevels.FindOrAdd(TEXT("bird"), 0);

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

bool UPetService::TryUnlockPet(const FString& PetId)
{
	if (!DefinitionById.Contains(PetId))
	{
		return false;
	}

	OwnedPetIds.Add(PetId);
	PetLevels.FindOrAdd(PetId, 0);
	return true;
}

bool UPetService::IsPetOwned(const FString& PetId) const
{
	return OwnedPetIds.Contains(PetId);
}

int32 UPetService::GetPetLevel(const FString& PetId) const
{
	const int32* Level = PetLevels.Find(PetId);
	return Level ? *Level : 0;
}

int32 UPetService::GetPetStar(const FString& PetId) const
{
	const int32* Star = PetStars.Find(PetId);
	return Star ? *Star : 0;
}

void UPetService::RestoreState(const FString& PetId, const TMap<FString, int32>& Levels)
{
	RestoreState(PetId, TSet<FString>(), Levels);
}

void UPetService::RestoreState(const FString& PetId, const TSet<FString>& InOwnedPetIds, const TMap<FString, int32>& Levels)
{
	RestoreState(PetId, InOwnedPetIds, Levels, TMap<FString, int32>());
}

void UPetService::RestoreState(const FString& PetId, const TSet<FString>& InOwnedPetIds, const TMap<FString, int32>& Levels, const TMap<FString, int32>& Stars)
{
	InitializeDefaultPets();
	if (!InOwnedPetIds.IsEmpty())
	{
		OwnedPetIds.Empty();
		for (const FString& OwnedPetId : InOwnedPetIds)
		{
			if (DefinitionById.Contains(OwnedPetId))
			{
				OwnedPetIds.Add(OwnedPetId);
				PetLevels.FindOrAdd(OwnedPetId, 0);
			}
		}
		OwnedPetIds.Add(TEXT("dog"));
		OwnedPetIds.Add(TEXT("bird"));
		PetLevels.FindOrAdd(TEXT("dog"), 0);
		PetLevels.FindOrAdd(TEXT("bird"), 0);
	}

	for (const TPair<FString, int32>& Pair : Levels)
	{
		if (OwnedPetIds.Contains(Pair.Key) && DefinitionById.Contains(Pair.Key))
		{
			PetLevels.FindOrAdd(Pair.Key) = FMath::Clamp(Pair.Value, 0, FPetLevelFormula::MaxPetLevel);
		}
	}

	for (const TPair<FString, int32>& Pair : Stars)
	{
		if (OwnedPetIds.Contains(Pair.Key) && DefinitionById.Contains(Pair.Key))
		{
			// 별은 무한 성장(상한 없음). 음수만 0성으로 가드.
			PetStars.FindOrAdd(Pair.Key) = FMath::Max(0, Pair.Value);
		}
	}

	if (OwnedPetIds.Contains(PetId) && DefinitionById.Contains(PetId))
	{
		EquippedPetId = PetId;
	}
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

bool UPetService::EvolvePet(const FString& PetId)
{
	if (!OwnedPetIds.Contains(PetId) || !DefinitionById.Contains(PetId))
	{
		return false;
	}

	int32& Star = PetStars.FindOrAdd(PetId, 0);
	++Star;
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

float UPetService::GetEquippedPetExpBonusPercent() const
{
	const FPetDefinition* Pet = GetEquippedPetDefinition();
	return Pet && Pet->BonusType == EPetBonusType::Exp
		? Pet->BonusPercent * FPetLevelFormula::GetBonusMultiplier(GetPetLevel(EquippedPetId))
		: 0.0f;
}

FPetStatBonus UPetService::GetEquippedPetStatBonus() const
{
	FPetStatBonus Bonus;
	const FPetDefinition* Pet = GetEquippedPetDefinition();
	if (!Pet)
	{
		return Bonus;
	}

	// 최종 보너스 = (정의 percent × 레벨 배수) × 별 배수. 별 배수는 장착 펫만, 여기 1곳에서만 곱(이중 계산 금지).
	const float StarMultiplier = FPetLevelFormula::GetPetStarMultiplier(GetPetStar(EquippedPetId));
	const float Ratio = Pet->BonusPercent * FPetLevelFormula::GetBonusMultiplier(GetPetLevel(EquippedPetId)) * StarMultiplier / 100.0f;
	switch (Pet->BonusType)
	{
	case EPetBonusType::PhysAtk:
		Bonus.PhysAtkPct = Ratio;
		break;
	case EPetBonusType::MagicAtk:
		Bonus.MagicAtkPct = Ratio;
		break;
	case EPetBonusType::Hp:
		Bonus.HpPct = Ratio;
		break;
	case EPetBonusType::Def:
		Bonus.PhysDefPct = Ratio;
		Bonus.MagicDefPct = Ratio;
		break;
	case EPetBonusType::AllStat:
		Bonus.PhysAtkPct = Ratio;
		Bonus.MagicAtkPct = Ratio;
		Bonus.PhysDefPct = Ratio;
		Bonus.MagicDefPct = Ratio;
		break;
	default:
		break;
	}
	return Bonus;
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
	AddDefinition(TEXT("cat"), TEXT("Cat"), EPetBonusType::Exp, 15.0f);
	AddDefinition(TEXT("wolf"), TEXT("Wolf"), EPetBonusType::PhysAtk, 10.0f);
	AddDefinition(TEXT("owl"), TEXT("Owl"), EPetBonusType::MagicAtk, 10.0f);
	AddDefinition(TEXT("bear"), TEXT("Bear"), EPetBonusType::Hp, 12.0f);
	AddDefinition(TEXT("turtle"), TEXT("Turtle"), EPetBonusType::Def, 12.0f);
	AddDefinition(TEXT("fox"), TEXT("Fox"), EPetBonusType::Gold, 30.0f);
	AddDefinition(TEXT("rabbit"), TEXT("Rabbit"), EPetBonusType::Drop, 25.0f);
	AddDefinition(TEXT("dragon"), TEXT("Dragon"), EPetBonusType::AllStat, 8.0f);
}

const FPetDefinition* UPetService::GetEquippedPetDefinition() const
{
	return DefinitionById.Find(EquippedPetId);
}
