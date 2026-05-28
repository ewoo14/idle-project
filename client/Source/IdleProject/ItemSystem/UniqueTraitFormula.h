#pragma once

#include "CoreMinimal.h"
#include "CharacterSystem/StatFormulas.h"
#include "ItemSystem/ItemTypes.h"

struct IDLEPROJECT_API FUniqueTraitCoreMultipliers
{
	float Hp = 0.0f;
	float PhysAtk = 0.0f;
	float MagicAtk = 0.0f;
	float PhysDef = 0.0f;
	float MagicDef = 0.0f;
};

struct IDLEPROJECT_API FUniqueTraitFormula
{
	static float GetTraitValue(EUniqueTrait Trait, EItemRarity Rarity);
	static bool RarityGrantsUnique(EItemRarity Rarity);
	static bool RarityGrantsTwoTraits(EItemRarity Rarity);
	static void RollUniqueTraits(EItemRarity Rarity, FRandomStream& Rng, EUniqueTrait& OutTrait1, EUniqueTrait& OutTrait2);
	static void AccumulateTraitBonus(const FItemInstance& Item, FDerivedStats& OutBonus);
	static void AccumulateTraitMultipliers(const FItemInstance& Item, FUniqueTraitCoreMultipliers& OutMultipliers);
	static void ApplyCoreMultipliers(FDerivedStats& InOutStats, const FUniqueTraitCoreMultipliers& Multipliers);
};
