#include "Misc/AutomationTest.h"

#include "GameCore/IdleGameInstance.h"
#include "GameCore/IdleSaveGame.h"
#include "GameCore/CloudSaveMergePolicy.h"
#include "GameCore/CloudSavePayloadMapper.h"
#include "GameCore/PetLevelFormula.h"
#include "GameCore/PetService.h"
#include "GameCore/QuestService.h"
#include "GameCore/SeasonService.h"
#include "GameCore/StageService.h"
#include "GameCore/TowerService.h"
#include "CharacterSystem/IdleCharacter.h"
#include "CharacterSystem/LevelFormulas.h"
#include "CombatSystem/SkillComponent.h"
#include "Engine/World.h"
#include "ItemSystem/InventoryComponent.h"
#include "ItemSystem/ItemTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Tests/SaveProgressTestReceiver.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
FItemInstance MakeSaveTestItem(
	FName ItemId,
	EItemSlot Slot,
	EItemRarity Rarity,
	float Atk,
	float Def,
	float Hp,
	int32 EnhanceLevel = 0,
	float CritRate = 0.0f,
	float AtkSpeed = 0.0f,
	float MagicAtk = 0.0f,
	EItemSet ItemSet = EItemSet::None)
{
	FItemInstance Item;
	Item.ItemId = ItemId;
	Item.Slot = Slot;
	Item.Rarity = Rarity;
	Item.ItemSet = ItemSet;
	Item.DisplayName = FText::FromName(ItemId);
	Item.BonusAtk = Atk;
	Item.BonusDef = Def;
	Item.BonusHp = Hp;
	Item.EnhanceLevel = EnhanceLevel;
	Item.BonusCritRate = CritRate;
	Item.BonusAtkSpeed = AtkSpeed;
	Item.BonusMagicAtk = MagicAtk;
	return Item;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleSaveGameDefaultsTest,
	"IdleProject.GameCore.SaveSystem.SaveGameDefaults",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleSaveGameDefaultsTest::RunTest(const FString& Parameters)
{
	const UIdleSaveGame* SaveGame = NewObject<UIdleSaveGame>();
	TestNotNull(TEXT("Save game object is created"), SaveGame);
	if (!SaveGame)
	{
		return false;
	}

	TestEqual(TEXT("SaveVersion starts at V12"), SaveGame->SaveVersion, static_cast<int32>(12));
	TestFalse(TEXT("Fresh save object is not marked as captured"), SaveGame->bHasSave);
	TestEqual(TEXT("Fresh save keeps level one"), SaveGame->CharacterLevel, static_cast<int32>(1));
	TestEqual(TEXT("Fresh save keeps first next exp value"), SaveGame->NextExp, static_cast<int64>(150));
	TestEqual(TEXT("Fresh save has no inventory payload"), SaveGame->InventoryItems.Num(), 0);
	TestEqual(TEXT("Fresh save has no skill ranks"), SaveGame->SkillRanks.Num(), 0);
	TestEqual(TEXT("Fresh save has no skill points"), SaveGame->SkillPoints, 0);
	TestEqual(TEXT("Fresh save defaults to current season"), SaveGame->SeasonId, USeasonService::CurrentSeasonId);
	TestEqual(TEXT("Fresh save has no season tokens"), SaveGame->SeasonTokens, 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleSaveSystemApplyCaptureRoundTripTest,
	"IdleProject.GameCore.SaveSystem.ApplyCaptureRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleSaveSystemApplyCaptureRoundTripTest::RunTest(const FString& Parameters)
{
	UIdleSaveGame* SourceSave = NewObject<UIdleSaveGame>();
	SourceSave->bHasSave = true;
	SourceSave->Gold = 123456;
	SourceSave->CharacterLevel = 37;
	SourceSave->CurrentExp = 89;
	SourceSave->NextExp = 9876;
	SourceSave->RebirthCount = 4;
	SourceSave->RebirthBonusPoints = 44;
	SourceSave->TranscendCount = 2;
	SourceSave->AvailableStatPoints = 6;
	SourceSave->AllocatedStats = FPrimaryStats(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f);
	SourceSave->bChapter1BossDefeated = true;
	SourceSave->LastSeenUnixSec = 1234567890;
	SourceSave->StageChapter = 2;
	SourceSave->StageStage = 4;
	SourceSave->StageKillsThisStage = 7;
	SourceSave->bStageFinalChapterCleared = false;
	SourceSave->StageHighestClearedChapter = 1;
	SourceSave->TowerHighestFloor = 25;
	SourceSave->EquippedPetId = TEXT("bird");
	SourceSave->OwnedPetIds.Add(TEXT("dog"));
	SourceSave->OwnedPetIds.Add(TEXT("bird"));
	SourceSave->OwnedPetIds.Add(TEXT("cat"));
	SourceSave->PetLevels.Add(TEXT("dog"), 2);
	SourceSave->PetLevels.Add(TEXT("bird"), 4);
	FQuestSaveEntry QuestEntry;
	QuestEntry.QuestId = TEXT("main_ch1_001");
	QuestEntry.Type = EQuestType::Main;
	QuestEntry.Progress = 5;
	QuestEntry.bCompleted = true;
	QuestEntry.bClaimed = true;
	SourceSave->Quests.Add(QuestEntry);
	SourceSave->QuestDailyResetDate = UQuestService::GetCurrentUtcDateString();
	SourceSave->SeasonId = USeasonService::CurrentSeasonId;
	SourceSave->SeasonTokens = 50;
	SourceSave->SeasonClaimedTiers.Add(1);

	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	TestNotNull(TEXT("Game instance is created"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	TestTrue(TEXT("ApplyFromSave accepts a populated save"), GameInstance->ApplyFromSave(SourceSave));

	UIdleSaveGame* CapturedSave = NewObject<UIdleSaveGame>();
	TestTrue(TEXT("CaptureToSave captures current game state"), GameInstance->CaptureToSave(CapturedSave));

	TestTrue(TEXT("Captured save is marked as populated"), CapturedSave->bHasSave);
	TestEqual(TEXT("Captured save writes V12"), CapturedSave->SaveVersion, static_cast<int32>(12));
	TestEqual(TEXT("Gold round trips"), CapturedSave->Gold, SourceSave->Gold);
	TestEqual(TEXT("Character level round trips"), CapturedSave->CharacterLevel, SourceSave->CharacterLevel);
	TestEqual(TEXT("Current exp round trips"), CapturedSave->CurrentExp, SourceSave->CurrentExp);
	TestEqual(TEXT("Next exp round trips"), CapturedSave->NextExp, SourceSave->NextExp);
	TestEqual(TEXT("Rebirth count round trips"), CapturedSave->RebirthCount, SourceSave->RebirthCount);
	TestEqual(TEXT("Rebirth bonus points round trip"), CapturedSave->RebirthBonusPoints, SourceSave->RebirthBonusPoints);
	TestEqual(TEXT("Transcend count round trips"), CapturedSave->TranscendCount, SourceSave->TranscendCount);
	TestEqual(TEXT("Available stat points round trip"), CapturedSave->AvailableStatPoints, SourceSave->AvailableStatPoints);
	TestEqual(TEXT("Allocated STR round trips"), CapturedSave->AllocatedStats.Str, SourceSave->AllocatedStats.Str);
	TestEqual(TEXT("Allocated DEX round trips"), CapturedSave->AllocatedStats.Dex, SourceSave->AllocatedStats.Dex);
	TestEqual(TEXT("Allocated INT round trips"), CapturedSave->AllocatedStats.Int_, SourceSave->AllocatedStats.Int_);
	TestEqual(TEXT("Allocated WIS round trips"), CapturedSave->AllocatedStats.Wis, SourceSave->AllocatedStats.Wis);
	TestEqual(TEXT("Allocated CON round trips"), CapturedSave->AllocatedStats.Con, SourceSave->AllocatedStats.Con);
	TestEqual(TEXT("Allocated LUK round trips"), CapturedSave->AllocatedStats.Luk, SourceSave->AllocatedStats.Luk);
	TestTrue(TEXT("Chapter one boss flag round trips"), CapturedSave->bChapter1BossDefeated);
	TestEqual(TEXT("Last seen timestamp round trips"), CapturedSave->LastSeenUnixSec, SourceSave->LastSeenUnixSec);
	TestEqual(TEXT("Stage chapter round trips"), CapturedSave->StageChapter, SourceSave->StageChapter);
	TestEqual(TEXT("Stage stage round trips"), CapturedSave->StageStage, SourceSave->StageStage);
	TestEqual(TEXT("Stage kills round trip"), CapturedSave->StageKillsThisStage, SourceSave->StageKillsThisStage);
	TestFalse(TEXT("Stage final clear flag round trips"), CapturedSave->bStageFinalChapterCleared);
	TestEqual(TEXT("Stage highest clear round trips"), CapturedSave->StageHighestClearedChapter, SourceSave->StageHighestClearedChapter);
	TestEqual(TEXT("Tower highest floor round trips"), CapturedSave->TowerHighestFloor, SourceSave->TowerHighestFloor);
	TestEqual(TEXT("Equipped pet round trips"), CapturedSave->EquippedPetId, SourceSave->EquippedPetId);
	TestTrue(TEXT("Owned cat round trips"), CapturedSave->OwnedPetIds.Contains(TEXT("cat")));
	TestEqual(TEXT("Dog level round trips"), CapturedSave->PetLevels.FindRef(TEXT("dog")), static_cast<int32>(2));
	TestEqual(TEXT("Bird level round trips"), CapturedSave->PetLevels.FindRef(TEXT("bird")), static_cast<int32>(4));
	TestEqual(TEXT("Quest payload round trips"), CapturedSave->Quests.Num(), 12);
	const FQuestSaveEntry* CapturedQuest = CapturedSave->Quests.FindByPredicate([](const FQuestSaveEntry& Entry)
	{
		return Entry.QuestId == TEXT("main_ch1_001");
	});
	TestNotNull(TEXT("Captured quests include restored main quest"), CapturedQuest);
	TestTrue(TEXT("Captured quest claimed flag round trips"), CapturedQuest ? CapturedQuest->bClaimed : false);
	TestEqual(TEXT("Season tokens round trip through game instance"), CapturedSave->SeasonTokens, SourceSave->SeasonTokens);
	TestTrue(TEXT("Season claimed tier round trips through game instance"), CapturedSave->SeasonClaimedTiers.Contains(1));

	const UStageService* StageService = GameInstance->GetStageService();
	TestNotNull(TEXT("ApplyFromSave ensures stage service"), StageService);
	TestEqual(TEXT("Stage service restore applies chapter"), StageService ? StageService->GetCurrentChapter() : INDEX_NONE, SourceSave->StageChapter);
	TestEqual(TEXT("Stage service restore applies stage"), StageService ? StageService->GetCurrentStage() : INDEX_NONE, SourceSave->StageStage);
	TestEqual(TEXT("Stage service restore applies kills"), StageService ? StageService->GetKillsThisStage() : INDEX_NONE, SourceSave->StageKillsThisStage);

	const UTowerService* TowerService = GameInstance->GetTowerService();
	TestNotNull(TEXT("ApplyFromSave ensures tower service"), TowerService);
	TestEqual(TEXT("Tower restore applies highest floor"), TowerService ? TowerService->GetHighestFloor() : INDEX_NONE, SourceSave->TowerHighestFloor);

	const UPetService* PetService = GameInstance->GetPetService();
	TestNotNull(TEXT("ApplyFromSave ensures pet service"), PetService);
	TestEqual(TEXT("Pet restore applies equipped pet"), PetService ? PetService->GetEquippedPetId() : FString(), SourceSave->EquippedPetId);
	TestTrue(TEXT("Pet restore applies owned cat"), PetService ? PetService->IsPetOwned(TEXT("cat")) : false);
	TestEqual(TEXT("Pet restore applies dog level"), PetService ? PetService->GetPetLevel(TEXT("dog")) : INDEX_NONE, static_cast<int32>(2));
	TestEqual(TEXT("Pet restore applies bird level"), PetService ? PetService->GetPetLevel(TEXT("bird")) : INDEX_NONE, static_cast<int32>(4));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleSaveSystemLegacyV7StageMigrationTest,
	"IdleProject.GameCore.SaveSystem.LegacyV7StageMigration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleSaveSystemLegacyV7StageMigrationTest::RunTest(const FString& Parameters)
{
	UIdleSaveGame* LegacySave = NewObject<UIdleSaveGame>();
	LegacySave->SaveVersion = 7;
	LegacySave->bHasSave = true;
	LegacySave->bChapter1BossDefeated = true;
	LegacySave->StageChapter = 1;
	LegacySave->StageStage = 5;
	LegacySave->StageKillsThisStage = 1;
	LegacySave->StageHighestClearedChapter = 1;

	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	TestTrue(TEXT("ApplyFromSave accepts legacy v7 stage save"), GameInstance->ApplyFromSave(LegacySave));

	const UStageService* StageService = GameInstance->GetStageService();
	TestNotNull(TEXT("Legacy apply restores stage service"), StageService);
	TestTrue(TEXT("Legacy chapter one boss flag survives migration"), GameInstance->HasDefeatedChapter1Boss());
	TestEqual(TEXT("Legacy stage position remains stage five in ten-stage structure"), StageService ? StageService->GetCurrentStage() : INDEX_NONE, 5);
	TestEqual(TEXT("Legacy highest cleared chapter is preserved for rebirth gate"), StageService ? StageService->GetHighestClearedChapter() : INDEX_NONE, 1);
	TestFalse(TEXT("Legacy stage five is no longer final chapter clear"), StageService ? StageService->HasFinalChapterCleared() : true);

	UIdleSaveGame* CapturedSave = NewObject<UIdleSaveGame>();
	TestTrue(TEXT("Capture after legacy migration succeeds"), GameInstance->CaptureToSave(CapturedSave));
	TestEqual(TEXT("Capture after legacy migration writes V12"), CapturedSave->SaveVersion, static_cast<int32>(12));
	TestEqual(TEXT("Migrated capture keeps stage five"), CapturedSave->StageStage, 5);
	TestEqual(TEXT("Migrated capture keeps highest cleared chapter"), CapturedSave->StageHighestClearedChapter, 1);

	UIdleGameInstance* ReappliedGameInstance = NewObject<UIdleGameInstance>();
	TestTrue(TEXT("Reapplying captured v11 save succeeds"), ReappliedGameInstance->ApplyFromSave(CapturedSave));
	const UStageService* ReappliedStageService = ReappliedGameInstance->GetStageService();
	TestEqual(TEXT("V12 reapply does not migrate stage twice"), ReappliedStageService ? ReappliedStageService->GetCurrentStage() : INDEX_NONE, 5);
	TestEqual(TEXT("V12 reapply keeps highest cleared chapter"), ReappliedStageService ? ReappliedStageService->GetHighestClearedChapter() : INDEX_NONE, 1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleSaveSystemInventoryRestoreRemapsEquippedIndexesTest,
	"IdleProject.GameCore.SaveSystem.InventoryRestoreRemapsEquippedIndexes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleSaveSystemInventoryRestoreRemapsEquippedIndexesTest::RunTest(const FString& Parameters)
{
	TArray<FItemInstance> SavedItems;
	SavedItems.Add(MakeSaveTestItem(TEXT("bad_none"), EItemSlot::None, EItemRarity::Rare, 1.0f, 0.0f, 0.0f));
	SavedItems.Add(MakeSaveTestItem(TEXT("rare_sword"), EItemSlot::Weapon, EItemRarity::Rare, 10.0f, 0.0f, 0.0f, 7, 0.03f, 0.10f, 4.0f, EItemSet::Warrior));
	SavedItems.Add(MakeSaveTestItem(TEXT("bad_rarity"), EItemSlot::Helmet, EItemRarity::None, 0.0f, 8.0f, 30.0f));
	SavedItems.Add(MakeSaveTestItem(TEXT("legendary_helmet"), EItemSlot::Helmet, EItemRarity::Legendary, 0.0f, 8.0f, 30.0f, 12, 0.04f, 0.12f, 9.0f, EItemSet::Guardian));

	TMap<EItemSlot, int32> SavedEquipped;
	SavedEquipped.Add(EItemSlot::Weapon, 1);
	SavedEquipped.Add(EItemSlot::Helmet, 3);

	UInventoryComponent* Inventory = NewObject<UInventoryComponent>();
	Inventory->RestoreState(SavedItems, SavedEquipped);

	TArray<FItemInstance> RestoredItems;
	TMap<EItemSlot, int32> RestoredEquipped;
	Inventory->CaptureState(RestoredItems, RestoredEquipped);

	TestEqual(TEXT("Restore keeps only valid items"), RestoredItems.Num(), 2);
	TestEqual(TEXT("Weapon equipped index is remapped after invalid item drop"), RestoredEquipped.FindRef(EItemSlot::Weapon), 0);
	TestEqual(TEXT("Helmet equipped index is remapped after invalid item drop"), RestoredEquipped.FindRef(EItemSlot::Helmet), 1);

	const FItemInstance* EquippedWeapon = Inventory->GetEquippedItem(EItemSlot::Weapon);
	const FItemInstance* EquippedHelmet = Inventory->GetEquippedItem(EItemSlot::Helmet);
	TestNotNull(TEXT("Restored weapon remains equipped"), EquippedWeapon);
	TestNotNull(TEXT("Restored helmet remains equipped"), EquippedHelmet);
	TestEqual(TEXT("Equipped weapon payload survives remap"), EquippedWeapon ? EquippedWeapon->ItemId : NAME_None, FName(TEXT("rare_sword")));
	TestEqual(TEXT("Equipped helmet payload survives remap"), EquippedHelmet ? EquippedHelmet->ItemId : NAME_None, FName(TEXT("legendary_helmet")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleSaveSystemPendingCharacterStateAppliesAfterPawnSpawnTest,
	"IdleProject.GameCore.SaveSystem.PendingCharacterStateAppliesAfterPawnSpawn",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleSaveSystemPendingCharacterStateAppliesAfterPawnSpawnTest::RunTest(const FString& Parameters)
{
	UIdleSaveGame* SourceSave = NewObject<UIdleSaveGame>();
	SourceSave->bHasSave = true;
	SourceSave->InventoryItems.Add(MakeSaveTestItem(TEXT("rare_sword"), EItemSlot::Weapon, EItemRarity::Rare, 10.0f, 0.0f, 0.0f, 7, 0.03f, 0.10f, 4.0f, EItemSet::Warrior));
	SourceSave->EquippedSlotIndex.Add(EItemSlot::Weapon, 0);
	SourceSave->SkillRanks.Add(TEXT("heavy_strike"), 3);
	SourceSave->SkillPoints = 2;

	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	TestTrue(TEXT("ApplyFromSave queues v2 character state when no pawn exists"), GameInstance->ApplyFromSave(SourceSave));

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("Test world is created"), World);
	if (!World)
	{
		return false;
	}

	AIdleCharacter* Character = World->SpawnActor<AIdleCharacter>();
	TestNotNull(TEXT("Character is spawned"), Character);
	if (!Character)
	{
		World->DestroyWorld(false);
		return false;
	}

	Character->SetClassId(EClassId::Warrior);
	GameInstance->ApplyPendingCharacterSaveToCharacter(Character);

	UInventoryComponent* Inventory = Character->FindComponentByClass<UInventoryComponent>();
	USkillComponent* Skills = Character->FindComponentByClass<USkillComponent>();
	TestNotNull(TEXT("Character has inventory"), Inventory);
	TestNotNull(TEXT("Character has skills"), Skills);
	TestEqual(TEXT("Pending inventory item is restored"), Inventory ? Inventory->GetItemCount() : INDEX_NONE, 1);
	TestEqual(TEXT("Pending equipped weapon is restored"), Inventory && Inventory->GetEquippedItem(EItemSlot::Weapon) ? Inventory->GetEquippedItem(EItemSlot::Weapon)->ItemId : NAME_None, FName(TEXT("rare_sword")));
	TestEqual(TEXT("Pending skill rank is restored"), Skills ? Skills->GetSkillRank(TEXT("heavy_strike")) : INDEX_NONE, 3);
	TestEqual(TEXT("Pending skill points are restored"), Skills ? Skills->GetSkillPoints() : INDEX_NONE, 2);

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleSaveSystemInventoryStateRoundTripTest,
	"IdleProject.GameCore.SaveSystem.InventoryStateRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleSaveSystemInventoryStateRoundTripTest::RunTest(const FString& Parameters)
{
	UInventoryComponent* SourceInventory = NewObject<UInventoryComponent>();
	SourceInventory->AddItem(MakeSaveTestItem(TEXT("rare_sword"), EItemSlot::Weapon, EItemRarity::Rare, 10.0f, 0.0f, 0.0f, 7, 0.03f, 0.10f, 4.0f, EItemSet::Warrior));
	SourceInventory->AddItem(MakeSaveTestItem(TEXT("legendary_helmet"), EItemSlot::Helmet, EItemRarity::Legendary, 0.0f, 8.0f, 30.0f, 12, 0.04f, 0.12f, 9.0f, EItemSet::Guardian));

	TArray<FItemInstance> CapturedItems;
	TMap<EItemSlot, int32> CapturedEquipped;
	SourceInventory->CaptureState(CapturedItems, CapturedEquipped);

	UInventoryComponent* RestoredInventory = NewObject<UInventoryComponent>();
	RestoredInventory->RestoreState(CapturedItems, CapturedEquipped);

	TArray<FItemInstance> RestoredItems;
	TMap<EItemSlot, int32> RestoredEquipped;
	RestoredInventory->CaptureState(RestoredItems, RestoredEquipped);

	TestEqual(TEXT("Inventory item count round trips"), RestoredItems.Num(), 2);
	TestEqual(TEXT("ItemId round trips"), RestoredItems[0].ItemId, FName(TEXT("rare_sword")));
	TestEqual(TEXT("Rarity round trips"), static_cast<int32>(RestoredItems[1].Rarity), static_cast<int32>(EItemRarity::Legendary));
	TestEqual(TEXT("Item set round trips"), static_cast<int32>(RestoredItems[0].ItemSet), static_cast<int32>(EItemSet::Warrior));
	TestEqual(TEXT("Enhance level round trips"), RestoredItems[1].EnhanceLevel, 12);
	TestEqual(TEXT("Affix crit round trips"), RestoredItems[0].BonusCritRate, 0.03f);
	TestEqual(TEXT("Affix speed round trips"), RestoredItems[0].BonusAtkSpeed, 0.10f);
	TestEqual(TEXT("Affix magic attack round trips"), RestoredItems[1].BonusMagicAtk, 9.0f);
	TestEqual(TEXT("Weapon equipped index round trips"), RestoredEquipped.FindRef(EItemSlot::Weapon), 0);
	TestEqual(TEXT("Helmet equipped index round trips"), RestoredEquipped.FindRef(EItemSlot::Helmet), 1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleSaveSystemExpandedItemFieldsRoundTripTest,
	"IdleProject.GameCore.SaveSystem.ExpandedItemFieldsRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleSaveSystemExpandedItemFieldsRoundTripTest::RunTest(const FString& Parameters)
{
	UInventoryComponent* SourceInventory = NewObject<UInventoryComponent>();
	FItemInstance SourceItem = MakeSaveTestItem(TEXT("mythic_runeblade_l77"), EItemSlot::Weapon, EItemRarity::Mythic, 40.0f, 0.0f, 0.0f, 9, 0.04f, 0.11f, 22.0f, EItemSet::Assassin);
	SourceItem.BaseItemId = TEXT("runeblade");
	SourceItem.BonusPhysDef = 8.0f;
	SourceItem.BonusMagicDef = 13.0f;
	SourceItem.BonusAffixHp = 75.0f;
	SourceItem.BonusCritDmg = 0.18f;
	SourceItem.PotentialGrade = EPotentialGrade::Unique;
	SourceItem.PotentialLine1.Stat = EPotentialStat::PhysAtkPercent;
	SourceItem.PotentialLine1.Value = 0.10f;
	SourceItem.PotentialLine2.Stat = EPotentialStat::HpPercent;
	SourceItem.PotentialLine2.Value = 0.08f;
	SourceItem.EnhanceFailStreak = 5;
	SourceItem.bLocked = true;
	SourceInventory->AddItem(SourceItem);

	TArray<FItemInstance> CapturedItems;
	TMap<EItemSlot, int32> CapturedEquipped;
	SourceInventory->CaptureState(CapturedItems, CapturedEquipped);

	UInventoryComponent* RestoredInventory = NewObject<UInventoryComponent>();
	RestoredInventory->RestoreState(CapturedItems, CapturedEquipped);

	TArray<FItemInstance> RestoredItems;
	TMap<EItemSlot, int32> RestoredEquipped;
	RestoredInventory->CaptureState(RestoredItems, RestoredEquipped);

	TestEqual(TEXT("Expanded inventory item count round trips"), RestoredItems.Num(), 1);
	const FItemInstance& RestoredItem = RestoredItems[0];
	TestEqual(TEXT("Base item id round trips"), RestoredItem.BaseItemId, FName(TEXT("runeblade")));
	TestEqual(TEXT("Expanded set enum round trips"), RestoredItem.ItemSet, EItemSet::Assassin);
	TestEqual(TEXT("Physical defense affix round trips"), RestoredItem.BonusPhysDef, 8.0f);
	TestEqual(TEXT("Magic defense affix round trips"), RestoredItem.BonusMagicDef, 13.0f);
	TestEqual(TEXT("HP affix round trips"), RestoredItem.BonusAffixHp, 75.0f);
	TestEqual(TEXT("Crit damage affix round trips"), RestoredItem.BonusCritDmg, 0.18f);
	TestEqual(TEXT("Potential grade round trips"), RestoredItem.PotentialGrade, EPotentialGrade::Unique);
	TestEqual(TEXT("Potential line stat round trips"), RestoredItem.PotentialLine1.Stat, EPotentialStat::PhysAtkPercent);
	TestEqual(TEXT("Potential line value round trips"), RestoredItem.PotentialLine2.Value, 0.08f);
	TestEqual(TEXT("Enhance fail streak round trips"), RestoredItem.EnhanceFailStreak, 5);
	TestTrue(TEXT("Locked item flag round trips"), RestoredItem.bLocked);
	TestEqual(TEXT("Expanded item stays equipped"), RestoredEquipped.FindRef(EItemSlot::Weapon), 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleSaveSystemSkillRankStateRoundTripTest,
	"IdleProject.GameCore.SaveSystem.SkillRankStateRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleSaveSystemSkillRankStateRoundTripTest::RunTest(const FString& Parameters)
{
	USkillComponent* SourceSkills = NewObject<USkillComponent>();
	SourceSkills->LoadDefaultWarriorSkills();
	SourceSkills->GrantSkillPoint(5);
	TestTrue(TEXT("Rank up setup succeeds"), SourceSkills->RankUpSkill(TEXT("heavy_strike")));
	TestTrue(TEXT("Second rank up setup succeeds"), SourceSkills->RankUpSkill(TEXT("heavy_strike")));
	TestTrue(TEXT("Passive rank up setup succeeds"), SourceSkills->RankUpSkill(TEXT("weapon_mastery")));

	TMap<FName, int32> CapturedRanks;
	int32 CapturedPoints = 0;
	SourceSkills->CaptureRankState(CapturedRanks, CapturedPoints);

	USkillComponent* RestoredSkills = NewObject<USkillComponent>();
	RestoredSkills->LoadDefaultWarriorSkills();
	RestoredSkills->RestoreRankState(CapturedRanks, CapturedPoints);

	TestEqual(TEXT("Skill points round trip"), RestoredSkills->GetSkillPoints(), 2);
	TestEqual(TEXT("Active skill rank round trips"), RestoredSkills->GetSkillRank(TEXT("heavy_strike")), 2);
	TestEqual(TEXT("Passive skill rank round trips"), RestoredSkills->GetSkillRank(TEXT("weapon_mastery")), 1);
	TestEqual(TEXT("Unknown skill rank remains zero"), RestoredSkills->GetSkillRank(TEXT("unknown_skill")), 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleSaveSystemQuestSeasonRoundTripTest,
	"IdleProject.GameCore.SaveSystem.QuestSeasonRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleSaveSystemQuestSeasonRoundTripTest::RunTest(const FString& Parameters)
{
	UQuestService* SourceQuests = NewObject<UQuestService>();
	SourceQuests->InitializeDefaultQuests(TEXT("2026-05-27"));
	SourceQuests->RecordProgress(EQuestObjective::KillMonster, 5);
	SourceQuests->ClaimQuest(TEXT("main_ch1_001"));

	TArray<FQuestSaveEntry> CapturedQuests;
	FString CapturedDailyReset;
	SourceQuests->CaptureState(CapturedQuests, CapturedDailyReset);

	UQuestService* RestoredQuests = NewObject<UQuestService>();
	RestoredQuests->RestoreState(CapturedQuests, CapturedDailyReset);

	FQuestState RestoredMainQuest;
	TestTrue(TEXT("Restored quest service contains main quest"), RestoredQuests->GetQuestState(TEXT("main_ch1_001"), RestoredMainQuest));
	TestEqual(TEXT("Quest progress round trips"), RestoredMainQuest.Progress, 5);
	TestTrue(TEXT("Quest completed flag round trips"), RestoredMainQuest.bCompleted);
	TestTrue(TEXT("Quest claimed flag round trips"), RestoredMainQuest.bClaimed);

	USeasonService* SourceSeason = NewObject<USeasonService>();
	SourceSeason->InitializeDefaultSeason();
	SourceSeason->AddSeasonTokens(50);
	SourceSeason->ClaimSeasonReward(1);

	int32 CapturedSeasonId = 0;
	int32 CapturedTokens = 0;
	TArray<int32> CapturedClaimedTiers;
	SourceSeason->CaptureState(CapturedSeasonId, CapturedTokens, CapturedClaimedTiers);

	USeasonService* RestoredSeason = NewObject<USeasonService>();
	RestoredSeason->RestoreState(CapturedSeasonId, CapturedTokens, CapturedClaimedTiers);

	TestEqual(TEXT("Season id round trips"), CapturedSeasonId, USeasonService::CurrentSeasonId);
	TestEqual(TEXT("Season tokens round trip"), RestoredSeason->GetSeasonTokens(), 50);
	TestTrue(TEXT("Season claimed tier round trips"), RestoredSeason->IsTierClaimed(1));
	TestFalse(TEXT("Unclaimed tier stays unclaimed"), RestoredSeason->IsTierClaimed(2));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleSaveSystemMalformedV2PayloadIsSanitizedTest,
	"IdleProject.GameCore.SaveSystem.MalformedV2PayloadIsSanitized",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleSaveSystemMalformedV2PayloadIsSanitizedTest::RunTest(const FString& Parameters)
{
	TArray<FItemInstance> Items;
	Items.Add(MakeSaveTestItem(TEXT("bad_none"), EItemSlot::None, EItemRarity::Rare, 1.0f, 0.0f, 0.0f));
	Items.Add(MakeSaveTestItem(TEXT("bad_rarity"), EItemSlot::Weapon, EItemRarity::None, 1.0f, 0.0f, 0.0f));
	Items.Add(MakeSaveTestItem(TEXT("over_enhanced"), EItemSlot::Weapon, EItemRarity::Rare, 10.0f, 0.0f, 0.0f, 99));
	TMap<EItemSlot, int32> Equipped;
	Equipped.Add(EItemSlot::Weapon, 50);
	Equipped.Add(EItemSlot::Helmet, 0);

	UInventoryComponent* Inventory = NewObject<UInventoryComponent>();
	Inventory->RestoreState(Items, Equipped);
	TArray<FItemInstance> RestoredItems;
	TMap<EItemSlot, int32> RestoredEquipped;
	Inventory->CaptureState(RestoredItems, RestoredEquipped);
	TestEqual(TEXT("Malformed inventory drops invalid items"), RestoredItems.Num(), 1);
	TestEqual(TEXT("Enhance level is clamped"), RestoredItems[0].EnhanceLevel, 50);
	TestEqual(TEXT("Invalid equipped index is cleared"), RestoredEquipped.FindRef(EItemSlot::Weapon), INDEX_NONE);
	TestEqual(TEXT("Mismatched equipped slot is cleared"), RestoredEquipped.FindRef(EItemSlot::Helmet), INDEX_NONE);

	USkillComponent* Skills = NewObject<USkillComponent>();
	Skills->LoadDefaultWarriorSkills();
	TMap<FName, int32> Ranks;
	Ranks.Add(TEXT("heavy_strike"), 999);
	Ranks.Add(TEXT("unknown_skill"), 10);
	Skills->RestoreRankState(Ranks, -5);
	TestEqual(TEXT("Known skill rank clamps to max"), Skills->GetSkillRank(TEXT("heavy_strike")), Skills->MaxRank);
	TestEqual(TEXT("Unknown skill rank is ignored"), Skills->GetSkillRank(TEXT("unknown_skill")), 0);
	TestEqual(TEXT("Negative skill points clamp to zero"), Skills->GetSkillPoints(), 0);

	UQuestService* Quests = NewObject<UQuestService>();
	TArray<FQuestSaveEntry> QuestEntries;
	FQuestSaveEntry UnknownQuest;
	UnknownQuest.QuestId = TEXT("unknown_quest");
	UnknownQuest.Progress = 999;
	QuestEntries.Add(UnknownQuest);
	FQuestSaveEntry KnownQuest;
	KnownQuest.QuestId = TEXT("main_ch1_001");
	KnownQuest.Progress = 999;
	KnownQuest.bClaimed = true;
	QuestEntries.Add(KnownQuest);
	Quests->RestoreState(QuestEntries, TEXT("2026-05-27"));
	FQuestState KnownState;
	TestFalse(TEXT("Unknown quest is ignored"), Quests->GetQuestState(TEXT("unknown_quest"), KnownState));
	TestTrue(TEXT("Known quest restores"), Quests->GetQuestState(TEXT("main_ch1_001"), KnownState));
	TestEqual(TEXT("Quest progress clamps to target"), KnownState.Progress, KnownState.TargetCount);
	TestTrue(TEXT("Claimed quest is completed after clamp"), KnownState.bCompleted);
	TestTrue(TEXT("Claimed quest survives when completed"), KnownState.bClaimed);

	USeasonService* Season = NewObject<USeasonService>();
	TArray<int32> MismatchedClaimedTiers;
	MismatchedClaimedTiers.Add(1);
	MismatchedClaimedTiers.Add(2);
	Season->RestoreState(USeasonService::CurrentSeasonId + 1, 500, MismatchedClaimedTiers);
	TestEqual(TEXT("Mismatched season id resets tokens"), Season->GetSeasonTokens(), 0);
	TestFalse(TEXT("Mismatched season id resets claimed tiers"), Season->IsTierClaimed(1));
	TArray<int32> ClaimedTiers;
	ClaimedTiers.Add(1);
	ClaimedTiers.Add(999);
	Season->RestoreState(USeasonService::CurrentSeasonId, 20, ClaimedTiers);
	TestEqual(TEXT("Matching season restores tokens"), Season->GetSeasonTokens(), 20);
	TestTrue(TEXT("Valid reached tier is restored"), Season->IsTierClaimed(1));
	TestFalse(TEXT("Unknown tier is ignored"), Season->IsTierClaimed(999));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleSaveSystemRestoreSanitizesServiceStateTest,
	"IdleProject.GameCore.SaveSystem.RestoreSanitizesServiceState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleSaveSystemRestoreSanitizesServiceStateTest::RunTest(const FString& Parameters)
{
	UIdleSaveGame* SourceSave = NewObject<UIdleSaveGame>();
	SourceSave->bHasSave = true;
	SourceSave->Gold = -100;
	SourceSave->CharacterLevel = -3;
	SourceSave->CurrentExp = -20;
	SourceSave->NextExp = 0;
	SourceSave->AvailableStatPoints = -5;
	SourceSave->StageChapter = 99;
	SourceSave->StageStage = -10;
	SourceSave->StageKillsThisStage = 9999;
	SourceSave->bStageFinalChapterCleared = true;
	SourceSave->StageHighestClearedChapter = -4;
	SourceSave->TowerHighestFloor = -7;
	SourceSave->EquippedPetId = TEXT("unknown_pet");
	SourceSave->OwnedPetIds.Add(TEXT("dog"));
	SourceSave->OwnedPetIds.Add(TEXT("bird"));
	SourceSave->OwnedPetIds.Add(TEXT("unknown_pet"));
	SourceSave->PetLevels.Add(TEXT("dog"), FPetLevelFormula::MaxPetLevel + 50);
	SourceSave->PetLevels.Add(TEXT("bird"), -5);
	SourceSave->PetLevels.Add(TEXT("unknown_pet"), 8);

	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	TestNotNull(TEXT("Game instance is created"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	TestTrue(TEXT("ApplyFromSave accepts malformed V1 payload and sanitizes it"), GameInstance->ApplyFromSave(SourceSave));
	TestEqual(TEXT("Negative gold is clamped"), GameInstance->GetGold(), static_cast<int64>(0));
	TestEqual(TEXT("Negative level is clamped"), GameInstance->GetCharacterLevel(), static_cast<int32>(1));
	TestEqual(TEXT("Invalid next exp is rebuilt"), GameInstance->GetNextExp(), static_cast<int64>(150));
	TestEqual(TEXT("Negative stat points are clamped"), GameInstance->GetAvailableStatPoints(), static_cast<int32>(0));

	const UStageService* StageService = GameInstance->GetStageService();
	TestNotNull(TEXT("Stage service is restored"), StageService);
	TestEqual(TEXT("Final clear forces final chapter"), StageService ? StageService->GetCurrentChapter() : INDEX_NONE, UStageService::TotalChapters);
	TestEqual(TEXT("Final clear forces final stage"), StageService ? StageService->GetCurrentStage() : INDEX_NONE, UStageService::StagesPerChapter);
	TestEqual(TEXT("Final clear records every chapter as cleared"), StageService ? StageService->GetHighestClearedChapter() : INDEX_NONE, UStageService::TotalChapters);
	TestTrue(TEXT("Final clear flag survives restore"), StageService ? StageService->HasFinalChapterCleared() : false);
	TestEqual(TEXT("Final clear clamps kills to the final stage requirement"), StageService ? StageService->GetKillsThisStage() : INDEX_NONE, StageService ? StageService->GetKillsToAdvance() : INDEX_NONE);

	const UTowerService* TowerService = GameInstance->GetTowerService();
	TestNotNull(TEXT("Tower service is restored"), TowerService);
	TestEqual(TEXT("Negative tower floor is clamped"), TowerService ? TowerService->GetHighestFloor() : INDEX_NONE, static_cast<int32>(0));

	const UPetService* PetService = GameInstance->GetPetService();
	TestNotNull(TEXT("Pet service is restored"), PetService);
	TestEqual(TEXT("Unknown equipped pet falls back to default dog"), PetService ? PetService->GetEquippedPetId() : FString(), FString(TEXT("dog")));
	TestTrue(TEXT("Malformed restore keeps default dog owned"), PetService ? PetService->IsPetOwned(TEXT("dog")) : false);
	TestFalse(TEXT("Malformed restore ignores unknown owned pet"), PetService ? PetService->IsPetOwned(TEXT("unknown_pet")) : true);
	TestEqual(TEXT("Pet level clamps to max"), PetService ? PetService->GetPetLevel(TEXT("dog")) : INDEX_NONE, FPetLevelFormula::MaxPetLevel);
	TestEqual(TEXT("Negative pet level clamps to zero"), PetService ? PetService->GetPetLevel(TEXT("bird")) : INDEX_NONE, static_cast<int32>(0));
	TestEqual(TEXT("Unknown pet level is ignored"), PetService ? PetService->GetPetLevel(TEXT("unknown_pet")) : INDEX_NONE, static_cast<int32>(0));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleSaveSystemInvalidLoadIsNoOpTest,
	"IdleProject.GameCore.SaveSystem.InvalidLoadIsNoOp",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleSaveSystemInvalidLoadIsNoOpTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	TestNotNull(TEXT("Game instance is created"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	GameInstance->AddGold(500);

	UIdleSaveGame* EmptySave = NewObject<UIdleSaveGame>();
	TestFalse(TEXT("Empty save payload is rejected"), GameInstance->ApplyFromSave(EmptySave));
	TestEqual(TEXT("Rejected save keeps gold unchanged"), GameInstance->GetGold(), static_cast<int64>(500));
	TestEqual(TEXT("Rejected save keeps level unchanged"), GameInstance->GetCharacterLevel(), static_cast<int32>(1));

	UIdleSaveGame* VersionlessSave = NewObject<UIdleSaveGame>();
	VersionlessSave->bHasSave = true;
	VersionlessSave->SaveVersion = 0;
	VersionlessSave->Gold = 9999;
	TestFalse(TEXT("Versionless save payload is rejected"), GameInstance->ApplyFromSave(VersionlessSave));
	TestEqual(TEXT("Versionless save keeps gold unchanged"), GameInstance->GetGold(), static_cast<int64>(500));

	UIdleSaveGame* OverCapSave = NewObject<UIdleSaveGame>();
	OverCapSave->bHasSave = true;
	OverCapSave->Gold = 500;
	OverCapSave->CharacterLevel = FLevelFormulas::LEVEL_CAP + 1;
	OverCapSave->NextExp = 0;
	TestTrue(TEXT("Over-cap save payload is accepted and clamped"), GameInstance->ApplyFromSave(OverCapSave));
	TestEqual(TEXT("Over-cap save clamps to level cap"), GameInstance->GetCharacterLevel(), FLevelFormulas::LEVEL_CAP);
	TestEqual(TEXT("Level cap rebuilds next exp sentinel"), GameInstance->GetNextExp(), static_cast<int64>(0));

	UGameplayStatics::DeleteGameInSlot(TEXT("IdleSave"), 0);
	GameInstance->LoadProgress();
	TestEqual(TEXT("Missing slot load keeps current gold unchanged"), GameInstance->GetGold(), static_cast<int64>(500));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleCloudSaveMergePolicyTest,
	"IdleProject.GameCore.SaveSystem.CloudMergePolicy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleCloudSaveMergePolicyTest::RunTest(const FString& Parameters)
{
	FCloudSaveProgressSnapshot Local;
	Local.RebirthCount = 2;
	Local.Level = 80;
	Local.Gold = 5000;
	Local.LastSeenUnixSec = 100;

	FCloudSaveProgressSnapshot Server = Local;
	Server.RebirthCount = 3;
	TestEqual(TEXT("Higher server rebirth is adopted"), FCloudSaveMergePolicy::Decide(Local, Server), ECloudSaveMergeDecision::AdoptServer);

	Server = Local;
	Server.Level = 90;
	TestEqual(TEXT("Higher server level is adopted when rebirth ties"), FCloudSaveMergePolicy::Decide(Local, Server), ECloudSaveMergeDecision::AdoptServer);

	Server = Local;
	Server.Gold = 6000;
	TestEqual(TEXT("Higher server gold is adopted when rebirth and level tie"), FCloudSaveMergePolicy::Decide(Local, Server), ECloudSaveMergeDecision::AdoptServer);

	Server = Local;
	Server.LastSeenUnixSec = 101;
	TestEqual(TEXT("Newer server timestamp is adopted when progress ties"), FCloudSaveMergePolicy::Decide(Local, Server), ECloudSaveMergeDecision::AdoptServer);

	Server = Local;
	Server.Level = 79;
	Server.Gold = 999999;
	TestEqual(TEXT("Lower server level keeps local even if gold is higher"), FCloudSaveMergePolicy::Decide(Local, Server), ECloudSaveMergeDecision::KeepLocal);

	Server = Local;
	TestEqual(TEXT("Exact ties keep local to avoid needless overwrite"), FCloudSaveMergePolicy::Decide(Local, Server), ECloudSaveMergeDecision::KeepLocal);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleCloudSavePayloadMapperRoundTripTest,
	"IdleProject.GameCore.SaveSystem.CloudPayloadRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleCloudSavePayloadMapperRoundTripTest::RunTest(const FString& Parameters)
{
	UIdleSaveGame* SourceSave = NewObject<UIdleSaveGame>();
	SourceSave->bHasSave = true;
	SourceSave->Gold = 987654;
	SourceSave->CharacterLevel = 1000;
	SourceSave->CurrentExp = 12345;
	SourceSave->NextExp = 0;
	SourceSave->RebirthCount = 7;
	SourceSave->TranscendCount = 2;
	SourceSave->LastSeenUnixSec = 1234567890;
	SourceSave->TowerHighestFloor = 42;
	SourceSave->SkillPoints = 9;
	SourceSave->InventoryItems.Add(MakeSaveTestItem(TEXT("mythic_sword"), EItemSlot::Weapon, EItemRarity::Mythic, 100.0f, 0.0f, 0.0f));
	SourceSave->EquippedSlotIndex.Add(EItemSlot::Weapon, 0);
	SourceSave->SkillRanks.Add(TEXT("heavy_strike"), 4);
	SourceSave->PetLevels.Add(TEXT("dog"), 3);
	SourceSave->OwnedPetIds.Add(TEXT("dog"));
	SourceSave->OwnedPetIds.Add(TEXT("cat"));
	FQuestSaveEntry QuestEntry;
	QuestEntry.QuestId = TEXT("main_ch1_001");
	QuestEntry.Type = EQuestType::Main;
	QuestEntry.Progress = 3;
	QuestEntry.bCompleted = true;
	QuestEntry.bClaimed = false;
	SourceSave->Quests.Add(QuestEntry);
	SourceSave->QuestDailyResetDate = TEXT("2026-05-27");
	SourceSave->SeasonId = USeasonService::CurrentSeasonId;
	SourceSave->SeasonTokens = 25;
	SourceSave->SeasonClaimedTiers.Add(1);

	FString PayloadJson;
	TestTrue(TEXT("Cloud payload serializes populated local save"), FCloudSavePayloadMapper::SaveToPayloadJson(*SourceSave, PayloadJson));
	TestTrue(TEXT("Payload includes level cap accepted by backend"), PayloadJson.Contains(TEXT("\"level\":1000")));
	TestTrue(TEXT("Payload includes Mythic max equipment grade"), PayloadJson.Contains(TEXT("\"maxEquipmentGrade\":7")));
	TestTrue(TEXT("Payload includes transcend extension field"), PayloadJson.Contains(TEXT("\"transcendCount\":2")));
	TestTrue(TEXT("Payload includes tower extension field"), PayloadJson.Contains(TEXT("\"towerHighestFloor\":42")));
	TestTrue(TEXT("Payload includes skill point extension field"), PayloadJson.Contains(TEXT("\"skillPoints\":9")));

	UIdleSaveGame* RestoredSave = NewObject<UIdleSaveGame>();
	TestTrue(TEXT("Cloud payload deserializes into local save"), FCloudSavePayloadMapper::PayloadJsonToSave(PayloadJson, *RestoredSave));
	TestTrue(TEXT("Restored cloud save is marked populated"), RestoredSave->bHasSave);
	TestEqual(TEXT("Gold round trips through cloud payload"), RestoredSave->Gold, SourceSave->Gold);
	TestEqual(TEXT("Level round trips through cloud payload"), RestoredSave->CharacterLevel, SourceSave->CharacterLevel);
	TestEqual(TEXT("Rebirth round trips through cloud payload"), RestoredSave->RebirthCount, SourceSave->RebirthCount);
	TestEqual(TEXT("Transcend count round trips through cloud payload"), RestoredSave->TranscendCount, SourceSave->TranscendCount);
	TestEqual(TEXT("Tower floor round trips through cloud payload"), RestoredSave->TowerHighestFloor, SourceSave->TowerHighestFloor);
	TestEqual(TEXT("Skill points round trips through cloud payload"), RestoredSave->SkillPoints, SourceSave->SkillPoints);
	TestEqual(TEXT("Inventory item round trips through cloud payload"), RestoredSave->InventoryItems.Num(), 1);
	TestEqual(TEXT("Inventory item id survives cloud payload"), RestoredSave->InventoryItems[0].ItemId, FName(TEXT("mythic_sword")));
	TestEqual(TEXT("Inventory display name survives cloud payload"), RestoredSave->InventoryItems[0].DisplayName.ToString(), FString(TEXT("mythic_sword")));
	TestEqual(TEXT("Mythic rarity survives cloud payload"), RestoredSave->InventoryItems[0].Rarity, EItemRarity::Mythic);
	TestEqual(TEXT("Equipped slot map survives cloud payload"), RestoredSave->EquippedSlotIndex.FindRef(EItemSlot::Weapon), static_cast<int32>(0));
	TestEqual(TEXT("Skill rank map survives cloud payload"), RestoredSave->SkillRanks.FindRef(TEXT("heavy_strike")), static_cast<int32>(4));
	TestEqual(TEXT("Pet level map survives cloud payload"), RestoredSave->PetLevels.FindRef(TEXT("dog")), static_cast<int32>(3));
	TestTrue(TEXT("Owned pet ids survive cloud payload"), RestoredSave->OwnedPetIds.Contains(TEXT("cat")));
	TestEqual(TEXT("Quest list survives cloud payload"), RestoredSave->Quests.Num(), 1);
	TestEqual(TEXT("Quest id survives cloud payload"), RestoredSave->Quests[0].QuestId, FString(TEXT("main_ch1_001")));
	TestEqual(TEXT("Quest reset date survives cloud payload"), RestoredSave->QuestDailyResetDate, FString(TEXT("2026-05-27")));
	TestEqual(TEXT("Season tokens survive cloud payload"), RestoredSave->SeasonTokens, static_cast<int32>(25));
	TestTrue(TEXT("Season claimed tiers survive cloud payload"), RestoredSave->SeasonClaimedTiers.Contains(1));

	FCloudSaveProgressSnapshot Snapshot;
	TestTrue(TEXT("Snapshot extracts from payload"), FCloudSavePayloadMapper::ExtractSnapshot(PayloadJson, Snapshot));
	TestEqual(TEXT("Snapshot level uses cloud level"), Snapshot.Level, SourceSave->CharacterLevel);
	TestEqual(TEXT("Snapshot rebirth uses cloud rebirth"), Snapshot.RebirthCount, SourceSave->RebirthCount);
	TestEqual(TEXT("Snapshot gold uses cloud gold"), Snapshot.Gold, SourceSave->Gold);
	TestEqual(TEXT("Snapshot timestamp uses cloud last seen"), Snapshot.LastSeenUnixSec, SourceSave->LastSeenUnixSec);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleSaveSystemProgressSavedBroadcastTest,
	"IdleProject.GameCore.SaveSystem.ProgressSavedBroadcast",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleSaveSystemProgressSavedBroadcastTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	USaveProgressTestReceiver* Receiver = NewObject<USaveProgressTestReceiver>();
	TestNotNull(TEXT("Game instance is created"), GameInstance);
	TestNotNull(TEXT("Receiver is created"), Receiver);
	if (!GameInstance || !Receiver)
	{
		return false;
	}

	GameInstance->OnProgressSaved.AddDynamic(Receiver, &USaveProgressTestReceiver::HandleProgressSaved);
	GameInstance->SaveProgress();

	TestEqual(TEXT("SaveProgress broadcasts after a successful save"), Receiver->SavedCount, 1);

	GameInstance->OnProgressSaved.RemoveDynamic(Receiver, &USaveProgressTestReceiver::HandleProgressSaved);
	UGameplayStatics::DeleteGameInSlot(TEXT("IdleSave"), 0);
	return true;
}

#endif
