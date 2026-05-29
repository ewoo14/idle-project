#pragma once

#include "CoreMinimal.h"
#include "CharacterSystem/StatFormulas.h"
#include "GameCore/AchievementFormula.h"
#include "GameCore/ConsumableTypes.h"
#include "GameCore/DungeonTypes.h"
#include "GameCore/MasteryTypes.h"
#include "GameFramework/SaveGame.h"
#include "GameCore/QuestService.h"
#include "ItemSystem/ItemTypes.h"
#include "RuneSystem/RuneCodexTypes.h"
#include "RuneSystem/RuneTypes.h"
#include "IdleSaveGame.generated.h"

UCLASS()
class IDLEPROJECT_API UIdleSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 SaveVersion = 17;

	UPROPERTY()
	bool bHasSave = false;

	UPROPERTY()
	int64 Gold = 0;

	UPROPERTY()
	int32 CharacterLevel = 1;

	UPROPERTY()
	int64 CurrentExp = 0;

	UPROPERTY()
	int64 NextExp = 150;

	UPROPERTY()
	int32 RebirthCount = 0;

	UPROPERTY()
	int32 RebirthBonusPoints = 0;

	UPROPERTY()
	int32 TranscendCount = 0;

	UPROPERTY()
	int32 AvailableStatPoints = 0;

	UPROPERTY()
	FPrimaryStats AllocatedStats;

	UPROPERTY()
	bool bChapter1BossDefeated = false;

	UPROPERTY()
	int64 LastSeenUnixSec = 0;

	UPROPERTY()
	int32 StageChapter = 1;

	UPROPERTY()
	int32 StageStage = 1;

	UPROPERTY()
	int32 StageKillsThisStage = 0;

	UPROPERTY()
	bool bStageFinalChapterCleared = false;

	UPROPERTY()
	int32 StageHighestClearedChapter = 0;

	UPROPERTY()
	int32 TowerHighestFloor = 0;

	UPROPERTY()
	TArray<int32> DungeonEntriesUsed;

	UPROPERTY()
	FString DungeonDailyResetDate;

	UPROPERTY()
	FString EquippedPetId;

	UPROPERTY()
	TSet<FString> OwnedPetIds;

	UPROPERTY()
	TMap<FString, int32> PetLevels;

	UPROPERTY()
	TArray<FItemInstance> InventoryItems;

	UPROPERTY()
	TMap<EItemSlot, int32> EquippedSlotIndex;

	UPROPERTY()
	TMap<FName, int32> SkillRanks;

	UPROPERTY()
	int32 SkillPoints = 0;

	UPROPERTY()
	TArray<FQuestSaveEntry> Quests;

	UPROPERTY()
	FString QuestDailyResetDate;

	UPROPERTY()
	FString QuestWeeklyResetId;

	UPROPERTY()
	int32 SeasonId = 1;

	UPROPERTY()
	int32 SeasonTokens = 0;

	UPROPERTY()
	TArray<int32> SeasonClaimedTiers;

	UPROPERTY()
	TArray<FAchievementMetricSaveEntry> AchievementMetrics;

	UPROPERTY()
	TArray<FAchievementSaveEntry> Achievements;

	UPROPERTY()
	TArray<FName> AchievementUniqueItemIds;

	UPROPERTY()
	TArray<FMasterySaveEntry> Mastery;

	UPROPERTY()
	TArray<FConsumableSaveEntry> Consumables;

	UPROPERTY()
	FString WeeklyBossWeekId;

	UPROPERTY()
	int64 WeeklyBossDamage = 0;

	UPROPERTY()
	int32 WeeklyBossChallengesUsed = 0;

	UPROPERTY()
	int32 WeeklyBossClaimedMilestones = 0;

	UPROPERTY()
	TArray<FRuneSaveEntry> Runes;

	UPROPERTY()
	TArray<int32> EquippedRuneSlots;

	UPROPERTY()
	TArray<FRuneCodexEntry> RuneCodex;

	UPROPERTY()
	int64 RuneEssence = 0;

	UPROPERTY()
	int64 ProtectionScrolls = 0;

	UPROPERTY()
	int64 ResetCubes = 0;

	UPROPERTY()
	int64 RankCubes = 0;

	// ── 길드 캐시(SaveVer 17, PR-G1) — 서버 권위 스냅샷의 최소 캐시 ──────────────
	UPROPERTY()
	FString CachedGuildId;

	UPROPERTY()
	uint8 CachedGuildRank = 0;
};
