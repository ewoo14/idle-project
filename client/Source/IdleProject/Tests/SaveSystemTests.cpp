#include "Misc/AutomationTest.h"

#include "GameCore/IdleGameInstance.h"
#include "GameCore/IdleSaveGame.h"
#include "GameCore/PetLevelFormula.h"
#include "GameCore/PetService.h"
#include "GameCore/StageService.h"
#include "GameCore/TowerService.h"
#include "Kismet/GameplayStatics.h"
#include "Tests/SaveProgressTestReceiver.h"

#if WITH_DEV_AUTOMATION_TESTS

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

	TestEqual(TEXT("SaveVersion starts at V1"), SaveGame->SaveVersion, static_cast<int32>(1));
	TestFalse(TEXT("Fresh save object is not marked as captured"), SaveGame->bHasSave);
	TestEqual(TEXT("Fresh save keeps level one"), SaveGame->CharacterLevel, static_cast<int32>(1));
	TestEqual(TEXT("Fresh save keeps first next exp value"), SaveGame->NextExp, static_cast<int64>(150));

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
	SourceSave->PetLevels.Add(TEXT("dog"), 2);
	SourceSave->PetLevels.Add(TEXT("bird"), 4);

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
	TestEqual(TEXT("Dog level round trips"), CapturedSave->PetLevels.FindRef(TEXT("dog")), static_cast<int32>(2));
	TestEqual(TEXT("Bird level round trips"), CapturedSave->PetLevels.FindRef(TEXT("bird")), static_cast<int32>(4));

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
	TestEqual(TEXT("Pet restore applies dog level"), PetService ? PetService->GetPetLevel(TEXT("dog")) : INDEX_NONE, static_cast<int32>(2));
	TestEqual(TEXT("Pet restore applies bird level"), PetService ? PetService->GetPetLevel(TEXT("bird")) : INDEX_NONE, static_cast<int32>(4));

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

	UGameplayStatics::DeleteGameInSlot(TEXT("IdleSave"), 0);
	GameInstance->LoadProgress();
	TestEqual(TEXT("Missing slot load keeps current gold unchanged"), GameInstance->GetGold(), static_cast<int64>(500));

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
