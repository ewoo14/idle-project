#include "Misc/AutomationTest.h"

#include "GameCore/IdleGameInstance.h"
#include "GameCore/IdleSaveGame.h"
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
	TestEqual(TEXT("Allocated INT round trips"), CapturedSave->AllocatedStats.Int_, SourceSave->AllocatedStats.Int_);
	TestTrue(TEXT("Chapter one boss flag round trips"), CapturedSave->bChapter1BossDefeated);
	TestEqual(TEXT("Last seen timestamp round trips"), CapturedSave->LastSeenUnixSec, SourceSave->LastSeenUnixSec);
	TestEqual(TEXT("Stage chapter round trips"), CapturedSave->StageChapter, SourceSave->StageChapter);
	TestEqual(TEXT("Stage stage round trips"), CapturedSave->StageStage, SourceSave->StageStage);
	TestEqual(TEXT("Stage kills round trip"), CapturedSave->StageKillsThisStage, SourceSave->StageKillsThisStage);
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
