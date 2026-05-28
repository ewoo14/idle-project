#include "GameCore/IdleGameInstance.h"

#include "CharacterSystem/IdleCharacter.h"
#include "CharacterSystem/LevelFormulas.h"
#include "CombatSystem/SkillComponent.h"
#include "GameCore/CloudSaveMergePolicy.h"
#include "GameCore/CloudSavePayloadMapper.h"
#include "GameCore/IdleSaveGame.h"
#include "GameCore/PetLevelFormula.h"
#include "GameCore/RebirthFormula.h"
#include "GameCore/RewardFormula.h"
#include "GameCore/TranscendFormula.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Internationalization/IdleLocalization.h"
#include "ItemSystem/EnhanceFormula.h"
#include "ItemSystem/InventoryComponent.h"
#include "ItemSystem/ItemFactory.h"
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
	EnsureRuneService();
	EnsureAchievementService();
	NextExp = FLevelFormulas::ExpToNext(CharacterLevel);
	EnhanceRandomStream.Initialize(FPlatformTime::Cycles());
	RuneRandomStream.Initialize(FPlatformTime::Cycles() ^ 0x51f15e);
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
	RuneService = nullptr;
	AchievementService = nullptr;
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

	if (Amount > 0 && Gold > MAX_int64 - Amount)
	{
		Gold = MAX_int64;
		RecordAchievementMetric(EAchievementMetric::GoldEarned, Amount);
		OnGoldChanged.Broadcast(Gold);
		RequestAutosave();
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

	Gold = Gold + Amount;
	if (Amount > 0)
	{
		RecordAchievementMetric(EAchievementMetric::GoldEarned, Amount);
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

bool UIdleGameInstance::CaptureToSave(UIdleSaveGame* SaveGame)
{
	if (!SaveGame)
	{
		return false;
	}

	EnsureStageService();
	EnsureTowerService();
	EnsureRuneService();
	EnsurePetService();
	EnsureQuestService();
	EnsureSeasonService();
	EnsureAchievementService();

	SaveGame->SaveVersion = 5;
	SaveGame->bHasSave = true;
	SaveGame->Gold = Gold;
	SaveGame->RuneEssence = RuneEssence;
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

	if (RuneService)
	{
		RuneService->CaptureState(SaveGame->Runes, SaveGame->EquippedRuneSlots, SaveGame->RuneCodex);
	}

	if (PetService)
	{
		SaveGame->EquippedPetId = PetService->GetEquippedPetId();
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
		StageService->RestoreState(
			SaveGame->StageChapter,
			SaveGame->StageStage,
			SaveGame->StageKillsThisStage,
			SaveGame->bStageFinalChapterCleared,
			SaveGame->StageHighestClearedChapter);
	}

	EnsureTowerService();
	if (TowerService)
	{
		TowerService->SetHighestFloor(SaveGame->TowerHighestFloor);
	}

	EnsureRuneService();
	if (RuneService)
	{
		RuneService->SetOwnerClassId(GetCurrentClassIdForRunes());
		if (SaveGame->SaveVersion >= 3)
		{
			if (SaveGame->SaveVersion >= 4)
			{
				RuneService->RestoreState(SaveGame->Runes, SaveGame->EquippedRuneSlots, SaveGame->RuneCodex);
			}
			else
			{
				RuneService->RestoreState(SaveGame->Runes, SaveGame->EquippedRuneSlots, {});
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
		PetService->RestoreState(SaveGame->EquippedPetId, SaveGame->PetLevels);
	}

	if (SaveGame->SaveVersion >= 2)
	{
		AIdleCharacter* Character = FindPlayerCharacter();
		PendingInventoryItems = SaveGame->InventoryItems;
		PendingEquippedSlotIndex = SaveGame->EquippedSlotIndex;
		PendingSkillRanks = SaveGame->SkillRanks;
		PendingSkillPoints = SaveGame->SkillPoints;
		bHasPendingCharacterSaveV2 = true;

		if (ApplyCharacterSaveState(
			Character,
			SaveGame->InventoryItems,
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
		AddGold(Reward);
		RequestAutosave();
	}
	return Reward;
}

FEnhanceAttemptResult UIdleGameInstance::TryEnhanceEquipped(EItemSlot Slot)
{
	return TryEnhanceEquipped(Slot, FindPlayerInventory());
}

FEnhanceAttemptResult UIdleGameInstance::TryEnhanceEquipped(EItemSlot Slot, UInventoryComponent* Inventory)
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
	const int64 Cost = FEnhanceFormula::GetEnhanceCost(CurrentLevel, Rarity);
	if (Cost <= 0 || Gold < Cost)
	{
		OnEnhanceResult.Broadcast(Result);
		return Result;
	}

	AddGold(-Cost);
	Result.bAttempted = true;
	Result.GoldSpent = Cost;

	const float SuccessRate = FEnhanceFormula::GetEnhanceSuccessRate(CurrentLevel);
	if (FEnhanceFormula::RollEnhanceSuccess(SuccessRate, EnhanceRandomStream) && Inventory->EnhanceEquippedItem(Slot))
	{
		Result.bSuccess = true;
		Result.NewLevel = CurrentLevel + 1;
	}

	RecordGearEnhanced();
	RecordAchievementMetric(EAchievementMetric::HighestEnhanceLevel, Result.NewLevel);
	OnEnhanceResult.Broadcast(Result);
	RequestAutosave();
	return Result;
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

void UIdleGameInstance::AddRune(const FRuneInstance& Rune)
{
	EnsureRuneService();
	if (!RuneService)
	{
		return;
	}

	const int32 PreviousCount = RuneService->GetOwnedRunes().Num();
	RuneService->AddRune(Rune);
	if (RuneService->GetOwnedRunes().Num() != PreviousCount)
	{
		RequestAutosave();
	}
}

bool UIdleGameInstance::TryEnhanceRune(int32 OwnedIndex)
{
	EnsureRuneService();
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

	RuneEssence = Refund > MAX_int64 - RuneEssence ? MAX_int64 : RuneEssence + Refund;
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

	CurrentExp += Amount;
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
	return RuneService ? RuneService->GetEquippedUtilValues().GoldFind : 0.0f;
}

float UIdleGameInstance::GetRuneExpBoostBonus() const
{
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
	const double OfflineMultiplier = 1.0 + static_cast<double>(GetRuneOfflineEffBonus());
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
	RecordQuestProgress(EQuestObjective::KillMonster, 1);
	RecordAchievementMetric(EAchievementMetric::MonstersKilled, 1);
}

void UIdleGameInstance::RecordGearEnhanced()
{
	RecordQuestProgress(EQuestObjective::Enhance, 1);
	RecordAchievementMetric(EAchievementMetric::GearEnhanced, 1);
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

	AddGold(Result.RewardGold);
	AddExp(Result.RewardExp);
	RecordAchievementMetric(EAchievementMetric::QuestsCompleted, 1);
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
	RequestAutosave();
	return true;
}

FPetFeedResult UIdleGameInstance::TryFeedPet(const FString& PetId)
{
	EnsurePetService();
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
	if (!bKnownPet || CurrentLevel >= FPetLevelFormula::MaxPetLevel)
	{
		OnPetFed.Broadcast(Result);
		return Result;
	}

	const int64 Cost = FPetLevelFormula::GetFeedCost(CurrentLevel);
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
	RecordQuestProgress(EQuestObjective::FeedPet, 1);
	RecordAchievementMetric(EAchievementMetric::PetsFed, 1);
	RecordAchievementMetric(EAchievementMetric::HighestPetLevel, Result.NewLevel);
	OnPetFed.Broadcast(Result);
	RequestAutosave();
	return Result;
}

float UIdleGameInstance::GetEquippedPetGoldBonusPercent() const
{
	return PetService ? PetService->GetEquippedPetGoldBonusPercent() : 0.0f;
}

float UIdleGameInstance::GetEquippedPetDropBonusPercent() const
{
	return PetService ? PetService->GetEquippedPetDropBonusPercent() : 0.0f;
}

int64 UIdleGameInstance::ApplyEquippedPetGoldBonus(int64 BaseAmount) const
{
	return PetService ? PetService->ApplyGoldBonus(BaseAmount) : BaseAmount;
}

float UIdleGameInstance::ApplyEquippedPetDropBonusChance(float BaseChance) const
{
	return PetService ? PetService->ApplyDropBonusChance(BaseChance) : BaseChance;
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

void UIdleGameInstance::RefreshPlayerCharacterStats()
{
	if (AIdleCharacter* Character = FindPlayerCharacter())
	{
		Character->RefreshDerivedStats();
	}
}

void UIdleGameInstance::HandleChapterBossDefeated(int32 ClearedChapter)
{
	if (ClearedChapter == 1)
	{
		MarkChapter1BossDefeated();
	}
	RecordQuestProgress(EQuestObjective::DefeatBoss, 1);
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
