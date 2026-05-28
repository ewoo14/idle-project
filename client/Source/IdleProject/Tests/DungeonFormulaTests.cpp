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

	const FDungeonRunResult Oversized = FDungeonFormula::GetRewardForCp(EDungeonType::Gold, MAX_int64);
	TestTrue(TEXT("Oversized CP still succeeds"), Oversized.bSuccess);
	TestTrue(TEXT("Oversized sqrt reward stays positive"), Oversized.GoldReward > 0);
	TestTrue(TEXT("Oversized sqrt reward stays within int64 range"), Oversized.GoldReward <= MAX_int64);

	return true;
}

#endif
