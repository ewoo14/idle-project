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
#include "GameCore/GuildService.h"
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
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
// jumbo(unity) 빌드 ODR 회피용 고유 prefix(GuildInstance~) 헬퍼.
const TCHAR* GuildInstanceRankToServerString(EGuildRank Rank)
{
	switch (Rank)
	{
	case EGuildRank::Master:
		return TEXT("master");
	case EGuildRank::Vice:
		return TEXT("vice");
	case EGuildRank::Officer:
		return TEXT("officer");
	default:
		return TEXT("member");
	}
}

EGuildJoinMode GuildInstanceParseJoinMode(const FString& JoinModeString)
{
	return JoinModeString == TEXT("approval") ? EGuildJoinMode::Approval : EGuildJoinMode::Open;
}

// 서버 `GET /v1/guilds` 목록 응답(`{ok,data:{guilds:[...]}}` 또는 `{ok,data:[...]}`)을 요약 배열로 파싱.
void GuildInstanceParseListResponse(const FString& JsonBody, TArray<FGuildSummary>& OutSummaries)
{
	OutSummaries.Reset();
	if (JsonBody.IsEmpty())
	{
		return;
	}

	TSharedPtr<FJsonObject> ResponseJson;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonBody);
	if (!FJsonSerializer::Deserialize(Reader, ResponseJson) || !ResponseJson.IsValid())
	{
		return;
	}

	bool bOk = false;
	if (!ResponseJson->TryGetBoolField(TEXT("ok"), bOk) || !bOk)
	{
		return;
	}

	// data 가 배열이거나 {guilds:[...]} 형태 모두 허용.
	const TArray<TSharedPtr<FJsonValue>>* GuildArray = nullptr;
	if (!ResponseJson->TryGetArrayField(TEXT("data"), GuildArray) || !GuildArray)
	{
		const TSharedPtr<FJsonObject>* DataObjectPtr = nullptr;
		if (ResponseJson->TryGetObjectField(TEXT("data"), DataObjectPtr) && DataObjectPtr && DataObjectPtr->IsValid())
		{
			(*DataObjectPtr)->TryGetArrayField(TEXT("guilds"), GuildArray);
		}
	}

	if (!GuildArray)
	{
		return;
	}

	OutSummaries.Reserve(GuildArray->Num());
	for (const TSharedPtr<FJsonValue>& Value : *GuildArray)
	{
		const TSharedPtr<FJsonObject> GuildObject = Value.IsValid() ? Value->AsObject() : nullptr;
		if (!GuildObject.IsValid())
		{
			continue;
		}

		FGuildSummary Summary;
		GuildObject->TryGetStringField(TEXT("id"), Summary.Id);
		GuildObject->TryGetStringField(TEXT("name"), Summary.Name);

		int32 Level = 1;
		if (GuildObject->TryGetNumberField(TEXT("level"), Level))
		{
			Summary.Level = FMath::Max(1, Level);
		}
		int32 MemberCount = 0;
		if (GuildObject->TryGetNumberField(TEXT("memberCount"), MemberCount))
		{
			Summary.MemberCount = FMath::Max(0, MemberCount);
		}
		FString JoinModeString;
		if (GuildObject->TryGetStringField(TEXT("joinMode"), JoinModeString))
		{
			Summary.JoinMode = GuildInstanceParseJoinMode(JoinModeString);
		}
		OutSummaries.Add(MoveTemp(Summary));
	}
}

// 길드 상점 응답(`{ok,data:{items:[{id,name,price,reward}]}}`) 파싱 — 가격은 정수 포인트.
void GuildInstanceParseShopResponse(const FString& JsonBody, TArray<FGuildShopItemInfo>& OutItems)
{
	OutItems.Reset();
	if (JsonBody.IsEmpty())
	{
		return;
	}

	TSharedPtr<FJsonObject> ResponseJson;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonBody);
	if (!FJsonSerializer::Deserialize(Reader, ResponseJson) || !ResponseJson.IsValid())
	{
		return;
	}

	bool bOk = false;
	if (!ResponseJson->TryGetBoolField(TEXT("ok"), bOk) || !bOk)
	{
		return;
	}

	const TSharedPtr<FJsonObject>* DataObjectPtr = nullptr;
	if (!ResponseJson->TryGetObjectField(TEXT("data"), DataObjectPtr) || !DataObjectPtr || !DataObjectPtr->IsValid())
	{
		return;
	}

	const TArray<TSharedPtr<FJsonValue>>* ItemArray = nullptr;
	if (!(*DataObjectPtr)->TryGetArrayField(TEXT("items"), ItemArray) || !ItemArray)
	{
		return;
	}

	OutItems.Reserve(ItemArray->Num());
	for (const TSharedPtr<FJsonValue>& Value : *ItemArray)
	{
		const TSharedPtr<FJsonObject> ItemObject = Value.IsValid() ? Value->AsObject() : nullptr;
		if (!ItemObject.IsValid())
		{
			continue;
		}

		FGuildShopItemInfo Item;
		ItemObject->TryGetStringField(TEXT("id"), Item.Id);
		ItemObject->TryGetStringField(TEXT("name"), Item.Name);
		double Price = 0.0;
		if (ItemObject->TryGetNumberField(TEXT("price"), Price))
		{
			Item.Price = FMath::Max<int64>(0, static_cast<int64>(Price));
		}
		if (!Item.Id.IsEmpty())
		{
			OutItems.Add(MoveTemp(Item));
		}
	}
}

/**
 * 길드 상점 구매 응답({ok,data:{reward:{type,amount},...}})에서 보상 타입/수량을 파싱한다.
 * 성공 시 true 와 OutType/OutAmount 를 채우며, ok≠true·reward 누락 시 false.
 */
bool GuildInstanceParseBuyReward(const FString& JsonBody, FString& OutType, int64& OutAmount)
{
	OutType.Reset();
	OutAmount = 0;
	if (JsonBody.IsEmpty())
	{
		return false;
	}

	TSharedPtr<FJsonObject> ResponseJson;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonBody);
	if (!FJsonSerializer::Deserialize(Reader, ResponseJson) || !ResponseJson.IsValid())
	{
		return false;
	}

	bool bOk = false;
	if (!ResponseJson->TryGetBoolField(TEXT("ok"), bOk) || !bOk)
	{
		return false;
	}

	const TSharedPtr<FJsonObject>* DataObjectPtr = nullptr;
	if (!ResponseJson->TryGetObjectField(TEXT("data"), DataObjectPtr) || !DataObjectPtr || !DataObjectPtr->IsValid())
	{
		return false;
	}

	const TSharedPtr<FJsonObject>* RewardObjectPtr = nullptr;
	if (!(*DataObjectPtr)->TryGetObjectField(TEXT("reward"), RewardObjectPtr) || !RewardObjectPtr || !RewardObjectPtr->IsValid())
	{
		return false;
	}

	const TSharedPtr<FJsonObject>& Reward = *RewardObjectPtr;
	if (!Reward->TryGetStringField(TEXT("type"), OutType) || OutType.IsEmpty())
	{
		return false;
	}

	// amount 는 서버가 number 로 직렬화(소비형 소량). 숫자/문자열 모두 허용.
	double AmountNumber = 0.0;
	FString AmountString;
	if (Reward->TryGetNumberField(TEXT("amount"), AmountNumber))
	{
		OutAmount = static_cast<int64>(AmountNumber);
	}
	else if (Reward->TryGetStringField(TEXT("amount"), AmountString))
	{
		OutAmount = FCString::Atoi64(*AmountString);
	}
	return true;
}

/** 서버 bigint(문자열) 또는 number 필드를 int64 로 파싱(랭킹 weeklyContribution 등). */
int64 GuildInstanceParseInt64Field(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName)
{
	if (!Object.IsValid())
	{
		return 0;
	}
	FString StringValue;
	if (Object->TryGetStringField(FieldName, StringValue))
	{
		return FCString::Atoi64(*StringValue);
	}
	double NumberValue = 0.0;
	if (Object->TryGetNumberField(FieldName, NumberValue))
	{
		return static_cast<int64>(NumberValue);
	}
	return 0;
}

