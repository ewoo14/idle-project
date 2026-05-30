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
	TestEqual(TEXT("Stage 1-2 scales stats by one step"), FStageFormula::ComputeMonsterStatMultiplier(2), 1.15f);
	TestEqual(TEXT("Stage 1-5 scales stats by four steps"), FStageFormula::ComputeMonsterStatMultiplier(5), 1.6f);

	TestEqual(TEXT("Stage 1-1 reward multiplier starts at one"), FStageFormula::ComputeRewardMultiplier(0), 1.0f);
	TestEqual(TEXT("Stage 1-5 reward multiplier mirrors stat scaling"), FStageFormula::ComputeRewardMultiplier(5), 1.6f);

	TestFalse(TEXT("Stage 1-5 is elite, not boss"), FStageFormula::IsBossStage(1, 5, 10));
	TestTrue(TEXT("Stage 1-10 is a boss stage"), FStageFormula::IsBossStage(1, 10, 10));
	TestTrue(TEXT("Stage five is an elite stage"), FStageFormula::IsEliteStage(5));
	TestFalse(TEXT("Stage ten is not an elite stage"), FStageFormula::IsEliteStage(10));

	TestEqual(TEXT("Stage 1-1 is weak to fire"), FStageFormula::GetStageWeakElement(1), ESkillElement::Fire);
	TestEqual(TEXT("Stage 1-2 is weak to ice"), FStageFormula::GetStageWeakElement(2), ESkillElement::Ice);
	TestEqual(TEXT("Stage 1-3 is weak to lightning"), FStageFormula::GetStageWeakElement(3), ESkillElement::Lightning);
	TestEqual(TEXT("Stage 1-4 is weak to holy"), FStageFormula::GetStageWeakElement(4), ESkillElement::Holy);
	TestEqual(TEXT("Stage 1-5 is weak to dark"), FStageFormula::GetStageWeakElement(5), ESkillElement::Dark);
	TestEqual(TEXT("Stage 1-10 is weak to dark"), FStageFormula::GetStageWeakElement(10), ESkillElement::Dark);
	TestEqual(TEXT("Stage 2-1 is weak to ice"), FStageFormula::GetStageWeakElement(11), ESkillElement::Ice);
	TestEqual(TEXT("Stage 2-5 is weak to dark"), FStageFormula::GetStageWeakElement(15), ESkillElement::Dark);
	TestEqual(TEXT("Stage 2-10 is weak to dark"), FStageFormula::GetStageWeakElement(20), ESkillElement::Dark);
	TestEqual(TEXT("Stage 3-1 is weak to dark"), FStageFormula::GetStageWeakElement(21), ESkillElement::Dark);
	TestEqual(TEXT("Stage 3-5 is weak to dark"), FStageFormula::GetStageWeakElement(25), ESkillElement::Dark);
	TestEqual(TEXT("Stage 3-10 is weak to holy"), FStageFormula::GetStageWeakElement(30), ESkillElement::Holy);
	TestEqual(TEXT("Stage 4-1 is weak to lightning"), FStageFormula::GetStageWeakElement(31), ESkillElement::Lightning);
	TestEqual(TEXT("Stage 4-2 is weak to holy"), FStageFormula::GetStageWeakElement(32), ESkillElement::Holy);
	TestEqual(TEXT("Stage 4-3 is weak to ice"), FStageFormula::GetStageWeakElement(33), ESkillElement::Ice);
	TestEqual(TEXT("Stage 4-4 is weak to fire"), FStageFormula::GetStageWeakElement(34), ESkillElement::Fire);
	TestEqual(TEXT("Stage 4-5 is weak to dark"), FStageFormula::GetStageWeakElement(35), ESkillElement::Dark);
	TestEqual(TEXT("Stage 4-6 is weak to lightning"), FStageFormula::GetStageWeakElement(36), ESkillElement::Lightning);
	TestEqual(TEXT("Stage 4-7 is weak to holy"), FStageFormula::GetStageWeakElement(37), ESkillElement::Holy);
	TestEqual(TEXT("Stage 4-8 is weak to ice"), FStageFormula::GetStageWeakElement(38), ESkillElement::Ice);
	TestEqual(TEXT("Stage 4-9 is weak to fire"), FStageFormula::GetStageWeakElement(39), ESkillElement::Fire);
	TestEqual(TEXT("Stage 4-10 is weak to dark"), FStageFormula::GetStageWeakElement(40), ESkillElement::Dark);
	TestEqual(TEXT("Stage 5-1 is weak to dark"), FStageFormula::GetStageWeakElement(41), ESkillElement::Dark);
	TestEqual(TEXT("Stage 5-2 is weak to fire"), FStageFormula::GetStageWeakElement(42), ESkillElement::Fire);
	TestEqual(TEXT("Stage 5-3 is weak to lightning"), FStageFormula::GetStageWeakElement(43), ESkillElement::Lightning);
	TestEqual(TEXT("Stage 5-4 is weak to holy"), FStageFormula::GetStageWeakElement(44), ESkillElement::Holy);
	TestEqual(TEXT("Stage 5-5 is weak to dark"), FStageFormula::GetStageWeakElement(45), ESkillElement::Dark);
	TestEqual(TEXT("Stage 5-6 is weak to ice"), FStageFormula::GetStageWeakElement(46), ESkillElement::Ice);
	TestEqual(TEXT("Stage 5-7 is weak to dark"), FStageFormula::GetStageWeakElement(47), ESkillElement::Dark);
	TestEqual(TEXT("Stage 5-8 is weak to lightning"), FStageFormula::GetStageWeakElement(48), ESkillElement::Lightning);
	TestEqual(TEXT("Stage 5-9 is weak to holy"), FStageFormula::GetStageWeakElement(49), ESkillElement::Holy);
	TestEqual(TEXT("Stage 5-10 is weak to dark"), FStageFormula::GetStageWeakElement(50), ESkillElement::Dark);
	TestEqual(TEXT("Stage 6-1 is weak to dark"), FStageFormula::GetStageWeakElement(51), ESkillElement::Dark);
	TestEqual(TEXT("Stage 6-2 is weak to lightning"), FStageFormula::GetStageWeakElement(52), ESkillElement::Lightning);
	TestEqual(TEXT("Stage 6-3 is weak to ice"), FStageFormula::GetStageWeakElement(53), ESkillElement::Ice);
	TestEqual(TEXT("Stage 6-4 is weak to holy"), FStageFormula::GetStageWeakElement(54), ESkillElement::Holy);
	TestEqual(TEXT("Stage 6-5 is weak to dark"), FStageFormula::GetStageWeakElement(55), ESkillElement::Dark);
	TestEqual(TEXT("Stage 6-6 is weak to fire"), FStageFormula::GetStageWeakElement(56), ESkillElement::Fire);
	TestEqual(TEXT("Stage 6-7 is weak to dark"), FStageFormula::GetStageWeakElement(57), ESkillElement::Dark);
	TestEqual(TEXT("Stage 6-8 is weak to ice"), FStageFormula::GetStageWeakElement(58), ESkillElement::Ice);
	TestEqual(TEXT("Stage 6-9 is weak to holy"), FStageFormula::GetStageWeakElement(59), ESkillElement::Holy);
	TestEqual(TEXT("Stage 6-10 is weak to dark"), FStageFormula::GetStageWeakElement(60), ESkillElement::Dark);
	TestEqual(TEXT("Stage 7-1 is weak to dark"), FStageFormula::GetStageWeakElement(61), ESkillElement::Dark);
	TestEqual(TEXT("Stage 7-2 is weak to fire"), FStageFormula::GetStageWeakElement(62), ESkillElement::Fire);
	TestEqual(TEXT("Stage 7-3 is weak to holy"), FStageFormula::GetStageWeakElement(63), ESkillElement::Holy);
	TestEqual(TEXT("Stage 7-4 is weak to lightning"), FStageFormula::GetStageWeakElement(64), ESkillElement::Lightning);
	TestEqual(TEXT("Stage 7-5 is weak to dark"), FStageFormula::GetStageWeakElement(65), ESkillElement::Dark);
	TestEqual(TEXT("Stage 7-6 is weak to ice"), FStageFormula::GetStageWeakElement(66), ESkillElement::Ice);
	TestEqual(TEXT("Stage 7-7 is weak to dark"), FStageFormula::GetStageWeakElement(67), ESkillElement::Dark);
	TestEqual(TEXT("Stage 7-8 is weak to fire"), FStageFormula::GetStageWeakElement(68), ESkillElement::Fire);
	TestEqual(TEXT("Stage 7-9 is weak to holy"), FStageFormula::GetStageWeakElement(69), ESkillElement::Holy);
	TestEqual(TEXT("Stage 7-10 is weak to dark"), FStageFormula::GetStageWeakElement(70), ESkillElement::Dark);

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

	TestEqual(TEXT("Stage service exposes seven chapters"), UStageService::TotalChapters, 7);
	TestEqual(TEXT("Initial chapter is one"), Stages->GetCurrentChapter(), 1);
	TestEqual(TEXT("Initial stage is one"), Stages->GetCurrentStage(), 1);
	TestEqual(TEXT("Initial global stage index is one"), Stages->GetGlobalStageIndex(), 1);
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
	TestEqual(TEXT("Global stage index advances"), Stages->GetGlobalStageIndex(), 2);

	FStageInfo StageTwoInfo = Stages->GetCurrentStageInfo();
	TestEqual(TEXT("Stage info reports chapter"), StageTwoInfo.Chapter, 1);
	TestEqual(TEXT("Stage info reports stage"), StageTwoInfo.Stage, 2);
	TestFalse(TEXT("Stage two is not boss"), StageTwoInfo.bBossStage);
	TestEqual(TEXT("Stage two has ice weakness"), StageTwoInfo.WeakElement, ESkillElement::Ice);
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

	while (Stages->GetCurrentStage() < 10)
	{
		Stages->RecordKill(false);
	}

	TestEqual(TEXT("Stage service reaches boss stage"), Stages->GetCurrentStage(), 10);
	TestTrue(TEXT("Current stage info flags boss stage"), Stages->GetCurrentStageInfo().bBossStage);
	TestEqual(TEXT("Boss stage uses one required boss kill"), Stages->GetKillsToAdvance(), 1);

	Stages->RecordKill(false);
	TestEqual(TEXT("Normal kill does not clear boss stage"), Stages->GetCurrentStage(), 10);
	TestEqual(TEXT("Normal kill does not increment boss progress"), Stages->GetKillsThisStage(), 0);

	Stages->RecordKill(true);
	TestEqual(TEXT("Chapter one boss kill advances to chapter two"), Stages->GetCurrentChapter(), 2);
	TestEqual(TEXT("Chapter one boss kill resets to stage one"), Stages->GetCurrentStage(), 1);
	TestEqual(TEXT("Chapter two starts with zero kill progress"), Stages->GetKillsThisStage(), 0);
	TestEqual(TEXT("Chapter two starts at global stage index eleven"), Stages->GetGlobalStageIndex(), 11);
	TestFalse(TEXT("Chapter two current boss is not yet cleared"), Stages->HasClearedCurrentChapterBoss());
	TestTrue(TEXT("Chapter one boss clear is queryable"), Stages->HasClearedChapterBoss(1));
	TestEqual(TEXT("Highest cleared chapter records chapter one"), Stages->GetHighestClearedChapter(), 1);
	TestEqual(TEXT("Chapter boss delegate broadcasts once"), Receiver->Count, 1);
	TestEqual(TEXT("Chapter boss delegate reports chapter one"), Receiver->LastClearedChapter, 1);

	while (Stages->GetCurrentChapter() < 2 || Stages->GetCurrentStage() < 10)
	{
		Stages->RecordKill(false);
	}

	TestEqual(TEXT("Stage service reaches chapter two boss stage"), Stages->GetGlobalStageIndex(), 20);
	TestEqual(TEXT("Chapter two boss has dark weakness"), Stages->GetCurrentStageInfo().WeakElement, ESkillElement::Dark);

	Stages->RecordKill(true);
	TestEqual(TEXT("Chapter two boss kill advances to chapter three"), Stages->GetCurrentChapter(), 3);
	TestEqual(TEXT("Chapter two boss kill starts chapter three stage one"), Stages->GetCurrentStage(), 1);
	TestEqual(TEXT("Chapter three starts with zero kill progress"), Stages->GetKillsThisStage(), 0);
	TestTrue(TEXT("Chapter two boss clear is queryable"), Stages->HasClearedChapterBoss(2));
	TestEqual(TEXT("Highest cleared chapter records chapter two"), Stages->GetHighestClearedChapter(), 2);
	TestEqual(TEXT("Chapter boss delegate broadcasts chapter two clear"), Receiver->Count, 2);
	TestEqual(TEXT("Chapter boss delegate reports chapter two"), Receiver->LastClearedChapter, 2);

	while (Stages->GetCurrentChapter() < 3 || Stages->GetCurrentStage() < 10)
	{
		Stages->RecordKill(false);
	}
	Stages->RecordKill(true);
	TestEqual(TEXT("Chapter three boss kill advances to chapter four"), Stages->GetCurrentChapter(), 4);
	TestEqual(TEXT("Chapter three boss kill starts chapter four stage one"), Stages->GetCurrentStage(), 1);
	TestEqual(TEXT("Chapter four starts with zero kill progress"), Stages->GetKillsThisStage(), 0);
	TestTrue(TEXT("Chapter three boss clear is queryable"), Stages->HasClearedChapterBoss(3));
	TestEqual(TEXT("Highest cleared chapter records chapter three"), Stages->GetHighestClearedChapter(), 3);
	TestEqual(TEXT("Chapter boss delegate broadcasts chapter three clear"), Receiver->Count, 3);
	TestEqual(TEXT("Chapter boss delegate reports chapter three"), Receiver->LastClearedChapter, 3);
	TestEqual(TEXT("Chapter four starts at global stage index thirty-one"), Stages->GetGlobalStageIndex(), 31);
	TestEqual(TEXT("Chapter four stage one has lightning weakness"), Stages->GetCurrentStageInfo().WeakElement, ESkillElement::Lightning);

	while (Stages->GetCurrentChapter() < 4 || Stages->GetCurrentStage() < 10)
	{
		Stages->RecordKill(false);
	}

	TestEqual(TEXT("Stage service reaches chapter four boss stage"), Stages->GetGlobalStageIndex(), 40);
	TestTrue(TEXT("Chapter four stage ten is a boss stage"), Stages->GetCurrentStageInfo().bBossStage);
	TestEqual(TEXT("Chapter four boss has dark weakness"), Stages->GetCurrentStageInfo().WeakElement, ESkillElement::Dark);

	Stages->RecordKill(true);
	TestEqual(TEXT("Chapter four boss kill advances to chapter five"), Stages->GetCurrentChapter(), 5);
	TestEqual(TEXT("Chapter four boss kill starts chapter five stage one"), Stages->GetCurrentStage(), 1);
	TestEqual(TEXT("Chapter five starts with zero kill progress"), Stages->GetKillsThisStage(), 0);
	TestTrue(TEXT("Chapter four boss clear is queryable"), Stages->HasClearedChapterBoss(4));
	TestEqual(TEXT("Highest cleared chapter records chapter four"), Stages->GetHighestClearedChapter(), 4);
	TestEqual(TEXT("Chapter boss delegate broadcasts chapter four clear"), Receiver->Count, 4);
	TestEqual(TEXT("Chapter boss delegate reports chapter four"), Receiver->LastClearedChapter, 4);
	TestEqual(TEXT("Chapter five starts at global stage index forty-one"), Stages->GetGlobalStageIndex(), 41);
	TestEqual(TEXT("Chapter five stage one has dark weakness"), Stages->GetCurrentStageInfo().WeakElement, ESkillElement::Dark);

	while (Stages->GetCurrentChapter() < 5 || Stages->GetCurrentStage() < 10)
	{
		Stages->RecordKill(false);
	}

	TestEqual(TEXT("Stage service reaches chapter five boss stage"), Stages->GetGlobalStageIndex(), 50);
	TestTrue(TEXT("Chapter five stage ten is a boss stage"), Stages->GetCurrentStageInfo().bBossStage);
	TestEqual(TEXT("Chapter five boss has dark weakness"), Stages->GetCurrentStageInfo().WeakElement, ESkillElement::Dark);

	Stages->RecordKill(true);
	TestEqual(TEXT("Chapter five boss kill advances to chapter six"), Stages->GetCurrentChapter(), 6);
	TestEqual(TEXT("Chapter five boss kill starts chapter six stage one"), Stages->GetCurrentStage(), 1);
	TestEqual(TEXT("Chapter six starts with zero kill progress"), Stages->GetKillsThisStage(), 0);
	TestTrue(TEXT("Chapter five boss clear is queryable"), Stages->HasClearedChapterBoss(5));
	TestEqual(TEXT("Highest cleared chapter records chapter five"), Stages->GetHighestClearedChapter(), 5);
	TestEqual(TEXT("Chapter boss delegate broadcasts chapter five clear"), Receiver->Count, 5);
	TestEqual(TEXT("Chapter boss delegate reports chapter five"), Receiver->LastClearedChapter, 5);
	TestEqual(TEXT("Chapter six starts at global stage index fifty-one"), Stages->GetGlobalStageIndex(), 51);
	TestEqual(TEXT("Chapter six stage one has dark weakness"), Stages->GetCurrentStageInfo().WeakElement, ESkillElement::Dark);

	while (Stages->GetCurrentChapter() < 6 || Stages->GetCurrentStage() < 10)
	{
		Stages->RecordKill(false);
	}

	TestEqual(TEXT("Stage service reaches chapter six boss stage"), Stages->GetGlobalStageIndex(), 60);
	TestTrue(TEXT("Chapter six stage ten is a boss stage"), Stages->GetCurrentStageInfo().bBossStage);
	TestEqual(TEXT("Chapter six boss has dark weakness"), Stages->GetCurrentStageInfo().WeakElement, ESkillElement::Dark);

	Stages->RecordKill(true);
	TestEqual(TEXT("Chapter six boss kill advances to chapter seven"), Stages->GetCurrentChapter(), 7);
	TestEqual(TEXT("Chapter six boss kill starts chapter seven stage one"), Stages->GetCurrentStage(), 1);
	TestEqual(TEXT("Chapter seven starts with zero kill progress"), Stages->GetKillsThisStage(), 0);
	TestTrue(TEXT("Chapter six boss clear is queryable"), Stages->HasClearedChapterBoss(6));
	TestEqual(TEXT("Highest cleared chapter records chapter six"), Stages->GetHighestClearedChapter(), 6);
	TestEqual(TEXT("Chapter boss delegate broadcasts chapter six clear"), Receiver->Count, 6);
	TestEqual(TEXT("Chapter boss delegate reports chapter six"), Receiver->LastClearedChapter, 6);
	TestEqual(TEXT("Chapter seven starts at global stage index sixty-one"), Stages->GetGlobalStageIndex(), 61);
	TestEqual(TEXT("Chapter seven stage one has dark weakness"), Stages->GetCurrentStageInfo().WeakElement, ESkillElement::Dark);

	while (Stages->GetCurrentChapter() < 7 || Stages->GetCurrentStage() < 10)
	{
		Stages->RecordKill(false);
	}

	TestEqual(TEXT("Stage service reaches chapter seven boss stage"), Stages->GetGlobalStageIndex(), 70);
	TestTrue(TEXT("Chapter seven stage ten is a boss stage"), Stages->GetCurrentStageInfo().bBossStage);
	TestEqual(TEXT("Chapter seven boss has dark weakness"), Stages->GetCurrentStageInfo().WeakElement, ESkillElement::Dark);

	Stages->RecordKill(true);
	TestEqual(TEXT("Final chapter clear stays on chapter seven"), Stages->GetCurrentChapter(), 7);
	TestEqual(TEXT("Final chapter clear stays on stage ten"), Stages->GetCurrentStage(), 10);
	TestEqual(TEXT("Final chapter boss progress is retained at clear target"), Stages->GetKillsThisStage(), 1);
	TestTrue(TEXT("Chapter seven boss clear is queryable"), Stages->HasClearedChapterBoss(7));
	TestEqual(TEXT("Highest cleared chapter records final chapter"), Stages->GetHighestClearedChapter(), 7);
	TestEqual(TEXT("Chapter boss delegate broadcasts final clear"), Receiver->Count, 7);
	TestEqual(TEXT("Chapter boss delegate reports chapter seven"), Receiver->LastClearedChapter, 7);

	Stages->RecordKill(true);
	TestEqual(TEXT("Final chapter ignores further kills"), Stages->GetKillsThisStage(), 1);
	TestEqual(TEXT("Final chapter clear remains idempotent"), Receiver->Count, 7);

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

	while (Stages->GetCurrentStage() < 10)
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

	while (Stages->GetCurrentStage() < 10)
	{
		Stages->RecordKill(false);
	}

	Stages->MarkCurrentChapterBossDefeated();

	TestEqual(TEXT("Manual chapter two boss completion advances to chapter three"), Stages->GetCurrentChapter(), 3);
	TestEqual(TEXT("Manual chapter two boss completion starts chapter three stage one"), Stages->GetCurrentStage(), 1);
	TestEqual(TEXT("Manual chapter two boss completion resets progress"), Stages->GetKillsThisStage(), 0);
	TestEqual(TEXT("Manual chapter two boss completion records chapter two"), Stages->GetHighestClearedChapter(), 2);
	TestEqual(TEXT("Manual chapter two boss completion broadcasts second clear"), Receiver->Count, 2);
	TestEqual(TEXT("Manual final boss completion reports chapter two"), Receiver->LastClearedChapter, 2);

	while (Stages->GetCurrentStage() < 10)
	{
		Stages->RecordKill(false);
	}

	Stages->MarkCurrentChapterBossDefeated();
	TestEqual(TEXT("Manual chapter three boss completion advances to chapter four"), Stages->GetCurrentChapter(), 4);
	TestEqual(TEXT("Manual chapter three boss completion starts chapter four stage one"), Stages->GetCurrentStage(), 1);
	TestEqual(TEXT("Manual chapter three boss completion resets progress"), Stages->GetKillsThisStage(), 0);
	TestEqual(TEXT("Manual chapter three boss completion records chapter three"), Stages->GetHighestClearedChapter(), 3);
	TestEqual(TEXT("Manual chapter three boss completion broadcasts third clear"), Receiver->Count, 3);
	TestEqual(TEXT("Manual chapter three boss completion reports chapter three"), Receiver->LastClearedChapter, 3);

	while (Stages->GetCurrentStage() < 10)
	{
		Stages->RecordKill(false);
	}

	Stages->MarkCurrentChapterBossDefeated();
	TestEqual(TEXT("Manual chapter four boss completion advances to chapter five"), Stages->GetCurrentChapter(), 5);
	TestEqual(TEXT("Manual chapter four boss completion starts chapter five stage one"), Stages->GetCurrentStage(), 1);
	TestEqual(TEXT("Manual chapter four boss completion resets progress"), Stages->GetKillsThisStage(), 0);
	TestEqual(TEXT("Manual chapter four boss completion records chapter four"), Stages->GetHighestClearedChapter(), 4);
	TestEqual(TEXT("Manual chapter four boss completion broadcasts fourth clear"), Receiver->Count, 4);
	TestEqual(TEXT("Manual chapter four boss completion reports chapter four"), Receiver->LastClearedChapter, 4);

	while (Stages->GetCurrentStage() < 10)
	{
		Stages->RecordKill(false);
	}

	Stages->MarkCurrentChapterBossDefeated();
	TestEqual(TEXT("Manual chapter five boss completion advances to chapter six"), Stages->GetCurrentChapter(), 6);
	TestEqual(TEXT("Manual chapter five boss completion starts chapter six stage one"), Stages->GetCurrentStage(), 1);
	TestEqual(TEXT("Manual chapter five boss completion resets progress"), Stages->GetKillsThisStage(), 0);
	TestEqual(TEXT("Manual chapter five boss completion records chapter five"), Stages->GetHighestClearedChapter(), 5);
	TestEqual(TEXT("Manual chapter five boss completion broadcasts fifth clear"), Receiver->Count, 5);
	TestEqual(TEXT("Manual chapter five boss completion reports chapter five"), Receiver->LastClearedChapter, 5);

	while (Stages->GetCurrentStage() < 10)
	{
		Stages->RecordKill(false);
	}

	Stages->MarkCurrentChapterBossDefeated();
	TestEqual(TEXT("Manual chapter six boss completion advances to chapter seven"), Stages->GetCurrentChapter(), 7);
	TestEqual(TEXT("Manual chapter six boss completion starts chapter seven stage one"), Stages->GetCurrentStage(), 1);
	TestEqual(TEXT("Manual chapter six boss completion resets progress"), Stages->GetKillsThisStage(), 0);
	TestEqual(TEXT("Manual chapter six boss completion records chapter six"), Stages->GetHighestClearedChapter(), 6);
	TestEqual(TEXT("Manual chapter six boss completion broadcasts sixth clear"), Receiver->Count, 6);
	TestEqual(TEXT("Manual chapter six boss completion reports chapter six"), Receiver->LastClearedChapter, 6);

	while (Stages->GetCurrentStage() < 10)
	{
		Stages->RecordKill(false);
	}

	Stages->MarkCurrentChapterBossDefeated();
	TestEqual(TEXT("Manual final boss completion stays on final chapter"), Stages->GetCurrentChapter(), 7);
	TestEqual(TEXT("Manual final boss completion stays on final stage"), Stages->GetCurrentStage(), 10);
	TestEqual(TEXT("Manual final boss completion retains target progress"), Stages->GetKillsThisStage(), 1);
	TestEqual(TEXT("Manual final boss completion records final chapter"), Stages->GetHighestClearedChapter(), 7);
	TestEqual(TEXT("Manual final boss completion broadcasts seventh clear"), Receiver->Count, 7);
	TestEqual(TEXT("Manual final boss completion reports chapter seven"), Receiver->LastClearedChapter, 7);

	Stages->MarkCurrentChapterBossDefeated();
	TestEqual(TEXT("Manual final completion is idempotent"), Receiver->Count, 7);

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
	TestEqual(TEXT("Stage service starts at global index one"), Stages->GetGlobalStageIndex(), 1);

	GameInstance->MarkChapter1BossDefeated();

	TestTrue(TEXT("Game instance stores chapter one boss defeat"), GameInstance->HasDefeatedChapter1Boss());
	TestFalse(TEXT("Legacy chapter one flag does not force current stage boss clear"), Stages->HasClearedCurrentChapterBoss());

	while (Stages->GetCurrentStage() < 10)
	{
		Stages->RecordKill(false);
	}
	Stages->RecordKill(true);

	TestTrue(TEXT("Stage service chapter one clear marks game instance exactly once"), GameInstance->HasDefeatedChapter1Boss());
	TestEqual(TEXT("Stage service advances after notifying game instance"), Stages->GetCurrentChapter(), 2);

	return true;
}

#endif
