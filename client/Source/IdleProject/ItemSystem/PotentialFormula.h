#pragma once

#include "CoreMinimal.h"
#include "ItemSystem/ItemTypes.h"

struct IDLEPROJECT_API FPotentialFormula
{
	static constexpr float RankCubeUpgradeChance = 0.08f;
	// 잠재 V2: Legendary→Transcendent 상승만 낮은 확률(무한 chase). 서버 RANK_CUBE_TRANSCENDENT_CHANCE parity.
	static constexpr float RankCubeTranscendentChance = 0.05f;

	static EPotentialGrade GetMaxPotentialGrade(EItemRarity Rarity);
	static int32 GetPotentialLineCount(EPotentialGrade Grade);
	static void GetPotentialRollRange(EPotentialGrade Grade, float& OutMin, float& OutMax);
	// 잠재 V2: 스탯별 값 범위(전투 8종 = 등급 기본, 신규 옵션 3종은 배수 적용). 서버 getPotentialStatRollRange parity.
	static void GetPotentialStatRollRange(EPotentialGrade Grade, EPotentialStat Stat, float& OutMin, float& OutMax);
	static TArray<FPotentialLine> RollPotentialLines(EPotentialGrade Grade, FRandomStream& Rng);
	static EPotentialGrade ApplyRankCube(EPotentialGrade CurrentGrade, EPotentialGrade MaxGrade, FRandomStream& Rng, TArray<FPotentialLine>& OutLines);
};