/**
 * 보스 격파 보상 수령 응답(`data.rewards` 배열)을 (type, amount) 쌍 배열로 파싱한다.
 * 서버 claimBossReward 응답: { defeats, claimedCount, rewards: [{type, amount}, ...] }.
 * 격파 N건 누적이므로 각 reward.amount 는 이미 N 배가 적용된 값(서버 권위).
 */
bool GuildInstanceParseBossRewards(const FString& JsonBody, TArray<TPair<FString, int64>>& OutRewards)
{
	OutRewards.Reset();
	if (JsonBody.IsEmpty())
	{
		return false;
	}

	TSharedPtr<FJsonObject> ResponseJson;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonBody);
	if (!FJsonSerializer::Deserialize(Reader, ResponseJson) || !ResponseJson.IsValid())
	{
		return false;
	}

	bool bOk = false;
	if (!ResponseJson->TryGetBoolField(TEXT("ok"), bOk) || !bOk)
	{
		return false;
	}

	const TSharedPtr<FJsonObject>* DataObjectPtr = nullptr;
	if (!ResponseJson->TryGetObjectField(TEXT("data"), DataObjectPtr) || !DataObjectPtr || !DataObjectPtr->IsValid())
	{
		return false;
	}

	const TArray<TSharedPtr<FJsonValue>>* RewardArray = nullptr;
	if (!(*DataObjectPtr)->TryGetArrayField(TEXT("rewards"), RewardArray) || !RewardArray)
	{
		return false;
	}

	OutRewards.Reserve(RewardArray->Num());
	for (const TSharedPtr<FJsonValue>& Value : *RewardArray)
	{
		const TSharedPtr<FJsonObject> RewardObject = Value.IsValid() ? Value->AsObject() : nullptr;
		if (!RewardObject.IsValid())
		{
			continue;
		}
		FString Type;
		if (!RewardObject->TryGetStringField(TEXT("type"), Type) || Type.IsEmpty())
		{
			continue;
		}
		const int64 Amount = GuildInstanceParseInt64Field(RewardObject, TEXT("amount"));
		OutRewards.Add(TPair<FString, int64>(Type, Amount));
	}
	return true;
}

/** 단일 랭킹 행 JSON → FGuildRankingEntry(서버 toRankingResponse parity). */
FGuildRankingEntry GuildInstanceParseRankingEntry(const TSharedPtr<FJsonObject>& RowObject)
{
	FGuildRankingEntry Entry;
	if (!RowObject.IsValid())
	{
		return Entry;
	}
	int32 RankValue = 0;
	if (RowObject->TryGetNumberField(TEXT("rank"), RankValue))
	{
		Entry.Rank = FMath::Max(0, RankValue);
	}
	RowObject->TryGetStringField(TEXT("guildId"), Entry.GuildId);
	RowObject->TryGetStringField(TEXT("name"), Entry.Name);
	int32 LevelValue = 1;
	if (RowObject->TryGetNumberField(TEXT("level"), LevelValue))
	{
		Entry.Level = FMath::Max(1, LevelValue);
	}
	Entry.WeeklyContribution = FMath::Max<int64>(0, GuildInstanceParseInt64Field(RowObject, TEXT("weeklyContribution")));
	return Entry;
}

/**
 * 주간 길드 랭킹 응답(`data.rankings[]` + `data.me`)을 파싱한다.
 * 서버 guildRankings: { rankings: [...], me: <행 or null> }.
 */
bool GuildInstanceParseRankings(const FString& JsonBody, TArray<FGuildRankingEntry>& OutRankings, FGuildRankingEntry& OutMyRank)
{
	OutRankings.Reset();
	OutMyRank = FGuildRankingEntry();
	if (JsonBody.IsEmpty())
	{
		return false;
	}

	TSharedPtr<FJsonObject> ResponseJson;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonBody);
	if (!FJsonSerializer::Deserialize(Reader, ResponseJson) || !ResponseJson.IsValid())
	{
		return false;
	}

	bool bOk = false;
	if (!ResponseJson->TryGetBoolField(TEXT("ok"), bOk) || !bOk)
	{
		return false;
	}

	const TSharedPtr<FJsonObject>* DataObjectPtr = nullptr;
	if (!ResponseJson->TryGetObjectField(TEXT("data"), DataObjectPtr) || !DataObjectPtr || !DataObjectPtr->IsValid())
	{
		return false;
	}
	const TSharedPtr<FJsonObject>& Data = *DataObjectPtr;

	const TArray<TSharedPtr<FJsonValue>>* RankingArray = nullptr;
	if (Data->TryGetArrayField(TEXT("rankings"), RankingArray) && RankingArray)
	{
		OutRankings.Reserve(RankingArray->Num());
		for (const TSharedPtr<FJsonValue>& Value : *RankingArray)
		{
			const TSharedPtr<FJsonObject> RowObject = Value.IsValid() ? Value->AsObject() : nullptr;
			if (!RowObject.IsValid())
			{
				continue;
			}
			OutRankings.Add(GuildInstanceParseRankingEntry(RowObject));
		}
	}

	// me 는 무소속이면 null — 그 경우 OutMyRank 는 기본값(rank 0) 유지.
	const TSharedPtr<FJsonObject>* MeObjectPtr = nullptr;
	if (Data->TryGetObjectField(TEXT("me"), MeObjectPtr) && MeObjectPtr && MeObjectPtr->IsValid())
	{
		OutMyRank = GuildInstanceParseRankingEntry(*MeObjectPtr);
	}
	return true;
}

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
	EnsureTitleService();
	EnsureMissionService();
	EnsureLeaderboardService();
	EnsureBuffService();
	EnsureWeeklyBossService();
	EnsureGuildService();
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
	RefreshGuildSnapshot();
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
	TitleService = nullptr;
	MissionService = nullptr;
	LeaderboardService = nullptr;
	BuffService = nullptr;
	WeeklyBossService = nullptr;
	GuildService = nullptr;
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
		EnsureGuildService();
		EnsureTitleService();
		// 길드 골드 버프는 소비 버프와 동일한 단일 합산 지점(여기)에만 더한다(이중 적용 금지, #72).
		const double GoldBuffPct = static_cast<double>(BuffService ? BuffService->GetGoldBuffPct(GetCurrentUnixSeconds()) : 0.0f);
		const double GuildGoldPct = static_cast<double>(GuildService ? GuildService->GetGuildBuff().GoldPct : 0.0f);
		// 칭호 GoldPct 도 같은 단일 골드획득 합산 지점(여기)에만 더한다(이중 적용 금지, #72).
		const FTitleBonus TitleBonus = TitleService ? TitleService->GetEquippedTitleBonus() : FTitleBonus();
		const double TitleGoldPct = TitleBonus.Type == ETitleBonus::GoldPct ? static_cast<double>(TitleBonus.Value) : 0.0;
		const double GoldMultiplier = 1.0 + GoldBuffPct + GuildGoldPct + TitleGoldPct;
		// (double)MAX_int64 * 배수는 int64 범위를 넘겨 RoundToInt64에서 오버플로(UB)하므로
		// 범위 초과 시 EffectiveAmount를 MAX_int64로 포화시킨다(아래 합산 포화 로직과 정합).
		const double ScaledAmount = static_cast<double>(Amount) * GoldMultiplier;
		EffectiveAmount = ScaledAmount >= static_cast<double>(MAX_int64)
			? MAX_int64
			: FMath::Max<int64>(0, FMath::RoundToInt64(ScaledAmount));
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
		// 세이브 시점에 미플러시 자동 기여를 서버로 플러시(소속·pending>0 일 때만 동작).
		FlushPendingGuildContribution();
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

