#include "GameCore/IdleGameInstance.h"

#include "CharacterSystem/IdleCharacter.h"
#include "CharacterSystem/LevelFormulas.h"
#include "CombatSystem/SkillComponent.h"
#include "GameCore/BuffService.h"
#include "GameCore/CloudSaveMergePolicy.h"
#include "GameCore/CloudSavePayloadMapper.h"
#include "GameCore/ConsumableFormula.h"
#include "GameCore/IdleSaveGame.h"
#include "GameCore/LeaderboardService.h"
#include "GameCore/MasteryService.h"
#include "GameCore/PetLevelFormula.h"
#include "GameCore/QuestService.h"
#include "GameCore/RebirthFormula.h"
#include "GameCore/RewardFormula.h"
#include "GameCore/TranscendFormula.h"
#include "GameCore/WeeklyBossFormula.h"
#include "GameCore/WeeklyBossService.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Internationalization/IdleLocalization.h"
#include "ItemSystem/EnhanceFormula.h"
#include "ItemSystem/InventoryComponent.h"
#include "ItemSystem/PotentialFormula.h"
#include "ItemSystem/ItemFactory.h"
#include "ItemSystem/RarityMigration.h"
#include "ItemSystem/ShopFormula.h"
#include "NetworkClient/ApiClient.h"
#include "RuneSystem/ClassRuneFormula.h"
#include "RuneSystem/RuneFormula.h"
#include "RuneSystem/RuneService.h"

namespace
{
FPrimaryStats ClampPrimaryStats(const FPrimaryStats& Stats)
{
	return FPrimaryStats(
		FMath::Max(0.0f, Stats.Str),
		FMath::Max(0.0f, Stats.Dex),
		FMath::Max(0.0f, Stats.Int_),
		FMath::Max(0.0f, Stats.Wis),
		FMath::Max(0.0f, Stats.Con),
		FMath::Max(0.0f, Stats.Luk));
}

TArray<FItemInstance> GetMigratedInventoryItems(const UIdleSaveGame& SaveGame)
{
	TArray<FItemInstance> Items = SaveGame.InventoryItems;
	if (SaveGame.SaveVersion < 7)
	{
		for (FItemInstance& Item : Items)
		{
			Item.Rarity = FRarityMigration::MigrateLegacy(static_cast<int32>(Item.Rarity));
		}
	}
	return Items;
}

TArray<FRuneSaveEntry> GetMigratedRunes(const UIdleSaveGame& SaveGame)
{
	TArray<FRuneSaveEntry> Runes = SaveGame.Runes;
	if (SaveGame.SaveVersion < 7)
	{
		for (FRuneSaveEntry& Rune : Runes)
		{
			Rune.Rarity = FRarityMigration::MigrateLegacy(static_cast<int32>(Rune.Rarity));
		}
	}
	return Runes;
}

TArray<FRuneCodexEntry> GetMigratedRuneCodex(const UIdleSaveGame& SaveGame)
{
	TArray<FRuneCodexEntry> Codex = SaveGame.RuneCodex;
	if (SaveGame.SaveVersion < 7)
	{
		for (FRuneCodexEntry& Entry : Codex)
		{
			Entry.Rarity = FRarityMigration::MigrateLegacy(static_cast<int32>(Entry.Rarity));
		}
	}
	return Codex;
}

}

const TCHAR* UIdleGameInstance::SaveSlotName = TEXT("IdleSave");

void UIdleGameInstance::Init()
{
	Super::Init();

	if (GConfig)
	{
		FString ConfigBaseUrl;
		if (GConfig->GetString(TEXT("/Script/IdleProject.IdleGameInstance"), TEXT("ServerBaseUrl"), ConfigBaseUrl, GEngineIni) && !ConfigBaseUrl.IsEmpty())
		{
			ApiBaseUrl = MoveTemp(ConfigBaseUrl);
		}
	}

	FString EnvBaseUrl = FPlatformMisc::GetEnvironmentVariable(TEXT("IDLE_API_BASE_URL"));
	if (!EnvBaseUrl.IsEmpty())
	{
		ApiBaseUrl = MoveTemp(EnvBaseUrl);
	}

	ApiClient = NewObject<UApiClient>(this);
	ApiClient->Initialize(ApiBaseUrl);
	EnsureQuestService();
	EnsurePetService();
	EnsureSeasonService();
	EnsureStageService();
	EnsureTowerService();
	EnsureDungeonService();
	EnsureRuneService();
	EnsureAchievementService();
	EnsureMasteryService();
	EnsureLeaderboardService();
	EnsureBuffService();
	EnsureWeeklyBossService();
	NextExp = FLevelFormulas::ExpToNext(CharacterLevel);
	EnhanceRandomStream.Initialize(FPlatformTime::Cycles());
	RuneRandomStream.Initialize(FPlatformTime::Cycles() ^ 0x51f15e);
	PotentialRandomStream.Initialize(FPlatformTime::Cycles() ^ 0x71e4d);
	LoadLanguage();
	LoadLastSeenUnixSec();
	LoadProgress();

	SyncFromCloud();
	ApiClient->RequestPetList();
	ApiClient->RequestSeasonState();
}

void UIdleGameInstance::Shutdown()
{
	LastSeenUnixSec = GetCurrentUnixSeconds();
	FlushPendingAutosave();
	SaveProgress();
	SaveLanguage();
	SaveLastSeenUnixSec();
	ApiClient = nullptr;
	QuestService = nullptr;
	PetService = nullptr;
	SeasonService = nullptr;
	StageService = nullptr;
	TowerService = nullptr;
	DungeonService = nullptr;
	RuneService = nullptr;
	AchievementService = nullptr;
	MasteryService = nullptr;
	LeaderboardService = nullptr;
	BuffService = nullptr;
	WeeklyBossService = nullptr;
	Super::Shutdown();
}

void UIdleGameInstance::SetLanguage(const FString& InLanguage)
{
	Language = IdleProject::Localization::NormalizeLanguage(InLanguage);
	IdleProject::Localization::SetCurrentLanguage(Language);
	SaveLanguage();
}

void UIdleGameInstance::AddGold(int64 Amount)
{
	if (Amount == 0)
	{
		return;
	}

	if (Amount < 0 && (Amount == MIN_int64 || Gold < -Amount))
	{
		RecordAchievementMetric(EAchievementMetric::GoldSpent, Gold);
		RecordQuestProgress(EQuestObjective::SpendGold, static_cast<int32>(FMath::Min<int64>(Gold, MAX_int32)));
		Gold = 0;
		OnGoldChanged.Broadcast(Gold);
		RequestAutosave();
		return;
	}

	int64 EffectiveAmount = Amount;
	if (Amount > 0)
	{
		EnsureBuffService();
		const double GoldMultiplier = 1.0 + static_cast<double>(BuffService ? BuffService->GetGoldBuffPct(GetCurrentUnixSeconds()) : 0.0f);
		EffectiveAmount = FMath::Max<int64>(0, FMath::RoundToInt64(static_cast<double>(Amount) * GoldMultiplier));
		if (Gold > MAX_int64 - EffectiveAmount)
		{
			Gold = MAX_int64;
			RecordAchievementMetric(EAchievementMetric::GoldEarned, EffectiveAmount);
			OnGoldChanged.Broadcast(Gold);
			RequestAutosave();
			return;
		}
	}

	Gold = Gold + EffectiveAmount;
	if (Amount > 0)
	{
		RecordAchievementMetric(EAchievementMetric::GoldEarned, EffectiveAmount);
	}
	else if (Amount < 0)
	{
		RecordAchievementMetric(EAchievementMetric::GoldSpent, -Amount);
		RecordQuestProgress(EQuestObjective::SpendGold, static_cast<int32>(FMath::Min<int64>(-Amount, MAX_int32)));
	}
	OnGoldChanged.Broadcast(Gold);
	RequestAutosave();
}

void UIdleGameInstance::SaveProgress()
{
	UIdleSaveGame* SaveGame = Cast<UIdleSaveGame>(UGameplayStatics::CreateSaveGameObject(UIdleSaveGame::StaticClass()));
	if (!CaptureToSave(SaveGame))
	{
		return;
	}

	if (UGameplayStatics::SaveGameToSlot(SaveGame, SaveSlotName, 0))
	{
		OnProgressSaved.Broadcast();
		if (!bCloudUploadSuppressed)
		{
			UploadToCloud();
		}
	}
}

void UIdleGameInstance::LoadProgress()
{
	if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		return;
	}

	UIdleSaveGame* SaveGame = Cast<UIdleSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
	ApplyFromSave(SaveGame);
}

void UIdleGameInstance::SyncFromCloud()
{
	if (!ApiClient)
	{
		SetCloudSyncState(ECloudSyncState::Offline);
		return;
	}

	UIdleSaveGame* LocalSave = NewObject<UIdleSaveGame>(this);
	if (!CaptureToSave(LocalSave))
	{
		SetCloudSyncState(ECloudSyncState::Offline);
		return;
	}

	FString LocalPayloadJson;
	FCloudSaveProgressSnapshot LocalSnapshot;
	if (!FCloudSavePayloadMapper::SaveToPayloadJson(*LocalSave, LocalPayloadJson)
		|| !FCloudSavePayloadMapper::ExtractSnapshot(LocalPayloadJson, LocalSnapshot))
	{
		SetCloudSyncState(ECloudSyncState::Offline);
		return;
	}

	SetCloudSyncState(ECloudSyncState::Syncing);
	TWeakObjectPtr<UIdleGameInstance> WeakThis(this);
	ApiClient->EnsureCharacter([WeakThis, LocalPayloadJson = MoveTemp(LocalPayloadJson), LocalSnapshot](bool bCharacterOk, FString CharacterId) mutable
	{
		UIdleGameInstance* StrongThis = WeakThis.Get();
		if (!StrongThis || !StrongThis->ApiClient)
		{
			return;
		}
		if (!bCharacterOk)
		{
			StrongThis->SetCloudSyncState(ECloudSyncState::Offline);
			return;
		}

		StrongThis->ApiClient->DownloadSave(CharacterId, [WeakThis, CharacterId, LocalPayloadJson = MoveTemp(LocalPayloadJson), LocalSnapshot](bool bDownloadOk, FString ServerPayloadJson) mutable
		{
			UIdleGameInstance* StrongThis = WeakThis.Get();
			if (!StrongThis || !StrongThis->ApiClient)
			{
				return;
			}
			if (!bDownloadOk)
			{
				StrongThis->SetCloudSyncState(ECloudSyncState::Offline);
				return;
			}

			if (ServerPayloadJson.IsEmpty())
			{
				StrongThis->ApiClient->UploadSave(CharacterId, UIdleGameInstance::CloudSaveApiVersion, LocalPayloadJson, [WeakThis](bool bUploadOk, FString)
				{
					if (UIdleGameInstance* StrongThis = WeakThis.Get())
					{
						StrongThis->SetCloudSyncState(bUploadOk ? ECloudSyncState::Synced : ECloudSyncState::Offline);
					}
				});
				return;
			}

			FCloudSaveProgressSnapshot ServerSnapshot;
			if (!FCloudSavePayloadMapper::ExtractSnapshot(ServerPayloadJson, ServerSnapshot))
			{
				StrongThis->SetCloudSyncState(ECloudSyncState::Offline);
				return;
			}

			if (FCloudSaveMergePolicy::Decide(LocalSnapshot, ServerSnapshot) == ECloudSaveMergeDecision::AdoptServer)
			{
				UIdleSaveGame* ServerSave = NewObject<UIdleSaveGame>(StrongThis);
				if (FCloudSavePayloadMapper::PayloadJsonToSave(ServerPayloadJson, *ServerSave) && StrongThis->ApplyFromSave(ServerSave))
				{
					TGuardValue<bool> CloudUploadGuard(StrongThis->bCloudUploadSuppressed, true);
					StrongThis->SaveProgress();
					StrongThis->SetCloudSyncState(ECloudSyncState::Synced);
				}
				else
				{
					StrongThis->SetCloudSyncState(ECloudSyncState::Conflict);
				}
				return;
			}

			StrongThis->SetCloudSyncState(ECloudSyncState::Conflict);
			StrongThis->ApiClient->UploadSave(CharacterId, UIdleGameInstance::CloudSaveApiVersion, LocalPayloadJson, [WeakThis](bool bUploadOk, FString)
			{
				if (UIdleGameInstance* StrongThis = WeakThis.Get())
				{
					StrongThis->SetCloudSyncState(bUploadOk ? ECloudSyncState::Synced : ECloudSyncState::Offline);
				}
			});
		});
	});
}

