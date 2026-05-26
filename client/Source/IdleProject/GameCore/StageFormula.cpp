#include "GameCore/StageFormula.h"

float FStageFormula::ComputeMonsterStatMultiplier(int32 GlobalStageIndex)
{
	const int32 ClampedIndex = FMath::Max(0, GlobalStageIndex);
	return 1.0f + static_cast<float>(ClampedIndex) * 0.15f;
}

float FStageFormula::ComputeRewardMultiplier(int32 GlobalStageIndex)
{
	return ComputeMonsterStatMultiplier(GlobalStageIndex);
}

bool FStageFormula::IsBossStage(int32 Chapter, int32 Stage, int32 StagesPerChapter)
{
	return Chapter > 0 && StagesPerChapter > 0 && Stage == StagesPerChapter;
}

ESkillElement FStageFormula::GetStageWeakElement(int32 GlobalStageIndex)
{
	switch (FMath::Max(0, GlobalStageIndex))
	{
	case 2:
		return ESkillElement::Ice;
	case 3:
		return ESkillElement::Holy;
	case 4:
		return ESkillElement::Fire;
	case 6:
		return ESkillElement::Lightning;
	case 7:
		return ESkillElement::Ice;
	case 8:
		return ESkillElement::Fire;
	case 9:
		return ESkillElement::Holy;
	default:
		return ESkillElement::None;
	}
}
