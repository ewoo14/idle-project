#pragma once

#include "CoreMinimal.h"
#include "ItemSystem/ItemTypes.h"

struct IDLEPROJECT_API FPotentialFormula
{
	static constexpr float RankCubeUpgradeChance = 0.08f;

	static EPotentialGrade GetMaxPotentialGrade(EItemRarity Rarity);
	static int32 GetPotentialLineCount(EPotentialGrade Grade);
	static void GetPotentialRollRange(EPotentialGrade Grade, float& OutMin, float& OutMax);
	static TArray<FPotentialLine> RollPotentialLines(EPotentialGrade Grade, FRandomStream& Rng);
	static EPotentialGrade ApplyRankCube(EPotentialGrade CurrentGrade, EPotentialGrade MaxGrade, FRandomStream& Rng, TArray<FPotentialLine>& OutLines);
};