void UIdleGameInstance::UploadToCloud()
{
	if (!ApiClient || bCloudUploadSuppressed)
	{
		return;
	}

	UIdleSaveGame* LocalSave = NewObject<UIdleSaveGame>(this);
	FString PayloadJson;
	if (!CaptureToSave(LocalSave) || !FCloudSavePayloadMapper::SaveToPayloadJson(*LocalSave, PayloadJson))
	{
		SetCloudSyncState(ECloudSyncState::Offline);
		return;
	}

	SetCloudSyncState(ECloudSyncState::Syncing);
	TWeakObjectPtr<UIdleGameInstance> WeakThis(this);
	ApiClient->EnsureCharacter([WeakThis, PayloadJson = MoveTemp(PayloadJson)](bool bCharacterOk, FString CharacterId) mutable
	{
		UIdleGameInstance* StrongThis = WeakThis.Get();
		if (!StrongThis || !StrongThis->ApiClient)
		{
			return;
		}
		if (!bCharacterOk)
		{
			StrongThis->SetCloudSyncState(ECloudSyncState::Offline);
			return;
		}

		StrongThis->ApiClient->UploadSave(CharacterId, UIdleGameInstance::CloudSaveApiVersion, PayloadJson, [WeakThis](bool bUploadOk, FString)
		{
			if (UIdleGameInstance* StrongThis = WeakThis.Get())
			{
				StrongThis->SetCloudSyncState(bUploadOk ? ECloudSyncState::Synced : ECloudSyncState::Offline);
			}
		});
	});
}

void UIdleGameInstance::RefreshLeaderboard(ELeaderboardKind Kind)
{
	if (!ApiClient)
	{
		EnsureLeaderboardService();
		if (LeaderboardService)
		{
			LeaderboardService->ParseListJson(FString(), Kind);
			LeaderboardService->ParseMyRankJson(FString(), Kind);
		}
		return;
	}

	EnsureLeaderboardService();
	if (!LeaderboardService)
	{
		return;
	}

	if (Kind == ELeaderboardKind::WeeklyDamage)
	{
		const FString Week = UQuestService::GetCurrentUtcWeekString();
		TWeakObjectPtr<UIdleGameInstance> WeakWeeklyThis(this);
		ApiClient->FetchWeeklyDamageLeaderboard(Week, [WeakWeeklyThis](bool bSuccess, FString Body) mutable
		{
			if (UIdleGameInstance* StrongThis = WeakWeeklyThis.Get())
			{
				StrongThis->EnsureLeaderboardService();
				if (StrongThis->LeaderboardService)
				{
					StrongThis->LeaderboardService->ParseListJson(bSuccess ? Body : FString(), ELeaderboardKind::WeeklyDamage);
				}
			}
		});

		ApiClient->EnsureCharacter([WeakWeeklyThis, Week](bool bCharacterOk, FString CharacterId) mutable
		{
			UIdleGameInstance* StrongThis = WeakWeeklyThis.Get();
			if (!StrongThis)
			{
				return;
			}

			StrongThis->EnsureLeaderboardService();
			if (!StrongThis->ApiClient || !StrongThis->LeaderboardService)
			{
				return;
			}

			if (!bCharacterOk || CharacterId.IsEmpty())
			{
				StrongThis->LeaderboardService->ParseMyRankJson(FString(), ELeaderboardKind::WeeklyDamage);
				return;
			}

			StrongThis->ApiClient->FetchMyWeeklyRank(Week, CharacterId, [WeakWeeklyThis](bool bSuccess, FString Body) mutable
			{
				if (UIdleGameInstance* StrongThis = WeakWeeklyThis.Get())
				{
					StrongThis->EnsureLeaderboardService();
					if (StrongThis->LeaderboardService)
					{
						StrongThis->LeaderboardService->ParseMyRankJson(bSuccess ? Body : FString(), ELeaderboardKind::WeeklyDamage);
					}
				}
			});
		});
		return;
	}

	EnsureSeasonService();
	if (!SeasonService)
	{
		return;
	}

	const int32 SeasonId = SeasonService->GetSeasonId();
	TWeakObjectPtr<UIdleGameInstance> WeakThis(this);
	ApiClient->FetchLeaderboard(Kind, SeasonId, [WeakThis, Kind](bool bSuccess, FString Body) mutable
	{
		if (UIdleGameInstance* StrongThis = WeakThis.Get())
		{
			StrongThis->EnsureLeaderboardService();
			if (StrongThis->LeaderboardService)
			{
				StrongThis->LeaderboardService->ParseListJson(bSuccess ? Body : FString(), Kind);
			}
		}
	});

	ApiClient->EnsureCharacter([WeakThis, Kind, SeasonId](bool bCharacterOk, FString CharacterId) mutable
	{
		UIdleGameInstance* StrongThis = WeakThis.Get();
		if (!StrongThis)
		{
			return;
		}

		StrongThis->EnsureLeaderboardService();
		if (!StrongThis->ApiClient || !StrongThis->LeaderboardService)
		{
			return;
		}

		if (!bCharacterOk)
		{
			StrongThis->LeaderboardService->ParseMyRankJson(FString(), Kind);
			return;
		}

		StrongThis->ApiClient->FetchMyRank(Kind, SeasonId, CharacterId, [WeakThis, Kind](bool bSuccess, FString Body) mutable
		{
			if (UIdleGameInstance* StrongThis = WeakThis.Get())
			{
				StrongThis->EnsureLeaderboardService();
				if (StrongThis->LeaderboardService)
				{
					StrongThis->LeaderboardService->ParseMyRankJson(bSuccess ? Body : FString(), Kind);
				}
			}
		});
	});
}

bool UIdleGameInstance::CaptureToSave(UIdleSaveGame* SaveGame)
{
	if (!SaveGame)
	{
		return false;
	}

	EnsureStageService();
	EnsureTowerService();
	EnsureDungeonService();
	EnsureRuneService();
	EnsurePetService();
	EnsureQuestService();
	EnsureSeasonService();
	EnsureAchievementService();
	EnsureMasteryService();
	EnsureBuffService();
	EnsureWeeklyBossService();

	SaveGame->SaveVersion = 16;
	SaveGame->bHasSave = true;
	SaveGame->Gold = Gold;
	SaveGame->RuneEssence = RuneEssence;
	SaveGame->ProtectionScrolls = ProtectionScrolls;
	SaveGame->ResetCubes = ResetCubes;
	SaveGame->RankCubes = RankCubes;
	SaveGame->CharacterLevel = CharacterLevel;
	SaveGame->CurrentExp = CurrentExp;
	SaveGame->NextExp = NextExp;
	SaveGame->RebirthCount = RebirthCount;
	SaveGame->RebirthBonusPoints = RebirthBonusPoints;
	SaveGame->TranscendCount = TranscendCount;
	SaveGame->AvailableStatPoints = AvailableStatPoints;
	SaveGame->AllocatedStats = ClampPrimaryStats(AllocatedStats);
	SaveGame->bChapter1BossDefeated = bChapter1BossDefeated;
	SaveGame->LastSeenUnixSec = LastSeenUnixSec;

	if (StageService)
	{
		SaveGame->StageChapter = StageService->GetCurrentChapter();
		SaveGame->StageStage = StageService->GetCurrentStage();
		SaveGame->StageKillsThisStage = StageService->GetKillsThisStage();
		SaveGame->bStageFinalChapterCleared = StageService->HasFinalChapterCleared();
		SaveGame->StageHighestClearedChapter = StageService->GetHighestClearedChapter();
	}

	if (TowerService)
	{
		SaveGame->TowerHighestFloor = TowerService->GetHighestFloor();
	}

	if (DungeonService)
	{
		TMap<EDungeonType, int32> DungeonEntries;
		DungeonService->CaptureState(DungeonEntries, SaveGame->DungeonDailyResetDate);
		SaveGame->DungeonEntriesUsed.Reset();
		for (const EDungeonType Type : { EDungeonType::Gold, EDungeonType::Exp, EDungeonType::Essence })
		{
			SaveGame->DungeonEntriesUsed.Add(DungeonEntries.FindRef(Type));
		}
	}

	if (RuneService)
	{
		RuneService->CaptureState(SaveGame->Runes, SaveGame->EquippedRuneSlots, SaveGame->RuneCodex);
	}

	if (PetService)
	{
		SaveGame->EquippedPetId = PetService->GetEquippedPetId();
		SaveGame->OwnedPetIds = PetService->GetOwnedPetIds();
		SaveGame->PetLevels = PetService->GetPetLevels();
	}

	if (UInventoryComponent* Inventory = FindPlayerInventory())
	{
		Inventory->CaptureState(SaveGame->InventoryItems, SaveGame->EquippedSlotIndex);
	}
	else if (bHasPendingCharacterSaveV2)
	{
		SaveGame->InventoryItems = PendingInventoryItems;
		SaveGame->EquippedSlotIndex = PendingEquippedSlotIndex;
	}

	if (AIdleCharacter* Character = FindPlayerCharacter())
	{
		if (USkillComponent* Skills = Character->FindComponentByClass<USkillComponent>())
		{
			Skills->CaptureRankState(SaveGame->SkillRanks, SaveGame->SkillPoints);
		}
	}
	else if (bHasPendingCharacterSaveV2)
	{
		SaveGame->SkillRanks = PendingSkillRanks;
		SaveGame->SkillPoints = PendingSkillPoints;
	}

	if (QuestService)
	{
		QuestService->CaptureState(SaveGame->Quests, SaveGame->QuestDailyResetDate, SaveGame->QuestWeeklyResetId);
	}

	if (SeasonService)
	{
		SeasonService->CaptureState(SaveGame->SeasonId, SaveGame->SeasonTokens, SaveGame->SeasonClaimedTiers);
	}

	if (AchievementService)
	{
		AchievementService->CaptureState(SaveGame->AchievementMetrics, SaveGame->Achievements, &SaveGame->AchievementUniqueItemIds);
	}

	if (MasteryService)
	{
		SaveGame->Mastery = MasteryService->ExportSave();
	}

	if (BuffService)
	{
		SaveGame->Consumables = BuffService->ExportSave();
	}

	if (WeeklyBossService)
	{
		const FWeeklyBossSaveState WeeklyBossState = WeeklyBossService->ExportSave();
		SaveGame->WeeklyBossWeekId = WeeklyBossState.WeekId;
		SaveGame->WeeklyBossDamage = WeeklyBossState.Damage;
		SaveGame->WeeklyBossChallengesUsed = WeeklyBossState.ChallengesUsed;
		SaveGame->WeeklyBossClaimedMilestones = WeeklyBossState.ClaimedMilestones;
	}

	return true;
}