void UIdleGameInstance::RefreshGuildSnapshot()
{
	EnsureGuildService();

	if (!ApiClient)
	{
		return;
	}

	TWeakObjectPtr<UIdleGameInstance> WeakThis(this);
	ApiClient->EnsureCharacter([WeakThis](bool bCharacterOk, FString CharacterId) mutable
	{
		UIdleGameInstance* StrongThis = WeakThis.Get();
		if (!StrongThis || !StrongThis->ApiClient)
		{
			return;
		}

		if (!bCharacterOk || CharacterId.IsEmpty())
		{
			return;
		}

		StrongThis->ApiClient->GetMyGuild(CharacterId, [WeakThis](bool bSuccess, FString Body) mutable
		{
			UIdleGameInstance* StrongInner = WeakThis.Get();
			if (!StrongInner)
			{
				return;
			}

			StrongInner->EnsureGuildService();
			if (StrongInner->GuildService)
			{
				// 서버 권위 — 성공 시 스냅샷 캐시 갱신(무소속이면 false 로 캐시 비움).
				const bool bParsed = StrongInner->GuildService->ParseSnapshotJson(bSuccess ? Body : FString());
				// 재접속 동기화 직후 미플러시 자동 기여가 있으면 서버로 플러시(주간 상한 클램프).
				if (bParsed && StrongInner->GuildService->GetPendingAutoContribution() > 0)
				{
					StrongInner->FlushPendingGuildContribution();
				}
			}
		});
	});
}

void UIdleGameInstance::FlushPendingGuildContribution()
{
	EnsureGuildService();
	if (!ApiClient || !GuildService || !GuildService->HasGuild())
	{
		return;
	}
	if (GuildService->GetPendingAutoContribution() <= 0)
	{
		return;
	}

	const FString GuildId = GuildService->GetCachedGuildId();
	if (GuildId.IsEmpty())
	{
		return;
	}

	TWeakObjectPtr<UIdleGameInstance> WeakThis(this);
	ApiClient->EnsureCharacter([WeakThis, GuildId](bool bCharacterOk, FString CharacterId) mutable
	{
		UIdleGameInstance* StrongThis = WeakThis.Get();
		if (!StrongThis || !StrongThis->ApiClient || !bCharacterOk || CharacterId.IsEmpty())
		{
			return;
		}

		StrongThis->EnsureGuildService();
		if (!StrongThis->GuildService || !StrongThis->GuildService->HasGuild())
		{
			return;
		}

		// 주간 상한 클램프 후 소비. 0 이면 플러시 불필요.
		const int64 Amount = StrongThis->GuildService->ConsumePendingAutoContribution();
		if (Amount <= 0)
		{
			return;
		}

		StrongThis->ApiClient->GuildContribute(GuildId, CharacterId, Amount, [WeakThis, Amount](bool bSuccess, FString)
		{
			UIdleGameInstance* StrongInner = WeakThis.Get();
			if (!StrongInner)
			{
				return;
			}
			StrongInner->EnsureGuildService();
			if (!bSuccess)
			{
				// 실패 시 소비분을 되돌려 다음 기회에 재시도(유실 방지).
				if (StrongInner->GuildService)
				{
					StrongInner->GuildService->AddPendingAutoContribution(Amount);
				}
				return;
			}
			// 성공 → 서버 권위 스냅샷 재동기화(레벨/버프/포인트 갱신).
			StrongInner->RefreshGuildSnapshot();
		});
	});
}

void UIdleGameInstance::GuildPanelRefreshList(const FString& Query, TFunction<void(bool, const TArray<FGuildSummary>&)> OnComplete)
{
	if (!ApiClient)
	{
		if (OnComplete)
		{
			OnComplete(false, TArray<FGuildSummary>());
		}
		return;
	}

	ApiClient->ListGuilds(Query, 20, 0, [OnComplete = MoveTemp(OnComplete)](bool bSuccess, FString Body)
	{
		TArray<FGuildSummary> Summaries;
		if (bSuccess)
		{
			GuildInstanceParseListResponse(Body, Summaries);
		}
		if (OnComplete)
		{
			OnComplete(bSuccess, Summaries);
		}
	});
}

void UIdleGameInstance::GuildPanelCreate(const FString& Name, TFunction<void(bool)> OnComplete)
{
	EnsureGuildService();
	if (!ApiClient)
	{
		if (OnComplete)
		{
			OnComplete(false);
		}
		return;
	}

	TWeakObjectPtr<UIdleGameInstance> WeakThis(this);
	ApiClient->EnsureCharacter([WeakThis, Name, OnComplete = MoveTemp(OnComplete)](bool bCharacterOk, FString CharacterId) mutable
	{
		UIdleGameInstance* StrongThis = WeakThis.Get();
		if (!StrongThis || !StrongThis->ApiClient || !bCharacterOk || CharacterId.IsEmpty())
		{
			if (OnComplete)
			{
				OnComplete(false);
			}
			return;
		}

		StrongThis->ApiClient->CreateGuild(CharacterId, Name, [WeakThis, OnComplete = MoveTemp(OnComplete)](bool bSuccess, FString)
		{
			if (UIdleGameInstance* StrongInner = WeakThis.Get())
			{
				if (bSuccess)
				{
					StrongInner->RefreshGuildSnapshot();
				}
			}
			if (OnComplete)
			{
				OnComplete(bSuccess);
			}
		});
	});
}

void UIdleGameInstance::GuildPanelJoin(const FString& GuildId, TFunction<void(bool)> OnComplete)
{
	EnsureGuildService();
	if (!ApiClient || GuildId.IsEmpty())
	{
		if (OnComplete)
		{
			OnComplete(false);
		}
		return;
	}

	TWeakObjectPtr<UIdleGameInstance> WeakThis(this);
	ApiClient->EnsureCharacter([WeakThis, GuildId, OnComplete = MoveTemp(OnComplete)](bool bCharacterOk, FString CharacterId) mutable
	{
		UIdleGameInstance* StrongThis = WeakThis.Get();
		if (!StrongThis || !StrongThis->ApiClient || !bCharacterOk || CharacterId.IsEmpty())
		{
			if (OnComplete)
			{
				OnComplete(false);
			}
			return;
		}

		StrongThis->ApiClient->JoinGuild(GuildId, CharacterId, [WeakThis, OnComplete = MoveTemp(OnComplete)](bool bSuccess, FString)
		{
			if (UIdleGameInstance* StrongInner = WeakThis.Get())
			{
				if (bSuccess)
				{
					StrongInner->RefreshGuildSnapshot();
				}
			}
			if (OnComplete)
			{
				OnComplete(bSuccess);
			}
		});
	});
}

void UIdleGameInstance::GuildPanelLeave(const FString& GuildId, TFunction<void(bool)> OnComplete)
{
	EnsureGuildService();
	if (!ApiClient || GuildId.IsEmpty())
	{
		if (OnComplete)
		{
			OnComplete(false);
		}
		return;
	}

	TWeakObjectPtr<UIdleGameInstance> WeakThis(this);
	ApiClient->EnsureCharacter([WeakThis, GuildId, OnComplete = MoveTemp(OnComplete)](bool bCharacterOk, FString CharacterId) mutable
	{
		UIdleGameInstance* StrongThis = WeakThis.Get();
		if (!StrongThis || !StrongThis->ApiClient || !bCharacterOk || CharacterId.IsEmpty())
		{
			if (OnComplete)
			{
				OnComplete(false);
			}
			return;
		}

		StrongThis->ApiClient->LeaveGuild(GuildId, CharacterId, [WeakThis, OnComplete = MoveTemp(OnComplete)](bool bSuccess, FString)
		{
			if (UIdleGameInstance* StrongInner = WeakThis.Get())
			{
				if (bSuccess)
				{
					StrongInner->RefreshGuildSnapshot();
				}
			}
			if (OnComplete)
			{
				OnComplete(bSuccess);
			}
		});
	});
}

