#include "GameCore/StageFormula.h"

float FStageFormula::ComputeMonsterStatMultiplier(int32 GlobalStageIndex)
{
	const int32 ProgressionStep = FMath::Max(0, GlobalStageIndex - 1);
	return 1.0f + static_cast<float>(ProgressionStep) * 0.15f;
}

float FStageFormula::ComputeRewardMultiplier(int32 GlobalStageIndex)
{
	return ComputeMonsterStatMultiplier(GlobalStageIndex);
}

bool FStageFormula::IsBossStage(int32 Chapter, int32 Stage, int32 StagesPerChapter)
{
	return Chapter > 0 && StagesPerChapter > 0 && Stage == StagesPerChapter;
}

bool FStageFormula::IsEliteStage(int32 Stage)
{
	return Stage == 5;
}

ESkillElement FStageFormula::GetStageWeakElement(int32 GlobalStageIndex)
{
	switch (FMath::Max(0, GlobalStageIndex))
	{
	case 1:
		return ESkillElement::Fire;
	case 2:
		return ESkillElement::Ice;
	case 3:
		return ESkillElement::Lightning;
	case 4:
		return ESkillElement::Holy;
	case 5:
		return ESkillElement::Dark;
	case 6:
		return ESkillElement::Fire;
	case 7:
		return ESkillElement::Ice;
	case 8:
		return ESkillElement::Lightning;
	case 9:
		return ESkillElement::Holy;
	case 10:
		return ESkillElement::Dark;
	case 11:
		return ESkillElement::Ice;
	case 12:
		return ESkillElement::Fire;
	case 13:
		return ESkillElement::Holy;
	case 14:
		return ESkillElement::Lightning;
	case 15:
		return ESkillElement::Dark;
	case 16:
		return ESkillElement::Ice;
	case 17:
		return ESkillElement::Fire;
	case 18:
		return ESkillElement::Holy;
	case 19:
		return ESkillElement::Lightning;
	case 20:
		return ESkillElement::Dark;
	case 21:
		return ESkillElement::Dark;
	case 22:
		return ESkillElement::Holy;
	case 23:
		return ESkillElement::Dark;
	case 24:
		return ESkillElement::Lightning;
	case 25:
		return ESkillElement::Dark;
	case 26:
		return ESkillElement::Fire;
	case 27:
		return ESkillElement::Dark;
	case 28:
		return ESkillElement::Ice;
	case 29:
		return ESkillElement::Dark;
	case 30:
		return ESkillElement::Holy;
	case 31:
		return ESkillElement::Lightning;
	case 32:
		return ESkillElement::Holy;
	case 33:
		return ESkillElement::Ice;
	case 34:
		return ESkillElement::Fire;
	case 35:
		return ESkillElement::Dark;
	case 36:
		return ESkillElement::Lightning;
	case 37:
		return ESkillElement::Holy;
	case 38:
		return ESkillElement::Ice;
	case 39:
		return ESkillElement::Fire;
	case 40:
		return ESkillElement::Dark;
	default:
		return ESkillElement::None;
	}
}