bool UIdleGameInstance::ApplyFromSave(const UIdleSaveGame* SaveGame)
{
	if (!SaveGame || SaveGame->SaveVersion <= 0 || !SaveGame->bHasSave)
	{
		return false;
	}

	TGuardValue<bool> AutosaveGuard(bAutosaveSuppressed, true);

	Gold = FMath::Max<int64>(0, SaveGame->Gold);
	RuneEssence = SaveGame->SaveVersion >= 3 ? FMath::Max<int64>(0, SaveGame->RuneEssence) : 0;
	ProtectionScrolls = SaveGame->SaveVersion >= 12 ? FMath::Max<int64>(0, SaveGame->ProtectionScrolls) : 0;
	ResetCubes = SaveGame->SaveVersion >= 12 ? FMath::Max<int64>(0, SaveGame->ResetCubes) : 0;
	RankCubes = SaveGame->SaveVersion >= 12 ? FMath::Max<int64>(0, SaveGame->RankCubes) : 0;
	CharacterLevel = FMath::Clamp(SaveGame->CharacterLevel, 1, FLevelFormulas::LEVEL_CAP);
	CurrentExp = FMath::Max<int64>(0, SaveGame->CurrentExp);
	NextExp = SaveGame->NextExp > 0 ? SaveGame->NextExp : FLevelFormulas::ExpToNext(CharacterLevel);
	RebirthCount = FMath::Max(0, SaveGame->RebirthCount);
	RebirthBonusPoints = FMath::Max(0, SaveGame->RebirthBonusPoints);
	TranscendCount = FMath::Max(0, SaveGame->TranscendCount);
	AvailableStatPoints = FMath::Max(0, SaveGame->AvailableStatPoints);
	AllocatedStats = ClampPrimaryStats(SaveGame->AllocatedStats);
	bChapter1BossDefeated = SaveGame->bChapter1BossDefeated;
	LastSeenUnixSec = FMath::Max<int64>(0, SaveGame->LastSeenUnixSec);

	EnsureStageService();
	if (StageService)
	{
		int32 RestoredHighestClearedChapter = SaveGame->StageHighestClearedChapter;
		bool bRestoredFinalChapterCleared = SaveGame->bStageFinalChapterCleared;
		if (SaveGame->SaveVersion < 8)
		{
			if (SaveGame->bChapter1BossDefeated)
			{
				RestoredHighestClearedChapter = FMath::Max(RestoredHighestClearedChapter, 1);
			}
			if (SaveGame->StageChapter < UStageService::TotalChapters)
			{
				bRestoredFinalChapterCleared = false;
			}
		}
		StageService->RestoreState(
			SaveGame->StageChapter,
			SaveGame->StageStage,
			SaveGame->StageKillsThisStage,
			bRestoredFinalChapterCleared,
			RestoredHighestClearedChapter);
	}

	EnsureTowerService();
	if (TowerService)
	{
		TowerService->SetHighestFloor(SaveGame->TowerHighestFloor);
	}

	EnsureDungeonService();
	if (DungeonService)
	{
		TMap<EDungeonType, int32> DungeonEntries;
		if (SaveGame->SaveVersion >= 10)
		{
			const EDungeonType Types[] = { EDungeonType::Gold, EDungeonType::Exp, EDungeonType::Essence };
			for (int32 Index = 0; Index < UE_ARRAY_COUNT(Types); ++Index)
			{
				DungeonEntries.Add(Types[Index], SaveGame->DungeonEntriesUsed.IsValidIndex(Index) ? SaveGame->DungeonEntriesUsed[Index] : 0);
			}
		}
		DungeonService->RestoreState(DungeonEntries, SaveGame->SaveVersion >= 10 ? SaveGame->DungeonDailyResetDate : UQuestService::GetCurrentUtcDateString());
	}

	EnsureRuneService();
	if (RuneService)
	{
		RuneService->SetOwnerClassId(GetCurrentClassIdForRunes());
		const TArray<FRuneSaveEntry> MigratedRunes = GetMigratedRunes(*SaveGame);
		const TArray<FRuneCodexEntry> MigratedCodex = GetMigratedRuneCodex(*SaveGame);
		if (SaveGame->SaveVersion >= 3)
		{
			if (SaveGame->SaveVersion >= 4)
			{
				RuneService->RestoreState(MigratedRunes, SaveGame->EquippedRuneSlots, MigratedCodex);
			}
			else
			{
				RuneService->RestoreState(MigratedRunes, SaveGame->EquippedRuneSlots, {});
			}
		}
		else
		{
			RuneService->RestoreState({}, {});
		}
	}

	EnsurePetService();
	if (PetService)
	{
		PetService->RestoreState(SaveGame->EquippedPetId, SaveGame->OwnedPetIds, SaveGame->PetLevels);
	}

	EnsureMasteryService();
	if (MasteryService)
	{
		MasteryService->ImportSave(SaveGame->SaveVersion >= 13 ? SaveGame->Mastery : TArray<FMasterySaveEntry>());
	}

	EnsureBuffService();
	if (BuffService)
	{
		BuffService->ImportSave(SaveGame->SaveVersion >= 14 ? SaveGame->Consumables : TArray<FConsumableSaveEntry>());
	}

	EnsureWeeklyBossService();
	if (WeeklyBossService)
	{
		if (SaveGame->SaveVersion >= 15)
		{
			WeeklyBossService->ImportSave(
				SaveGame->WeeklyBossWeekId,
				SaveGame->WeeklyBossDamage,
				SaveGame->WeeklyBossChallengesUsed,
				SaveGame->WeeklyBossClaimedMilestones);
		}
		else
		{
			WeeklyBossService->ImportSave(UQuestService::GetCurrentUtcWeekString(), 0, 0, 0);
		}
	}

	if (SaveGame->SaveVersion >= 2)
	{
		AIdleCharacter* Character = FindPlayerCharacter();
		const TArray<FItemInstance> MigratedInventoryItems = GetMigratedInventoryItems(*SaveGame);
		PendingInventoryItems = MigratedInventoryItems;
		PendingEquippedSlotIndex = SaveGame->EquippedSlotIndex;
		PendingSkillRanks = SaveGame->SkillRanks;
		PendingSkillPoints = SaveGame->SkillPoints;
		bHasPendingCharacterSaveV2 = true;

		if (ApplyCharacterSaveState(
			Character,
			MigratedInventoryItems,
			SaveGame->EquippedSlotIndex,
			SaveGame->SkillRanks,
			SaveGame->SkillPoints))
		{
			bHasPendingCharacterSaveV2 = false;
			PendingInventoryItems.Reset();
			PendingEquippedSlotIndex.Reset();
			PendingSkillRanks.Reset();
			PendingSkillPoints = 0;
		}

		EnsureQuestService();
		if (QuestService)
		{
			QuestService->RestoreState(SaveGame->Quests, SaveGame->QuestDailyResetDate, SaveGame->QuestWeeklyResetId);
			QuestService->ResetDailyQuestsIfNeeded(UQuestService::GetCurrentUtcDateString());
			QuestService->ResetWeeklyQuestsIfNeeded(UQuestService::GetCurrentUtcWeekString());
		}

		EnsureSeasonService();
		if (SeasonService)
		{
			SeasonService->RestoreState(SaveGame->SeasonId, SaveGame->SeasonTokens, SaveGame->SeasonClaimedTiers);
		}

		EnsureAchievementService();
		if (AchievementService)
		{
			AchievementService->RestoreState(SaveGame->AchievementMetrics, SaveGame->Achievements, &SaveGame->AchievementUniqueItemIds);
		}
	}
	else
	{
		EnsureQuestService();
		EnsureSeasonService();
		EnsureAchievementService();
	}

	OnGoldChanged.Broadcast(Gold);
	OnExpChanged.Broadcast(CurrentExp, NextExp);
	OnStatPointsChanged.Broadcast();
	OnLevelUp.Broadcast(CharacterLevel);

	return true;
}

void UIdleGameInstance::ApplyPendingCharacterSaveToCharacter(AIdleCharacter* Character)
{
	if (!bHasPendingCharacterSaveV2)
	{
		return;
	}

	if (ApplyCharacterSaveState(
		Character,
		PendingInventoryItems,
		PendingEquippedSlotIndex,
		PendingSkillRanks,
		PendingSkillPoints))
	{
		bHasPendingCharacterSaveV2 = false;
		PendingInventoryItems.Reset();
		PendingEquippedSlotIndex.Reset();
		PendingSkillRanks.Reset();
		PendingSkillPoints = 0;
	}
}

bool UIdleGameInstance::ApplyCharacterSaveState(
	AIdleCharacter* Character,
	const TArray<FItemInstance>& InventoryItems,
	const TMap<EItemSlot, int32>& EquippedSlotIndex,
	const TMap<FName, int32>& SkillRanks,
	int32 SkillPoints)
{
	if (!Character)
	{
		return false;
	}

	bool bAppliedAnyState = false;
	if (UInventoryComponent* Inventory = Character->FindComponentByClass<UInventoryComponent>())
	{
		Inventory->RestoreState(InventoryItems, EquippedSlotIndex);
		bAppliedAnyState = true;
	}

	if (USkillComponent* Skills = Character->FindComponentByClass<USkillComponent>())
	{
		Skills->RestoreRankState(SkillRanks, SkillPoints);
		bAppliedAnyState = true;
	}

	if (bAppliedAnyState)
	{
		Character->RefreshDerivedStats();
	}
	return bAppliedAnyState;
}