void UIdleGameInstance::GuildPanelApprove(const FString& GuildId, const FString& TargetCharacterId, TFunction<void(bool)> OnComplete)
{
	EnsureGuildService();
	if (!ApiClient || GuildId.IsEmpty() || TargetCharacterId.IsEmpty())
	{
		if (OnComplete)
		{
			OnComplete(false);
		}
		return;
	}

	TWeakObjectPtr<UIdleGameInstance> WeakThis(this);
	ApiClient->EnsureCharacter([WeakThis, GuildId, TargetCharacterId, OnComplete = MoveTemp(OnComplete)](bool bCharacterOk, FString ActorCharacterId) mutable
	{
		UIdleGameInstance* StrongThis = WeakThis.Get();
		if (!StrongThis || !StrongThis->ApiClient || !bCharacterOk || ActorCharacterId.IsEmpty())
		{
			if (OnComplete)
			{
				OnComplete(false);
			}
			return;
		}

		StrongThis->ApiClient->ApproveRequest(GuildId, TargetCharacterId, ActorCharacterId, [WeakThis, OnComplete = MoveTemp(OnComplete)](bool bSuccess, FString)
		{
			if (UIdleGameInstance* StrongInner = WeakThis.Get())
			{
				if (bSuccess)
				{
					StrongInner->RefreshGuildSnapshot();
				}
			}
			if (OnComplete)
			{
				OnComplete(bSuccess);
			}
		});
	});
}

void UIdleGameInstance::GuildPanelReject(const FString& GuildId, const FString& TargetCharacterId, TFunction<void(bool)> OnComplete)
{
	EnsureGuildService();
	if (!ApiClient || GuildId.IsEmpty() || TargetCharacterId.IsEmpty())
	{
		if (OnComplete)
		{
			OnComplete(false);
		}
		return;
	}

	TWeakObjectPtr<UIdleGameInstance> WeakThis(this);
	ApiClient->EnsureCharacter([WeakThis, GuildId, TargetCharacterId, OnComplete = MoveTemp(OnComplete)](bool bCharacterOk, FString ActorCharacterId) mutable
	{
		UIdleGameInstance* StrongThis = WeakThis.Get();
		if (!StrongThis || !StrongThis->ApiClient || !bCharacterOk || ActorCharacterId.IsEmpty())
		{
			if (OnComplete)
			{
				OnComplete(false);
			}
			return;
		}

		StrongThis->ApiClient->RejectRequest(GuildId, TargetCharacterId, ActorCharacterId, [WeakThis, OnComplete = MoveTemp(OnComplete)](bool bSuccess, FString)
		{
			if (UIdleGameInstance* StrongInner = WeakThis.Get())
			{
				if (bSuccess)
				{
					StrongInner->RefreshGuildSnapshot();
				}
			}
			if (OnComplete)
			{
				OnComplete(bSuccess);
			}
		});
	});
}

void UIdleGameInstance::GuildPanelSetRank(const FString& GuildId, const FString& TargetCharacterId, EGuildRank NewRank, TFunction<void(bool)> OnComplete)
{
	EnsureGuildService();
	if (!ApiClient || GuildId.IsEmpty() || TargetCharacterId.IsEmpty())
	{
		if (OnComplete)
		{
			OnComplete(false);
		}
		return;
	}

	const FString RankString = GuildInstanceRankToServerString(NewRank);
	TWeakObjectPtr<UIdleGameInstance> WeakThis(this);
	ApiClient->EnsureCharacter([WeakThis, GuildId, TargetCharacterId, RankString, OnComplete = MoveTemp(OnComplete)](bool bCharacterOk, FString ActorCharacterId) mutable
	{
		UIdleGameInstance* StrongThis = WeakThis.Get();
		if (!StrongThis || !StrongThis->ApiClient || !bCharacterOk || ActorCharacterId.IsEmpty())
		{
			if (OnComplete)
			{
				OnComplete(false);
			}
			return;
		}

		StrongThis->ApiClient->SetMemberRank(GuildId, TargetCharacterId, ActorCharacterId, RankString, [WeakThis, OnComplete = MoveTemp(OnComplete)](bool bSuccess, FString)
		{
			if (UIdleGameInstance* StrongInner = WeakThis.Get())
			{
				if (bSuccess)
				{
					StrongInner->RefreshGuildSnapshot();
				}
			}
			if (OnComplete)
			{
				OnComplete(bSuccess);
			}
		});
	});
}

void UIdleGameInstance::GuildPanelUpdateJoinMode(const FString& GuildId, EGuildJoinMode NewJoinMode, TFunction<void(bool)> OnComplete)
{
	EnsureGuildService();
	if (!ApiClient || GuildId.IsEmpty())
	{
		if (OnComplete)
		{
			OnComplete(false);
		}
		return;
	}

	const FString JoinModeString = NewJoinMode == EGuildJoinMode::Approval ? TEXT("approval") : TEXT("open");
	TWeakObjectPtr<UIdleGameInstance> WeakThis(this);
	ApiClient->EnsureCharacter([WeakThis, GuildId, JoinModeString, OnComplete = MoveTemp(OnComplete)](bool bCharacterOk, FString ActorCharacterId) mutable
	{
		UIdleGameInstance* StrongThis = WeakThis.Get();
		if (!StrongThis || !StrongThis->ApiClient || !bCharacterOk || ActorCharacterId.IsEmpty())
		{
			if (OnComplete)
			{
				OnComplete(false);
			}
			return;
		}

		// name/notice 는 본 PR UI 범위 밖 — 빈 문자열은 서버에서 미변경으로 처리(설정 PATCH).
		StrongThis->ApiClient->UpdateGuildSettings(GuildId, ActorCharacterId, FString(), FString(), JoinModeString, [WeakThis, OnComplete = MoveTemp(OnComplete)](bool bSuccess, FString)
		{
			if (UIdleGameInstance* StrongInner = WeakThis.Get())
			{
				if (bSuccess)
				{
					StrongInner->RefreshGuildSnapshot();
				}
			}
			if (OnComplete)
			{
				OnComplete(bSuccess);
			}
		});
	});
}

void UIdleGameInstance::GuildPanelAttendance(TFunction<void(bool)> OnComplete)
{
	EnsureGuildService();
	const FString GuildId = GuildService ? GuildService->GetCachedGuildId() : FString();
	if (!ApiClient || GuildId.IsEmpty())
	{
		if (OnComplete)
		{
			OnComplete(false);
		}
		return;
	}

	TWeakObjectPtr<UIdleGameInstance> WeakThis(this);
	ApiClient->EnsureCharacter([WeakThis, GuildId, OnComplete = MoveTemp(OnComplete)](bool bCharacterOk, FString CharacterId) mutable
	{
		UIdleGameInstance* StrongThis = WeakThis.Get();
		if (!StrongThis || !StrongThis->ApiClient || !bCharacterOk || CharacterId.IsEmpty())
		{
			if (OnComplete)
			{
				OnComplete(false);
			}
			return;
		}

		StrongThis->ApiClient->GuildAttendance(GuildId, CharacterId, [WeakThis, OnComplete = MoveTemp(OnComplete)](bool bSuccess, FString)
		{
			if (UIdleGameInstance* StrongInner = WeakThis.Get())
			{
				if (bSuccess)
				{
					StrongInner->RefreshGuildSnapshot();
				}
			}
			if (OnComplete)
			{
				OnComplete(bSuccess);
			}
		});
	});
}

