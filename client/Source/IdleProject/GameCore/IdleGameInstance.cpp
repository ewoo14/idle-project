#include "GameCore/IdleGameInstance.h"

#include "CharacterSystem/IdleCharacter.h"
#include "CharacterSystem/LevelFormulas.h"
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
	NextExp = FLevelFormulas::ExpToNext(CharacterLevel);
	EnhanceRandomStream.Initialize(FPlatformTime::Cycles());
	LoadLanguage();
	LoadLastSeenUnixSec();
	LoadProgress();

	ApiClient->RegisterGuest([](bool bSuccess, FString Message)
	{
		UE_LOG(LogTemp, Display, TEXT("[NetworkClient] guest register callback success=%s message=%s"),
			bSuccess ? TEXT("true") : TEXT("false"),
			*Message);
	});
	ApiClient->RequestPetList();
	ApiClient->RequestSeasonState();
}

void UIdleGameInstance::Shutdown()
{
	LastSeenUnixSec = GetCurrentUnixSeconds();
	SaveProgress();
	SaveLanguage();
	SaveLastSeenUnixSec();
	ApiClient = nullptr;
	QuestService = nullptr;
	PetService = nullptr;
	SeasonService = nullptr;
	StageService = nullptr;
	TowerService = nullptr;
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
		OnGoldChanged.Broadcast(Gold);
		RequestAutosave();
		return;
	}
	if (Amount < 0 && (Amount == MIN_int64 || Gold < -Amount))
	{
		Gold = 0;
		OnGoldChanged.Broadcast(Gold);
		RequestAutosave();
		return;
	}

	Gold = Gold + Amount;
	OnGoldChanged.Broadcast(Gold);
	RequestAutosave();
}

void UIdleGameInstance::SaveProgress() const
{
	UIdleSaveGame* SaveGame = Cast<UIdleSaveGame>(UGameplayStatics::CreateSaveGameObject(UIdleSaveGame::StaticClass()));
	if (!CaptureToSave(SaveGame))
	{
		return;
	}

	UGameplayStatics::SaveGameToSlot(SaveGame, SaveSlotName, 0);
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

bool UIdleGameInstance::CaptureToSave(UIdleSaveGame* SaveGame) const
{
	if (!SaveGame)
	{
		return false;
	}

	SaveGame->SaveVersion = 1;
	SaveGame->bHasSave = true;
	SaveGame->Gold = Gold;
	SaveGame->CharacterLevel = CharacterLevel;
	SaveGame->CurrentExp = CurrentExp;
	SaveGame->NextExp = NextExp;
	SaveGame->RebirthCount = RebirthCount;
	SaveGame->RebirthBonusPoints = RebirthBonusPoints;
	SaveGame->TranscendCount = TranscendCount;
	SaveGame->AvailableStatPoints = AvailableStatPoints;
	SaveGame->AllocatedStats = AllocatedStats;
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

	if (PetService)
	{
		SaveGame->EquippedPetId = PetService->GetEquippedPetId();
		SaveGame->PetLevels = PetService->GetPetLevels();
	}

	return true;
}

bool UIdleGameInstance::ApplyFromSave(const UIdleSaveGame* SaveGame)
{
	if (!SaveGame || SaveGame->SaveVersion <= 0 || !SaveGame->bHasSave)
	{
		return false;
	}

	const bool bWasAutosaveSuppressed = bAutosaveSuppressed;
	bAutosaveSuppressed = true;

	Gold = FMath::Max<int64>(0, SaveGame->Gold);
	CharacterLevel = FMath::Max(1, SaveGame->CharacterLevel);
	CurrentExp = FMath::Max<int64>(0, SaveGame->CurrentExp);
	NextExp = SaveGame->NextExp > 0 ? SaveGame->NextExp : FLevelFormulas::ExpToNext(CharacterLevel);
	RebirthCount = FMath::Max(0, SaveGame->RebirthCount);
	RebirthBonusPoints = FMath::Max(0, SaveGame->RebirthBonusPoints);
	TranscendCount = FMath::Max(0, SaveGame->TranscendCount);
	AvailableStatPoints = FMath::Max(0, SaveGame->AvailableStatPoints);
	AllocatedStats = SaveGame->AllocatedStats;
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

	EnsurePetService();
	if (PetService)
	{
		PetService->RestoreState(SaveGame->EquippedPetId, SaveGame->PetLevels);
	}

	OnGoldChanged.Broadcast(Gold);
	OnExpChanged.Broadcast(CurrentExp, NextExp);
	OnStatPointsChanged.Broadcast();
	OnLevelUp.Broadcast(CharacterLevel);

	bAutosaveSuppressed = bWasAutosaveSuppressed;
	return true;
}

int64 UIdleGameInstance::ClimbTower()
{
	EnsureTowerService();
	AIdleCharacter* Character = FindPlayerCharacter();
	if (!TowerService || !Character)
	{
		return 0;
	}

	const int64 Reward = TowerService->TryClimbTower(Character->GetCombatPower());
	if (Reward > 0)
	{
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
	Inventory->AddItem(Item);

	Result.bPurchased = true;
	Result.GoldSpent = Cost;
	Result.Rarity = Item.Rarity;
	Result.Slot = Item.Slot;
	Result.ItemName = Item.DisplayName;
	OnShopPurchase.Broadcast(Result);
	RequestAutosave();
	return Result;
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
	LastSeenUnixSec = FMath::Max(LastSeenUnixSec, NowUnixSec);
	RequestAutosave();
	return Reward;
}

FOfflineRewardResult UIdleGameInstance::PreviewOfflineRewards(int64 NowUnixSec, int32 RebirthCountOverride) const
{
	const int32 EffectiveRebirthCount = RebirthCountOverride >= 0 ? RebirthCountOverride : RebirthCount;
	return FOfflineRewardFormula::ComputeOfflineRewards(CharacterLevel, LastSeenUnixSec, NowUnixSec, EffectiveRebirthCount);
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
	QuestService->RecordProgress(Objective, Amount);
}

void UIdleGameInstance::RecordMonsterKilled()
{
	RecordQuestProgress(EQuestObjective::KillMonster, 1);
}

void UIdleGameInstance::RecordGearEnhanced()
{
	RecordQuestProgress(EQuestObjective::Enhance, 1);
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
	Result = QuestService->ClaimQuest(QuestId);
	if (!Result.bSuccess)
	{
		return Result;
	}

	AddGold(Result.RewardGold);
	AddExp(Result.RewardExp);
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
	RequestAutosave();
	return Result;
}

int64 UIdleGameInstance::GetCurrentUnixSeconds()
{
	return FDateTime::UtcNow().ToUnixTimestamp();
}

void UIdleGameInstance::RequestAutosave() const
{
	if (!bAutosaveSuppressed)
	{
		SaveProgress();
	}
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

void UIdleGameInstance::HandleChapterBossDefeated(int32 ClearedChapter)
{
	if (ClearedChapter == 1)
	{
		MarkChapter1BossDefeated();
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