int64 UIdleGameInstance::ClimbTower()
{
	EnsureTowerService();
	EnsureMasteryService();
	AIdleCharacter* Character = FindPlayerCharacter();
	if (!TowerService || !Character)
	{
		return 0;
	}

	const int32 PreviousHighestFloor = TowerService->GetHighestFloor();
	const int64 Reward = TowerService->TryClimbTower(Character->GetCombatPower());
	if (Reward > 0)
	{
		RecordAchievementMetric(EAchievementMetric::TowerHighestFloor, TowerService->GetHighestFloor());
		RecordQuestProgress(EQuestObjective::ClimbTower, FMath::Max(1, TowerService->GetHighestFloor() - PreviousHighestFloor));
		if (MasteryService)
		{
			MasteryService->AddXp(EMasteryTrack::Abyss, 10);
			RefreshPlayerCharacterStats();
		}
		AddGold(Reward);
		RequestAutosave();
	}
	return Reward;
}

FDungeonRunResult UIdleGameInstance::TryRunDungeon(EDungeonType Type, int32 Tier)
{
	EnsureDungeonService();
	EnsureMasteryService();
	FDungeonRunResult Result;
	Result.Type = Type;
	Result.Tier = Tier;

	const AIdleCharacter* Character = FindPlayerCharacter();
	if (!DungeonService || !Character)
	{
		return Result;
	}

	// 심연 마스터리 2종: 던전 일일 입장 +N(정수 보너스)을 한도에 더함.
	const int32 AbyssBonusEntries = MasteryService ? MasteryService->GetAbyssBonusEntries() : 0;
	Result = DungeonService->TryRunDungeon(Type, Character->GetCombatPower(), UQuestService::GetCurrentUtcDateString(), Tier, AbyssBonusEntries);
	if (!Result.bSuccess)
	{
		return Result;
	}

	const double AbyssRewardMultiplier = 1.0 + static_cast<double>(MasteryService ? MasteryService->GetLocalBonus(EMasteryTrack::Abyss) : 0.0f);
	Result.GoldReward = FMath::Max<int64>(0, FMath::RoundToInt64(static_cast<double>(Result.GoldReward) * AbyssRewardMultiplier));
	Result.ExpReward = FMath::Max<int64>(0, FMath::RoundToInt64(static_cast<double>(Result.ExpReward) * AbyssRewardMultiplier));
	Result.EssenceReward = FMath::Max<int64>(0, FMath::RoundToInt64(static_cast<double>(Result.EssenceReward) * AbyssRewardMultiplier));

	AddGold(Result.GoldReward);
	AddExp(Result.ExpReward);
	if (MasteryService)
	{
		MasteryService->AddXp(EMasteryTrack::Abyss, 30);
		RefreshPlayerCharacterStats();
	}
	if (Result.EssenceReward > 0)
	{
		if (RuneEssence > MAX_int64 - Result.EssenceReward)
		{
			RuneEssence = MAX_int64;
		}
		else
		{
			RuneEssence += Result.EssenceReward;
		}
		RequestAutosave();
	}
	RequestAutosave();
	return Result;
}

FWeeklyBossChallengeResult UIdleGameInstance::TryChallengeWeeklyBoss()
{
	EnsureWeeklyBossService();

	FWeeklyBossChallengeResult Result;
	const AIdleCharacter* Character = FindPlayerCharacter();
	if (!WeeklyBossService || !Character)
	{
		return Result;
	}

	Result = WeeklyBossService->Challenge(Character->GetCombatPower(), UQuestService::GetCurrentUtcWeekString());
	if (Result.bSuccess)
	{
		RequestAutosave();
	}
	return Result;
}

bool UIdleGameInstance::ClaimWeeklyBossMilestone(int32 Milestone)
{
	EnsureWeeklyBossService();
	if (!WeeklyBossService)
	{
		return false;
	}

	const int32 PreviousClaimed = WeeklyBossService->GetClaimedMilestones();
	if (!WeeklyBossService->ClaimMilestone(Milestone))
	{
		return false;
	}

	// 이전 수령분 다음부터 요청 마일스톤까지 누적 지급한다(상위 마일스톤을 바로
	// 수령해도 중간 마일스톤 보상이 유실되지 않도록).
	for (int32 N = PreviousClaimed + 1; N <= Milestone; ++N)
	{
		AddGold(FWeeklyBossFormula::MilestoneGoldReward(N));
		const int64 EssenceReward = FWeeklyBossFormula::MilestoneEssenceReward(N);
		if (EssenceReward > 0)
		{
			RuneEssence = RuneEssence > MAX_int64 - EssenceReward ? MAX_int64 : RuneEssence + EssenceReward;
		}
	}
	RequestAutosave();
	return true;
}

FEnhanceAttemptResult UIdleGameInstance::TryEnhanceEquipped(EItemSlot Slot, bool bUseProtection)
{
	return TryEnhanceEquipped(Slot, bUseProtection, FindPlayerInventory());
}

FEnhanceAttemptResult UIdleGameInstance::TryEnhanceEquipped(EItemSlot Slot, UInventoryComponent* Inventory)
{
	return TryEnhanceEquipped(Slot, false, Inventory);
}

FEnhanceAttemptResult UIdleGameInstance::TryEnhanceEquipped(EItemSlot Slot, bool bUseProtection, UInventoryComponent* Inventory)
{
	FEnhanceAttemptResult Result;
	if (!Inventory || Slot == EItemSlot::None)
	{
		OnEnhanceResult.Broadcast(Result);
		return Result;
	}

	const int32 CurrentLevel = Inventory->GetEquippedEnhanceLevel(Slot);
	Result.NewLevel = CurrentLevel;
	if (CurrentLevel == INDEX_NONE || CurrentLevel >= FEnhanceFormula::MaxEnhanceLevel)
	{
		OnEnhanceResult.Broadcast(Result);
		return Result;
	}

	const FItemInstance* EquippedItem = Inventory->GetEquippedItem(Slot);
	const EItemRarity Rarity = EquippedItem ? EquippedItem->Rarity : EItemRarity::None;
	EnsureMasteryService();
	const float EquipmentCostReduction = MasteryService ? MasteryService->GetLocalBonus(EMasteryTrack::Equipment) : 0.0f;
	const int64 BaseCost = FEnhanceFormula::GetEnhanceCost(CurrentLevel, Rarity);
	const int64 Cost = FMath::Max<int64>(0, FMath::RoundToInt64(static_cast<double>(BaseCost) * (1.0 - static_cast<double>(EquipmentCostReduction))));
	const bool bHasProtection = ProtectionScrolls > 0;
	if (Cost <= 0 || Gold < Cost || (bUseProtection && FEnhanceFormula::IsRiskLevel(CurrentLevel) && !bHasProtection))
	{
		OnEnhanceResult.Broadcast(Result);
		return Result;
	}

	AddGold(-Cost);
	Result.bAttempted = true;
	Result.GoldSpent = Cost;

	const FEnhanceAttemptOutcome Outcome = FEnhanceFormula::ResolveAttempt(
		CurrentLevel,
		EquippedItem ? EquippedItem->EnhanceFailStreak : 0,
		bUseProtection,
		bHasProtection,
		EnhanceRandomStream.GetFraction());
	Inventory->ApplyEnhanceOutcome(Slot, Outcome);
	if (Outcome.bConsumedProtection)
	{
		ProtectionScrolls = FMath::Max<int64>(0, ProtectionScrolls - 1);
	}
	Result.bSuccess = Outcome.bSuccess;
	Result.bConsumedProtection = Outcome.bConsumedProtection;
	Result.NewLevel = Outcome.NewLevel;
	Result.NewFailStreak = Outcome.NewFailStreak;

	RecordGearEnhanced();
	RecordAchievementMetric(EAchievementMetric::HighestEnhanceLevel, Result.NewLevel);
	OnEnhanceResult.Broadcast(Result);
	RequestAutosave();
	return Result;
}

FPotentialRerollResult UIdleGameInstance::TryRerollPotential(EItemSlot Slot, EPotentialCubeType CubeType)
{
	return TryRerollPotential(Slot, CubeType, FindPlayerInventory());
}

FPotentialRerollResult UIdleGameInstance::TryRerollPotential(EItemSlot Slot, EPotentialCubeType CubeType, UInventoryComponent* Inventory)
{
	FPotentialRerollResult Result;
	Result.CubeType = CubeType;
	if (!Inventory || Slot == EItemSlot::None)
	{
		return Result;
	}

	const FItemInstance* EquippedItem = Inventory->GetEquippedItem(Slot);
	if (!EquippedItem || EquippedItem->PotentialGrade == EPotentialGrade::None)
	{
		return Result;
	}

	TArray<FPotentialLine> Lines;
	EPotentialGrade NewGrade = EquippedItem->PotentialGrade;
	if (CubeType == EPotentialCubeType::Reset)
	{
		if (ResetCubes <= 0)
		{
			return Result;
		}
		--ResetCubes;
		Lines = FPotentialFormula::RollPotentialLines(NewGrade, PotentialRandomStream);
	}
	else
	{
		if (RankCubes <= 0)
		{
			return Result;
		}
		--RankCubes;
		NewGrade = FPotentialFormula::ApplyRankCube(EquippedItem->PotentialGrade, FPotentialFormula::GetMaxPotentialGrade(EquippedItem->Rarity), PotentialRandomStream, Lines);
	}

	if (!Inventory->SetEquippedPotential(Slot, NewGrade, Lines))
	{
		return Result;
	}

	Result.bRerolled = true;
	Result.NewGrade = NewGrade;
	RefreshPlayerCharacterStats();
	RequestAutosave();
	return Result;
}

bool UIdleGameInstance::SetItemLocked(EItemSlot Slot, bool bLocked)
{
	UInventoryComponent* Inventory = FindPlayerInventory();
	const bool bUpdated = Inventory && Inventory->SetItemLocked(Slot, bLocked);
	if (bUpdated)
	{
		RequestAutosave();
	}
	return bUpdated;
}

FShopPurchaseResult UIdleGameInstance::TryBuyGearRoll()
{
	return TryBuyGearRoll(FindPlayerInventory());
}