void UIdleGameInstance::GuildPanelDonate(int64 DonateGold, TFunction<void(bool)> OnComplete)
{
	EnsureGuildService();
	const FString GuildId = GuildService ? GuildService->GetCachedGuildId() : FString();
	// 보유 골드 검증은 UI 가 선처리하지만, 안전하게 여기서도 클램프(음수/초과 거부).
	if (!ApiClient || GuildId.IsEmpty() || DonateGold <= 0 || DonateGold > GetGold())
	{
		if (OnComplete)
		{
			OnComplete(false);
		}
		return;
	}

	TWeakObjectPtr<UIdleGameInstance> WeakThis(this);
	ApiClient->EnsureCharacter([WeakThis, GuildId, DonateGold, OnComplete = MoveTemp(OnComplete)](bool bCharacterOk, FString CharacterId) mutable
	{
		UIdleGameInstance* StrongThis = WeakThis.Get();
		if (!StrongThis || !StrongThis->ApiClient || !bCharacterOk || CharacterId.IsEmpty())
		{
			if (OnComplete)
			{
				OnComplete(false);
			}
			return;
		}

		StrongThis->ApiClient->GuildDonate(GuildId, CharacterId, DonateGold, [WeakThis, DonateGold, OnComplete = MoveTemp(OnComplete)](bool bSuccess, FString)
		{
			if (UIdleGameInstance* StrongInner = WeakThis.Get())
			{
				if (bSuccess)
				{
					// 헌납분 골드 차감(서버 권위 — 성공 응답 후 로컬 재화 반영).
					StrongInner->AddGold(-DonateGold);
					StrongInner->RequestAutosave();
					StrongInner->RefreshGuildSnapshot();
				}
			}
			if (OnComplete)
			{
				OnComplete(bSuccess);
			}
		});
	});
}

void UIdleGameInstance::GuildPanelFetchShop(TFunction<void(bool, const TArray<FGuildShopItemInfo>&)> OnComplete)
{
	EnsureGuildService();
	const FString GuildId = GuildService ? GuildService->GetCachedGuildId() : FString();
	if (!ApiClient || GuildId.IsEmpty())
	{
		if (OnComplete)
		{
			OnComplete(false, TArray<FGuildShopItemInfo>());
		}
		return;
	}

	TWeakObjectPtr<UIdleGameInstance> WeakThis(this);
	ApiClient->EnsureCharacter([WeakThis, GuildId, OnComplete = MoveTemp(OnComplete)](bool bCharacterOk, FString CharacterId) mutable
	{
		UIdleGameInstance* StrongThis = WeakThis.Get();
		if (!StrongThis || !StrongThis->ApiClient || !bCharacterOk || CharacterId.IsEmpty())
		{
			if (OnComplete)
			{
				OnComplete(false, TArray<FGuildShopItemInfo>());
			}
			return;
		}

		StrongThis->ApiClient->GetGuildShop(GuildId, CharacterId, [OnComplete = MoveTemp(OnComplete)](bool bSuccess, FString Body)
		{
			TArray<FGuildShopItemInfo> Items;
			if (bSuccess)
			{
				GuildInstanceParseShopResponse(Body, Items);
			}
			if (OnComplete)
			{
				OnComplete(bSuccess, Items);
			}
		});
	});
}

void UIdleGameInstance::ApplyGuildShopReward(const FString& RewardType, int64 Amount)
{
	// 음수/0 보상은 지급하지 않는다(서버 카탈로그는 항상 양수지만 방어).
	if (Amount <= 0)
	{
		return;
	}

	if (RewardType == TEXT("gold"))
	{
		// 골드: 펫/룬 보너스가 곱해지지 않는 직접 지급(정식 AddGold 경로).
		AddGold(Amount);
		return;
	}
	if (RewardType == TEXT("expPotion"))
	{
		// 경험치 물약: EXP 부스트 소비 아이템(WisdomBooster) 으로 누적(정식 AddConsumable 경로).
		AddConsumable(EConsumableType::WisdomBooster, EConsumableGrade::Standard, static_cast<int32>(FMath::Min<int64>(Amount, MAX_int32)));
		return;
	}
	if (RewardType == TEXT("essence"))
	{
		// 룬 에센스: 던전 Essence 보상과 동일한 오버플로 안전 누적 경로.
		RuneEssence = RuneEssence > MAX_int64 - Amount ? MAX_int64 : RuneEssence + Amount;
		RequestAutosave();
		return;
	}
	if (RewardType == TEXT("protectionScroll"))
	{
		// 강화 보호서(#71 강화 재화): 실존 카운터에 오버플로 안전 누적.
		ProtectionScrolls = ProtectionScrolls > MAX_int64 - Amount ? MAX_int64 : ProtectionScrolls + Amount;
		RequestAutosave();
		return;
	}
	if (RewardType == TEXT("resetCube"))
	{
		// 잠재 재설정 큐브(#71 잠재 재화): 실존 카운터에 오버플로 안전 누적.
		ResetCubes = ResetCubes > MAX_int64 - Amount ? MAX_int64 : ResetCubes + Amount;
		RequestAutosave();
		return;
	}
	if (RewardType == TEXT("rankCube"))
	{
		// 잠재 등급 큐브(#71 잠재 재화): 실존 카운터에 오버플로 안전 누적.
		RankCubes = RankCubes > MAX_int64 - Amount ? MAX_int64 : RankCubes + Amount;
		RequestAutosave();
		return;
	}

	// 알 수 없는 보상 타입은 무시(서버 카탈로그 확장 시 클라 미배선 케이스).
	UE_LOG(LogTemp, Warning, TEXT("[GuildShop] 알 수 없는 보상 타입 무시: type=%s amount=%lld"), *RewardType, Amount);
}

void UIdleGameInstance::GuildPanelBuyShopItem(const FString& ItemId, TFunction<void(bool)> OnComplete)
{
	EnsureGuildService();
	const FString GuildId = GuildService ? GuildService->GetCachedGuildId() : FString();
	if (!ApiClient || GuildId.IsEmpty() || ItemId.IsEmpty())
	{
		if (OnComplete)
		{
			OnComplete(false);
		}
		return;
	}

	TWeakObjectPtr<UIdleGameInstance> WeakThis(this);
	ApiClient->EnsureCharacter([WeakThis, GuildId, ItemId, OnComplete = MoveTemp(OnComplete)](bool bCharacterOk, FString CharacterId) mutable
	{
		UIdleGameInstance* StrongThis = WeakThis.Get();
		if (!StrongThis || !StrongThis->ApiClient || !bCharacterOk || CharacterId.IsEmpty())
		{
			if (OnComplete)
			{
				OnComplete(false);
			}
			return;
		}

		StrongThis->ApiClient->BuyGuildShopItem(GuildId, CharacterId, ItemId, [WeakThis, OnComplete = MoveTemp(OnComplete)](bool bSuccess, FString Body)
		{
			// 성공 시 응답 본문(data.reward)을 파싱해 실제 보상을 캐릭터에 지급한 뒤
			// 세이브/스냅샷을 갱신한다(포인트 차감은 서버 권위 — 스냅샷 재동기화로 반영).
			if (UIdleGameInstance* StrongInner = WeakThis.Get())
			{
				if (bSuccess)
				{
					FString RewardType;
					int64 RewardAmount = 0;
					if (GuildInstanceParseBuyReward(Body, RewardType, RewardAmount))
					{
						StrongInner->ApplyGuildShopReward(RewardType, RewardAmount);
						StrongInner->RequestAutosave();
					}
					StrongInner->RefreshGuildSnapshot();
				}
			}
			if (OnComplete)
			{
				OnComplete(bSuccess);
			}
		});
	});
}

