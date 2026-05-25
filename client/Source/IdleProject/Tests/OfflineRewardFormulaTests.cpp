#include "Misc/AutomationTest.h"

#include "GameCore/IdleGameInstance.h"
#include "GameCore/OfflineRewardFormula.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FOfflineRewardFormulaTest,
	"IdleProject.GameCore.OfflineRewardFormula.ComputeOfflineRewards",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FOfflineRewardFormulaTest::RunTest(const FString& Parameters)
{
	const FOfflineRewardResult Capped = FOfflineRewardFormula::ComputeOfflineRewards(
		10,
		1000,
		1000 + FOfflineRewardFormula::OFFLINE_CAP_SECONDS + 60,
		0);
	TestEqual(TEXT("Elapsed time is capped at twelve hours"), Capped.CappedSeconds, FOfflineRewardFormula::OFFLINE_CAP_SECONDS);
	TestEqual(TEXT("Capped level 10 gold mirrors server offline.ts"), Capped.Gold, static_cast<int64>(1373760));
	TestEqual(TEXT("Capped level 10 exp mirrors server offline.ts"), Capped.Exp, static_cast<int64>(1373760));
	TestEqual(TEXT("Twelve hour time bonus mirrors server offline.ts"), Capped.TimeBonusMultiplier, 1.06);

	const FOfflineRewardResult NoElapsed = FOfflineRewardFormula::ComputeOfflineRewards(10, 2000, 1000, 0);
	TestEqual(TEXT("Negative elapsed time is clamped to zero seconds"), NoElapsed.CappedSeconds, static_cast<int64>(0));
	TestEqual(TEXT("Zero elapsed gold is zero"), NoElapsed.Gold, static_cast<int64>(0));
	TestEqual(TEXT("Zero elapsed exp is zero"), NoElapsed.Exp, static_cast<int64>(0));
	TestEqual(TEXT("Zero elapsed time bonus stays neutral"), NoElapsed.TimeBonusMultiplier, 1.0);

	const FOfflineRewardResult Efficient = FOfflineRewardFormula::ComputeOfflineRewards(5, 1000, 1060, 0);
	TestEqual(TEXT("Level 5 sixty second gold applies 0.75 efficiency"), Efficient.Gold, static_cast<int64>(900));
	TestEqual(TEXT("Level 5 sixty second exp applies 0.75 efficiency"), Efficient.Exp, static_cast<int64>(900));

	const FOfflineRewardResult Base = FOfflineRewardFormula::ComputeOfflineRewards(20, 1000, 1600, 0);
	const FOfflineRewardResult Rebirthed = FOfflineRewardFormula::ComputeOfflineRewards(20, 1000, 1600, 2);
	TestTrue(TEXT("Rebirth increases time bonus multiplier"), Rebirthed.TimeBonusMultiplier > Base.TimeBonusMultiplier);
	TestEqual(TEXT("Two rebirth level 20 gold mirrors server offline.ts"), Rebirthed.Gold, static_cast<int64>(39630));
	TestEqual(TEXT("Two rebirth level 20 exp mirrors server offline.ts"), Rebirthed.Exp, static_cast<int64>(39630));

	const FOfflineRewardResult InvalidLevel = FOfflineRewardFormula::ComputeOfflineRewards(0, 1000, 1100, 0);
	TestEqual(TEXT("Invalid level returns no reward"), InvalidLevel.CappedSeconds, static_cast<int64>(0));
	TestEqual(TEXT("Invalid level gold is zero"), InvalidLevel.Gold, static_cast<int64>(0));
	TestEqual(TEXT("Invalid level exp is zero"), InvalidLevel.Exp, static_cast<int64>(0));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleGameInstanceOfflineRewardTest,
	"IdleProject.GameCore.IdleGameInstance.ClaimOfflineRewards",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleGameInstanceOfflineRewardTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	TestNotNull(TEXT("Game instance is created"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	GameInstance->SetLastSeenUnixSec(1000);

	const FOfflineRewardResult Preview = GameInstance->PreviewOfflineRewards(1060, 0);
	TestEqual(TEXT("Preview uses current character level"), Preview.Gold, static_cast<int64>(180));
	TestEqual(TEXT("Preview exp mirrors gold at level 1"), Preview.Exp, static_cast<int64>(180));
	TestEqual(TEXT("Preview does not mutate gold"), GameInstance->GetGold(), static_cast<int64>(0));

	const FOfflineRewardResult Claimed = GameInstance->ClaimOfflineRewardsAt(1060, 0);
	TestEqual(TEXT("Claim returns calculated gold"), Claimed.Gold, static_cast<int64>(180));
	TestEqual(TEXT("Claim adds gold"), GameInstance->GetGold(), static_cast<int64>(180));
	TestEqual(TEXT("Claim applies exp through existing AddExp path"), GameInstance->GetCharacterLevel(), static_cast<int32>(2));
	TestEqual(TEXT("Claim keeps overflow exp after level up"), GameInstance->GetCurrentExp(), static_cast<int64>(30));
	TestEqual(TEXT("Claim updates last seen"), GameInstance->GetLastSeenUnixSec(), static_cast<int64>(1060));

	const FOfflineRewardResult DuplicateClaim = GameInstance->ClaimOfflineRewardsAt(1060, 0);
	TestEqual(TEXT("Duplicate claim at same timestamp has no reward"), DuplicateClaim.Gold, static_cast<int64>(0));
	TestEqual(TEXT("Duplicate claim does not add gold"), GameInstance->GetGold(), static_cast<int64>(180));

	return true;
}

#endif