FShopPurchaseResult UIdleGameInstance::TryBuyGearRoll(UInventoryComponent* Inventory)
{
	FShopPurchaseResult Result;
	if (!Inventory)
	{
		OnShopPurchase.Broadcast(Result);
		return Result;
	}

	const int32 GlobalStageIndex = StageService ? StageService->GetGlobalStageIndex() : 0;
	const int64 Cost = FShopFormula::GetGearRollCost(GlobalStageIndex);
	if (Cost <= 0 || Gold < Cost)
	{
		OnShopPurchase.Broadcast(Result);
		return Result;
	}

	const int32 MonsterLevel = FRewardFormula::GetMonsterLevelForStage(GlobalStageIndex);
	const FItemInstance Item = FItemFactory::GuaranteedDropForLevel(MonsterLevel);
	if (!Inventory->CanAddItem(Item))
	{
		OnShopPurchase.Broadcast(Result);
		return Result;
	}

	AddGold(-Cost);
	RecordAchievementMetric(EAchievementMetric::GearRollsPurchased, 1);
	RecordQuestProgress(EQuestObjective::RollGearShop, 1);
	Inventory->AddItem(Item);
	RecordAchievementItemCollected(Item);

	Result.bPurchased = true;
	Result.GoldSpent = Cost;
	Result.Rarity = Item.Rarity;
	Result.Slot = Item.Slot;
	Result.ItemName = Item.DisplayName;
	OnShopPurchase.Broadcast(Result);
	RequestAutosave();
	return Result;
}

bool UIdleGameInstance::TryBuyProtectionScroll()
{
	const int32 GlobalStageIndex = StageService ? StageService->GetGlobalStageIndex() : 0;
	return TryBuyShopResource(FShopFormula::GetProtectionScrollCost(GlobalStageIndex), ProtectionScrolls);
}

int64 UIdleGameInstance::ApplyEquipmentCubeCostReduction(int64 BaseCost)
{
	// 장비 마스터리 2종: 잠재 큐브 골드 가격 ×(1 - min(0.5, GetLocalBonus2(Equipment))). 1종 강화 비용 절감과 별도.
	EnsureMasteryService();
	const float Reduction = MasteryService ? MasteryService->GetLocalBonus2(EMasteryTrack::Equipment) : 0.0f;
	return FMath::Max<int64>(0, FMath::RoundToInt64(static_cast<double>(BaseCost) * (1.0 - static_cast<double>(Reduction))));
}

bool UIdleGameInstance::TryBuyResetCube()
{
	const int32 GlobalStageIndex = StageService ? StageService->GetGlobalStageIndex() : 0;
	const int64 Cost = ApplyEquipmentCubeCostReduction(FShopFormula::GetResetCubeCost(GlobalStageIndex));
	return TryBuyShopResource(Cost, ResetCubes);
}

bool UIdleGameInstance::TryBuyRankCube()
{
	const int32 GlobalStageIndex = StageService ? StageService->GetGlobalStageIndex() : 0;
	const int64 Cost = ApplyEquipmentCubeCostReduction(FShopFormula::GetRankCubeCost(GlobalStageIndex));
	return TryBuyShopResource(Cost, RankCubes);
}

void UIdleGameInstance::AddConsumable(EConsumableType Type, EConsumableGrade Grade, int32 Amount)
{
	EnsureBuffService();
	if (!BuffService)
	{
		return;
	}

	BuffService->AddConsumable(Type, Grade, Amount);
	RequestAutosave();
}

bool UIdleGameInstance::TryUseConsumable(EConsumableType Type, EConsumableGrade Grade)
{
	EnsureBuffService();
	if (!BuffService || !BuffService->UseConsumable(Type, Grade, GetCurrentUnixSeconds()))
	{
		return false;
	}

	RefreshPlayerCharacterStats();
	RequestAutosave();
	return true;
}

int64 UIdleGameInstance::GetConsumableShopCost(EConsumableGrade Grade) const
{
	const int32 GlobalStageIndex = StageService ? StageService->GetGlobalStageIndex() : 0;
	const int64 BaseCost = FShopFormula::GetGearRollCost(GlobalStageIndex);
	if (BaseCost <= 0 || !FConsumableFormula::IsValidGrade(Grade))
	{
		return 0;
	}

	double Multiplier = 1.0;
	switch (Grade)
	{
	case EConsumableGrade::Lesser:
		Multiplier = 0.6;
		break;
	case EConsumableGrade::Standard:
		Multiplier = 1.0;
		break;
	case EConsumableGrade::Greater:
		Multiplier = 2.5;
		break;
	default:
		Multiplier = 1.0;
		break;
	}

	return FMath::Max<int64>(1, FMath::RoundToInt64(static_cast<double>(BaseCost) * Multiplier));
}

bool UIdleGameInstance::TryBuyConsumable(EConsumableType Type, EConsumableGrade Grade)
{
	EnsureBuffService();
	if (!BuffService || !FConsumableFormula::IsValidType(Type) || !FConsumableFormula::IsValidGrade(Grade))
	{
		return false;
	}

	const int64 Cost = GetConsumableShopCost(Grade);
	if (Cost <= 0 || Gold < Cost)
	{
		return false;
	}

	AddGold(-Cost);
	BuffService->AddConsumable(Type, Grade, 1);
	RequestAutosave();
	return true;
}

bool UIdleGameInstance::TryBuyShopResource(int64 Cost, int64& ResourceCount)
{
	if (Cost <= 0 || Gold < Cost || ResourceCount >= MAX_int64)
	{
		return false;
	}

	AddGold(-Cost);
	++ResourceCount;
	RequestAutosave();
	return true;
}

void UIdleGameInstance::AddRune(const FRuneInstance& Rune)
{
	EnsureRuneService();
	EnsureMasteryService();
	if (!RuneService)
	{
		return;
	}

	const int32 PreviousCount = RuneService->GetOwnedRunes().Num();
	RuneService->AddRune(Rune);
	if (RuneService->GetOwnedRunes().Num() != PreviousCount)
	{
		if (MasteryService)
		{
			MasteryService->AddXp(EMasteryTrack::Rune, 5);
			RefreshPlayerCharacterStats();
		}
		RequestAutosave();
	}
}

bool UIdleGameInstance::TryEnhanceRune(int32 OwnedIndex)
{
	EnsureRuneService();
	EnsureMasteryService();
	if (!RuneService || !RuneService->GetOwnedRunes().IsValidIndex(OwnedIndex))
	{
		return false;
	}

	const int32 CurrentLevel = RuneService->GetOwnedRunes()[OwnedIndex].EnhanceLevel;
	const int64 EssenceCost = FRuneFormula::GetEnhanceEssenceCost(CurrentLevel);
	const int64 GoldCost = FRuneFormula::GetEnhanceGoldCost(CurrentLevel);
	if (RuneEssence < EssenceCost || Gold < GoldCost)
	{
		return false;
	}

	RuneEssence -= EssenceCost;
	AddGold(-GoldCost);
	if (!RuneService->EnhanceRune(OwnedIndex))
	{
		RuneEssence += EssenceCost;
		AddGold(GoldCost);
		return false;
	}

	RefreshPlayerCharacterStats();
	if (MasteryService)
	{
		MasteryService->AddXp(EMasteryTrack::Rune, 5);
		RefreshPlayerCharacterStats();
	}
	RequestAutosave();
	return true;
}

bool UIdleGameInstance::TryDisenchantRune(int32 OwnedIndex)
{
	EnsureRuneService();
	if (!RuneService)
	{
		return false;
	}

	int64 Refund = 0;
	if (!RuneService->TryDisenchantRune(OwnedIndex, Refund))
	{
		return false;
	}

	// 룬 마스터리 2종: 분해 에센스 획득 ×(1 + GetLocalBonus2(Rune)). 1종 코어 가산과 별도 단일.
	EnsureMasteryService();
	const float RuneEssenceBonus = MasteryService ? MasteryService->GetLocalBonus2(EMasteryTrack::Rune) : 0.0f;
	const int64 BonusRefund = FMath::Max<int64>(0, FMath::RoundToInt64(static_cast<double>(Refund) * (1.0 + static_cast<double>(RuneEssenceBonus))));
	RuneEssence = BonusRefund > MAX_int64 - RuneEssence ? MAX_int64 : RuneEssence + BonusRefund;
	RequestAutosave();
	return true;
}

bool UIdleGameInstance::TryBuyRuneRoll()
{
	EnsureRuneService();
	if (!RuneService)
	{
		return false;
	}

	const int32 ProgressIndex = StageService ? StageService->GetGlobalStageIndex() : 0;
	const int64 Cost = FRuneFormula::GetShopRuneRollCost(ProgressIndex);
	if (Cost <= 0 || Gold < Cost)
	{
		return false;
	}

	AddGold(-Cost);
	RuneService->AddRune(FRuneFormula::RollShopRune(ProgressIndex, RuneRandomStream));
	RequestAutosave();
	return true;
}

bool UIdleGameInstance::TryCraftClassRune(EItemRarity Rarity)
{
	EnsureRuneService();
	if (!RuneService)
	{
		return false;
	}

	const EClassId ClassId = GetCurrentClassIdForRunes();
	const int64 Cost = FClassRuneFormula::GetClassRuneCraftCost(Rarity);
	if (Cost <= 0 || RuneEssence < Cost)
	{
		return false;
	}

	RuneEssence -= Cost;
	const int32 ProgressIndex = StageService ? StageService->GetGlobalStageIndex() : 0;
	RuneService->AddRune(FClassRuneFormula::MakeClassRune(ClassId, Rarity, ProgressIndex));
	RequestAutosave();
	return true;
}

bool UIdleGameInstance::TryRollClassRuneDrop(int32 MonsterLevel, bool bIsBoss)
{
	EnsureRuneService();
	if (!RuneService)
	{
		return false;
	}

	FRuneInstance Rune;
	if (!FClassRuneFormula::RollClassRuneDrop(MonsterLevel, bIsBoss, GetCurrentClassIdForRunes(), RuneRandomStream, Rune))
	{
		return false;
	}

	RuneService->AddRune(Rune);
	RequestAutosave();
	return true;
}

bool UIdleGameInstance::TryEquipRune(int32 SlotIndex, int32 OwnedIndex)
{
	EnsureRuneService();
	if (!RuneService || !RuneService->TryEquipRune(SlotIndex, OwnedIndex))
	{
		return false;
	}

	RefreshPlayerCharacterStats();
	RequestAutosave();
	return true;
}

bool UIdleGameInstance::UnequipRune(int32 SlotIndex)
{
	EnsureRuneService();
	if (!RuneService || !RuneService->UnequipRune(SlotIndex))
	{
		return false;
	}

	RefreshPlayerCharacterStats();
	RequestAutosave();
	return true;
}

void UIdleGameInstance::SetEnhanceRandomSeed(int32 Seed)
{
	EnhanceRandomStream.Initialize(Seed);
}

