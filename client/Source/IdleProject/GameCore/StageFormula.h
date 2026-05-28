#pragma once

#include "CoreMinimal.h"
#include "CombatSystem/StatusElementTypes.h"

struct IDLEPROJECT_API FStageFormula
{
	static float ComputeMonsterStatMultiplier(int32 GlobalStageIndex);
	static float ComputeRewardMultiplier(int32 GlobalStageIndex);
	static bool IsBossStage(int32 Chapter, int32 Stage, int32 StagesPerChapter);
	static bool IsEliteStage(int32 Stage);
	static ESkillElement GetStageWeakElement(int32 GlobalStageIndex);
};
