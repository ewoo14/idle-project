#pragma once

#include "CoreMinimal.h"
#include "CharacterSystem/StatFormulas.h"
#include "GameCore/AchievementFormula.h"
#include "GameCore/AutomationTypes.h"
#include "GameCore/ConsumableTypes.h"
#include "GameCore/DungeonTypes.h"
#include "GameCore/MasteryTypes.h"
#include "GameCore/RebirthPerkTypes.h"
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
	// SaveVer 28: 자동 장비 정책(자동 장착/자동 매각/매각 등급) 추가. <28 세이브는 정책 OFF/Common 마이그레이션.
	// SaveVer 27: 자동화 스킬 규칙(SkillRules) 추가. <27 세이브는 빈 규칙(전부 Always) 마이그레이션.
	UPROPERTY()
	int32 SaveVersion = 28;

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

	// 펫 진화 별 수치(SaveVer 20). PetLevels 와 동일 직렬화 방식. <20 세이브는 빈 맵(0성).
	UPROPERTY()
	TMap<FString, int32> PetStars;

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

	// ── 칭호(SaveVer 21) — 해금 id 집합 + 장착 1개. 클라 로컬 권위. <21 세이브는 빈 값(회귀 안전) ──
	UPROPERTY()
	TSet<FString> UnlockedTitleIds;

	UPROPERTY()
	FString EquippedTitleId;

	// ── 미션(SaveVer 22) — 일일/주간 미션 진행/수령/리셋 마커. 클라 로컬 권위. <22 세이브는 빈 값(회귀 안전) ──
	UPROPERTY()
	TMap<FString, int64> MissionProgress;

	UPROPERTY()
	TSet<FString> MissionClaimed;

	UPROPERTY()
	FString MissionDailyResetDate;

	UPROPERTY()
	FString MissionWeeklyResetWeek;

	// ── 출석 보상(SaveVer 23) — 누적 출석일/마지막 출석 UTC 날짜/수령 마일스톤 집합. 클라 로컬 권위. <23 세이브는 빈 값(회귀 안전) ──
	UPROPERTY()
	int64 AttendanceTotal = 0;

	UPROPERTY()
	FString LastAttendanceDate;

	UPROPERTY()
	TSet<int32> AttendanceClaimedMilestones;

	// ── 보물 상자(SaveVer 25) — 마지막 뽑기 UTC 날짜/누적 뽑기 횟수. 클라 로컬 권위. <25 세이브는 빈 값(회귀 안전) ──
	UPROPERTY()
	FString LastTreasureDrawDate;

	UPROPERTY()
	int64 TotalTreasureDraws = 0;

	// ── 환생 특성(SaveVer 24) — perk→분배 레벨 맵. 클라 로컬 권위(서버 rebirthPerk.ts parity).
	// 총 포인트는 RebirthCount 에서 파생(별도 저장 없음). <24 세이브는 빈 맵(회귀 안전) ──
	UPROPERTY()
	TMap<ERebirthPerk, int32> RebirthPerkAllocations;

	// 자동화 정책(P1: 자동 진행). SaveVer 26+. 클라 세이브 권위.
	UPROPERTY()
	EProgressionMode AutomationProgressionMode = EProgressionMode::Advance;

	UPROPERTY()
	int32 AutomationFarmLockStage = 1;

	UPROPERTY()
	bool bAutomationAutoBossChallenge = true;

	UPROPERTY()
	int32 AutomationPushDeathThreshold = 3;

	// 자동화 스킬 자동 전술 규칙(P2). SaveVer 27+. 클라 세이브 권위.
	UPROPERTY()
	TArray<FSkillAutoRule> AutomationSkillRules;

	// 자동 장비 정책(P3). SaveVer 28+. 클라 세이브 권위.
	UPROPERTY()
	bool bAutomationAutoEquipByPower = false;

	UPROPERTY()
	bool bAutomationAutoSell = false;

	UPROPERTY()
	EItemRarity AutomationAutoSellMaxRarity = EItemRarity::Common;

	// ── 길드 캐시(SaveVer 17, PR-G1) — 서버 권위 스냅샷의 최소 캐시 ──────────────
	UPROPERTY()
	FString CachedGuildId;

	UPROPERTY()
	uint8 CachedGuildRank = 0;

	// ── 길드 레벨/버프/기여 캐시(SaveVer 18, PR-G2) — 오프라인 버프 적용/델타 플러시 ──
	UPROPERTY()
	int32 CachedGuildLevel = 1;

	/** 길드 공격력 버프 비율(0.04 = +4%). 오프라인에도 캐시 적용. */
	UPROPERTY()
	float CachedGuildAttackPct = 0.0f;

	/** 길드 골드획득 버프 비율(0.04 = +4%). */
	UPROPERTY()
	float CachedGuildGoldPct = 0.0f;

	/** 내 개인 기여 포인트(상점 화폐). */
	UPROPERTY()
	int64 CachedContributionPoints = 0;

	/** 미플러시 자동 기여 델타(재접속 시 서버로 플러시). */
	UPROPERTY()
	int64 PendingAutoContribution = 0;

	/** 마지막 길드 출석 UTC 날짜(YYYY-MM-DD). */
	UPROPERTY()
	FString LastGuildAttendanceDate;

	// ── 길드 보스 진행 표시 캐시(SaveVer 19, PR-G3) — 서버 권위, 재접속 직후 표시용 ──
	/** 이번 주 누적 격파 횟수(서버 boss.defeatedCount 미러). 새 재화 아님. */
	UPROPERTY()
	int32 CachedBossDefeatedCount = 0;

	/** 내 이번 주 남은 보스 도전 횟수(서버 boss.challengesRemaining 미러). */
	UPROPERTY()
	int32 CachedBossChallengesRemaining = 0;
};