void UIdleGameInstance::AddExp(int64 Amount)
{
	if (Amount <= 0 || CharacterLevel >= FLevelFormulas::LEVEL_CAP)
	{
		return;
	}

	EnsureBuffService();
	EnsureMasteryService();
	const double ExpMultiplier =
		1.0
		+ static_cast<double>(BuffService ? BuffService->GetExpBuffPct(GetCurrentUnixSeconds()) : 0.0f)
		+ static_cast<double>(MasteryService ? MasteryService->GetGlobalBonus().ExpBoostPct : 0.0f);
	CurrentExp += FMath::Max<int64>(0, FMath::RoundToInt64(static_cast<double>(Amount) * ExpMultiplier));
	const bool bWasAutosaveSuppressed = bAutosaveSuppressed;
	bAutosaveSuppressed = true;
	while (NextExp > 0 && CurrentExp >= NextExp && CharacterLevel < FLevelFormulas::LEVEL_CAP)
	{
		CurrentExp -= NextExp;
		LevelUp();
	}
	bAutosaveSuppressed = bWasAutosaveSuppressed;

	OnExpChanged.Broadcast(CurrentExp, NextExp);
	RequestAutosave();
}

void UIdleGameInstance::LevelUp()
{
	if (CharacterLevel >= FLevelFormulas::LEVEL_CAP)
	{
		NextExp = 0;
		return;
	}

	++CharacterLevel;
	RecordAchievementMetric(EAchievementMetric::HighestLevelReached, CharacterLevel);
	RecordQuestProgress(EQuestObjective::ReachLevel, CharacterLevel);
	NextExp = FLevelFormulas::ExpToNext(CharacterLevel);
	GrantStatPoints(FStatPointFormula::GetStatPointsForLevelUp(CharacterLevel));
	OnLevelUp.Broadcast(CharacterLevel);
	RequestAutosave();
}

void UIdleGameInstance::GrantStatPoints(int32 Points)
{
	if (Points <= 0)
	{
		return;
	}

	AvailableStatPoints += Points;
	OnStatPointsChanged.Broadcast();
	RequestAutosave();
}

bool UIdleGameInstance::AllocateStatPoint(EPrimaryStat Stat)
{
	if (AvailableStatPoints <= 0)
	{
		return false;
	}

	switch (Stat)
	{
	case EPrimaryStat::Str:
		AllocatedStats.Str += 1.0f;
		break;
	case EPrimaryStat::Dex:
		AllocatedStats.Dex += 1.0f;
		break;
	case EPrimaryStat::Int:
		AllocatedStats.Int_ += 1.0f;
		break;
	case EPrimaryStat::Wis:
		AllocatedStats.Wis += 1.0f;
		break;
	case EPrimaryStat::Con:
		AllocatedStats.Con += 1.0f;
		break;
	case EPrimaryStat::Luk:
		AllocatedStats.Luk += 1.0f;
		break;
	default:
		return false;
	}

	--AvailableStatPoints;
	OnStatPointsChanged.Broadcast();
	RequestAutosave();
	return true;
}

void UIdleGameInstance::ResetStatPoints()
{
	const int32 SpentPoints =
		FMath::RoundToInt(AllocatedStats.Str) +
		FMath::RoundToInt(AllocatedStats.Dex) +
		FMath::RoundToInt(AllocatedStats.Int_) +
		FMath::RoundToInt(AllocatedStats.Wis) +
		FMath::RoundToInt(AllocatedStats.Con) +
		FMath::RoundToInt(AllocatedStats.Luk);

	if (SpentPoints <= 0)
	{
		return;
	}

	AvailableStatPoints += SpentPoints;
	AllocatedStats = FPrimaryStats();
	OnStatPointsChanged.Broadcast();
	RequestAutosave();
}

bool UIdleGameInstance::CanRebirth() const
{
	return bChapter1BossDefeated && CharacterLevel >= 100;
}

bool UIdleGameInstance::Rebirth()
{
	if (!CanRebirth())
	{
		return false;
	}

	const int32 Reward = FRebirthFormula::GetRebirthPointsReward(RebirthCount, CharacterLevel);
	++RebirthCount;
	RecordQuestProgress(EQuestObjective::Rebirth, 1);
	RecordAchievementMetric(EAchievementMetric::RebirthCount, RebirthCount);
	RebirthBonusPoints += Reward;
	CharacterLevel = 1;
	CurrentExp = 0;
	NextExp = FLevelFormulas::ExpToNext(CharacterLevel);
	AvailableStatPoints = 0;
	AllocatedStats = FPrimaryStats();
	Gold = FMath::FloorToInt64(static_cast<double>(Gold) * 0.1);
	bChapter1BossDefeated = false;
	if (StageService)
	{
		StageService->InitializeDefaultStages();
	}

	OnGoldChanged.Broadcast(Gold);
	OnExpChanged.Broadcast(CurrentExp, NextExp);
	OnStatPointsChanged.Broadcast();
	OnLevelUp.Broadcast(CharacterLevel);
	RequestAutosave();
	return true;
}

int32 UIdleGameInstance::PreviewRebirthReward() const
{
	return FRebirthFormula::GetRebirthPointsReward(RebirthCount, CharacterLevel);
}

bool UIdleGameInstance::CanTranscend() const
{
	return FTranscendFormula::CanTranscend(RebirthCount);
}

bool UIdleGameInstance::Transcend()
{
	if (!CanTranscend())
	{
		return false;
	}

	++TranscendCount;
	RecordQuestProgress(EQuestObjective::Transcend, 1);
	RecordAchievementMetric(EAchievementMetric::TranscendCount, TranscendCount);
	RebirthCount = 0;
	RebirthBonusPoints = 0;
	CharacterLevel = 1;
	CurrentExp = 0;
	NextExp = FLevelFormulas::ExpToNext(CharacterLevel);
	AvailableStatPoints = 0;
	AllocatedStats = FPrimaryStats();
	Gold = 0;
	bChapter1BossDefeated = false;
	if (StageService)
	{
		StageService->InitializeDefaultStages();
	}

	OnTranscend.Broadcast();
	OnGoldChanged.Broadcast(Gold);
	OnExpChanged.Broadcast(CurrentExp, NextExp);
	OnStatPointsChanged.Broadcast();
	OnLevelUp.Broadcast(CharacterLevel);
	RequestAutosave();
	return true;
}

float UIdleGameInstance::GetTranscendStatMultiplier() const
{
	return FTranscendFormula::GetTranscendStatMultiplier(TranscendCount);
}

float UIdleGameInstance::GetTowerMilestoneMultiplier() const
{
	return TowerService ? TowerService->GetMilestoneMultiplier() : 1.0f;
}

float UIdleGameInstance::GetAchievementStatMultiplier() const
{
	return AchievementService ? AchievementService->GetStatMultiplier() : 1.0f;
}

float UIdleGameInstance::GetMasteryCoreStatMultiplier() const
{
	return MasteryService ? MasteryService->GetGlobalBonus().CoreStatMultiplier : 1.0f;
}

int32 UIdleGameInstance::GetAchievementTotalPoints() const
{
	return AchievementService ? AchievementService->GetTotalPoints() : 0;
}

int64 UIdleGameInstance::GetAchievementMetricValue(EAchievementMetric Metric) const
{
	return AchievementService ? AchievementService->GetMetricValue(Metric) : 0;
}

float UIdleGameInstance::GetRuneGoldFindBonus() const
{
	const float RuneBonus = RuneService ? RuneService->GetEquippedUtilValues().GoldFind : 0.0f;
	const float MasteryBonus = MasteryService ? MasteryService->GetGlobalBonus().GoldFindPct : 0.0f;
	return RuneBonus + MasteryBonus;
}

float UIdleGameInstance::GetRuneExpBoostBonus() const
{
	// EXP 마스터리 보너스는 AddExp 보편 경로에서 단일 적용된다. 이 getter는
	// 처치 EXP 계산(IdleMonster)에 쓰이므로 마스터리를 더하면 이중 적용된다.
	return RuneService ? RuneService->GetEquippedUtilValues().ExpBoost : 0.0f;
}

float UIdleGameInstance::GetRuneOfflineEffBonus() const
{
	return RuneService ? RuneService->GetEquippedUtilValues().OfflineEff : 0.0f;
}

float UIdleGameInstance::GetRuneCritDamageBonus() const
{
	return RuneService ? RuneService->GetEquippedUtilValues().CritDamage : 0.0f;
}

float UIdleGameInstance::PreviewTranscendMultiplier() const
{
	return FTranscendFormula::GetTranscendStatMultiplier(TranscendCount + 1);
}

void UIdleGameInstance::MarkChapter1BossDefeated()
{
	if (bChapter1BossDefeated)
	{
		return;
	}

	bChapter1BossDefeated = true;
	RequestAutosave();
}

FOfflineRewardResult UIdleGameInstance::ClaimOfflineRewards()
{
	return ClaimOfflineRewardsAt(GetCurrentUnixSeconds());
}

FOfflineRewardResult UIdleGameInstance::ClaimOfflineRewardsAt(int64 NowUnixSec, int32 RebirthCountOverride)
{
	const int32 EffectiveRebirthCount = RebirthCountOverride >= 0 ? RebirthCountOverride : RebirthCount;
	const FOfflineRewardResult Reward = PreviewOfflineRewards(NowUnixSec, EffectiveRebirthCount);
	if (ApiClient)
	{
		ApiClient->ClaimOfflineRewards(CharacterLevel, LastSeenUnixSec, NowUnixSec, EffectiveRebirthCount);
	}
	if (Reward.CappedSeconds <= 0)
	{
		LastSeenUnixSec = FMath::Max(LastSeenUnixSec, NowUnixSec);
		return Reward;
	}

	AddGold(Reward.Gold);
	AddExp(Reward.Exp);
	RecordQuestProgress(EQuestObjective::ClaimOffline, 1);
	RecordAchievementMetric(EAchievementMetric::OfflineRewardsClaimed, 1);
	LastSeenUnixSec = FMath::Max(LastSeenUnixSec, NowUnixSec);
	RequestAutosave();
	return Reward;
}

FOfflineRewardResult UIdleGameInstance::PreviewOfflineRewards(int64 NowUnixSec, int32 RebirthCountOverride) const
{
	const int32 EffectiveRebirthCount = RebirthCountOverride >= 0 ? RebirthCountOverride : RebirthCount;
	FOfflineRewardResult Reward = FOfflineRewardFormula::ComputeOfflineRewards(CharacterLevel, LastSeenUnixSec, NowUnixSec, EffectiveRebirthCount);
	// 탐험 마스터리 2종: 오프라인 보상 ×(1 + GetLocalBonus2(Explore)). 1종 퀘스트 보상과 별도 단일.
	const double ExploreOfflineBonus = static_cast<double>(MasteryService ? MasteryService->GetLocalBonus2(EMasteryTrack::Explore) : 0.0f);
	const double OfflineMultiplier = (1.0 + static_cast<double>(GetRuneOfflineEffBonus())) * (1.0 + ExploreOfflineBonus);
	Reward.Gold = FMath::Max<int64>(0, FMath::RoundToInt64(static_cast<double>(Reward.Gold) * OfflineMultiplier));
	Reward.Exp = FMath::Max<int64>(0, FMath::RoundToInt64(static_cast<double>(Reward.Exp) * OfflineMultiplier));
	return Reward;
}

