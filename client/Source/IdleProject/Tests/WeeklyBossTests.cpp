#include "Misc/AutomationTest.h"

#include "GameCore/WeeklyBossFormula.h"
#include "GameCore/WeeklyBossService.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWeeklyBossFormulaParityTest,
	"IdleProject.WeeklyBoss.FormulaParity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWeeklyBossFormulaParityTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Weekly challenge limit is seven"), FWeeklyBossFormula::WeeklyChallengeLimit, 7);
	TestEqual(TEXT("Challenge damage truncates CP"), FWeeklyBossFormula::GetChallengeDamage(1234), static_cast<int64>(1234));
	TestEqual(TEXT("Challenge damage clamps negatives"), FWeeklyBossFormula::GetChallengeDamage(-10), static_cast<int64>(0));

	TestEqual(TEXT("Milestone one threshold"), FWeeklyBossFormula::MilestoneThreshold(1), static_cast<int64>(1000));
	TestEqual(TEXT("Milestone two threshold"), FWeeklyBossFormula::MilestoneThreshold(2), static_cast<int64>(1500));
	TestEqual(TEXT("Milestone three threshold"), FWeeklyBossFormula::MilestoneThreshold(3), static_cast<int64>(2250));
	TestEqual(TEXT("Milestone five threshold"), FWeeklyBossFormula::MilestoneThreshold(5), static_cast<int64>(5062));
	TestEqual(TEXT("Invalid milestone threshold is zero"), FWeeklyBossFormula::MilestoneThreshold(0), static_cast<int64>(0));

	TestEqual(TEXT("Damage below first milestone reaches zero"), FWeeklyBossFormula::GetReachedMilestones(999), 0);
	TestEqual(TEXT("Exact first threshold reaches one"), FWeeklyBossFormula::GetReachedMilestones(1000), 1);
	TestEqual(TEXT("Exact fifth threshold reaches five"), FWeeklyBossFormula::GetReachedMilestones(5062), 5);

	TestEqual(TEXT("Milestone one gold reward"), FWeeklyBossFormula::MilestoneGoldReward(1), static_cast<int64>(5000));
	TestEqual(TEXT("Milestone five gold reward"), FWeeklyBossFormula::MilestoneGoldReward(5), static_cast<int64>(25312));
	TestEqual(TEXT("Milestone five essence reward"), FWeeklyBossFormula::MilestoneEssenceReward(5), static_cast<int64>(15));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWeeklyBossServiceChallengeClaimResetTest,
	"IdleProject.WeeklyBoss.ServiceChallengeClaimReset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWeeklyBossServiceChallengeClaimResetTest::RunTest(const FString& Parameters)
{
	UWeeklyBossService* Service = NewObject<UWeeklyBossService>();
	Service->EnsureWeek(TEXT("2026-W22"));

	for (int32 Index = 0; Index < FWeeklyBossFormula::WeeklyChallengeLimit; ++Index)
	{
		const FWeeklyBossChallengeResult Result = Service->Challenge(1000, TEXT("2026-W22"));
		TestTrue(TEXT("Challenge succeeds while attempts remain"), Result.bSuccess);
	}

	TestEqual(TEXT("Seven challenges consume the weekly limit"), Service->GetRemainingChallenges(), 0);
	TestEqual(TEXT("Seven 1000 CP challenges accumulate damage"), Service->GetDamage(), static_cast<int64>(7000));
	TestEqual(TEXT("Seven thousand damage reaches milestone five"), Service->GetReachedMilestones(), 5);

	const FWeeklyBossChallengeResult Eighth = Service->Challenge(1000, TEXT("2026-W22"));
	TestFalse(TEXT("Eighth weekly challenge fails"), Eighth.bSuccess);
	TestEqual(TEXT("Failed eighth challenge does not add damage"), Service->GetDamage(), static_cast<int64>(7000));

	TestTrue(TEXT("Reached milestone can be claimed"), Service->ClaimMilestone(3));
	TestEqual(TEXT("Claimed milestone advances highest claimed"), Service->GetClaimedMilestones(), 3);
	TestFalse(TEXT("Duplicate or lower milestone cannot be claimed"), Service->ClaimMilestone(3));
	TestFalse(TEXT("Unreached milestone cannot be claimed"), Service->ClaimMilestone(6));

	const FWeeklyBossSaveState Exported = Service->ExportSave();
	TestEqual(TEXT("Exported week id round trips"), Exported.WeekId, FString(TEXT("2026-W22")));
	TestEqual(TEXT("Exported damage round trips"), Exported.Damage, static_cast<int64>(7000));
	TestEqual(TEXT("Exported challenges round trips"), Exported.ChallengesUsed, 7);
	TestEqual(TEXT("Exported claimed milestones round trips"), Exported.ClaimedMilestones, 3);

	UWeeklyBossService* Restored = NewObject<UWeeklyBossService>();
	Restored->ImportSave(TEXT("2026-W22"), 7000, 7, 3);
	TestEqual(TEXT("Restored remaining challenge count"), Restored->GetRemainingChallenges(), 0);
	TestEqual(TEXT("Restored damage"), Restored->GetDamage(), static_cast<int64>(7000));
	TestEqual(TEXT("Restored claimed milestones"), Restored->GetClaimedMilestones(), 3);

	Restored->EnsureWeek(TEXT("2026-W23"));
	TestEqual(TEXT("New week resets damage"), Restored->GetDamage(), static_cast<int64>(0));
	TestEqual(TEXT("New week resets challenge count"), Restored->GetRemainingChallenges(), FWeeklyBossFormula::WeeklyChallengeLimit);
	TestEqual(TEXT("New week resets claimed milestones"), Restored->GetClaimedMilestones(), 0);

	return true;
}

#endif
