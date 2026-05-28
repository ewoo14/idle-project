#include "Misc/AutomationTest.h"

#include "RuneSystem/RuneFormula.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRuneFormulaCoreScalingTest,
	"IdleProject.Rune.Formula.CoreScaling",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRuneFormulaCoreScalingTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Physical attack is a core rune type"), FRuneFormula::IsCoreType(ERuneType::PhysAtk));
	TestTrue(TEXT("HP is a core rune type"), FRuneFormula::IsCoreType(ERuneType::Hp));
	TestFalse(TEXT("Gold find is not a core rune type"), FRuneFormula::IsCoreType(ERuneType::GoldFind));
	TestFalse(TEXT("None is not a core rune type"), FRuneFormula::IsCoreType(ERuneType::None));

	TestEqual(TEXT("Common +0 core rune starts at two percent"), FRuneFormula::GetCoreRuneMultiplier(EItemRarity::Common, 0), 0.02f);
	TestEqual(TEXT("Mythic +50 core rune has no cap"), FRuneFormula::GetCoreRuneMultiplier(EItemRarity::Mythic, 50), 1.68f);
	TestTrue(
		TEXT("Core rune multiplier grows monotonically with enhance level"),
		FRuneFormula::GetCoreRuneMultiplier(EItemRarity::Rare, 11) > FRuneFormula::GetCoreRuneMultiplier(EItemRarity::Rare, 10));
	TestEqual(TEXT("Invalid core rarity contributes zero"), FRuneFormula::GetCoreRuneMultiplier(EItemRarity::None, 10), 0.0f);
	TestEqual(TEXT("Negative enhance level clamps to zero"), FRuneFormula::GetCoreRuneMultiplier(EItemRarity::Common, -5), 0.02f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRuneFormulaUtilScalingTest,
	"IdleProject.Rune.Formula.UtilScaling",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRuneFormulaUtilScalingTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Crit damage is a util rune type"), FRuneFormula::IsUtilType(ERuneType::CritDamage));
	TestTrue(TEXT("Offline efficiency is a util rune type"), FRuneFormula::IsUtilType(ERuneType::OfflineEff));
	TestFalse(TEXT("Physical attack is not a util rune type"), FRuneFormula::IsUtilType(ERuneType::PhysAtk));
	TestFalse(TEXT("None is not a util rune type"), FRuneFormula::IsUtilType(ERuneType::None));

	TestEqual(TEXT("Mythic +0 gold find starts at twelve percent"), FRuneFormula::GetUtilRuneValue(ERuneType::GoldFind, EItemRarity::Mythic, 0), 0.12f);
	TestEqual(TEXT("Gold find clamps at cap"), FRuneFormula::GetUtilRuneValue(ERuneType::GoldFind, EItemRarity::Mythic, 10000), FRuneFormula::GetUtilCap(ERuneType::GoldFind));
	TestEqual(TEXT("Gold find cap is two hundred percent"), FRuneFormula::GetUtilCap(ERuneType::GoldFind), 2.0f);
	TestEqual(TEXT("Exp boost cap is two hundred percent"), FRuneFormula::GetUtilCap(ERuneType::ExpBoost), 2.0f);
	TestEqual(TEXT("Crit damage cap is one hundred percent"), FRuneFormula::GetUtilCap(ERuneType::CritDamage), 1.0f);
	TestEqual(TEXT("Offline efficiency cap is fifty percent"), FRuneFormula::GetUtilCap(ERuneType::OfflineEff), 0.5f);
	TestEqual(TEXT("Offline efficiency clamps at cap"), FRuneFormula::GetUtilRuneValue(ERuneType::OfflineEff, EItemRarity::Mythic, 10000), 0.5f);
	TestEqual(TEXT("Invalid util rarity contributes zero"), FRuneFormula::GetUtilRuneValue(ERuneType::GoldFind, EItemRarity::None, 10), 0.0f);
	TestEqual(TEXT("Core type has no util value"), FRuneFormula::GetUtilRuneValue(ERuneType::PhysAtk, EItemRarity::Mythic, 10), 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRuneFormulaEconomyTest,
	"IdleProject.Rune.Formula.Economy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRuneFormulaEconomyTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Level zero essence enhance cost"), FRuneFormula::GetEnhanceEssenceCost(0), static_cast<int64>(10));
	TestEqual(TEXT("Level one essence enhance cost"), FRuneFormula::GetEnhanceEssenceCost(1), static_cast<int64>(40));
	TestEqual(TEXT("Level four essence enhance cost"), FRuneFormula::GetEnhanceEssenceCost(4), static_cast<int64>(250));
	TestEqual(TEXT("Negative enhance cost clamps to level zero"), FRuneFormula::GetEnhanceEssenceCost(-3), static_cast<int64>(10));
	TestEqual(TEXT("Level zero gold enhance cost"), FRuneFormula::GetEnhanceGoldCost(0), static_cast<int64>(1000));
	TestEqual(TEXT("Level four gold enhance cost"), FRuneFormula::GetEnhanceGoldCost(4), static_cast<int64>(25000));

	TestEqual(TEXT("Common disenchant base essence"), FRuneFormula::GetDisenchantEssence(EItemRarity::Common, 0), static_cast<int64>(1));
	TestEqual(TEXT("Mythic disenchant adds level scaling"), FRuneFormula::GetDisenchantEssence(EItemRarity::Mythic, 3), static_cast<int64>(86));
	TestEqual(TEXT("Invalid rarity disenchants to zero"), FRuneFormula::GetDisenchantEssence(EItemRarity::None, 10), static_cast<int64>(0));

	TestEqual(TEXT("First rune shop roll starts at 5000 gold"), FRuneFormula::GetShopRuneRollCost(0), static_cast<int64>(5000));
	TestEqual(TEXT("Progress index ten scales shop roll cost"), FRuneFormula::GetShopRuneRollCost(10), static_cast<int64>(10000));
	TestEqual(TEXT("Negative progress index clamps shop cost"), FRuneFormula::GetShopRuneRollCost(-5), static_cast<int64>(5000));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRuneFormulaDeterministicRollTest,
	"IdleProject.Rune.Formula.DeterministicRoll",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRuneFormulaDeterministicRollTest::RunTest(const FString& Parameters)
{
	FRandomStream First(617);
	FRandomStream Second(617);
	const FRuneInstance FirstRune = FRuneFormula::RollShopRune(3, First);
	const FRuneInstance SecondRune = FRuneFormula::RollShopRune(3, Second);

	TestEqual(TEXT("Shop rune roll is deterministic by seed type"), FirstRune.RuneType, SecondRune.RuneType);
	TestEqual(TEXT("Shop rune roll is deterministic by seed rarity"), FirstRune.Rarity, SecondRune.Rarity);
	TestEqual(TEXT("Shop rune roll starts unenhanced"), FirstRune.EnhanceLevel, 0);
	TestTrue(TEXT("Shop rune roll returns a valid rune type"), FirstRune.RuneType != ERuneType::None);
	TestTrue(TEXT("Shop rune roll returns a valid rarity"), FirstRune.Rarity != EItemRarity::None);

	FRandomStream NormalDropSeed(11);
	FRuneInstance NormalDrop;
	TestFalse(TEXT("Normal drop can deterministically miss at two percent"), FRuneFormula::RollRuneDrop(10, false, NormalDropSeed, NormalDrop));

	FRandomStream BossDropSeed(11);
	FRuneInstance BossDrop;
	const bool bBossDropped = FRuneFormula::RollRuneDrop(10, true, BossDropSeed, BossDrop);
	if (bBossDropped)
	{
		TestTrue(TEXT("Boss drop produces a valid rune type"), BossDrop.RuneType != ERuneType::None);
		TestTrue(TEXT("Boss drop produces a valid rarity"), BossDrop.Rarity != EItemRarity::None);
		TestEqual(TEXT("Dropped rune starts unenhanced"), BossDrop.EnhanceLevel, 0);
	}

	return true;
}

#endif