void UIdleGameInstance::SetLastSeenUnixSec(int64 UnixSec)
{
	LastSeenUnixSec = FMath::Max<int64>(0, UnixSec);
	RequestAutosave();
}

void UIdleGameInstance::RecordQuestProgress(EQuestObjective Objective, int32 Amount)
{
	EnsureQuestService();
	if (!QuestService)
	{
		return;
	}

	QuestService->ResetDailyQuestsIfNeeded(UQuestService::GetCurrentUtcDateString());
	QuestService->ResetWeeklyQuestsIfNeeded(UQuestService::GetCurrentUtcWeekString());
	QuestService->RecordProgress(Objective, Amount);
}

void UIdleGameInstance::RecordMonsterKilled()
{
	EnsureMasteryService();
	RecordQuestProgress(EQuestObjective::KillMonster, 1);
	RecordAchievementMetric(EAchievementMetric::MonstersKilled, 1);
	if (MasteryService)
	{
		MasteryService->AddXp(EMasteryTrack::Combat, 1);
		RefreshPlayerCharacterStats();
	}
}

void UIdleGameInstance::RecordGearEnhanced()
{
	EnsureMasteryService();
	RecordQuestProgress(EQuestObjective::Enhance, 1);
	RecordAchievementMetric(EAchievementMetric::GearEnhanced, 1);
	if (MasteryService)
	{
		MasteryService->AddXp(EMasteryTrack::Equipment, 5);
		RefreshPlayerCharacterStats();
	}
}

void UIdleGameInstance::RecordAchievementMetric(EAchievementMetric Metric, int64 AmountOrValue)
{
	EnsureAchievementService();
	if (AchievementService)
	{
		AchievementService->RecordMetric(Metric, AmountOrValue);
	}
}

void UIdleGameInstance::RecordAchievementItemCollected(const FItemInstance& Item)
{
	EnsureAchievementService();
	if (AchievementService)
	{
		AchievementService->RecordItemCollected(Item.ItemId);
	}
}

FQuestClaimResult UIdleGameInstance::ClaimQuest(const FString& QuestId)
{
	EnsureQuestService();
	EnsureMasteryService();
	FQuestClaimResult Result;
	if (!QuestService)
	{
		Result.Message = TEXT("quest_service_unavailable");
		return Result;
	}

	QuestService->ResetDailyQuestsIfNeeded(UQuestService::GetCurrentUtcDateString());
	QuestService->ResetWeeklyQuestsIfNeeded(UQuestService::GetCurrentUtcWeekString());
	Result = QuestService->ClaimQuest(QuestId);
	if (!Result.bSuccess)
	{
		return Result;
	}

	const double ExploreRewardMultiplier = 1.0 + static_cast<double>(MasteryService ? MasteryService->GetLocalBonus(EMasteryTrack::Explore) : 0.0f);
	Result.RewardGold = FMath::Max<int64>(0, FMath::RoundToInt64(static_cast<double>(Result.RewardGold) * ExploreRewardMultiplier));
	Result.RewardExp = FMath::Max<int64>(0, FMath::RoundToInt64(static_cast<double>(Result.RewardExp) * ExploreRewardMultiplier));

	AddGold(Result.RewardGold);
	AddExp(Result.RewardExp);
	RecordAchievementMetric(EAchievementMetric::QuestsCompleted, 1);
	if (MasteryService)
	{
		MasteryService->AddXp(EMasteryTrack::Explore, 20);
		RefreshPlayerCharacterStats();
	}
	EnsureSeasonService();
	if (SeasonService)
	{
		SeasonService->AddSeasonTokens(USeasonService::QuestClaimSeasonTokenReward);
	}
	if (ApiClient)
	{
		ApiClient->ClaimQuestReward(QuestId, FString());
	}
	RequestAutosave();
	return Result;
}

TArray<FQuestState> UIdleGameInstance::GetActiveQuestStates() const
{
	return QuestService ? QuestService->GetActiveQuestStates() : TArray<FQuestState>();
}

bool UIdleGameInstance::GetQuestState(const FString& QuestId, FQuestState& OutState) const
{
	return QuestService ? QuestService->GetQuestState(QuestId, OutState) : false;
}

void UIdleGameInstance::InitializeQuestServiceForTests(const FString& CurrentUtcDate)
{
	QuestService = NewObject<UQuestService>(this);
	QuestService->InitializeDefaultQuests(CurrentUtcDate);
}

void UIdleGameInstance::InitializePetSeasonServicesForTests()
{
	PetService = NewObject<UPetService>(this);
	PetService->InitializeDefaultPets();
	SeasonService = NewObject<USeasonService>(this);
	SeasonService->InitializeDefaultSeason();
}

void UIdleGameInstance::InitializeStageServiceForTests()
{
	StageService = NewObject<UStageService>(this);
	StageService->InitializeDefaultStages();
	StageService->OnChapterBossDefeated.AddUniqueDynamic(this, &UIdleGameInstance::HandleChapterBossDefeated);
}

void UIdleGameInstance::InitializeTowerServiceForTests()
{
	TowerService = NewObject<UTowerService>(this);
	TowerService->InitializeTower();
}

void UIdleGameInstance::InitializeDungeonServiceForTests(const FString& CurrentUtcDate)
{
	DungeonService = NewObject<UDungeonService>(this);
	DungeonService->EnsureDailyReset(CurrentUtcDate);
}

void UIdleGameInstance::InitializeWeeklyBossServiceForTests(const FString& CurrentWeek)
{
	WeeklyBossService = NewObject<UWeeklyBossService>(this);
	WeeklyBossService->EnsureWeek(CurrentWeek);
}

void UIdleGameInstance::InitializeRuneServiceForTests()
{
	RuneService = NewObject<URuneService>(this);
	RuneService->SetOwnerClassId(EClassId::Warrior);
	RuneEssence = 0;
	RuneRandomStream.Initialize(12345);
}

#if WITH_DEV_AUTOMATION_TESTS
void UIdleGameInstance::AddRuneForTests(const FRuneInstance& Rune)
{
	AddRune(Rune);
}

void UIdleGameInstance::AddRuneEssenceForTests(int64 Amount)
{
	RuneEssence = FMath::Max<int64>(0, RuneEssence + Amount);
}

void UIdleGameInstance::SetRuneOwnerClassForTests(EClassId ClassId)
{
	EnsureRuneService();
	if (RuneService)
	{
		RuneService->SetOwnerClassId(ClassId);
	}
}
#endif

bool UIdleGameInstance::EquipPet(const FString& PetId)
{
	EnsurePetService();
	if (!PetService || !PetService->EquipPet(PetId))
	{
		return false;
	}

	if (ApiClient)
	{
		ApiClient->EquipPet(PetId);
	}
	RefreshPlayerCharacterStats();
	RequestAutosave();
	return true;
}

bool UIdleGameInstance::TryUnlockPet(const FString& PetId)
{
	EnsurePetService();
	if (!PetService || !PetService->TryUnlockPet(PetId))
	{
		return false;
	}

	RequestAutosave();
	return true;
}

FPetFeedResult UIdleGameInstance::TryFeedPet(const FString& PetId)
{
	EnsurePetService();
	EnsureMasteryService();
	FPetFeedResult Result;
	if (!PetService)
	{
		OnPetFed.Broadcast(Result);
		return Result;
	}

	bool bKnownPet = false;
	for (const FPetDefinition& Definition : PetService->GetPetDefinitions())
	{
		if (Definition.PetId == PetId)
		{
			bKnownPet = true;
			break;
		}
	}

	const int32 CurrentLevel = PetService->GetPetLevel(PetId);
	Result.NewLevel = CurrentLevel;
	if (!bKnownPet || !PetService->IsPetOwned(PetId) || CurrentLevel >= FPetLevelFormula::MaxPetLevel)
	{
		OnPetFed.Broadcast(Result);
		return Result;
	}

	// 야성 마스터리 2종: 펫 먹이 골드 비용 ×(1 - min(0.5, GetLocalBonus2(Beast))). 1종 펫 보너스와 별도 단일.
	const float BeastFeedReduction = MasteryService ? MasteryService->GetLocalBonus2(EMasteryTrack::Beast) : 0.0f;
	const int64 BaseFeedCost = FPetLevelFormula::GetFeedCost(CurrentLevel);
	// 비용 절감이 양수 비용을 0으로 만들어 먹이 가능 판정을 가리지 않도록 최소 1로 유지.
	const int64 Cost = BaseFeedCost > 0
		? FMath::Max<int64>(1, FMath::RoundToInt64(static_cast<double>(BaseFeedCost) * (1.0 - static_cast<double>(BeastFeedReduction))))
		: BaseFeedCost;
	if (Cost <= 0 || Gold < Cost)
	{
		OnPetFed.Broadcast(Result);
		return Result;
	}

	AddGold(-Cost);
	if (!PetService->FeedPet(PetId))
	{
		AddGold(Cost);
		OnPetFed.Broadcast(Result);
		return Result;
	}

	Result.bFed = true;
	Result.GoldSpent = Cost;
	Result.NewLevel = CurrentLevel + 1;
	if (PetService->GetEquippedPetId() == PetId)
	{
		RefreshPlayerCharacterStats();
	}
	RecordQuestProgress(EQuestObjective::FeedPet, 1);
	RecordAchievementMetric(EAchievementMetric::PetsFed, 1);
	RecordAchievementMetric(EAchievementMetric::HighestPetLevel, Result.NewLevel);
	if (MasteryService)
	{
		MasteryService->AddXp(EMasteryTrack::Beast, 5);
		RefreshPlayerCharacterStats();
	}
	OnPetFed.Broadcast(Result);
	RequestAutosave();
	return Result;
}

float UIdleGameInstance::GetEquippedPetGoldBonusPercent() const
{
	const float BaseBonus = PetService ? PetService->GetEquippedPetGoldBonusPercent() : 0.0f;
	const float BeastBonus = MasteryService ? MasteryService->GetLocalBonus(EMasteryTrack::Beast) : 0.0f;
	return BaseBonus * (1.0f + BeastBonus);
}

float UIdleGameInstance::GetEquippedPetDropBonusPercent() const
{
	const float BaseBonus = PetService ? PetService->GetEquippedPetDropBonusPercent() : 0.0f;
	const float BeastBonus = MasteryService ? MasteryService->GetLocalBonus(EMasteryTrack::Beast) : 0.0f;
	return BaseBonus * (1.0f + BeastBonus);
}

