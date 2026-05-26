#include "Misc/AutomationTest.h"

#include "CharacterSystem/IdleMonster.h"
#include "CombatSystem/StatusElementTypes.h"
#include "GameCore/IdleGameInstance.h"
#include "GameCore/StageFormula.h"
#include "GameCore/StageService.h"
#include "IdleProjectGameModeBase.h"
#include "Tests/StageEventTestReceiver.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleMonsterStageScalingTest,
	"IdleProject.Combat.Monster.StageScaling",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleMonsterStageScalingTest::RunTest(const FString& Parameters)
{
	AIdleMonster* Monster = NewObject<AIdleMonster>();
	TestNotNull(TEXT("Monster is created"), Monster);
	if (!Monster)
	{
		return false;
	}

	Monster->SetBoss(false);
	Monster->SetStageStatMultiplier(1.6f);
	Monster->SetWeakElement(ESkillElement::Fire);

	TestEqual(TEXT("Normal monster HP uses stage multiplier"), Monster->GetConfiguredMaxHp(), 80.0f);
	TestEqual(TEXT("Normal monster attack uses stage multiplier"), Monster->GetConfiguredAttack(), 12.8f);
	TestEqual(TEXT("Stage weak element overrides default monster weakness"), Monster->GetWeakElement(), ESkillElement::Fire);

	Monster->SetBoss(true);
	Monster->SetStageStatMultiplier(1.15f);
	Monster->SetWeakElement(ESkillElement::Ice);

	TestEqual(TEXT("Boss monster HP uses stage multiplier"), Monster->GetConfiguredMaxHp(), 575.0f);
	TestEqual(TEXT("Boss monster attack uses stage multiplier"), Monster->GetConfiguredAttack(), 27.6f);
	TestEqual(TEXT("Stage weak element can override boss default weakness"), Monster->GetWeakElement(), ESkillElement::Ice);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGameModeStageBossSpawnPolicyTest,
	"IdleProject.GameMode.StageBossSpawnPolicy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGameModeStageBossSpawnPolicyTest::RunTest(const FString& Parameters)
{
	FStageInfo NormalStage;
	NormalStage.bBossStage = false;

	TestFalse(TEXT("Stage service normal stage ignores legacy initial boss slot"),
		AIdleProjectGameModeBase::ShouldSpawnMonsterAsBoss(true, true, NormalStage));
	TestFalse(TEXT("Stage service normal stage keeps regular respawns normal"),
		AIdleProjectGameModeBase::ShouldSpawnMonsterAsBoss(false, true, NormalStage));

	FStageInfo BossStage = NormalStage;
	BossStage.bBossStage = true;
	TestTrue(TEXT("Stage service boss stage always spawns boss"),
		AIdleProjectGameModeBase::ShouldSpawnMonsterAsBoss(false, true, BossStage));

	TestTrue(TEXT("Missing stage service keeps legacy explicit boss spawn fallback"),
		AIdleProjectGameModeBase::ShouldSpawnMonsterAsBoss(true, false, NormalStage));
	TestFalse(TEXT("Missing stage service keeps legacy normal spawn fallback"),
		AIdleProjectGameModeBase::ShouldSpawnMonsterAsBoss(false, false, NormalStage));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FStageFormulaScalingTest,
	"IdleProject.GameCore.StageFormula.ScalingAndWeakElements",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStageFormulaScalingTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Stage 1-1 has no stat scaling"), FStageFormula::ComputeMonsterStatMultiplier(0), 1.0f);
	TestEqual(TEXT("Stage 1-2 scales stats by one step"), FStageFormula::ComputeMonsterStatMultiplier(1), 1.15f);
	TestEqual(TEXT("Stage 1-5 scales stats by four steps"), FStageFormula::ComputeMonsterStatMultiplier(4), 1.6f);

	TestEqual(TEXT("Stage 1-1 reward multiplier starts at one"), FStageFormula::ComputeRewardMultiplier(0), 1.0f);
	TestEqual(TEXT("Stage 1-5 reward multiplier mirrors stat scaling"), FStageFormula::ComputeRewardMultiplier(4), 1.6f);

	TestFalse(TEXT("Stage 1-4 is not a boss stage"), FStageFormula::IsBossStage(1, 4, 5));
	TestTrue(TEXT("Stage 1-5 is a boss stage"), FStageFormula::IsBossStage(1, 5, 5));

	TestEqual(TEXT("Stage 1-1 has no weakness"), FStageFormula::GetStageWeakElement(0), ESkillElement::None);
	TestEqual(TEXT("Stage 1-2 has no weakness"), FStageFormula::GetStageWeakElement(1), ESkillElement::None);
	TestEqual(TEXT("Stage 1-3 is weak to ice"), FStageFormula::GetStageWeakElement(2), ESkillElement::Ice);
	TestEqual(TEXT("Stage 1-4 is weak to holy"), FStageFormula::GetStageWeakElement(3), ESkillElement::Holy);
	TestEqual(TEXT("Stage 1-5 is weak to fire"), FStageFormula::GetStageWeakElement(4), ESkillElement::Fire);
	TestEqual(TEXT("Stage 2-1 has no weakness"), FStageFormula::GetStageWeakElement(5), ESkillElement::None);
	TestEqual(TEXT("Stage 2-2 is weak to lightning"), FStageFormula::GetStageWeakElement(6), ESkillElement::Lightning);
	TestEqual(TEXT("Stage 2-3 is weak to ice"), FStageFormula::GetStageWeakElement(7), ESkillElement::Ice);
	TestEqual(TEXT("Stage 2-4 is weak to fire"), FStageFormula::GetStageWeakElement(8), ESkillElement::Fire);
	TestEqual(TEXT("Stage 2-5 is weak to holy"), FStageFormula::GetStageWeakElement(9), ESkillElement::Holy);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FStageServiceProgressionTest,
	"IdleProject.GameCore.StageService.Progression",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStageServiceProgressionTest::RunTest(const FString& Parameters)
{
	UStageService* Stages = NewObject<UStageService>();
	Stages->InitializeDefaultStages();

	TestEqual(TEXT("Initial chapter is one"), Stages->GetCurrentChapter(), 1);
	TestEqual(TEXT("Initial stage is one"), Stages->GetCurrentStage(), 1);
	TestEqual(TEXT("Initial global stage index is zero"), Stages->GetGlobalStageIndex(), 0);
	TestEqual(TEXT("Initial kills to advance mirrors stage one"), Stages->GetKillsToAdvance(), 5);
	TestEqual(TEXT("Initial kill progress is zero"), Stages->GetKillsThisStage(), 0);

	Stages->RecordKill(false);
	TestEqual(TEXT("Normal kill increments current stage progress"), Stages->GetKillsThisStage(), 1);
	TestEqual(TEXT("One kill does not advance stage one"), Stages->GetCurrentStage(), 1);

	for (int32 Index = 0; Index < 4; ++Index)
	{
		Stages->RecordKill(false);
	}

	TestEqual(TEXT("Fifth stage-one kill advances to stage two"), Stages->GetCurrentStage(), 2);
	TestEqual(TEXT("Kill progress resets after stage advance"), Stages->GetKillsThisStage(), 0);
	TestEqual(TEXT("Global stage index advances"), Stages->GetGlobalStageIndex(), 1);

	FStageInfo StageTwoInfo = Stages->GetCurrentStageInfo();
	TestEqual(TEXT("Stage info reports chapter"), StageTwoInfo.Chapter, 1);
	TestEqual(TEXT("Stage info reports stage"), StageTwoInfo.Stage, 2);
	TestFalse(TEXT("Stage two is not boss"), StageTwoInfo.bBossStage);
	TestEqual(TEXT("Stage two has no weakness"), StageTwoInfo.WeakElement, ESkillElement::None);
	TestEqual(TEXT("Stage two reports progress target"), StageTwoInfo.KillsToAdvance, 8);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FStageServiceBossCompletionTest,
	"IdleProject.GameCore.StageService.BossCompletion",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStageServiceBossCompletionTest::RunTest(const FString& Parameters)
{
	UStageService* Stages = NewObject<UStageService>();
	Stages->InitializeDefaultStages();
	UStageEventTestReceiver* Receiver = NewObject<UStageEventTestReceiver>();
	Stages->OnChapterBossDefeated.AddDynamic(Receiver, &UStageEventTestReceiver::CaptureChapterBossDefeated);

	while (Stages->GetCurrentStage() < 5)
	{
		Stages->RecordKill(false);
	}

	TestEqual(TEXT("Stage service reaches boss stage"), Stages->GetCurrentStage(), 5);
	TestTrue(TEXT("Current stage info flags boss stage"), Stages->GetCurrentStageInfo().bBossStage);
	TestEqual(TEXT("Boss stage uses one required boss kill"), Stages->GetKillsToAdvance(), 1);

	Stages->RecordKill(false);
	TestEqual(TEXT("Normal kill does not clear boss stage"), Stages->GetCurrentStage(), 5);
	TestEqual(TEXT("Normal kill does not increment boss progress"), Stages->GetKillsThisStage(), 0);

	Stages->RecordKill(true);
	TestEqual(TEXT("Chapter one boss kill advances to chapter two"), Stages->GetCurrentChapter(), 2);
	TestEqual(TEXT("Chapter one boss kill resets to stage one"), Stages->GetCurrentStage(), 1);
	TestEqual(TEXT("Chapter two starts with zero kill progress"), Stages->GetKillsThisStage(), 0);
	TestEqual(TEXT("Chapter two starts at global stage index five"), Stages->GetGlobalStageIndex(), 5);
	TestFalse(TEXT("Chapter two current boss is not yet cleared"), Stages->HasClearedCurrentChapterBoss());
	TestTrue(TEXT("Chapter one boss clear is queryable"), Stages->HasClearedChapterBoss(1));
	TestEqual(TEXT("Highest cleared chapter records chapter one"), Stages->GetHighestClearedChapter(), 1);
	TestEqual(TEXT("Chapter boss delegate broadcasts once"), Receiver->Count, 1);
	TestEqual(TEXT("Chapter boss delegate reports chapter one"), Receiver->LastClearedChapter, 1);

	while (Stages->GetCurrentChapter() < 2 || Stages->GetCurrentStage() < 5)
	{
		Stages->RecordKill(false);
	}

	TestEqual(TEXT("Stage service reaches chapter two boss stage"), Stages->GetGlobalStageIndex(), 9);
	TestEqual(TEXT("Chapter two boss has holy weakness"), Stages->GetCurrentStageInfo().WeakElement, ESkillElement::Holy);

	Stages->RecordKill(true);
	TestEqual(TEXT("Final chapter clear stays on chapter two"), Stages->GetCurrentChapter(), 2);
	TestEqual(TEXT("Final chapter clear stays on stage five"), Stages->GetCurrentStage(), 5);
	TestEqual(TEXT("Final chapter boss progress is retained at clear target"), Stages->GetKillsThisStage(), 1);
	TestTrue(TEXT("Chapter two boss clear is queryable"), Stages->HasClearedChapterBoss(2));
	TestEqual(TEXT("Highest cleared chapter records final chapter"), Stages->GetHighestClearedChapter(), 2);
	TestEqual(TEXT("Chapter boss delegate broadcasts final clear"), Receiver->Count, 2);
	TestEqual(TEXT("Chapter boss delegate reports chapter two"), Receiver->LastClearedChapter, 2);

	Stages->RecordKill(true);
	TestEqual(TEXT("Final chapter ignores further kills"), Stages->GetKillsThisStage(), 1);
	TestEqual(TEXT("Final chapter clear remains idempotent"), Receiver->Count, 2);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FStageServiceManualBossCompletionTest,
	"IdleProject.GameCore.StageService.ManualBossCompletion",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStageServiceManualBossCompletionTest::RunTest(const FString& Parameters)
{
	UStageService* Stages = NewObject<UStageService>();
	Stages->InitializeDefaultStages();
	UStageEventTestReceiver* Receiver = NewObject<UStageEventTestReceiver>();
	Stages->OnChapterBossDefeated.AddDynamic(Receiver, &UStageEventTestReceiver::CaptureChapterBossDefeated);

	while (Stages->GetCurrentStage() < 5)
	{
		Stages->RecordKill(false);
	}

	Stages->MarkCurrentChapterBossDefeated();

	TestEqual(TEXT("Manual chapter one boss completion advances to chapter two"), Stages->GetCurrentChapter(), 2);
	TestEqual(TEXT("Manual chapter one boss completion starts chapter two stage one"), Stages->GetCurrentStage(), 1);
	TestEqual(TEXT("Manual chapter one boss completion resets progress"), Stages->GetKillsThisStage(), 0);
	TestEqual(TEXT("Manual chapter one boss completion records highest cleared chapter"), Stages->GetHighestClearedChapter(), 1);
	TestEqual(TEXT("Manual chapter one boss completion broadcasts once"), Receiver->Count, 1);
	TestEqual(TEXT("Manual chapter one boss completion reports chapter one"), Receiver->LastClearedChapter, 1);

	Stages->MarkCurrentChapterBossDefeated();
	TestEqual(TEXT("Manual completion is idempotent on non-boss chapter two start"), Receiver->Count, 1);
	TestEqual(TEXT("Idempotent manual completion remains on chapter two stage one"), Stages->GetCurrentStage(), 1);

	while (Stages->GetCurrentStage() < 5)
	{
		Stages->RecordKill(false);
	}

	Stages->MarkCurrentChapterBossDefeated();

	TestEqual(TEXT("Manual final boss completion stays on final chapter"), Stages->GetCurrentChapter(), 2);
	TestEqual(TEXT("Manual final boss completion stays on final stage"), Stages->GetCurrentStage(), 5);
	TestEqual(TEXT("Manual final boss completion retains target progress"), Stages->GetKillsThisStage(), 1);
	TestEqual(TEXT("Manual final boss completion records final chapter"), Stages->GetHighestClearedChapter(), 2);
	TestEqual(TEXT("Manual final boss completion broadcasts second clear"), Receiver->Count, 2);
	TestEqual(TEXT("Manual final boss completion reports chapter two"), Receiver->LastClearedChapter, 2);

	Stages->MarkCurrentChapterBossDefeated();
	TestEqual(TEXT("Manual final completion is idempotent"), Receiver->Count, 2);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleGameInstanceStageServiceHooksTest,
	"IdleProject.GameCore.IdleGameInstance.StageServiceHooks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleGameInstanceStageServiceHooksTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	GameInstance->InitializeStageServiceForTests();

	UStageService* Stages = GameInstance->GetStageService();
	TestNotNull(TEXT("Game instance creates stage service for tests"), Stages);
	TestEqual(TEXT("Stage service starts at global index zero"), Stages->GetGlobalStageIndex(), 0);

	GameInstance->MarkChapter1BossDefeated();

	TestTrue(TEXT("Game instance stores chapter one boss defeat"), GameInstance->HasDefeatedChapter1Boss());
	TestFalse(TEXT("Legacy chapter one flag does not force current stage boss clear"), Stages->HasClearedCurrentChapterBoss());

	while (Stages->GetCurrentStage() < 5)
	{
		Stages->RecordKill(false);
	}
	Stages->RecordKill(true);

	TestTrue(TEXT("Stage service chapter one clear marks game instance exactly once"), GameInstance->HasDefeatedChapter1Boss());
	TestEqual(TEXT("Stage service advances after notifying game instance"), Stages->GetCurrentChapter(), 2);

	return true;
}

#endif
