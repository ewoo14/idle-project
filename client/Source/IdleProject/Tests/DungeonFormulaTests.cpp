#include "Misc/AutomationTest.h"

#include "GameCore/DungeonFormula.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDungeonFormulaTest,
	"IdleProject.GameCore.Dungeon.Formula",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDungeonFormulaTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Gold dungeon daily limit is three"), FDungeonFormula::GetDailyEntryLimit(EDungeonType::Gold), 3);
	TestEqual(TEXT("Exp dungeon daily limit is three"), FDungeonFormula::GetDailyEntryLimit(EDungeonType::Exp), 3);
	TestEqual(TEXT("Essence dungeon daily limit is three"), FDungeonFormula::GetDailyEntryLimit(EDungeonType::Essence), 3);
	TestEqual(TEXT("Invalid dungeon daily limit is zero"), FDungeonFormula::GetDailyEntryLimit(EDungeonType::None), 0);

	TestEqual(TEXT("Gold dungeon minimum CP"), FDungeonFormula::GetMinimumCp(EDungeonType::Gold), static_cast<int64>(100));
	TestEqual(TEXT("Exp dungeon minimum CP"), FDungeonFormula::GetMinimumCp(EDungeonType::Exp), static_cast<int64>(250));
	TestEqual(TEXT("Essence dungeon minimum CP"), FDungeonFormula::GetMinimumCp(EDungeonType::Essence), static_cast<int64>(500));

	TestEqual(TEXT("Gold tier one CP equals minimum"), FDungeonFormula::GetTierCpRequirement(EDungeonType::Gold, 1), static_cast<int64>(100));
	TestEqual(TEXT("Gold tier two CP doubles"), FDungeonFormula::GetTierCpRequirement(EDungeonType::Gold, 2), static_cast<int64>(200));
	TestEqual(TEXT("Gold tier three CP quadruples"), FDungeonFormula::GetTierCpRequirement(EDungeonType::Gold, 3), static_cast<int64>(400));
	TestEqual(TEXT("Essence tier three CP quadruples"), FDungeonFormula::GetTierCpRequirement(EDungeonType::Essence, 3), static_cast<int64>(2000));
	TestEqual(TEXT("Below minimum CP unlocks zero tiers"), FDungeonFormula::GetMaxAccessibleTier(EDungeonType::Gold, 99), 0);
	TestEqual(TEXT("Minimum CP unlocks tier one"), FDungeonFormula::GetMaxAccessibleTier(EDungeonType::Gold, 100), 1);
	TestEqual(TEXT("Just below four times minimum unlocks tier two"), FDungeonFormula::GetMaxAccessibleTier(EDungeonType::Gold, 399), 2);
	TestEqual(TEXT("Four times minimum unlocks tier three"), FDungeonFormula::GetMaxAccessibleTier(EDungeonType::Gold, 400), 3);

	const FDungeonRunResult Gold = FDungeonFormula::GetRewardForCp(EDungeonType::Gold, 350);
	TestTrue(TEXT("Gold reward succeeds at minimum or higher CP"), Gold.bSuccess);
	TestEqual(TEXT("Gold dungeon only grants gold"), Gold.GoldReward, static_cast<int64>(37417));
	TestEqual(TEXT("Gold dungeon grants no exp"), Gold.ExpReward, static_cast<int64>(0));
	TestEqual(TEXT("Gold dungeon grants no essence"), Gold.EssenceReward, static_cast<int64>(0));

	const FDungeonRunResult Exp = FDungeonFormula::GetRewardForCp(EDungeonType::Exp, 750);
	TestTrue(TEXT("Exp reward succeeds at minimum or higher CP"), Exp.bSuccess);
	TestEqual(TEXT("Exp dungeon only grants exp"), Exp.ExpReward, static_cast<int64>(34641));
	TestEqual(TEXT("Exp dungeon grants no gold"), Exp.GoldReward, static_cast<int64>(0));
	TestEqual(TEXT("Exp dungeon grants no essence"), Exp.EssenceReward, static_cast<int64>(0));

	const FDungeonRunResult Essence = FDungeonFormula::GetRewardForCp(EDungeonType::Essence, 1200);
	TestTrue(TEXT("Essence reward succeeds at minimum or higher CP"), Essence.bSuccess);
	TestEqual(TEXT("Essence dungeon only grants essence"), Essence.EssenceReward, static_cast<int64>(19));
	TestEqual(TEXT("Essence dungeon grants no gold"), Essence.GoldReward, static_cast<int64>(0));
	TestEqual(TEXT("Essence dungeon grants no exp"), Essence.ExpReward, static_cast<int64>(0));

	const FDungeonRunResult Blocked = FDungeonFormula::GetRewardForCp(EDungeonType::Essence, 499);
	TestFalse(TEXT("Below minimum CP returns failed result"), Blocked.bSuccess);
	TestEqual(TEXT("Below minimum CP grants no essence"), Blocked.EssenceReward, static_cast<int64>(0));

	const FDungeonRunResult TierOne = FDungeonFormula::GetRewardForCp(EDungeonType::Gold, 400, 1);
	TestEqual(TEXT("Tier one reward remains compatible with default reward"), TierOne.GoldReward, FDungeonFormula::GetRewardForCp(EDungeonType::Gold, 400).GoldReward);
	const FDungeonRunResult TierTwo = FDungeonFormula::GetRewardForCp(EDungeonType::Gold, 400, 2);
	TestTrue(TEXT("Accessible tier two succeeds"), TierTwo.bSuccess);
	TestEqual(TEXT("Tier two reward doubles tier one"), TierTwo.GoldReward, TierOne.GoldReward * 2);
	const FDungeonRunResult TierBlocked = FDungeonFormula::GetRewardForCp(EDungeonType::Gold, 199, 2);
	TestFalse(TEXT("Inaccessible tier fails"), TierBlocked.bSuccess);
	TestEqual(TEXT("Inaccessible tier grants zero gold"), TierBlocked.GoldReward, static_cast<int64>(0));

	const FDungeonRunResult Oversized = FDungeonFormula::GetRewardForCp(EDungeonType::Gold, MAX_int64);
	TestTrue(TEXT("Oversized CP still succeeds"), Oversized.bSuccess);
	TestTrue(TEXT("Oversized sqrt reward stays positive"), Oversized.GoldReward > 0);
	TestTrue(TEXT("Oversized sqrt reward stays within int64 range"), Oversized.GoldReward <= MAX_int64);

	return true;
}

#endif