float UIdleGameInstance::GetEquippedPetExpBonusPercent() const
{
	const float BaseBonus = PetService ? PetService->GetEquippedPetExpBonusPercent() : 0.0f;
	const float BeastBonus = MasteryService ? MasteryService->GetLocalBonus(EMasteryTrack::Beast) : 0.0f;
	return BaseBonus * (1.0f + BeastBonus);
}

FPetStatBonus UIdleGameInstance::GetEquippedPetStatBonus() const
{
	FPetStatBonus Bonus = PetService ? PetService->GetEquippedPetStatBonus() : FPetStatBonus();
	const float BeastMultiplier = 1.0f + (MasteryService ? MasteryService->GetLocalBonus(EMasteryTrack::Beast) : 0.0f);
	Bonus.PhysAtkPct *= BeastMultiplier;
	Bonus.MagicAtkPct *= BeastMultiplier;
	Bonus.PhysDefPct *= BeastMultiplier;
	Bonus.MagicDefPct *= BeastMultiplier;
	Bonus.HpPct *= BeastMultiplier;
	return Bonus;
}

int64 UIdleGameInstance::ApplyEquippedPetGoldBonus(int64 BaseAmount) const
{
	if (BaseAmount <= 0)
	{
		return 0;
	}

	const double Multiplier = 1.0 + static_cast<double>(GetEquippedPetGoldBonusPercent()) / 100.0;
	return FMath::FloorToInt64(static_cast<double>(BaseAmount) * Multiplier);
}

float UIdleGameInstance::ApplyEquippedPetDropBonusChance(float BaseChance) const
{
	const float PetMultiplier = 1.0f + GetEquippedPetDropBonusPercent() / 100.0f;
	const float PetAdjusted = FMath::Clamp(BaseChance * PetMultiplier, 0.0f, 1.0f);
	const float ConsumableBonus = BuffService ? BuffService->GetDropBuffAdd(GetCurrentUnixSeconds()) : 0.0f;
	const float MasteryBonus = MasteryService ? MasteryService->GetGlobalBonus().DropRateAdd : 0.0f;
	return FMath::Clamp(PetAdjusted + ConsumableBonus + MasteryBonus, 0.0f, 1.0f);
}

int32 UIdleGameInstance::GetSeasonTokens() const
{
	return SeasonService ? SeasonService->GetSeasonTokens() : 0;
}

int32 UIdleGameInstance::GetReachedSeasonTier() const
{
	return SeasonService ? SeasonService->GetReachedTier() : 0;
}

FSeasonClaimResult UIdleGameInstance::ClaimSeasonReward(int32 Tier)
{
	EnsureSeasonService();
	FSeasonClaimResult Result;
	if (!SeasonService)
	{
		Result.Message = TEXT("season_service_unavailable");
		return Result;
	}

	Result = SeasonService->ClaimSeasonReward(Tier);
	if (!Result.bSuccess)
	{
		return Result;
	}

	if (Result.RewardType == ESeasonRewardType::Gold)
	{
		AddGold(Result.RewardAmount);
	}
	else if (Result.RewardType == ESeasonRewardType::Exp)
	{
		AddExp(Result.RewardAmount);
	}

	if (ApiClient)
	{
		ApiClient->ClaimSeasonReward(Tier);
	}
	RecordAchievementMetric(EAchievementMetric::SeasonRewardsClaimed, 1);
	RequestAutosave();
	return Result;
}

int64 UIdleGameInstance::GetCurrentUnixSeconds()
{
	return FDateTime::UtcNow().ToUnixTimestamp();
}

void UIdleGameInstance::RequestAutosave()
{
	if (bAutosaveSuppressed)
	{
		return;
	}

	bAutosavePending = true;
	UWorld* World = GetWorld();
	if (!World)
	{
		FlushPendingAutosave();
		return;
	}

	FTimerManager& WorldTimerManager = World->GetTimerManager();
	if (!WorldTimerManager.IsTimerActive(AutosaveTimerHandle))
	{
		WorldTimerManager.SetTimer(
			AutosaveTimerHandle,
			this,
			&UIdleGameInstance::FlushPendingAutosave,
			AutosaveDebounceSeconds,
			false);
	}
}

void UIdleGameInstance::FlushPendingAutosave()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutosaveTimerHandle);
	}

	if (!bAutosavePending || bAutosaveSuppressed)
	{
		return;
	}

	bAutosavePending = false;
	SaveProgress();
}

void UIdleGameInstance::SetCloudSyncState(ECloudSyncState NewState)
{
	if (CloudSyncState == NewState)
	{
		return;
	}

	CloudSyncState = NewState;
	OnCloudSyncStateChanged.Broadcast(CloudSyncState);
}

UInventoryComponent* UIdleGameInstance::FindPlayerInventory() const
{
	const UWorld* World = GetWorld();
	APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
	APawn* Pawn = PlayerController ? PlayerController->GetPawn() : nullptr;
	return Pawn ? Pawn->FindComponentByClass<UInventoryComponent>() : nullptr;
}

AIdleCharacter* UIdleGameInstance::FindPlayerCharacter() const
{
	const UWorld* World = GetWorld();
	APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
	APawn* Pawn = PlayerController ? PlayerController->GetPawn() : nullptr;
	return Cast<AIdleCharacter>(Pawn);
}

void UIdleGameInstance::EnsureQuestService()
{
	if (!QuestService)
	{
		QuestService = NewObject<UQuestService>(this);
		QuestService->InitializeDefaultQuests(UQuestService::GetCurrentUtcDateString());
	}
}

void UIdleGameInstance::EnsurePetService()
{
	if (!PetService)
	{
		PetService = NewObject<UPetService>(this);
		PetService->InitializeDefaultPets();
	}
}

void UIdleGameInstance::EnsureSeasonService()
{
	if (!SeasonService)
	{
		SeasonService = NewObject<USeasonService>(this);
		SeasonService->InitializeDefaultSeason();
	}
}

void UIdleGameInstance::EnsureStageService()
{
	if (!StageService)
	{
		StageService = NewObject<UStageService>(this);
		StageService->InitializeDefaultStages();
	}

	StageService->OnChapterBossDefeated.AddUniqueDynamic(this, &UIdleGameInstance::HandleChapterBossDefeated);
}

void UIdleGameInstance::EnsureTowerService()
{
	if (!TowerService)
	{
		TowerService = NewObject<UTowerService>(this);
		TowerService->InitializeTower();
	}
}

void UIdleGameInstance::EnsureDungeonService()
{
	if (!DungeonService)
	{
		DungeonService = NewObject<UDungeonService>(this);
		DungeonService->EnsureDailyReset(UQuestService::GetCurrentUtcDateString());
	}
}

void UIdleGameInstance::EnsureRuneService()
{
	if (!RuneService)
	{
		RuneService = NewObject<URuneService>(this);
	}
	RuneService->SetOwnerClassId(GetCurrentClassIdForRunes());
}

EClassId UIdleGameInstance::GetCurrentClassIdForRunes() const
{
	if (const AIdleCharacter* Character = FindPlayerCharacter())
	{
		return Character->GetClassId();
	}
	return EClassId::Warrior;
}

void UIdleGameInstance::EnsureAchievementService()
{
	if (!AchievementService)
	{
		AchievementService = NewObject<UAchievementService>(this);
		AchievementService->InitializeDefaultAchievements();
	}
}

void UIdleGameInstance::EnsureMasteryService()
{
	if (!MasteryService)
	{
		MasteryService = NewObject<UMasteryService>(this);
		MasteryService->Initialize();
	}
}

void UIdleGameInstance::EnsureLeaderboardService()
{
	if (!LeaderboardService)
	{
		LeaderboardService = NewObject<ULeaderboardService>(this);
	}
}

void UIdleGameInstance::EnsureBuffService()
{
	if (!BuffService)
	{
		BuffService = NewObject<UBuffService>(this);
		BuffService->Initialize();
	}
}

void UIdleGameInstance::EnsureWeeklyBossService()
{
	if (!WeeklyBossService)
	{
		WeeklyBossService = NewObject<UWeeklyBossService>(this);
		WeeklyBossService->EnsureWeek(UQuestService::GetCurrentUtcWeekString());
	}
}

void UIdleGameInstance::RefreshPlayerCharacterStats()
{
	if (AIdleCharacter* Character = FindPlayerCharacter())
	{
		Character->RefreshDerivedStats();
	}
}

void UIdleGameInstance::HandleChapterBossDefeated(int32 ClearedChapter)
{
	EnsureMasteryService();
	if (ClearedChapter == 1)
	{
		MarkChapter1BossDefeated();
	}
	RecordQuestProgress(EQuestObjective::DefeatBoss, 1);
	if (MasteryService)
	{
		MasteryService->AddXp(EMasteryTrack::Combat, 20);
		RefreshPlayerCharacterStats();
	}
}

void UIdleGameInstance::LoadLastSeenUnixSec()
{
	LastSeenUnixSec = GetCurrentUnixSeconds();
	if (!GConfig)
	{
		return;
	}

	FString SavedLastSeen;
	if (GConfig->GetString(TEXT("/Script/IdleProject.IdleGameInstance"), TEXT("LastSeenUnixSec"), SavedLastSeen, GGameUserSettingsIni))
	{
		int64 ParsedLastSeen = 0;
		if (LexTryParseString(ParsedLastSeen, *SavedLastSeen))
		{
			LastSeenUnixSec = FMath::Max<int64>(0, ParsedLastSeen);
		}
	}
}

void UIdleGameInstance::LoadLanguage()
{
	Language = TEXT("ko");
	if (GConfig)
	{
		FString SavedLanguage;
		if (GConfig->GetString(TEXT("/Script/IdleProject.IdleGameInstance"), TEXT("Language"), SavedLanguage, GGameUserSettingsIni))
		{
			Language = IdleProject::Localization::NormalizeLanguage(SavedLanguage);
		}
	}

	IdleProject::Localization::SetCurrentLanguage(Language);
}

void UIdleGameInstance::SaveLanguage() const
{
	if (!GConfig)
	{
		return;
	}

	GConfig->SetString(
		TEXT("/Script/IdleProject.IdleGameInstance"),
		TEXT("Language"),
		*Language,
		GGameUserSettingsIni);
	GConfig->Flush(false, GGameUserSettingsIni);
}

void UIdleGameInstance::SaveLastSeenUnixSec() const
{
	if (!GConfig)
	{
		return;
	}

	GConfig->SetString(
		TEXT("/Script/IdleProject.IdleGameInstance"),
		TEXT("LastSeenUnixSec"),
		*FString::Printf(TEXT("%lld"), LastSeenUnixSec),
		GGameUserSettingsIni);
	GConfig->Flush(false, GGameUserSettingsIni);
}