void UIdleGameInstance::GuildPanelChallengeBoss(TFunction<void(bool)> OnComplete)
{
	EnsureGuildService();
	const FString GuildId = GuildService ? GuildService->GetCachedGuildId() : FString();
	const AIdleCharacter* Character = FindPlayerCharacter();
	if (!ApiClient || GuildId.IsEmpty() || !Character)
	{
		if (OnComplete)
		{
			OnComplete(false);
		}
		return;
	}

	// 현재 캐릭터 CP 를 도전 데미지로 전달(서버 getChallengeDamage 가 trunc·클램프).
	// 데미지→기여 적립은 서버 applyContribution 권위 — 클라는 별도 기여 적립하지 않는다.
	const int64 Cp = Character->GetCombatPower();

	TWeakObjectPtr<UIdleGameInstance> WeakThis(this);
	ApiClient->EnsureCharacter([WeakThis, GuildId, Cp, OnComplete = MoveTemp(OnComplete)](bool bCharacterOk, FString CharacterId) mutable
	{
		UIdleGameInstance* StrongThis = WeakThis.Get();
		if (!StrongThis || !StrongThis->ApiClient || !bCharacterOk || CharacterId.IsEmpty())
		{
			if (OnComplete)
			{
				OnComplete(false);
			}
			return;
		}

		StrongThis->ApiClient->ChallengeGuildBoss(GuildId, CharacterId, Cp, [WeakThis, OnComplete = MoveTemp(OnComplete)](bool bSuccess, FString)
		{
			if (UIdleGameInstance* StrongInner = WeakThis.Get())
			{
				if (bSuccess)
				{
					// 보스/기여/격파는 서버 권위 — 스냅샷 재동기화로 최신 상태 반영.
					StrongInner->RefreshGuildSnapshot();
				}
			}
			if (OnComplete)
			{
				OnComplete(bSuccess);
			}
		});
	});
}

void UIdleGameInstance::GuildPanelClaimBossReward(TFunction<void(bool)> OnComplete)
{
	EnsureGuildService();
	const FString GuildId = GuildService ? GuildService->GetCachedGuildId() : FString();
	if (!ApiClient || GuildId.IsEmpty())
	{
		if (OnComplete)
		{
			OnComplete(false);
		}
		return;
	}

	TWeakObjectPtr<UIdleGameInstance> WeakThis(this);
	ApiClient->EnsureCharacter([WeakThis, GuildId, OnComplete = MoveTemp(OnComplete)](bool bCharacterOk, FString CharacterId) mutable
	{
		UIdleGameInstance* StrongThis = WeakThis.Get();
		if (!StrongThis || !StrongThis->ApiClient || !bCharacterOk || CharacterId.IsEmpty())
		{
			if (OnComplete)
			{
				OnComplete(false);
			}
			return;
		}

		StrongThis->ApiClient->ClaimGuildBossReward(GuildId, CharacterId, [WeakThis, OnComplete = MoveTemp(OnComplete)](bool bSuccess, FString Body)
		{
			// 성공 시 rewards 배열(격파 N건 누적)을 각각 ApplyGuildShopReward(G2 재사용)로 지급한다.
			if (UIdleGameInstance* StrongInner = WeakThis.Get())
			{
				if (bSuccess)
				{
					TArray<TPair<FString, int64>> Rewards;
					if (GuildInstanceParseBossRewards(Body, Rewards))
					{
						for (const TPair<FString, int64>& Reward : Rewards)
						{
							StrongInner->ApplyGuildShopReward(Reward.Key, Reward.Value);
						}
					}
					StrongInner->RequestAutosave();
					StrongInner->RefreshGuildSnapshot();
				}
			}
			if (OnComplete)
			{
				OnComplete(bSuccess);
			}
		});
	});
}

void UIdleGameInstance::GuildPanelFetchRankings(int32 Limit, TFunction<void(bool, const TArray<FGuildRankingEntry>&, const FGuildRankingEntry&)> OnComplete)
{
	if (!ApiClient)
	{
		if (OnComplete)
		{
			OnComplete(false, TArray<FGuildRankingEntry>(), FGuildRankingEntry());
		}
		return;
	}

	TWeakObjectPtr<UIdleGameInstance> WeakThis(this);
	ApiClient->EnsureCharacter([WeakThis, Limit, OnComplete = MoveTemp(OnComplete)](bool bCharacterOk, FString CharacterId) mutable
	{
		UIdleGameInstance* StrongThis = WeakThis.Get();
		if (!StrongThis || !StrongThis->ApiClient)
		{
			if (OnComplete)
			{
				OnComplete(false, TArray<FGuildRankingEntry>(), FGuildRankingEntry());
			}
			return;
		}

		// characterId 가 있으면 내 길드 순위(me)도 함께 요청. 실패해도 랭킹은 누구나 조회 가능.
		const FString MyCharacterId = (bCharacterOk && !CharacterId.IsEmpty()) ? CharacterId : FString();
		StrongThis->ApiClient->GetGuildRankings(Limit, MyCharacterId, [OnComplete = MoveTemp(OnComplete)](bool bSuccess, FString Body)
		{
			TArray<FGuildRankingEntry> Rankings;
			FGuildRankingEntry MyRank;
			if (bSuccess)
			{
				GuildInstanceParseRankings(Body, Rankings, MyRank);
			}
			if (OnComplete)
			{
				OnComplete(bSuccess, Rankings, MyRank);
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
	EnsureTitleService();
	EnsureMissionService();
	EnsureBuffService();
	EnsureWeeklyBossService();
	EnsureGuildService();

	SaveGame->SaveVersion = 22;
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
		SaveGame->PetStars = PetService->GetPetStars();
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

	if (TitleService)
	{
		// 세이브 시점에도 최신 메트릭 기준 해금 재계산(누락 방지).
		if (AchievementService)
		{
			TitleService->RecomputeUnlocked(AchievementService);
		}
		SaveGame->UnlockedTitleIds = TitleService->GetUnlockedTitleIds();
		SaveGame->EquippedTitleId = TitleService->GetEquippedTitleId();
	}

	if (MissionService)
	{
		// 세이브 시점 기간 경계 처리(stale 진행 리셋) 후 진행/수령/마커 직렬화.
		MissionService->EnsurePeriodFresh(UQuestService::GetCurrentUtcDateString(), UQuestService::GetCurrentUtcWeekString());
		SaveGame->MissionProgress = MissionService->GetProgressMap();
		SaveGame->MissionClaimed = MissionService->GetClaimedSet();
		SaveGame->MissionDailyResetDate = MissionService->GetDailyResetDate();
		SaveGame->MissionWeeklyResetWeek = MissionService->GetWeeklyResetWeek();
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

	if (GuildService)
	{
		FString GuildId;
		uint8 GuildRank = 0;
		int32 GuildLevel = 1;
		float GuildAttackPct = 0.0f;
		float GuildGoldPct = 0.0f;
		int64 ContributionPoints = 0;
		int64 PendingAutoContribution = 0;
		FString LastAttendanceDate;
		int32 BossDefeatedCount = 0;
		int32 BossChallengesRemaining = 0;
		GuildService->ExportSave(
			GuildId,
			GuildRank,
			GuildLevel,
			GuildAttackPct,
			GuildGoldPct,
			ContributionPoints,
			PendingAutoContribution,
			LastAttendanceDate,
			BossDefeatedCount,
			BossChallengesRemaining);
		SaveGame->CachedGuildId = GuildId;
		SaveGame->CachedGuildRank = GuildRank;
		SaveGame->CachedGuildLevel = GuildLevel;
		SaveGame->CachedGuildAttackPct = GuildAttackPct;
		SaveGame->CachedGuildGoldPct = GuildGoldPct;
		SaveGame->CachedContributionPoints = ContributionPoints;
		SaveGame->PendingAutoContribution = PendingAutoContribution;
		SaveGame->LastGuildAttendanceDate = LastAttendanceDate;
		SaveGame->CachedBossDefeatedCount = BossDefeatedCount;
		SaveGame->CachedBossChallengesRemaining = BossChallengesRemaining;
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
		// SaveVer 20+ 만 PetStars 복원, 이전 버전은 빈 맵(0성)으로 회귀 안전.
		const TMap<FString, int32> RestoredPetStars = SaveGame->SaveVersion >= 20 ? SaveGame->PetStars : TMap<FString, int32>();
		PetService->RestoreState(SaveGame->EquippedPetId, SaveGame->OwnedPetIds, SaveGame->PetLevels, RestoredPetStars);
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

	EnsureGuildService();
	if (GuildService)
	{
		if (SaveGame->SaveVersion >= 19)
		{
			// v19+(PR-G3): v18 캐시 + 보스 진행 표시 캐시(격파·도전잔여) 전체 복원.
			GuildService->ImportSave(
				SaveGame->CachedGuildId,
				SaveGame->CachedGuildRank,
				SaveGame->CachedGuildLevel,
				SaveGame->CachedGuildAttackPct,
				SaveGame->CachedGuildGoldPct,
				SaveGame->CachedContributionPoints,
				SaveGame->PendingAutoContribution,
				SaveGame->LastGuildAttendanceDate,
				SaveGame->CachedBossDefeatedCount,
				SaveGame->CachedBossChallengesRemaining);
		}
		else if (SaveGame->SaveVersion >= 18)
		{
			// v18(PR-G2): 보스 캐시 이전 — 레벨/버프/포인트/pending/출석일만 복원(보스 0).
			GuildService->ImportSave(
				SaveGame->CachedGuildId,
				SaveGame->CachedGuildRank,
				SaveGame->CachedGuildLevel,
				SaveGame->CachedGuildAttackPct,
				SaveGame->CachedGuildGoldPct,
				SaveGame->CachedContributionPoints,
				SaveGame->PendingAutoContribution,
				SaveGame->LastGuildAttendanceDate,
				0,
				0);
		}
		else if (SaveGame->SaveVersion >= 17)
		{
			// v17(PR-G1): 길드 id/rank 만 존재 → 무길드버프(레벨1·0%)로 복원. 서버 재동기화 시 갱신.
			GuildService->ImportSave(SaveGame->CachedGuildId, SaveGame->CachedGuildRank, 1, 0.0f, 0.0f, 0, 0, FString(), 0, 0);
		}
		else
		{
			GuildService->ImportSave(FString(), 0, 1, 0.0f, 0.0f, 0, 0, FString(), 0, 0);
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

	EnsureTitleService();
	if (TitleService)
	{
		// SaveVer 21+ 만 칭호 해금/장착 복원, 이전 버전은 빈 값(회귀 안전).
		const TSet<FString> RestoredUnlocked = SaveGame->SaveVersion >= 21 ? SaveGame->UnlockedTitleIds : TSet<FString>();
		const FString RestoredEquipped = SaveGame->SaveVersion >= 21 ? SaveGame->EquippedTitleId : FString();
		TitleService->RestoreState(RestoredUnlocked, RestoredEquipped);
		// 로그인 시 현재 메트릭 기준으로 누락 해금 보강(레거시 세이브 마이그레이션 포함).
		RecomputeUnlockedTitles();
	}

	EnsureMissionService();
	if (MissionService)
	{
		// SaveVer 22+ 만 미션 진행/수령/마커 복원, 이전 버전은 빈 값(회귀 안전).
		const TMap<FString, int64> RestoredProgress = SaveGame->SaveVersion >= 22 ? SaveGame->MissionProgress : TMap<FString, int64>();
		const TSet<FString> RestoredClaimed = SaveGame->SaveVersion >= 22 ? SaveGame->MissionClaimed : TSet<FString>();
		const FString RestoredMissionDate = SaveGame->SaveVersion >= 22 ? SaveGame->MissionDailyResetDate : FString();
		const FString RestoredMissionWeek = SaveGame->SaveVersion >= 22 ? SaveGame->MissionWeeklyResetWeek : FString();
		MissionService->RestoreState(RestoredProgress, RestoredClaimed, RestoredMissionDate, RestoredMissionWeek);
		// 로그인 시점 기간 경계 처리(날짜/주 경과분 리셋). 빈 마커(레거시)는 현재 UTC 로 초기화.
		MissionService->EnsurePeriodFresh(UQuestService::GetCurrentUtcDateString(), UQuestService::GetCurrentUtcWeekString());
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
	// 길드 자동 기여: 던전 클리어당 소량 누적(플러시는 세이브/재접속 시).
	EnsureGuildService();
	if (GuildService)
	{
		GuildService->AddPendingAutoContribution(GuildAutoContributionPerDungeon);
	}
	// 미션 후크: 던전 1회 성공 = DungeonRuns +1(누적 업적 메트릭이 없어 별도 후크, 이중 카운트 없음).
	EnsureMissionService();
	if (MissionService)
	{
		MissionService->RecordProgress(EMissionMetric::DungeonRuns, 1);
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
		// 길드 자동 기여: 보스 도전당 소량 누적(플러시는 세이브/재접속 시).
		EnsureGuildService();
		if (GuildService)
		{
			GuildService->AddPendingAutoContribution(GuildAutoContributionPerBoss);
		}
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

bool UIdleGameInstance::TryRerollRuneSet(int32 OwnedIndex)
{
	EnsureRuneService();
	if (!RuneService || !RuneService->GetOwnedRunes().IsValidIndex(OwnedIndex))
	{
		return false;
	}

	const FRuneInstance& Rune = RuneService->GetOwnedRunes()[OwnedIndex];
	const int64 EssenceCost = FRuneFormula::GetRerollSetEssenceCost(Rune.Rarity);
	if (EssenceCost <= 0 || RuneEssence < EssenceCost)
	{
		return false;
	}

	if (!RuneService->RerollRuneSet(OwnedIndex, RuneRandomStream))
	{
		return false;
	}

	// 단일 지점 차감(서비스 성공 이후).
	RuneEssence -= EssenceCost;
	RefreshPlayerCharacterStats();
	RequestAutosave();
	return true;
}

bool UIdleGameInstance::TryUpgradeRuneRarity(int32 OwnedIndex, bool& bOutSucceeded)
{
	bOutSucceeded = false;
	EnsureRuneService();
	if (!RuneService || !RuneService->GetOwnedRunes().IsValidIndex(OwnedIndex))
	{
		return false;
	}

	const EItemRarity CurrentRarity = RuneService->GetOwnedRunes()[OwnedIndex].Rarity;
	if (CurrentRarity < EItemRarity::Common || CurrentRarity >= EItemRarity::Mythic)
	{
		// Mythic 상한: 시도 불가(자원 차감 없음).
		return false;
	}

	const int64 EssenceCost = FRuneFormula::GetRarityUpgradeEssenceCost(CurrentRarity);
	const int64 GoldCost = FRuneFormula::GetRarityUpgradeGoldCost(CurrentRarity);
	if (EssenceCost <= 0 || GoldCost <= 0 || RuneEssence < EssenceCost || Gold < GoldCost)
	{
		return false;
	}

	const float SuccessChance = FRuneFormula::GetRarityUpgradeChance(CurrentRarity);
	if (!RuneService->TryUpgradeRuneRarity(OwnedIndex, SuccessChance, RuneRandomStream, bOutSucceeded))
	{
		return false;
	}

	// 단일 지점 차감(성공/실패 무관, 시도 성립 시 소모).
	RuneEssence -= EssenceCost;
	AddGold(-GoldCost);
	RefreshPlayerCharacterStats();
	RequestAutosave();
	return true;
}

bool UIdleGameInstance::TransferRuneEnhancement(int32 SrcIndex, int32 DstIndex)
{
	EnsureRuneService();
	if (!RuneService || SrcIndex == DstIndex
		|| !RuneService->GetOwnedRunes().IsValidIndex(SrcIndex)
		|| !RuneService->GetOwnedRunes().IsValidIndex(DstIndex))
	{
		return false;
	}

	const int32 SourceLevel = RuneService->GetOwnedRunes()[SrcIndex].EnhanceLevel;
	const int64 EssenceCost = FRuneFormula::GetTransferEssenceCost(SourceLevel);
	if (EssenceCost <= 0 || RuneEssence < EssenceCost)
	{
		return false;
	}

	if (!RuneService->TransferEnhancement(SrcIndex, DstIndex))
	{
		return false;
	}

	// 단일 지점 차감(서비스 성공 이후).
	RuneEssence -= EssenceCost;
	RefreshPlayerCharacterStats();
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
	EnsureTitleService();
	// 칭호 ExpPct 도 같은 단일 EXP 배수 합산 지점(여기)에만 더한다(이중 적용 금지, #72).
	const FTitleBonus TitleExpBonus = TitleService ? TitleService->GetEquippedTitleBonus() : FTitleBonus();
	const double TitleExpPct = TitleExpBonus.Type == ETitleBonus::ExpPct ? static_cast<double>(TitleExpBonus.Value) : 0.0;
	const double ExpMultiplier =
		1.0
		+ static_cast<double>(BuffService ? BuffService->GetExpBuffPct(GetCurrentUnixSeconds()) : 0.0f)
		+ static_cast<double>(MasteryService ? MasteryService->GetGlobalBonus().ExpBoostPct : 0.0f)
		+ TitleExpPct;
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

float UIdleGameInstance::GetTitleAllStatMultiplier() const
{
	if (!TitleService)
	{
		return 1.0f;
	}
	const FTitleBonus Bonus = TitleService->GetEquippedTitleBonus();
	return Bonus.Type == ETitleBonus::AllStatPct ? 1.0f + Bonus.Value : 1.0f;
}

float UIdleGameInstance::GetTitleCritDamageBonus() const
{
	if (!TitleService)
	{
		return 0.0f;
	}
	const FTitleBonus Bonus = TitleService->GetEquippedTitleBonus();
	return Bonus.Type == ETitleBonus::CritDmgPct ? Bonus.Value : 0.0f;
}

bool UIdleGameInstance::EquipTitle(const FString& TitleId)
{
	EnsureTitleService();
	if (!TitleService || !TitleService->EquipTitle(TitleId))
	{
		return false;
	}

	RefreshPlayerCharacterStats();
	RequestAutosave();
	return true;
}

void UIdleGameInstance::UnequipTitle()
{
	EnsureTitleService();
	if (!TitleService)
	{
		return;
	}

	TitleService->UnequipTitle();
	RefreshPlayerCharacterStats();
	RequestAutosave();
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
		// 메트릭 갱신 직후 칭호 해금 재계산(영구 해금 — 추가 only).
		RecomputeUnlockedTitles();
	}

	// 미션 중앙 후크: 미션 메트릭과 1:1 대응하는 누적형 업적 메트릭만 진행에 누적한다.
	// (Maximum 모드 메트릭(HighestLevelReached/TowerHighestFloor 등)은 매핑 제외 — 누적형 아님.)
	// 던전은 누적 메트릭이 없어 TryRunDungeon 에서 별도 후크한다(이중 카운트 방지).
	EMissionMetric MissionMetric = EMissionMetric::MonstersKilled;
	bool bHasMissionMetric = false;
	switch (Metric)
	{
	case EAchievementMetric::MonstersKilled:
		MissionMetric = EMissionMetric::MonstersKilled;
		bHasMissionMetric = true;
		break;
	case EAchievementMetric::BossesKilled:
		MissionMetric = EMissionMetric::BossesKilled;
		bHasMissionMetric = true;
		break;
	case EAchievementMetric::StagesCleared:
		MissionMetric = EMissionMetric::StagesCleared;
		bHasMissionMetric = true;
		break;
	case EAchievementMetric::GearEnhanced:
		MissionMetric = EMissionMetric::GearEnhanced;
		bHasMissionMetric = true;
		break;
	case EAchievementMetric::GoldEarned:
		MissionMetric = EMissionMetric::GoldEarned;
		bHasMissionMetric = true;
		break;
	default:
		break;
	}

	if (bHasMissionMetric && AmountOrValue > 0)
	{
		EnsureMissionService();
		if (MissionService)
		{
			MissionService->RecordProgress(MissionMetric, AmountOrValue);
		}
	}
}

void UIdleGameInstance::RecordAchievementItemCollected(const FItemInstance& Item)
{
	EnsureAchievementService();
	if (AchievementService)
	{
		AchievementService->RecordItemCollected(Item.ItemId);
		// 수집 메트릭(ItemsCollected/UniqueItemsFound) 갱신 → 칭호 해금 재계산.
		RecomputeUnlockedTitles();
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

bool UIdleGameInstance::ClaimMission(const FString& Id)
{
	EnsureMissionService();
	if (!MissionService)
	{
		return false;
	}

	// 수령 전 기간 경계 처리(리셋 직후의 stale 완료 수령 방지).
	MissionService->EnsurePeriodFresh(UQuestService::GetCurrentUtcDateString(), UQuestService::GetCurrentUtcWeekString());

	FMissionDefinition Definition;
	if (!MissionService->GetDefinition(Id, Definition))
	{
		return false;
	}

	// 완료 && 미수령 검증 + 수령 마킹(MissionService 가 단일 판정). 실패 시 보상 미지급.
	if (!MissionService->ClaimMission(Id))
	{
		return false;
	}

	// 보상 단일 지급 지점. rewardType 별로 골드/룬 정수/소비 아이템 지급.
	switch (Definition.RewardType)
	{
	case EMissionReward::Gold:
		AddGold(Definition.RewardValue);
		break;
	case EMissionReward::Essence:
		RuneEssence = RuneEssence > MAX_int64 - Definition.RewardValue ? MAX_int64 : RuneEssence + Definition.RewardValue;
		break;
	case EMissionReward::Consumable:
		AddConsumable(EConsumableType::AllStatElixir, EConsumableGrade::Standard, static_cast<int32>(FMath::Clamp<int64>(Definition.RewardValue, 0, MAX_int32)));
		break;
	default:
		break;
	}

	RequestAutosave();
	return true;
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

void UIdleGameInstance::InitializeGuildServiceForTests()
{
	GuildService = NewObject<UGuildService>(this);
	GuildService->ClearSnapshot();
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

bool UIdleGameInstance::EvolvePet(const FString& PetId)
{
	EnsurePetService();
	if (!PetService || !PetService->IsPetOwned(PetId))
	{
		return false;
	}

	// 현재 별 기준 진화 비용 산출 → 골드 검증 → 단일 차감.
	const int32 CurrentStar = PetService->GetPetStar(PetId);
	const int64 Cost = FPetLevelFormula::GetPetEvolveCost(CurrentStar);
	if (Cost <= 0 || GetGold() < Cost)
	{
		return false;
	}

	AddGold(-Cost);
	if (!PetService->EvolvePet(PetId))
	{
		// 진화 실패 시 차감 골드 복원(이론상 IsPetOwned 통과 후 도달 불가, 방어적).
		AddGold(Cost);
		return false;
	}

	// 장착 펫 별 배수 반영(GetEquippedPetStatBonus 경유).
	if (PetService->GetEquippedPetId() == PetId)
	{
		RefreshPlayerCharacterStats();
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

void UIdleGameInstance::EnsureTitleService()
{
	if (!TitleService)
	{
		TitleService = NewObject<UTitleService>(this);
		TitleService->InitializeDefaultTitles();
	}
}

void UIdleGameInstance::RecomputeUnlockedTitles()
{
	EnsureTitleService();
	EnsureAchievementService();
	if (TitleService && AchievementService)
	{
		TitleService->RecomputeUnlocked(AchievementService);
	}
}

void UIdleGameInstance::EnsureMissionService()
{
	if (!MissionService)
	{
		MissionService = NewObject<UMissionService>(this);
		MissionService->InitializeDefaultMissions();
		// 신규 인스턴스는 현재 UTC 기간으로 마커를 초기화(최초 호출 시 리셋 없음).
		MissionService->EnsurePeriodFresh(UQuestService::GetCurrentUtcDateString(), UQuestService::GetCurrentUtcWeekString());
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

void UIdleGameInstance::EnsureGuildService()
{
	if (!GuildService)
	{
		GuildService = NewObject<UGuildService>(this);
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
