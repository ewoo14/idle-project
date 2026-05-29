#include "Misc/AutomationTest.h"

#include "Internationalization/IdleLocalization.h"
#include "ItemSystem/EnhanceFormula.h"
#include "ItemSystem/DropFormula.h"
#include "ItemSystem/InventoryComponent.h"
#include "ItemSystem/ItemFactory.h"
#include "ItemSystem/PotentialFormula.h"
#include "ItemSystem/SetBonusFormula.h"
#include "ItemSystem/ShopFormula.h"
#include "ItemSystem/ItemTypes.h"
#include "UI/IdleHUD.h"
#include "UI/UIThemeTokens.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
FItemInstance MakeTestItem(FName ItemId, EItemSlot Slot, EItemRarity Rarity, float Atk, float Def, float Hp, int32 EnhanceLevel = 0, float CritRate = 0.0f, float AtkSpeed = 0.0f, float MagicAtk = 0.0f, EItemSet ItemSet = EItemSet::None)
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

int32 CountAffixes(const FItemInstance& Item)
{
	return (Item.BonusCritRate > 0.0f ? 1 : 0)
		+ (Item.BonusAtkSpeed > 0.0f ? 1 : 0)
		+ (Item.BonusMagicAtk > 0.0f ? 1 : 0)
		+ (Item.BonusPhysDef > 0.0f ? 1 : 0)
		+ (Item.BonusMagicDef > 0.0f ? 1 : 0)
		+ (Item.BonusAffixHp > 0.0f ? 1 : 0)
		+ (Item.BonusCritDmg > 0.0f ? 1 : 0);
}

void TestAffixCountForRarity(FAutomationTestBase& Test, const TCHAR* Context, const FItemInstance& Item)
{
	const int32 AffixCount = CountAffixes(Item);
	switch (Item.Rarity)
	{
	case EItemRarity::Common:
		Test.TestEqual(FString::Printf(TEXT("%s Common has no affixes"), Context), AffixCount, 0);
		break;
	case EItemRarity::Rare:
		Test.TestEqual(FString::Printf(TEXT("%s Rare has one affix"), Context), AffixCount, 1);
		break;
	case EItemRarity::Epic:
	case EItemRarity::Unique:
		Test.TestEqual(FString::Printf(TEXT("%s Epic has two affixes"), Context), AffixCount, 2);
		break;
	case EItemRarity::Legendary:
	case EItemRarity::Transcendent:
		Test.TestTrue(FString::Printf(TEXT("%s Legendary has two or three affixes"), Context), AffixCount >= 2 && AffixCount <= 3);
		break;
	case EItemRarity::Mythic:
		Test.TestEqual(FString::Printf(TEXT("%s Mythic has three affixes"), Context), AffixCount, 3);
		break;
	case EItemRarity::None:
	default:
		Test.TestEqual(FString::Printf(TEXT("%s None has no affixes"), Context), AffixCount, 0);
		break;
	}
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEnhanceFormulaRiskOutcomeTest,
	"IdleProject.Inventory.EnhanceFormula.RiskOutcome",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEnhanceFormulaRiskOutcomeTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Safe max level"), FEnhanceFormula::SafeMaxLevel, 9);
	TestEqual(TEXT("Pity threshold"), FEnhanceFormula::PityThreshold, 12);
	TestFalse(TEXT("+9 is safe"), FEnhanceFormula::IsRiskLevel(9));
	TestTrue(TEXT("+10 is risk"), FEnhanceFormula::IsRiskLevel(10));

	const FEnhanceAttemptOutcome SafeFailure = FEnhanceFormula::ResolveAttempt(5, 2, false, false, 0.999f);
	TestFalse(TEXT("Safe failure does not succeed"), SafeFailure.bSuccess);
	TestEqual(TEXT("Safe failure keeps level"), SafeFailure.NewLevel, 5);
	TestEqual(TEXT("Safe failure increments streak"), SafeFailure.NewFailStreak, 3);

	const FEnhanceAttemptOutcome RiskFailure = FEnhanceFormula::ResolveAttempt(20, 4, false, false, 0.999f);
	TestFalse(TEXT("Risk failure does not succeed"), RiskFailure.bSuccess);
	TestEqual(TEXT("Risk failure downgrades"), RiskFailure.NewLevel, 19);
	TestEqual(TEXT("Risk failure increments streak"), RiskFailure.NewFailStreak, 5);

	const FEnhanceAttemptOutcome ProtectedFailure = FEnhanceFormula::ResolveAttempt(20, 4, true, true, 0.999f);
	TestTrue(TEXT("Protection is consumed"), ProtectedFailure.bConsumedProtection);
	TestEqual(TEXT("Protection keeps risk level"), ProtectedFailure.NewLevel, 20);

	const FEnhanceAttemptOutcome Pity = FEnhanceFormula::ResolveAttempt(20, 12, false, false, 0.999f);
	TestTrue(TEXT("Pity succeeds"), Pity.bSuccess);
	TestTrue(TEXT("Pity flag set"), Pity.bPityTriggered);
	TestEqual(TEXT("Pity advances level"), Pity.NewLevel, 21);
	TestEqual(TEXT("Pity resets streak"), Pity.NewFailStreak, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPotentialFormulaRulesTest,
	"IdleProject.Inventory.PotentialFormula.Rules",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPotentialFormulaRulesTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Common has no potential cap"), FPotentialFormula::GetMaxPotentialGrade(EItemRarity::Common), EPotentialGrade::None);
	TestEqual(TEXT("Rare caps at Epic"), FPotentialFormula::GetMaxPotentialGrade(EItemRarity::Rare), EPotentialGrade::Epic);
	TestEqual(TEXT("Unique caps at Legendary"), FPotentialFormula::GetMaxPotentialGrade(EItemRarity::Unique), EPotentialGrade::Legendary);
	TestEqual(TEXT("Legendary has three lines"), FPotentialFormula::GetPotentialLineCount(EPotentialGrade::Legendary), 3);

	// 잠재 V2: 고레어도(Legendary/Transcendent/Mythic) → Transcendent 상한, Epic → Unique 등 단조 검증(서버 parity).
	TestEqual(TEXT("Epic caps at Unique"), FPotentialFormula::GetMaxPotentialGrade(EItemRarity::Epic), EPotentialGrade::Unique);
	TestEqual(TEXT("Legendary caps at Transcendent"), FPotentialFormula::GetMaxPotentialGrade(EItemRarity::Legendary), EPotentialGrade::Transcendent);
	TestEqual(TEXT("Transcendent rarity caps at Transcendent"), FPotentialFormula::GetMaxPotentialGrade(EItemRarity::Transcendent), EPotentialGrade::Transcendent);
	TestEqual(TEXT("Mythic caps at Transcendent"), FPotentialFormula::GetMaxPotentialGrade(EItemRarity::Mythic), EPotentialGrade::Transcendent);
	// 잠재 V2: Transcendent 4줄.
	TestEqual(TEXT("Transcendent has four lines"), FPotentialFormula::GetPotentialLineCount(EPotentialGrade::Transcendent), 4);

	float MinValue = 0.0f;
	float MaxValue = 0.0f;
	FPotentialFormula::GetPotentialRollRange(EPotentialGrade::Unique, MinValue, MaxValue);
	TestEqual(TEXT("Unique min roll"), MinValue, 0.06f);
	TestEqual(TEXT("Unique max roll"), MaxValue, 0.10f);

	// 잠재 V2: Transcendent = Legendary × 1.3 상향(0.13 ~ 0.195). 서버 getPotentialRollRange parity.
	float TransMin = 0.0f;
	float TransMax = 0.0f;
	FPotentialFormula::GetPotentialRollRange(EPotentialGrade::Transcendent, TransMin, TransMax);
	TestEqual(TEXT("Transcendent min roll is 0.13"), TransMin, 0.13f);
	TestEqual(TEXT("Transcendent max roll is 0.195"), TransMax, 0.195f);

	FRandomStream Rng(7101);
	const TArray<FPotentialLine> Lines = FPotentialFormula::RollPotentialLines(EPotentialGrade::Unique, Rng);
	TestEqual(TEXT("Unique rolls three potential lines"), Lines.Num(), 3);
	for (const FPotentialLine& Line : Lines)
	{
		TestTrue(TEXT("Potential stat is populated"), Line.Stat != EPotentialStat::None);
		TestTrue(TEXT("Potential value stays in range"), Line.Value >= 0.06f && Line.Value <= 0.10f);
	}
	return true;
}

namespace
{
// 잠재 V2: 신규 옵션 값 배수(서버 NEW_OPTION_VALUE_SCALE parity). 익명 헬퍼는 Potential~ prefix(jumbo ODR).
float PotentialExpectedOptionScale(EPotentialStat Stat)
{
	switch (Stat)
	{
	case EPotentialStat::AllStatPercent:
		return 0.4f;
	case EPotentialStat::GoldFindPercent:
	case EPotentialStat::DropRatePercent:
		return 1.5f;
	default:
		return 1.0f;
	}
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPotentialFormulaV2OptionRangesTest,
	"IdleProject.Inventory.PotentialFormula.V2OptionRanges",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPotentialFormulaV2OptionRangesTest::RunTest(const FString& Parameters)
{
	// 잠재 V2: Transcendent 기준 신규 옵션 3종 값 범위 = 등급 범위 × 배수(3자리 라운딩). 서버 getPotentialStatRollRange parity.
	const EPotentialStat NewStats[] = {
		EPotentialStat::AllStatPercent,
		EPotentialStat::GoldFindPercent,
		EPotentialStat::DropRatePercent
	};
	for (const EPotentialStat Stat : NewStats)
	{
		const float Scale = PotentialExpectedOptionScale(Stat);
		float StatMin = 0.0f;
		float StatMax = 0.0f;
		FPotentialFormula::GetPotentialStatRollRange(EPotentialGrade::Transcendent, Stat, StatMin, StatMax);
		const float ExpectedMin = FMath::RoundToFloat(0.13f * Scale * 1000.0f) / 1000.0f;
		const float ExpectedMax = FMath::RoundToFloat(0.195f * Scale * 1000.0f) / 1000.0f;
		TestEqual(FString::Printf(TEXT("New option stat %d Transcendent min"), static_cast<int32>(Stat)), StatMin, ExpectedMin);
		TestEqual(FString::Printf(TEXT("New option stat %d Transcendent max"), static_cast<int32>(Stat)), StatMax, ExpectedMax);
	}

	// 전투 8종은 배수 1.0 — 등급 기본 범위와 동일.
	float CombatMin = 0.0f;
	float CombatMax = 0.0f;
	FPotentialFormula::GetPotentialStatRollRange(EPotentialGrade::Transcendent, EPotentialStat::PhysAtkPercent, CombatMin, CombatMax);
	TestEqual(TEXT("Combat stat keeps grade min"), CombatMin, 0.13f);
	TestEqual(TEXT("Combat stat keeps grade max"), CombatMax, 0.195f);

	// Transcendent 롤은 4줄이며 각 줄 값이 스탯별 범위 안에 들어간다.
	FRandomStream Rng(990011);
	const TArray<FPotentialLine> Lines = FPotentialFormula::RollPotentialLines(EPotentialGrade::Transcendent, Rng);
	TestEqual(TEXT("Transcendent rolls four lines"), Lines.Num(), 4);
	for (const FPotentialLine& Line : Lines)
	{
		float LineMin = 0.0f;
		float LineMax = 0.0f;
		FPotentialFormula::GetPotentialStatRollRange(EPotentialGrade::Transcendent, Line.Stat, LineMin, LineMax);
		TestTrue(TEXT("Transcendent line value within stat range"), Line.Value >= LineMin - KINDA_SMALL_NUMBER && Line.Value <= LineMax + KINDA_SMALL_NUMBER);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPotentialFormulaV2RankCubeTranscendentTest,
	"IdleProject.Inventory.PotentialFormula.V2RankCubeTranscendent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPotentialFormulaV2RankCubeTranscendentTest::RunTest(const FString& Parameters)
{
	// 잠재 V2: Legendary→Transcendent 상승 확률 = 0.05 (서버 RANK_CUBE_TRANSCENDENT_CHANCE parity).
	TestEqual(TEXT("Rank cube transcendent chance is 0.05"), FPotentialFormula::RankCubeTranscendentChance, 0.05f);
	TestEqual(TEXT("Rank cube base chance is 0.08"), FPotentialFormula::RankCubeUpgradeChance, 0.08f);

	// 결정적 경계: GetFraction 첫 값이 0.05 미만이면 상승, 이상이면 유지. 시드 탐색으로 1/0 경계 검증.
	bool bFoundUpgrade = false;
	bool bFoundHold = false;
	for (int32 Seed = 1; Seed <= 4000 && !(bFoundUpgrade && bFoundHold); ++Seed)
	{
		FRandomStream ProbeRng(Seed);
		const float FirstFraction = FRandomStream(Seed).GetFraction();
		TArray<FPotentialLine> OutLines;
		const EPotentialGrade Result = FPotentialFormula::ApplyRankCube(EPotentialGrade::Legendary, EPotentialGrade::Transcendent, ProbeRng, OutLines);
		if (FirstFraction < FPotentialFormula::RankCubeTranscendentChance)
		{
			TestEqual(TEXT("Below transcendent chance upgrades to Transcendent"), Result, EPotentialGrade::Transcendent);
			TestEqual(TEXT("Upgraded Transcendent rolls four lines"), OutLines.Num(), 4);
			bFoundUpgrade = true;
		}
		else
		{
			TestEqual(TEXT("At/above transcendent chance holds Legendary"), Result, EPotentialGrade::Legendary);
			TestEqual(TEXT("Held Legendary rolls three lines"), OutLines.Num(), 3);
			bFoundHold = true;
		}
	}
	TestTrue(TEXT("Found a transcendent upgrade outcome"), bFoundUpgrade);
	TestTrue(TEXT("Found a hold outcome"), bFoundHold);

	// 상한 도달(이미 Transcendent) → 상승 불가, 유지.
	FRandomStream CapRng(42);
	TArray<FPotentialLine> CapLines;
	const EPotentialGrade Capped = FPotentialFormula::ApplyRankCube(EPotentialGrade::Transcendent, EPotentialGrade::Transcendent, CapRng, CapLines);
	TestEqual(TEXT("Already-capped grade stays Transcendent"), Capped, EPotentialGrade::Transcendent);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPotentialEconomyAggregationTest,
	"IdleProject.Inventory.PotentialFormula.V2EconomyAggregation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPotentialEconomyAggregationTest::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inventory = NewObject<UInventoryComponent>();

	// 무장착 → 합산 0.
	const FEquippedPotentialEconomyBonus Empty = Inventory->ComputeEquippedPotentialEconomyBonus();
	TestEqual(TEXT("Empty AllStat sum"), Empty.AllStatPercent, 0.0f);
	TestEqual(TEXT("Empty Gold sum"), Empty.GoldFindPercent, 0.0f);
	TestEqual(TEXT("Empty Drop sum"), Empty.DropRatePercent, 0.0f);

	// 무기: AllStat 0.05 + Gold 0.1 (2줄).
	FItemInstance Weapon = MakeTestItem(TEXT("trans_weapon"), EItemSlot::Weapon, EItemRarity::Legendary, 10.0f, 0.0f, 0.0f);
	Weapon.PotentialGrade = EPotentialGrade::Transcendent;
	Weapon.PotentialLine1.Stat = EPotentialStat::AllStatPercent;
	Weapon.PotentialLine1.Value = 0.05f;
	Weapon.PotentialLine2.Stat = EPotentialStat::GoldFindPercent;
	Weapon.PotentialLine2.Value = 0.10f;
	// 4번째 줄: DropRate 0.2 (Transcendent 줄4 수용 검증).
	Weapon.PotentialLine4.Stat = EPotentialStat::DropRatePercent;
	Weapon.PotentialLine4.Value = 0.20f;
	Inventory->AddItem(Weapon);

	// 헬멧: AllStat 0.03 + Drop 0.05 (두 슬롯 합산 누적 검증).
	FItemInstance Helmet = MakeTestItem(TEXT("trans_helmet"), EItemSlot::Helmet, EItemRarity::Legendary, 0.0f, 5.0f, 0.0f);
	Helmet.PotentialGrade = EPotentialGrade::Transcendent;
	Helmet.PotentialLine1.Stat = EPotentialStat::AllStatPercent;
	Helmet.PotentialLine1.Value = 0.03f;
	Helmet.PotentialLine2.Stat = EPotentialStat::DropRatePercent;
	Helmet.PotentialLine2.Value = 0.05f;
	Inventory->AddItem(Helmet);

	const FEquippedPotentialEconomyBonus Bonus = Inventory->ComputeEquippedPotentialEconomyBonus();
	TestEqual(TEXT("AllStat sums across slots and lines"), Bonus.AllStatPercent, 0.08f);
	TestEqual(TEXT("Gold sums equipped potential"), Bonus.GoldFindPercent, 0.10f);
	TestEqual(TEXT("Drop sums across line1+line4"), Bonus.DropRatePercent, 0.25f);

	// 신규 옵션은 전투 스탯 보너스(ComputeEquipmentBonus)에 합류하지 않는다(이중 적용 가드 #72).
	const FDerivedStats Combat = Inventory->ComputeEquipmentBonus();
	TestEqual(TEXT("New options do not inflate PhysAtk multiplier"), Combat.PhysAtk, 10.0f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPotentialSaveRoundTripV22Test,
	"IdleProject.Inventory.PotentialFormula.V2SaveRoundTripV22",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPotentialSaveRoundTripV22Test::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inventory = NewObject<UInventoryComponent>();

	FItemInstance Weapon = MakeTestItem(TEXT("trans_weapon"), EItemSlot::Weapon, EItemRarity::Legendary, 10.0f, 0.0f, 0.0f);
	Weapon.PotentialGrade = EPotentialGrade::Transcendent;
	const TArray<FPotentialLine> Lines = {
		FPotentialLine{ EPotentialStat::PhysAtkPercent, 0.18f },
		FPotentialLine{ EPotentialStat::AllStatPercent, 0.07f },
		FPotentialLine{ EPotentialStat::GoldFindPercent, 0.25f },
		FPotentialLine{ EPotentialStat::DropRatePercent, 0.28f }
	};
	Inventory->AddItem(Weapon);
	TestTrue(TEXT("SetEquippedPotential accepts four Transcendent lines"), Inventory->SetEquippedPotential(EItemSlot::Weapon, EPotentialGrade::Transcendent, Lines));

	// Capture → Restore 라운드트립(SaveVer 22 무변경, UE 태그드 직렬화). 4줄/신규 옵션 보존 확인.
	TArray<FItemInstance> CapturedItems;
	TMap<EItemSlot, int32> CapturedEquipped;
	Inventory->CaptureState(CapturedItems, CapturedEquipped);

	UInventoryComponent* Restored = NewObject<UInventoryComponent>();
	Restored->RestoreState(CapturedItems, CapturedEquipped);

	const FItemInstance* RestoredWeapon = Restored->GetEquippedItem(EItemSlot::Weapon);
	TestNotNull(TEXT("Restored weapon present"), RestoredWeapon);
	if (RestoredWeapon)
	{
		TestEqual(TEXT("Restored grade is Transcendent"), RestoredWeapon->PotentialGrade, EPotentialGrade::Transcendent);
		TestEqual(TEXT("Line1 stat preserved"), RestoredWeapon->PotentialLine1.Stat, EPotentialStat::PhysAtkPercent);
		TestEqual(TEXT("Line1 value preserved"), RestoredWeapon->PotentialLine1.Value, 0.18f);
		TestEqual(TEXT("Line2 new option preserved"), RestoredWeapon->PotentialLine2.Stat, EPotentialStat::AllStatPercent);
		TestEqual(TEXT("Line2 value preserved"), RestoredWeapon->PotentialLine2.Value, 0.07f);
		TestEqual(TEXT("Line3 gold preserved"), RestoredWeapon->PotentialLine3.Stat, EPotentialStat::GoldFindPercent);
		TestEqual(TEXT("Line4 drop preserved"), RestoredWeapon->PotentialLine4.Stat, EPotentialStat::DropRatePercent);
		TestEqual(TEXT("Line4 value preserved"), RestoredWeapon->PotentialLine4.Value, 0.28f);
	}

	// 기존 v22(3줄, 신규 옵션 없음) 세이브 전방호환: 4번째 줄 기본값(None) 로드.
	FItemInstance LegacyWeapon = MakeTestItem(TEXT("legacy_weapon"), EItemSlot::Weapon, EItemRarity::Unique, 8.0f, 0.0f, 0.0f);
	LegacyWeapon.PotentialGrade = EPotentialGrade::Legendary;
	LegacyWeapon.PotentialLine1.Stat = EPotentialStat::PhysAtkPercent;
	LegacyWeapon.PotentialLine1.Value = 0.12f;
	// PotentialLine4 의도적으로 기본값(None/0) 유지.
	UInventoryComponent* LegacyInv = NewObject<UInventoryComponent>();
	LegacyInv->AddItem(LegacyWeapon);
	TArray<FItemInstance> LegacyItems;
	TMap<EItemSlot, int32> LegacyEquipped;
	LegacyInv->CaptureState(LegacyItems, LegacyEquipped);
	UInventoryComponent* LegacyRestored = NewObject<UInventoryComponent>();
	LegacyRestored->RestoreState(LegacyItems, LegacyEquipped);
	const FItemInstance* RestoredLegacy = LegacyRestored->GetEquippedItem(EItemSlot::Weapon);
	TestNotNull(TEXT("Legacy weapon present"), RestoredLegacy);
	if (RestoredLegacy)
	{
		TestEqual(TEXT("Legacy line4 defaults to None (forward compat)"), RestoredLegacy->PotentialLine4.Stat, EPotentialStat::None);
		TestEqual(TEXT("Legacy line1 preserved"), RestoredLegacy->PotentialLine1.Value, 0.12f);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FItemFactoryBaseCatalogDeterministicTest,
	"IdleProject.Inventory.ItemFactory.BaseCatalogDeterministic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FItemFactoryBaseCatalogDeterministicTest::RunTest(const FString& Parameters)
{
	IdleProject::Localization::SetLanguageForTests(TEXT("en"));
	FRandomStream SeedA(5801);
	FRandomStream SeedB(5801);
	const FItemInstance First = FItemFactory::GuaranteedDropForLevel(35, SeedA);
	const FItemInstance Second = FItemFactory::GuaranteedDropForLevel(35, SeedB);

	TestTrue(TEXT("Generated item id includes selected base item, not only rarity and slot"), First.ItemId.ToString().Find(TEXT("_L35")) != INDEX_NONE);
	TestEqual(TEXT("Deterministic item id repeats with the same stream"), First.ItemId, Second.ItemId);
	TestEqual(TEXT("Deterministic display name repeats with the same stream"), First.DisplayName.ToString(), Second.DisplayName.ToString());
	TestFalse(TEXT("Display name no longer uses generic enum slot noun"), First.DisplayName.ToString().Contains(TEXT("EItemSlot")));
	TestTrue(TEXT("Generated item keeps playable stats"), FItemPowerScore::Compute(First) > 0);

	FRandomStream VarietyRng(5802);
	TSet<FName> WeaponBaseIds;
	for (int32 Index = 0; Index < 240; ++Index)
	{
		const FItemInstance Drop = FItemFactory::GuaranteedDropForLevel(60, VarietyRng);
		if (Drop.Slot == EItemSlot::Weapon)
		{
			WeaponBaseIds.Add(Drop.BaseItemId);
		}
	}
	TestTrue(TEXT("Weapon base catalog exposes at least six distinct weapon bases"), WeaponBaseIds.Num() >= 6);

	FRandomStream EnglishRng(5810);
	const FItemInstance EnglishDrop = FItemFactory::GuaranteedDropForLevel(40, EnglishRng);
	TestTrue(TEXT("English localized item name contains a rarity and base item"), EnglishDrop.DisplayName.ToString().Contains(TEXT(" ")));
	TestFalse(TEXT("English localized item name does not expose localization keys"), EnglishDrop.DisplayName.ToString().Contains(TEXT("BASE_ITEM_")));

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	FRandomStream KoreanRng(5810);
	const FItemInstance KoreanDrop = FItemFactory::GuaranteedDropForLevel(40, KoreanRng);
	TestNotEqual(TEXT("Korean and English item names localize from the same deterministic drop"), KoreanDrop.DisplayName.ToString(), EnglishDrop.DisplayName.ToString());
	TestFalse(TEXT("Korean localized item name does not expose localization keys"), KoreanDrop.DisplayName.ToString().Contains(TEXT("BASE_ITEM_")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FShopFormulaGearRollCostTest,
	"IdleProject.Inventory.ShopFormula.GearRollCost",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FShopFormulaGearRollCostTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Negative stage uses stage zero cost"), FShopFormula::GetGearRollCost(-10), static_cast<int64>(300));
	TestEqual(TEXT("Stage zero cost"), FShopFormula::GetGearRollCost(0), static_cast<int64>(300));
	TestEqual(TEXT("Stage one cost keeps the 1-1 baseline"), FShopFormula::GetGearRollCost(1), static_cast<int64>(300));
	TestEqual(TEXT("Stage five cost rounds 300 * 1.6"), FShopFormula::GetGearRollCost(5), static_cast<int64>(480));
	TestTrue(TEXT("Gear roll cost increases with stage"), FShopFormula::GetGearRollCost(10) > FShopFormula::GetGearRollCost(1));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FShopFormulaMaterialCostTest,
	"IdleProject.Inventory.ShopFormula.MaterialCosts",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FShopFormulaMaterialCostTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Protection scroll uses gear roll base at stage zero"), FShopFormula::GetProtectionScrollCost(0), static_cast<int64>(300));
	TestEqual(TEXT("Reset cube uses reset base at stage zero"), FShopFormula::GetResetCubeCost(0), static_cast<int64>(800));
	TestEqual(TEXT("Rank cube uses rank base at stage zero"), FShopFormula::GetRankCubeCost(0), static_cast<int64>(4000));

	TestEqual(TEXT("Protection scroll rounds stage four multiplier"), FShopFormula::GetProtectionScrollCost(4), static_cast<int64>(435));
	TestEqual(TEXT("Reset cube rounds stage four multiplier"), FShopFormula::GetResetCubeCost(4), static_cast<int64>(1160));
	TestEqual(TEXT("Rank cube rounds stage four multiplier"), FShopFormula::GetRankCubeCost(4), static_cast<int64>(5800));

	TestEqual(TEXT("Protection scroll rounds stage nine multiplier"), FShopFormula::GetProtectionScrollCost(9), static_cast<int64>(660));
	TestEqual(TEXT("Reset cube rounds stage nine multiplier"), FShopFormula::GetResetCubeCost(9), static_cast<int64>(1760));
	TestEqual(TEXT("Rank cube rounds stage nine multiplier"), FShopFormula::GetRankCubeCost(9), static_cast<int64>(8800));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDropFormulaRarityMultiplierTest,
	"IdleProject.Inventory.DropFormula.RarityMultiplier",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDropFormulaRarityMultiplierTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("None rarity has no stat multiplier"), FDropFormula::GetRarityStatMultiplier(EItemRarity::None), 0.0f);
	TestEqual(TEXT("Common stat multiplier"), FDropFormula::GetRarityStatMultiplier(EItemRarity::Common), 1.0f);
	TestEqual(TEXT("Rare stat multiplier"), FDropFormula::GetRarityStatMultiplier(EItemRarity::Rare), 1.7f);
	TestEqual(TEXT("Epic stat multiplier"), FDropFormula::GetRarityStatMultiplier(EItemRarity::Epic), 2.3f);
	TestEqual(TEXT("Unique stat multiplier"), FDropFormula::GetRarityStatMultiplier(EItemRarity::Unique), 2.75f);
	TestEqual(TEXT("Legendary stat multiplier"), FDropFormula::GetRarityStatMultiplier(EItemRarity::Legendary), 3.2f);
	TestEqual(TEXT("Transcendent stat multiplier"), FDropFormula::GetRarityStatMultiplier(EItemRarity::Transcendent), 3.85f);
	TestEqual(TEXT("Mythic stat multiplier"), FDropFormula::GetRarityStatMultiplier(EItemRarity::Mythic), 4.5f);
	TestTrue(TEXT("Mythic stat multiplier exceeds Legendary"), FDropFormula::GetRarityStatMultiplier(EItemRarity::Mythic) > FDropFormula::GetRarityStatMultiplier(EItemRarity::Legendary));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDropFormulaLevelRarityTrendTest,
	"IdleProject.Inventory.DropFormula.LevelRarityTrend",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDropFormulaLevelRarityTrendTest::RunTest(const FString& Parameters)
{
	constexpr int32 SampleCount = 10000;
	int32 LowRareOrBetter = 0;
	int32 HighRareOrBetter = 0;
	int32 HighEpicOrBetter = 0;
	int32 LowMythic = 0;
	int32 HighMythic = 0;

	FRandomStream LowLevelRng(3601);
	FRandomStream HighLevelRng(3601);
	for (int32 Index = 0; Index < SampleCount; ++Index)
	{
		const EItemRarity LowRarity = FDropFormula::RollRarityForLevel(1, LowLevelRng);
		if (LowRarity >= EItemRarity::Rare)
		{
			++LowRareOrBetter;
		}
		if (LowRarity == EItemRarity::Mythic)
		{
			++LowMythic;
		}

		const EItemRarity HighRarity = FDropFormula::RollRarityForLevel(100, HighLevelRng);
		if (HighRarity >= EItemRarity::Rare)
		{
			++HighRareOrBetter;
		}
		if (HighRarity >= EItemRarity::Epic)
		{
			++HighEpicOrBetter;
		}
		if (HighRarity == EItemRarity::Mythic)
		{
			++HighMythic;
		}
	}

	TestTrue(TEXT("High-level drops roll Rare+ more often than low-level drops"), HighRareOrBetter > LowRareOrBetter);
	TestTrue(TEXT("High-level drops can roll Epic+"), HighEpicOrBetter > 0);
	TestEqual(TEXT("Level 1 drops never roll Mythic"), LowMythic, 0);
	TestTrue(TEXT("Level 100 drops can roll Mythic"), HighMythic > 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FItemRarityMythicHudMappingTest,
	"IdleProject.Inventory.Rarity.MythicHudMapping",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FItemRarityMythicHudMappingTest::RunTest(const FString& Parameters)
{
	IdleProject::Localization::SetLanguageForTests(TEXT("en"));
	TestEqual(TEXT("Mythic rarity label is localized"), IdleProject::UI::RarityToLabel(EItemRarity::Mythic).ToString(), FString(TEXT("Mythic")));
	TestEqual(TEXT("Mythic rarity color uses designer token"), IdleProject::UI::RarityToColor(EItemRarity::Mythic), IdleProject::UI::Theme::RarityMythicStart);

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDropFormulaComputeItemBonusTest,
	"IdleProject.Inventory.DropFormula.ComputeItemBonus",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDropFormulaComputeItemBonusTest::RunTest(const FString& Parameters)
{
	const FItemInstance LegendaryWeapon = FDropFormula::ComputeItemBonus(EItemSlot::Weapon, 10, EItemRarity::Legendary, 1.5f);
	TestEqual(TEXT("Legendary weapon attack scales by level variance and rarity"), LegendaryWeapon.BonusAtk, 48.0f);
	TestEqual(TEXT("Legendary weapon has no defense bonus"), LegendaryWeapon.BonusDef, 0.0f);
	TestEqual(TEXT("Legendary weapon has no HP bonus"), LegendaryWeapon.BonusHp, 0.0f);
	TestEqual(TEXT("Computed base item has no crit affix"), LegendaryWeapon.BonusCritRate, 0.0f);
	TestEqual(TEXT("Computed base item has no attack speed affix"), LegendaryWeapon.BonusAtkSpeed, 0.0f);
	TestEqual(TEXT("Computed base item has no magic attack affix"), LegendaryWeapon.BonusMagicAtk, 0.0f);

	const FItemInstance EpicAccessory = FDropFormula::ComputeItemBonus(EItemSlot::Accessory, 10, EItemRarity::Epic, 1.0f);
	TestEqual(TEXT("Epic accessory attack split"), EpicAccessory.BonusAtk, 11.5f);
	TestEqual(TEXT("Epic accessory defense split"), EpicAccessory.BonusDef, 6.9f);
	TestEqual(TEXT("Epic accessory HP split"), EpicAccessory.BonusHp, 46.0f);

	const FItemInstance CommonWeapon = FDropFormula::ComputeItemBonus(EItemSlot::Weapon, 1, EItemRarity::Common, 1.0f);
	TestEqual(TEXT("Level 1 Common keeps legacy base attack"), CommonWeapon.BonusAtk, 1.0f);
	TestEqual(TEXT("Level 1 Common keeps legacy base defense"), CommonWeapon.BonusDef, 0.0f);
	TestEqual(TEXT("Level 1 Common keeps legacy base HP"), CommonWeapon.BonusHp, 0.0f);

	const FItemInstance RareArmor = FDropFormula::ComputeItemBonus(EItemSlot::Helmet, 10, EItemRarity::Rare, 2.0f);
	TestEqual(TEXT("Rare armor defense uses float slot split"), RareArmor.BonusDef, 23.8f);
	TestEqual(TEXT("Rare armor HP uses float slot split"), RareArmor.BonusHp, 102.0f);

	const FItemInstance MythicWeapon = FDropFormula::ComputeItemBonus(EItemSlot::Weapon, 10, EItemRarity::Mythic, 1.0f);
	TestEqual(TEXT("Mythic weapon attack scales by level variance and rarity"), MythicWeapon.BonusAtk, 45.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDropFormulaRollAffixesTest,
	"IdleProject.Inventory.DropFormula.RollAffixes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDropFormulaRollAffixesTest::RunTest(const FString& Parameters)
{
	FItemInstance Common = MakeTestItem(TEXT("common_sword"), EItemSlot::Weapon, EItemRarity::Common, 1.0f, 0.0f, 0.0f);
	Common.BonusCritRate = 0.05f;
	Common.BonusAtkSpeed = 0.15f;
	Common.BonusMagicAtk = 30.0f;
	Common.BonusPhysDef = 20.0f;
	Common.BonusMagicDef = 20.0f;
	Common.BonusAffixHp = 100.0f;
	Common.BonusCritDmg = 0.20f;
	FRandomStream CommonRng(4001);
	FDropFormula::RollAffixes(Common.Rarity, 20, CommonRng, Common);
	TestEqual(TEXT("Common items roll no crit affix"), Common.BonusCritRate, 0.0f);
	TestEqual(TEXT("Common items roll no speed affix"), Common.BonusAtkSpeed, 0.0f);
	TestEqual(TEXT("Common items roll no magic affix"), Common.BonusMagicAtk, 0.0f);
	TestEqual(TEXT("Common items roll no physical defense affix"), Common.BonusPhysDef, 0.0f);
	TestEqual(TEXT("Common items roll no magic defense affix"), Common.BonusMagicDef, 0.0f);
	TestEqual(TEXT("Common items roll no HP affix"), Common.BonusAffixHp, 0.0f);
	TestEqual(TEXT("Common items roll no crit damage affix"), Common.BonusCritDmg, 0.0f);

	FItemInstance Rare = MakeTestItem(TEXT("rare_sword"), EItemSlot::Weapon, EItemRarity::Rare, 1.0f, 0.0f, 0.0f);
	Rare.BonusCritRate = 0.05f;
	Rare.BonusAtkSpeed = 0.15f;
	Rare.BonusMagicAtk = 30.0f;
	FRandomStream RareRng(4001);
	FDropFormula::RollAffixes(Rare.Rarity, 20, RareRng, Rare);
	const int32 RareAffixCount = CountAffixes(Rare);
	TestEqual(TEXT("Rare rolls one affix"), RareAffixCount, 1);

	FItemInstance Epic = MakeTestItem(TEXT("epic_sword"), EItemSlot::Weapon, EItemRarity::Epic, 1.0f, 0.0f, 0.0f);
	FRandomStream EpicRng(4002);
	FDropFormula::RollAffixes(Epic.Rarity, 20, EpicRng, Epic);
	const int32 EpicAffixCount = CountAffixes(Epic);
	TestEqual(TEXT("Epic rolls two unique affixes"), EpicAffixCount, 2);
	TestTrue(TEXT("Crit affix stays in range when present"), Epic.BonusCritRate == 0.0f || (Epic.BonusCritRate >= 0.01f && Epic.BonusCritRate <= 0.05f));
	TestTrue(TEXT("Attack speed affix stays in range when present"), Epic.BonusAtkSpeed == 0.0f || (Epic.BonusAtkSpeed >= 0.05f && Epic.BonusAtkSpeed <= 0.15f));
	TestTrue(TEXT("Magic attack affix scales by level when present"), Epic.BonusMagicAtk == 0.0f || (Epic.BonusMagicAtk >= 10.0f && Epic.BonusMagicAtk <= 30.0f));

	FItemInstance LegendaryA = MakeTestItem(TEXT("legendary_a"), EItemSlot::Weapon, EItemRarity::Legendary, 1.0f, 0.0f, 0.0f);
	FItemInstance LegendaryB = MakeTestItem(TEXT("legendary_b"), EItemSlot::Weapon, EItemRarity::Legendary, 1.0f, 0.0f, 0.0f);
	FRandomStream LegendaryRngA(4003);
	FRandomStream LegendaryRngB(4003);
	FDropFormula::RollAffixes(LegendaryA.Rarity, 20, LegendaryRngA, LegendaryA);
	FDropFormula::RollAffixes(LegendaryB.Rarity, 20, LegendaryRngB, LegendaryB);
	const int32 LegendaryAffixCount = CountAffixes(LegendaryA);
	TestTrue(TEXT("Legendary rolls two or three affixes"), LegendaryAffixCount >= 2 && LegendaryAffixCount <= 3);
	TestEqual(TEXT("Legendary roll is deterministic for crit"), LegendaryA.BonusCritRate, LegendaryB.BonusCritRate);
	TestEqual(TEXT("Legendary roll is deterministic for speed"), LegendaryA.BonusAtkSpeed, LegendaryB.BonusAtkSpeed);
	TestEqual(TEXT("Legendary roll is deterministic for magic"), LegendaryA.BonusMagicAtk, LegendaryB.BonusMagicAtk);

	FItemInstance Mythic = MakeTestItem(TEXT("mythic_sword"), EItemSlot::Weapon, EItemRarity::Mythic, 1.0f, 0.0f, 0.0f);
	FRandomStream MythicRng(4004);
	FDropFormula::RollAffixes(Mythic.Rarity, 20, MythicRng, Mythic);
	TestEqual(TEXT("Mythic rolls three affixes from the expanded pool"), CountAffixes(Mythic), 3);
	TestTrue(TEXT("Mythic crit affix stays in range when present"), Mythic.BonusCritRate == 0.0f || (Mythic.BonusCritRate >= 0.01f && Mythic.BonusCritRate <= 0.05f));
	TestTrue(TEXT("Mythic attack speed affix stays in range when present"), Mythic.BonusAtkSpeed == 0.0f || (Mythic.BonusAtkSpeed >= 0.05f && Mythic.BonusAtkSpeed <= 0.15f));
	TestTrue(TEXT("Mythic magic attack affix scales by level when present"), Mythic.BonusMagicAtk == 0.0f || (Mythic.BonusMagicAtk >= 10.0f && Mythic.BonusMagicAtk <= 30.0f));
	TestTrue(TEXT("Mythic physical defense affix scales by level when present"), Mythic.BonusPhysDef == 0.0f || (Mythic.BonusPhysDef >= 6.0f && Mythic.BonusPhysDef <= 20.0f));
	TestTrue(TEXT("Mythic magic defense affix scales by level when present"), Mythic.BonusMagicDef == 0.0f || (Mythic.BonusMagicDef >= 6.0f && Mythic.BonusMagicDef <= 20.0f));
	TestTrue(TEXT("Mythic HP affix scales by level when present"), Mythic.BonusAffixHp == 0.0f || (Mythic.BonusAffixHp >= 40.0f && Mythic.BonusAffixHp <= 100.0f));
	TestTrue(TEXT("Mythic crit damage affix stays in range when present"), Mythic.BonusCritDmg == 0.0f || (Mythic.BonusCritDmg >= 0.05f && Mythic.BonusCritDmg <= 0.20f));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDropFormulaExpandedAffixesCoverageTest,
	"IdleProject.Inventory.DropFormula.ExpandedAffixesCoverage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDropFormulaExpandedAffixesCoverageTest::RunTest(const FString& Parameters)
{
	bool bFoundPhysDef = false;
	bool bFoundMagicDef = false;
	bool bFoundHp = false;
	bool bFoundCritDmg = false;
	for (int32 Seed = 5803; Seed < 6203; ++Seed)
	{
		FItemInstance Item = MakeTestItem(TEXT("mythic_affix_scan"), EItemSlot::Weapon, EItemRarity::Mythic, 1.0f, 0.0f, 0.0f);
		FRandomStream Rng(Seed);
		FDropFormula::RollAffixes(Item.Rarity, 25, Rng, Item);
		TestEqual(TEXT("Mythic still rolls exactly three unique affixes from expanded pool"), CountAffixes(Item), 3);
		bFoundPhysDef = bFoundPhysDef || Item.BonusPhysDef > 0.0f;
		bFoundMagicDef = bFoundMagicDef || Item.BonusMagicDef > 0.0f;
		bFoundHp = bFoundHp || Item.BonusAffixHp > 0.0f;
		bFoundCritDmg = bFoundCritDmg || Item.BonusCritDmg > 0.0f;
	}

	TestTrue(TEXT("Expanded pool can roll physical defense"), bFoundPhysDef);
	TestTrue(TEXT("Expanded pool can roll magic defense"), bFoundMagicDef);
	TestTrue(TEXT("Expanded pool can roll HP"), bFoundHp);
	TestTrue(TEXT("Expanded pool can roll crit damage"), bFoundCritDmg);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDropFormulaRollItemSetTest,
	"IdleProject.Inventory.DropFormula.RollItemSet",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDropFormulaRollItemSetTest::RunTest(const FString& Parameters)
{
	FRandomStream CommonRngA(4301);
	FRandomStream CommonRngB(4301);
	TestEqual(TEXT("Common items do not roll an item set"), FDropFormula::RollItemSet(EItemRarity::Common, CommonRngA), EItemSet::None);
	TestEqual(TEXT("Common item set roll is deterministic"), FDropFormula::RollItemSet(EItemRarity::Common, CommonRngB), EItemSet::None);

	bool bFoundWarrior = false;
	bool bFoundGuardian = false;
	bool bFoundArcane = false;
	bool bFoundAssassin = false;
	bool bFoundHunter = false;
	bool bFoundHoly = false;
	bool bFoundBerserker = false;
	FRandomStream RareRng(4302);
	for (int32 Index = 0; Index < 320; ++Index)
	{
		const EItemSet ItemSet = FDropFormula::RollItemSet(EItemRarity::Rare, RareRng);
		TestTrue(TEXT("Rare+ set roll stays in enum range"), ItemSet >= EItemSet::Warrior && ItemSet <= EItemSet::Berserker);
		bFoundWarrior = bFoundWarrior || ItemSet == EItemSet::Warrior;
		bFoundGuardian = bFoundGuardian || ItemSet == EItemSet::Guardian;
		bFoundArcane = bFoundArcane || ItemSet == EItemSet::Arcane;
		bFoundAssassin = bFoundAssassin || ItemSet == EItemSet::Assassin;
		bFoundHunter = bFoundHunter || ItemSet == EItemSet::Hunter;
		bFoundHoly = bFoundHoly || ItemSet == EItemSet::Holy;
		bFoundBerserker = bFoundBerserker || ItemSet == EItemSet::Berserker;
	}

	TestTrue(TEXT("Rare+ rolls can produce Warrior"), bFoundWarrior);
	TestTrue(TEXT("Rare+ rolls can produce Guardian"), bFoundGuardian);
	TestTrue(TEXT("Rare+ rolls can produce Arcane"), bFoundArcane);
	TestTrue(TEXT("Rare+ rolls can produce Assassin"), bFoundAssassin);
	TestTrue(TEXT("Rare+ rolls can produce Hunter"), bFoundHunter);
	TestTrue(TEXT("Rare+ rolls can produce Holy"), bFoundHoly);
	TestTrue(TEXT("Rare+ rolls can produce Berserker"), bFoundBerserker);

	FRandomStream MythicRng(4303);
	const EItemSet MythicSet = FDropFormula::RollItemSet(EItemRarity::Mythic, MythicRng);
	TestTrue(TEXT("Mythic items roll an item set"), MythicSet >= EItemSet::Warrior && MythicSet <= EItemSet::Berserker);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSetBonusFormulaDefinitionParityTest,
	"IdleProject.Inventory.SetBonusFormula.DefinitionParity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSetBonusFormulaDefinitionParityTest::RunTest(const FString& Parameters)
{
	const FDerivedStats WarriorTwoPiece = FSetBonusFormula::GetTwoPieceBonus(EItemSet::Warrior);
	TestEqual(TEXT("Warrior 2-piece attack matches server"), WarriorTwoPiece.PhysAtk, 20.0f);
	TestEqual(TEXT("Warrior 2-piece crit remains zero"), WarriorTwoPiece.CritRate, 0.0f);

	const FDerivedStats WarriorFourPiece = FSetBonusFormula::GetFourPieceBonus(EItemSet::Warrior);
	TestEqual(TEXT("Warrior 4-piece adds attack"), WarriorFourPiece.PhysAtk, 50.0f);
	TestEqual(TEXT("Warrior 4-piece adds crit"), WarriorFourPiece.CritRate, 0.05f);

	const FDerivedStats GuardianTwoPiece = FSetBonusFormula::GetTwoPieceBonus(EItemSet::Guardian);
	TestEqual(TEXT("Guardian 2-piece defense matches server"), GuardianTwoPiece.PhysDef, 15.0f);
	TestEqual(TEXT("Guardian 2-piece HP matches server"), GuardianTwoPiece.Hp, 100.0f);

	const FDerivedStats GuardianFourPiece = FSetBonusFormula::GetFourPieceBonus(EItemSet::Guardian);
	TestEqual(TEXT("Guardian 4-piece defense matches server"), GuardianFourPiece.PhysDef, 35.0f);
	TestEqual(TEXT("Guardian 4-piece HP matches server"), GuardianFourPiece.Hp, 250.0f);

	const FDerivedStats ArcaneTwoPiece = FSetBonusFormula::GetTwoPieceBonus(EItemSet::Arcane);
	TestEqual(TEXT("Arcane 2-piece magic attack matches server"), ArcaneTwoPiece.MagicAtk, 20.0f);

	const FDerivedStats ArcaneFourPiece = FSetBonusFormula::GetFourPieceBonus(EItemSet::Arcane);
	TestEqual(TEXT("Arcane 4-piece magic attack matches server"), ArcaneFourPiece.MagicAtk, 50.0f);
	TestEqual(TEXT("Arcane 4-piece crit damage matches server"), ArcaneFourPiece.CritDmg, 0.10f);

	const FDerivedStats AssassinTwoPiece = FSetBonusFormula::GetTwoPieceBonus(EItemSet::Assassin);
	TestEqual(TEXT("Assassin 2-piece crit matches server"), AssassinTwoPiece.CritRate, 0.03f);
	const FDerivedStats AssassinFourPiece = FSetBonusFormula::GetFourPieceBonus(EItemSet::Assassin);
	TestEqual(TEXT("Assassin 4-piece crit damage matches server"), AssassinFourPiece.CritDmg, 0.15f);

	const FDerivedStats HunterTwoPiece = FSetBonusFormula::GetTwoPieceBonus(EItemSet::Hunter);
	TestEqual(TEXT("Hunter 2-piece speed matches server"), HunterTwoPiece.AtkSpeed, 0.05f);
	const FDerivedStats HunterFourPiece = FSetBonusFormula::GetFourPieceBonus(EItemSet::Hunter);
	TestEqual(TEXT("Hunter 4-piece attack matches server"), HunterFourPiece.PhysAtk, 35.0f);

	const FDerivedStats HolyTwoPiece = FSetBonusFormula::GetTwoPieceBonus(EItemSet::Holy);
	TestEqual(TEXT("Holy 2-piece HP matches server"), HolyTwoPiece.Hp, 120.0f);
	const FDerivedStats HolyFourPiece = FSetBonusFormula::GetFourPieceBonus(EItemSet::Holy);
	TestEqual(TEXT("Holy 4-piece magic defense matches server"), HolyFourPiece.MagicDef, 30.0f);

	const FDerivedStats BerserkerTwoPiece = FSetBonusFormula::GetTwoPieceBonus(EItemSet::Berserker);
	TestEqual(TEXT("Berserker 2-piece attack matches server"), BerserkerTwoPiece.PhysAtk, 30.0f);
	const FDerivedStats BerserkerFourPiece = FSetBonusFormula::GetFourPieceBonus(EItemSet::Berserker);
	TestEqual(TEXT("Berserker 4-piece crit matches server"), BerserkerFourPiece.CritRate, 0.04f);

	const FDerivedStats NoneBonus = FSetBonusFormula::GetTwoPieceBonus(EItemSet::None);
	TestEqual(TEXT("None set has no two-piece attack"), NoneBonus.PhysAtk, 0.0f);
	TestEqual(TEXT("None set has no two-piece HP"), NoneBonus.Hp, 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSetBonusFormulaThresholdsTest,
	"IdleProject.Inventory.SetBonusFormula.Thresholds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSetBonusFormulaThresholdsTest::RunTest(const FString& Parameters)
{
	TArray<FItemInstance> EquippedItems;
	EquippedItems.Add(MakeTestItem(TEXT("warrior_weapon"), EItemSlot::Weapon, EItemRarity::Rare, 3.0f, 0.0f, 0.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Warrior));
	EquippedItems.Add(MakeTestItem(TEXT("warrior_helmet"), EItemSlot::Helmet, EItemRarity::Rare, 0.0f, 1.0f, 5.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Warrior));

	FDerivedStats Bonus = FSetBonusFormula::ComputeSetBonus(EquippedItems);
	TestEqual(TEXT("Warrior 2-piece grants physical attack"), Bonus.PhysAtk, 20.0f);
	TestEqual(TEXT("Warrior 2-piece does not grant crit"), Bonus.CritRate, 0.0f);

	EquippedItems.Add(MakeTestItem(TEXT("warrior_top"), EItemSlot::Top, EItemRarity::Rare, 0.0f, 1.0f, 5.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Warrior));
	EquippedItems.Add(MakeTestItem(TEXT("warrior_bottom"), EItemSlot::Bottom, EItemRarity::Rare, 0.0f, 1.0f, 5.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Warrior));

	Bonus = FSetBonusFormula::ComputeSetBonus(EquippedItems);
	TestEqual(TEXT("Warrior 4-piece includes 2-piece and 4-piece attack"), Bonus.PhysAtk, 70.0f);
	TestEqual(TEXT("Warrior 4-piece grants crit rate"), Bonus.CritRate, 0.05f);

	TArray<FItemInstance> MixedItems;
	MixedItems.Add(MakeTestItem(TEXT("none_weapon"), EItemSlot::Weapon, EItemRarity::Rare, 3.0f, 0.0f, 0.0f));
	MixedItems.Add(MakeTestItem(TEXT("guardian_helmet"), EItemSlot::Helmet, EItemRarity::Rare, 0.0f, 1.0f, 5.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Guardian));
	Bonus = FSetBonusFormula::ComputeSetBonus(MixedItems);
	TestEqual(TEXT("None and under-threshold sets grant no attack"), Bonus.PhysAtk, 0.0f);
	TestEqual(TEXT("None and under-threshold sets grant no defense"), Bonus.PhysDef, 0.0f);
	TestEqual(TEXT("None and under-threshold sets grant no HP"), Bonus.Hp, 0.0f);

	TArray<FItemInstance> HolyItems;
	HolyItems.Add(MakeTestItem(TEXT("holy_weapon"), EItemSlot::Weapon, EItemRarity::Rare, 3.0f, 0.0f, 0.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Holy));
	HolyItems.Add(MakeTestItem(TEXT("holy_helmet"), EItemSlot::Helmet, EItemRarity::Rare, 0.0f, 1.0f, 5.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Holy));
	HolyItems.Add(MakeTestItem(TEXT("holy_top"), EItemSlot::Top, EItemRarity::Rare, 0.0f, 1.0f, 5.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Holy));
	HolyItems.Add(MakeTestItem(TEXT("holy_bottom"), EItemSlot::Bottom, EItemRarity::Rare, 0.0f, 1.0f, 5.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Holy));
	Bonus = FSetBonusFormula::ComputeSetBonus(HolyItems);
	TestEqual(TEXT("Holy 4-piece includes HP"), Bonus.Hp, 120.0f);
	TestEqual(TEXT("Holy 4-piece adds both defenses"), Bonus.PhysDef, 20.0f);
	TestEqual(TEXT("Holy 4-piece adds magic defense"), Bonus.MagicDef, 30.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryEquipmentBonusSetBonusTest,
	"IdleProject.Inventory.Bonus.SetBonus",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInventoryEquipmentBonusSetBonusTest::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inventory = NewObject<UInventoryComponent>();
	const FItemInstance WarriorWeapon = MakeTestItem(TEXT("warrior_weapon"), EItemSlot::Weapon, EItemRarity::Rare, 10.0f, 0.0f, 0.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Warrior);
	Inventory->AddItem(WarriorWeapon);
	Inventory->AddItem(MakeTestItem(TEXT("warrior_helmet"), EItemSlot::Helmet, EItemRarity::Rare, 0.0f, 5.0f, 10.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Warrior));
	Inventory->AddItem(MakeTestItem(TEXT("warrior_top"), EItemSlot::Top, EItemRarity::Rare, 0.0f, 5.0f, 10.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Warrior));
	Inventory->AddItem(MakeTestItem(TEXT("warrior_bottom"), EItemSlot::Bottom, EItemRarity::Rare, 0.0f, 5.0f, 10.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Warrior));

	const FDerivedStats Bonus = Inventory->ComputeEquipmentBonus();
	TestEqual(TEXT("Equipment bonus includes per-item attack and Warrior set attack"), Bonus.PhysAtk, 80.0f);
	TestEqual(TEXT("Equipment bonus includes per-item defense only for defense"), Bonus.PhysDef, 15.0f);
	TestEqual(TEXT("Equipment bonus includes Warrior set crit"), Bonus.CritRate, 0.05f);

	const FPrimaryStats Primary(10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f);
	const FDerivedStats Derived = FStatFormulas::DeriveStats(Primary, 1, Bonus);
	TestEqual(TEXT("Derived stats include set physical attack"), Derived.PhysAtk, 100.0f);
	TestEqual(TEXT("Derived stats include set crit rate"), Derived.CritRate, 0.07f);
	TestEqual(TEXT("PowerScore ignores item set membership"), FItemPowerScore::Compute(WarriorWeapon), 10);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEnhanceFormulaCurveTest,
	"IdleProject.Inventory.EnhanceFormula.Curve",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEnhanceFormulaCurveTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Max enhance level matches item clamp"), FEnhanceFormula::MaxEnhanceLevel, 50);
	TestEqual(TEXT("Level 0 cost"), FEnhanceFormula::GetEnhanceCost(0), static_cast<int64>(100));
	TestEqual(TEXT("Level 1 cost"), FEnhanceFormula::GetEnhanceCost(1), static_cast<int64>(400));
	TestEqual(TEXT("Negative level uses level 0 cost"), FEnhanceFormula::GetEnhanceCost(-1), static_cast<int64>(100));
	TestEqual(TEXT("Max level has no next enhance cost"), FEnhanceFormula::GetEnhanceCost(FEnhanceFormula::MaxEnhanceLevel), static_cast<int64>(0));
	TestEqual(TEXT("None rarity has no enhance cost multiplier"), FEnhanceFormula::GetRarityCostMultiplier(EItemRarity::None), static_cast<int64>(0));
	TestEqual(TEXT("Common rarity keeps legacy cost multiplier"), FEnhanceFormula::GetRarityCostMultiplier(EItemRarity::Common), static_cast<int64>(1));
	TestEqual(TEXT("Rare rarity doubles enhance cost"), FEnhanceFormula::GetRarityCostMultiplier(EItemRarity::Rare), static_cast<int64>(2));
	TestEqual(TEXT("Epic rarity multiplies enhance cost by four"), FEnhanceFormula::GetRarityCostMultiplier(EItemRarity::Epic), static_cast<int64>(4));
	TestEqual(TEXT("Unique rarity multiplies enhance cost by eight"), FEnhanceFormula::GetRarityCostMultiplier(EItemRarity::Unique), static_cast<int64>(8));
	TestEqual(TEXT("Legendary rarity multiplies enhance cost by sixteen"), FEnhanceFormula::GetRarityCostMultiplier(EItemRarity::Legendary), static_cast<int64>(16));
	TestEqual(TEXT("Transcendent rarity multiplies enhance cost by thirty-two"), FEnhanceFormula::GetRarityCostMultiplier(EItemRarity::Transcendent), static_cast<int64>(32));
	TestEqual(TEXT("Mythic rarity multiplies enhance cost by sixty-four"), FEnhanceFormula::GetRarityCostMultiplier(EItemRarity::Mythic), static_cast<int64>(64));
	TestEqual(TEXT("Common overload matches legacy single-argument cost"), FEnhanceFormula::GetEnhanceCost(1, EItemRarity::Common), static_cast<int64>(400));
	TestEqual(TEXT("Rare level 1 cost applies rarity multiplier"), FEnhanceFormula::GetEnhanceCost(1, EItemRarity::Rare), static_cast<int64>(800));
	TestEqual(TEXT("Legendary level 0 cost applies rarity multiplier"), FEnhanceFormula::GetEnhanceCost(0, EItemRarity::Legendary), static_cast<int64>(1600));
	TestEqual(TEXT("Mythic level 0 cost applies rarity multiplier"), FEnhanceFormula::GetEnhanceCost(0, EItemRarity::Mythic), static_cast<int64>(6400));
	TestEqual(TEXT("Max level rarity cost remains zero"), FEnhanceFormula::GetEnhanceCost(FEnhanceFormula::MaxEnhanceLevel, EItemRarity::Legendary), static_cast<int64>(0));
	TestEqual(TEXT("Rare level 4 cost matches server parity table"), FEnhanceFormula::GetEnhanceCost(4, EItemRarity::Rare), static_cast<int64>(5000));
	TestEqual(TEXT("Epic level 4 cost matches server parity table"), FEnhanceFormula::GetEnhanceCost(4, EItemRarity::Epic), static_cast<int64>(10000));
	TestEqual(TEXT("Unique level 4 cost matches server parity table"), FEnhanceFormula::GetEnhanceCost(4, EItemRarity::Unique), static_cast<int64>(20000));
	TestEqual(TEXT("Legendary level 4 cost matches server parity table"), FEnhanceFormula::GetEnhanceCost(4, EItemRarity::Legendary), static_cast<int64>(40000));
	TestEqual(TEXT("Common level 49 cost reaches 50-level sink"), FEnhanceFormula::GetEnhanceCost(49), static_cast<int64>(250000));
	TestEqual(TEXT("Legendary level 49 cost applies rarity multiplier"), FEnhanceFormula::GetEnhanceCost(49, EItemRarity::Legendary), static_cast<int64>(4000000));

	TestEqual(TEXT("Level 0 success rate"), FEnhanceFormula::GetEnhanceSuccessRate(0), 0.95f);
	TestTrue(TEXT("Level 4 success rate remains on the 50-level curve"), FMath::IsNearlyEqual(FEnhanceFormula::GetEnhanceSuccessRate(4), 0.878f, 0.0001f));
	TestTrue(TEXT("Level 10 success rate follows linear curve"), FMath::IsNearlyEqual(FEnhanceFormula::GetEnhanceSuccessRate(10), 0.77f, 0.0001f));
	TestTrue(TEXT("Level 25 success rate reaches midpoint pressure"), FMath::IsNearlyEqual(FEnhanceFormula::GetEnhanceSuccessRate(25), 0.50f, 0.0001f));
	TestTrue(TEXT("Level 40 success rate reaches high-level pressure"), FMath::IsNearlyEqual(FEnhanceFormula::GetEnhanceSuccessRate(40), 0.23f, 0.0001f));
	TestTrue(TEXT("Level 49 success rate stays above floor"), FMath::IsNearlyEqual(FEnhanceFormula::GetEnhanceSuccessRate(49), 0.068f, 0.0001f));
	TestEqual(TEXT("Max level success rate"), FEnhanceFormula::GetEnhanceSuccessRate(FEnhanceFormula::MaxEnhanceLevel), 0.0f);

	FRandomStream AlwaysSucceeds(123);
	TestTrue(TEXT("Success rate 1 always succeeds"), FEnhanceFormula::RollEnhanceSuccess(1.0f, AlwaysSucceeds));

	FRandomStream AlwaysFails(123);
	TestFalse(TEXT("Success rate 0 always fails"), FEnhanceFormula::RollEnhanceSuccess(0.0f, AlwaysFails));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FItemPowerScoreDeterministicTest,
	"IdleProject.Inventory.ItemPowerScore.Deterministic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FItemPowerScoreDeterministicTest::RunTest(const FString& Parameters)
{
	const FItemInstance Sword = MakeTestItem(TEXT("common_sword"), EItemSlot::Weapon, EItemRarity::Common, 10.0f, 0.0f, 0.0f, 2);

	TestEqual(TEXT("같은 아이템 점수 1"), FItemPowerScore::Compute(Sword), 12);
	TestEqual(TEXT("같은 아이템 점수 2"), FItemPowerScore::Compute(Sword), 12);

	const FItemInstance AffixSword = MakeTestItem(TEXT("affix_sword"), EItemSlot::Weapon, EItemRarity::Rare, 10.0f, 0.0f, 0.0f, 2, 0.02f, 0.10f, 5.0f);
	TestEqual(TEXT("Affixes contribute weighted power before enhance multiplier"), FItemPowerScore::Compute(AffixSword), 54);

	FItemInstance ExpandedAffixSword = MakeTestItem(TEXT("expanded_affix_sword"), EItemSlot::Weapon, EItemRarity::Mythic, 10.0f, 0.0f, 0.0f, 1);
	ExpandedAffixSword.BonusPhysDef = 5.0f;
	ExpandedAffixSword.BonusMagicDef = 7.0f;
	ExpandedAffixSword.BonusAffixHp = 40.0f;
	ExpandedAffixSword.BonusCritDmg = 0.20f;
	TestEqual(TEXT("Expanded affixes contribute weighted power before enhance multiplier"), FItemPowerScore::Compute(ExpandedAffixSword), 51);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryAutoEquipBetterWeaponTest,
	"IdleProject.Inventory.AutoEquip.BetterWeapon",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInventoryAutoEquipBetterWeaponTest::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inventory = NewObject<UInventoryComponent>();
	Inventory->AddItem(MakeTestItem(TEXT("common_sword"), EItemSlot::Weapon, EItemRarity::Common, 5.0f, 0.0f, 0.0f));
	Inventory->AddItem(MakeTestItem(TEXT("rare_sword"), EItemSlot::Weapon, EItemRarity::Rare, 8.0f, 0.0f, 0.0f));

	const FItemInstance* Equipped = Inventory->GetEquippedItem(EItemSlot::Weapon);
	TestNotNull(TEXT("무기 자동 장착"), Equipped);
	if (Equipped)
	{
		TestEqual(TEXT("더 강한 무기로 교체"), Equipped->ItemId, FName(TEXT("rare_sword")));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryEnhanceEquippedItemTest,
	"IdleProject.Inventory.EnhanceEquippedItem.SuccessAndMaxGuard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInventoryEnhanceEquippedItemTest::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inventory = NewObject<UInventoryComponent>();
	Inventory->AddItem(MakeTestItem(TEXT("rare_sword"), EItemSlot::Weapon, EItemRarity::Rare, 10.0f, 0.0f, 0.0f, 0));

	TestEqual(TEXT("Initial enhance level"), Inventory->GetEquippedEnhanceLevel(EItemSlot::Weapon), 0);
	TestTrue(TEXT("Enhance equipped weapon succeeds below max"), Inventory->EnhanceEquippedItem(EItemSlot::Weapon));
	TestEqual(TEXT("Enhance level increments"), Inventory->GetEquippedEnhanceLevel(EItemSlot::Weapon), 1);

	const FDerivedStats Bonus = Inventory->ComputeEquipmentBonus();
	TestEqual(TEXT("Enhanced item increases equipment attack bonus"), Bonus.PhysAtk, 11.0f);

	for (int32 Attempt = 1; Attempt < FEnhanceFormula::MaxEnhanceLevel; ++Attempt)
	{
		Inventory->EnhanceEquippedItem(EItemSlot::Weapon);
	}

	TestEqual(TEXT("Enhance level reaches max"), Inventory->GetEquippedEnhanceLevel(EItemSlot::Weapon), FEnhanceFormula::MaxEnhanceLevel);
	const FDerivedStats MaxBonus = Inventory->ComputeEquipmentBonus();
	TestEqual(TEXT("Lv50 enhanced item applies x6 equipment attack bonus"), MaxBonus.PhysAtk, 60.0f);
	TestFalse(TEXT("Enhance fails at max level"), Inventory->EnhanceEquippedItem(EItemSlot::Weapon));
	TestFalse(TEXT("Enhance fails when slot is empty"), Inventory->EnhanceEquippedItem(EItemSlot::Helmet));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEnhancePanelViewModelStateTest,
	"IdleProject.UI.HUD.EnhancePanelViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEnhancePanelViewModelStateTest::RunTest(const FString& Parameters)
{
	IdleProject::Localization::SetLanguageForTests(TEXT("en"));

	UInventoryComponent* Inventory = NewObject<UInventoryComponent>();
	Inventory->AddItem(MakeTestItem(TEXT("rare_sword"), EItemSlot::Weapon, EItemRarity::Rare, 10.0f, 0.0f, 0.0f, 0));
	Inventory->AddItem(MakeTestItem(TEXT("legendary_gloves"), EItemSlot::Gloves, EItemRarity::Legendary, 0.0f, 1.0f, 0.0f, 0));
	Inventory->AddItem(MakeTestItem(TEXT("common_cloak"), EItemSlot::Cloak, EItemRarity::Common, 0.0f, 1.0f, 0.0f, 0));

	const FIdleHUDEnhancePanelViewModel NoGold = IdleProject::UI::BuildEnhancePanelViewModel(
		*Inventory,
		0,
		FText::GetEmpty(),
		false);
	TestEqual(TEXT("Enhance panel exposes all equipment slots"), NoGold.Rows.Num(), 8);
	TestEqual(TEXT("Weapon row uses current enhance level"), NoGold.Rows[0].EnhanceLevel, 0);
	TestEqual(TEXT("Rare weapon row shows rarity-scaled level zero cost"), NoGold.Rows[0].Cost, static_cast<int64>(200));
	TestEqual(TEXT("Weapon row level label shows +N / 50 without a plus on the cap"), NoGold.Rows[0].LevelLabel.ToString(), FString(TEXT("+0 / 50")));
	TestEqual(TEXT("Weapon row success label shows integer percent"), NoGold.Rows[0].SuccessRateLabel.ToString(), FString(TEXT("Success 95%")));
	TestEqual(TEXT("Safe enhance row states no downgrade risk"), NoGold.Rows[0].RiskLabel.ToString(), FString(TEXT("Safe")));
	TestEqual(TEXT("Safe enhance row exposes current fail streak"), NoGold.Rows[0].FailStreakLabel.ToString(), FString(TEXT("Pity 0/12")));
	TestFalse(TEXT("Safe enhance row does not enable protection"), NoGold.Rows[0].bCanUseProtection);
	TestEqual(TEXT("Legendary gloves row shows rarity-scaled level zero cost"), NoGold.Rows[5].Cost, static_cast<int64>(1600));
	TestEqual(TEXT("Common cloak row preserves legacy level zero cost"), NoGold.Rows[6].Cost, static_cast<int64>(100));
	TestFalse(TEXT("Weapon row is disabled without gold"), NoGold.Rows[0].bCanEnhance);
	TestFalse(TEXT("Weapon row reports insufficient gold"), NoGold.Rows[0].bGoldEnough);
	TestFalse(TEXT("Empty helmet row is not enhanceable"), NoGold.Rows[1].bCanEnhance);
	TestFalse(TEXT("Empty helmet row reports no equipment"), NoGold.Rows[1].bEquipped);

	const FIdleHUDEnhancePanelViewModel EnoughGold = IdleProject::UI::BuildEnhancePanelViewModel(
		*Inventory,
		FEnhanceFormula::GetEnhanceCost(0, EItemRarity::Rare),
		FText::GetEmpty(),
		false);
	TestTrue(TEXT("Weapon row enables when equipped and gold is enough"), EnoughGold.Rows[0].bCanEnhance);

	UInventoryComponent* RiskInventory = NewObject<UInventoryComponent>();
	FItemInstance RiskSword = MakeTestItem(TEXT("risk_sword"), EItemSlot::Weapon, EItemRarity::Rare, 10.0f, 0.0f, 0.0f, 10);
	RiskSword.EnhanceFailStreak = 7;
	RiskInventory->AddItem(RiskSword);
	const FIdleHUDEnhancePanelViewModel RiskView = IdleProject::UI::BuildEnhancePanelViewModel(
		*RiskInventory,
		FEnhanceFormula::GetEnhanceCost(10, EItemRarity::Rare),
		2,
		FText::GetEmpty(),
		false);
	TestTrue(TEXT("Risk enhance row marks downgrade risk"), RiskView.Rows[0].bRiskLevel);
	TestEqual(TEXT("Risk enhance row explains downgrade penalty"), RiskView.Rows[0].RiskLabel.ToString(), FString(TEXT("Fail: -1 level")));
	TestEqual(TEXT("Risk enhance row shows pity progress"), RiskView.Rows[0].FailStreakLabel.ToString(), FString(TEXT("Pity 7/12")));
	TestEqual(TEXT("Enhance panel exposes protection scroll count"), RiskView.ProtectionLabel.ToString(), FString(TEXT("Protection 2")));
	TestTrue(TEXT("Risk enhance row enables protection when scrolls are available"), RiskView.Rows[0].bCanUseProtection);
	TestTrue(TEXT("Risk enhance row keeps normal enhance available"), RiskView.Rows[0].bCanEnhance);

	UInventoryComponent* MaxInventory = NewObject<UInventoryComponent>();
	MaxInventory->AddItem(MakeTestItem(TEXT("max_sword"), EItemSlot::Weapon, EItemRarity::Rare, 10.0f, 0.0f, 0.0f, FEnhanceFormula::MaxEnhanceLevel));
	const FIdleHUDEnhancePanelViewModel MaxLevel = IdleProject::UI::BuildEnhancePanelViewModel(
		*MaxInventory,
		100000,
		FText::GetEmpty(),
		false);
	TestTrue(TEXT("Max level row reports max state"), MaxLevel.Rows[0].bMaxLevel);
	TestFalse(TEXT("Max level row is disabled"), MaxLevel.Rows[0].bCanEnhance);
	TestEqual(TEXT("Max level row has zero next cost"), MaxLevel.Rows[0].Cost, static_cast<int64>(0));
	TestEqual(TEXT("Max level row shows +50 / 50 level label"), MaxLevel.Rows[0].LevelLabel.ToString(), FString(TEXT("+50 / 50")));
	TestEqual(TEXT("Max level row shows localized max status"), MaxLevel.Rows[0].StatusLabel.ToString(), FString(TEXT("MAX")));
	TestEqual(TEXT("Max level row shows max instead of next cost"), MaxLevel.Rows[0].CostLabel.ToString(), FString(TEXT("MAX")));
	TestEqual(TEXT("Max level row shows max instead of success percent"), MaxLevel.Rows[0].SuccessRateLabel.ToString(), FString(TEXT("MAX")));

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPotentialPanelViewModelStateTest,
	"IdleProject.UI.HUD.PotentialPanelViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPotentialPanelViewModelStateTest::RunTest(const FString& Parameters)
{
	IdleProject::Localization::SetLanguageForTests(TEXT("en"));

	UInventoryComponent* Inventory = NewObject<UInventoryComponent>();
	FItemInstance RareSword = MakeTestItem(TEXT("potential_sword"), EItemSlot::Weapon, EItemRarity::Rare, 10.0f, 0.0f, 0.0f);
	RareSword.PotentialGrade = EPotentialGrade::Rare;
	RareSword.PotentialLine1.Stat = EPotentialStat::PhysAtkPercent;
	RareSword.PotentialLine1.Value = 0.05f;
	RareSword.PotentialLine2.Stat = EPotentialStat::CritRatePercent;
	RareSword.PotentialLine2.Value = 0.03f;
	RareSword.bLocked = true;
	Inventory->AddItem(RareSword);

	const FIdleHUDPotentialPanelViewModel ViewModel = IdleProject::UI::BuildPotentialPanelViewModel(*Inventory, 3, 1);
	TestEqual(TEXT("Potential panel exposes all equipment slots"), ViewModel.Rows.Num(), 8);
	TestEqual(TEXT("Potential panel reset cube count label"), ViewModel.ResetCubeLabel.ToString(), FString(TEXT("Reset Cubes 3")));
	TestEqual(TEXT("Potential panel rank cube count label"), ViewModel.RankCubeLabel.ToString(), FString(TEXT("Rank Cubes 1")));

	const FIdleHUDPotentialSlotViewModel& WeaponRow = ViewModel.Rows[0];
	TestTrue(TEXT("Potential row knows equipped item"), WeaponRow.bEquipped);
	TestTrue(TEXT("Potential row exposes lock state"), WeaponRow.bLocked);
	TestEqual(TEXT("Potential row lock action toggles to unlock"), WeaponRow.LockActionLabel.ToString(), FString(TEXT("Unlock")));
	TestEqual(TEXT("Potential row shows grade and cap"), WeaponRow.GradeLabel.ToString(), FString(TEXT("Rare / Epic")));
	TestEqual(TEXT("Potential row summarizes rolled lines"), WeaponRow.LineSummaryLabel.ToString(), FString(TEXT("PATK +5% / Crit +3%")));
	TestTrue(TEXT("Potential row can use reset cube"), WeaponRow.bCanResetPotential);
	TestTrue(TEXT("Potential row can use rank cube below cap"), WeaponRow.bCanRankPotential);

	const FIdleHUDPotentialSlotViewModel& EmptyHelmet = ViewModel.Rows[1];
	TestFalse(TEXT("Empty potential row is not equipped"), EmptyHelmet.bEquipped);
	TestFalse(TEXT("Empty potential row cannot reset"), EmptyHelmet.bCanResetPotential);
	TestFalse(TEXT("Empty potential row cannot rank"), EmptyHelmet.bCanRankPotential);

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEquipmentAffixHudSummaryTest,
	"IdleProject.UI.HUD.EquipmentAffixSummary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEquipmentAffixHudSummaryTest::RunTest(const FString& Parameters)
{
	IdleProject::Localization::SetLanguageForTests(TEXT("en"));

	const FItemInstance PlainItem = MakeTestItem(TEXT("plain_sword"), EItemSlot::Weapon, EItemRarity::Common, 10.0f, 0.0f, 0.0f);
	TestTrue(TEXT("Zero affixes produce an empty summary"), IdleProject::UI::BuildAffixSummary(PlainItem).IsEmpty());

	const FItemInstance PartialAffixItem = MakeTestItem(TEXT("partial_sword"), EItemSlot::Weapon, EItemRarity::Rare, 10.0f, 0.0f, 0.0f, 0, 0.03f, 0.0f, 12.0f);
	TestEqual(
		TEXT("Only positive affixes are shown"),
		IdleProject::UI::BuildAffixSummary(PartialAffixItem).ToString(),
		FString(TEXT("Crit +3% / MATK +12")));

	const FItemInstance FullAffixItem = MakeTestItem(TEXT("full_sword"), EItemSlot::Weapon, EItemRarity::Legendary, 10.0f, 0.0f, 0.0f, 0, 0.03f, 0.10f, 12.0f);
	TestEqual(
		TEXT("Affix summary uses stable crit speed magic order"),
		IdleProject::UI::BuildAffixSummary(FullAffixItem).ToString(),
		FString(TEXT("Crit +3% / ASPD +0.10 / MATK +12")));

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEquipmentUniqueTraitHudSummaryTest,
	"IdleProject.UI.HUD.EquipmentUniqueTraitSummary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEquipmentUniqueTraitHudSummaryTest::RunTest(const FString& Parameters)
{
	IdleProject::Localization::SetLanguageForTests(TEXT("en"));

	const FItemInstance PlainItem = MakeTestItem(TEXT("plain_sword"), EItemSlot::Weapon, EItemRarity::Common, 10.0f, 0.0f, 0.0f);
	TestTrue(TEXT("No unique traits produce an empty summary"), IdleProject::UI::BuildUniqueTraitSummary(PlainItem).IsEmpty());

	FItemInstance UniqueItem = MakeTestItem(TEXT("unique_sword"), EItemSlot::Weapon, EItemRarity::Unique, 10.0f, 0.0f, 0.0f);
	UniqueItem.UniqueTrait1 = EUniqueTrait::AllStatSurge;
	TestEqual(
		TEXT("Unique item shows one trait with its rarity-scaled value"),
		IdleProject::UI::BuildUniqueTraitSummary(UniqueItem).ToString(),
		FString(TEXT("All-Stat Surge +8%")));

	FItemInstance TranscendentItem = MakeTestItem(TEXT("transcendent_sword"), EItemSlot::Weapon, EItemRarity::Transcendent, 10.0f, 0.0f, 0.0f);
	TranscendentItem.UniqueTrait1 = EUniqueTrait::CritDamageSurge;
	TranscendentItem.UniqueTrait2 = EUniqueTrait::SwiftSurge;
	TestEqual(
		TEXT("Transcendent item shows two traits in slot order"),
		IdleProject::UI::BuildUniqueTraitSummary(TranscendentItem).ToString(),
		FString(TEXT("Crit Damage Surge +23% / Swift Surge +0.12")));

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEquipmentSetHudSummaryTest,
	"IdleProject.UI.HUD.EquipmentSetSummary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEquipmentSetHudSummaryTest::RunTest(const FString& Parameters)
{
	IdleProject::Localization::SetLanguageForTests(TEXT("en"));

	TArray<FItemInstance> EquippedItems;
	EquippedItems.Add(MakeTestItem(TEXT("warrior_weapon"), EItemSlot::Weapon, EItemRarity::Rare, 10.0f, 0.0f, 0.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Warrior));
	EquippedItems.Add(MakeTestItem(TEXT("warrior_helmet"), EItemSlot::Helmet, EItemRarity::Rare, 0.0f, 5.0f, 10.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Warrior));
	EquippedItems.Add(MakeTestItem(TEXT("warrior_top"), EItemSlot::Top, EItemRarity::Rare, 0.0f, 5.0f, 10.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Warrior));
	EquippedItems.Add(MakeTestItem(TEXT("common_shoes"), EItemSlot::Shoes, EItemRarity::Common, 0.0f, 2.0f, 5.0f));

	const FIdleHUDSetSummaryViewModel ViewModel = IdleProject::UI::BuildSetSummaryViewModel(EquippedItems);
	TestEqual(TEXT("Only non-empty item sets are summarized"), ViewModel.Rows.Num(), 1);
	TestEqual(TEXT("Warrior row counts three pieces toward four-piece cap"), ViewModel.Rows[0].PieceCount, 3);
	TestEqual(TEXT("Warrior row exposes current tier label"), ViewModel.Rows[0].TierLabel.ToString(), FString(TEXT("2-piece active")));
	TestEqual(TEXT("Warrior row exposes next tier label"), ViewModel.Rows[0].NextTierLabel.ToString(), FString(TEXT("Next 4-piece: 1 more")));
	TestEqual(TEXT("Warrior row summarizes active set bonus"), ViewModel.Rows[0].BonusLabel.ToString(), FString(TEXT("Bonus: PATK +20")));
	TestEqual(TEXT("Warrior row summary is compact for equipment HUD"), ViewModel.Rows[0].SummaryLabel.ToString(), FString(TEXT("Warrior Set 3/4 (2-piece active)")));

	EquippedItems.Add(MakeTestItem(TEXT("warrior_bottom"), EItemSlot::Bottom, EItemRarity::Rare, 0.0f, 5.0f, 10.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Warrior));
	const FIdleHUDSetSummaryViewModel FourPieceViewModel = IdleProject::UI::BuildSetSummaryViewModel(EquippedItems);
	TestEqual(TEXT("Four-piece row exposes four-piece tier label"), FourPieceViewModel.Rows[0].TierLabel.ToString(), FString(TEXT("4-piece active")));
	TestEqual(TEXT("Four-piece row reports complete state"), FourPieceViewModel.Rows[0].NextTierLabel.ToString(), FString(TEXT("Set complete")));
	TestEqual(TEXT("Four-piece row summarizes cumulative set bonus"), FourPieceViewModel.Rows[0].BonusLabel.ToString(), FString(TEXT("Bonus: PATK +70 / Crit +5%")));

	TArray<FItemInstance> UnderThresholdItems;
	UnderThresholdItems.Add(MakeTestItem(TEXT("guardian_helmet"), EItemSlot::Helmet, EItemRarity::Rare, 0.0f, 5.0f, 10.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Guardian));
	const FIdleHUDSetSummaryViewModel UnderThresholdViewModel = IdleProject::UI::BuildSetSummaryViewModel(UnderThresholdItems);
	TestEqual(TEXT("Under-threshold row exposes inactive tier"), UnderThresholdViewModel.Rows[0].TierLabel.ToString(), FString(TEXT("No set bonus")));
	TestEqual(TEXT("Under-threshold row points to two-piece"), UnderThresholdViewModel.Rows[0].NextTierLabel.ToString(), FString(TEXT("Next 2-piece: 1 more")));
	TestEqual(TEXT("Under-threshold row has no active bonus"), UnderThresholdViewModel.Rows[0].BonusLabel.ToString(), FString(TEXT("Bonus: -")));

	TArray<FItemInstance> NoSetItems;
	NoSetItems.Add(MakeTestItem(TEXT("plain_sword"), EItemSlot::Weapon, EItemRarity::Common, 10.0f, 0.0f, 0.0f));
	const FIdleHUDSetSummaryViewModel NoSetViewModel = IdleProject::UI::BuildSetSummaryViewModel(NoSetItems);
	TestEqual(TEXT("No-set equipment does not create a set row"), NoSetViewModel.Rows.Num(), 0);

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FShopPanelViewModelStateTest,
	"IdleProject.UI.HUD.ShopPanelViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FShopPanelViewModelStateTest::RunTest(const FString& Parameters)
{
	FShopPurchaseResult PurchaseResult;
	PurchaseResult.bPurchased = true;
	PurchaseResult.GoldSpent = 300;
	PurchaseResult.Rarity = EItemRarity::Rare;
	PurchaseResult.Slot = EItemSlot::Weapon;
	PurchaseResult.ItemName = FText::FromString(TEXT("rare_sword"));

	const FIdleHUDShopPanelViewModel Ready = IdleProject::UI::BuildShopPanelViewModel(300, 300, 800, 4000, 450, PurchaseResult);
	TestEqual(TEXT("Shop panel exposes gear roll cost"), Ready.GearRollCost, static_cast<int64>(300));
	TestTrue(TEXT("Shop panel enables purchase when gold is enough"), Ready.bCanBuyGearRoll);
	TestEqual(TEXT("Shop gear roll hitbox is stable"), Ready.GearRollHitBoxName, FName(TEXT("ShopGearRoll")));
	TestEqual(TEXT("Shop result carries purchased rarity"), Ready.LastResultRarity, EItemRarity::Rare);
	TestTrue(TEXT("Shop result is visible after purchase"), Ready.bHasLastResult);
	TestFalse(TEXT("Shop result is not an error after purchase"), Ready.bLastResultError);

	const FIdleHUDShopPanelViewModel NoGold = IdleProject::UI::BuildShopPanelViewModel(300, 300, 800, 4000, 250, FShopPurchaseResult());
	TestFalse(TEXT("Shop panel disables purchase when gold is short"), NoGold.bCanBuyGearRoll);
	TestFalse(TEXT("Empty shop result is hidden"), NoGold.bHasLastResult);

	FShopPurchaseResult FailedResult;
	FailedResult.bPurchased = false;
	FailedResult.GoldSpent = 300;
	const FIdleHUDShopPanelViewModel Failed = IdleProject::UI::BuildShopPanelViewModel(300, 300, 800, 4000, 250, FailedResult);
	TestTrue(TEXT("Failed purchase result is visible when a cost was attempted"), Failed.bHasLastResult);
	TestTrue(TEXT("Failed purchase result is flagged as an error"), Failed.bLastResultError);

	const FIdleHUDShopPanelViewModel Materials = IdleProject::UI::BuildShopPanelViewModel(
		300,
		300,
		800,
		4000,
		1000,
		FShopPurchaseResult());
	TestEqual(TEXT("Shop panel exposes protection scroll cost"), Materials.ProtectionScrollCost, static_cast<int64>(300));
	TestEqual(TEXT("Shop panel exposes reset cube cost"), Materials.ResetCubeCost, static_cast<int64>(800));
	TestEqual(TEXT("Shop panel exposes rank cube cost"), Materials.RankCubeCost, static_cast<int64>(4000));
	TestTrue(TEXT("Shop panel enables protection scroll when gold is enough"), Materials.bCanBuyProtectionScroll);
	TestTrue(TEXT("Shop panel enables reset cube when gold is enough"), Materials.bCanBuyResetCube);
	TestFalse(TEXT("Shop panel disables rank cube when gold is short"), Materials.bCanBuyRankCube);
	TestEqual(TEXT("Protection scroll hitbox is stable"), Materials.ProtectionScrollHitBoxName, FName(TEXT("ShopProtectionScroll")));
	TestEqual(TEXT("Reset cube hitbox is stable"), Materials.ResetCubeHitBoxName, FName(TEXT("ShopResetCube")));
	TestEqual(TEXT("Rank cube hitbox is stable"), Materials.RankCubeHitBoxName, FName(TEXT("ShopRankCube")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryEquipmentBonusTwoSlotsTest,
	"IdleProject.Inventory.Bonus.TwoSlots",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInventoryEquipmentBonusTwoSlotsTest::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inventory = NewObject<UInventoryComponent>();
	Inventory->AddItem(MakeTestItem(TEXT("rare_sword"), EItemSlot::Weapon, EItemRarity::Rare, 12.0f, 0.0f, 0.0f));
	Inventory->AddItem(MakeTestItem(TEXT("common_helmet"), EItemSlot::Helmet, EItemRarity::Common, 0.0f, 4.0f, 20.0f));

	const FDerivedStats Bonus = Inventory->ComputeEquipmentBonus();
	TestEqual(TEXT("무기 공격력 보너스"), Bonus.PhysAtk, 12.0f);
	TestEqual(TEXT("투구 방어력 보너스"), Bonus.PhysDef, 4.0f);
	TestEqual(TEXT("투구 체력 보너스"), Bonus.Hp, 20.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryEquipmentBonusAffixTest,
	"IdleProject.Inventory.Bonus.Affixes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInventoryEquipmentBonusAffixTest::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inventory = NewObject<UInventoryComponent>();
	FItemInstance AffixItem = MakeTestItem(TEXT("rare_sword"), EItemSlot::Weapon, EItemRarity::Rare, 10.0f, 0.0f, 0.0f, 2, 0.02f, 0.10f, 5.0f);
	AffixItem.BonusPhysDef = 4.0f;
	AffixItem.BonusMagicDef = 6.0f;
	AffixItem.BonusAffixHp = 30.0f;
	AffixItem.BonusCritDmg = 0.12f;
	Inventory->AddItem(AffixItem);

	const FDerivedStats Bonus = Inventory->ComputeEquipmentBonus();
	TestEqual(TEXT("Enhanced item increases physical attack"), Bonus.PhysAtk, 12.0f);
	TestEqual(TEXT("Enhanced item increases crit affix"), Bonus.CritRate, 0.024f);
	TestEqual(TEXT("Enhanced item increases attack speed affix"), Bonus.AtkSpeed, 0.12f);
	TestEqual(TEXT("Enhanced item increases magic attack affix"), Bonus.MagicAtk, 6.0f);
	TestEqual(TEXT("Enhanced item increases physical defense affix"), Bonus.PhysDef, 4.8f);
	TestEqual(TEXT("Enhanced item increases magic defense affix"), Bonus.MagicDef, 7.2f);
	TestEqual(TEXT("Enhanced item increases HP affix"), Bonus.Hp, 36.0f);
	TestEqual(TEXT("Enhanced item increases crit damage affix"), Bonus.CritDmg, 0.144f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryEquipmentBonusPotentialTest,
	"IdleProject.Inventory.Bonus.Potential",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInventoryEquipmentBonusPotentialTest::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inventory = NewObject<UInventoryComponent>();
	FItemInstance Item = MakeTestItem(TEXT("potential_sword"), EItemSlot::Weapon, EItemRarity::Rare, 100.0f, 0.0f, 50.0f, 2);
	Item.PotentialGrade = EPotentialGrade::Unique;
	Item.PotentialLine1.Stat = EPotentialStat::PhysAtkPercent;
	Item.PotentialLine1.Value = 0.10f;
	Item.PotentialLine2.Stat = EPotentialStat::HpPercent;
	Item.PotentialLine2.Value = 0.08f;
	Item.PotentialLine3.Stat = EPotentialStat::CritRatePercent;
	Item.PotentialLine3.Value = 0.02f;
	Inventory->AddItem(Item);

	const FDerivedStats Bonus = Inventory->ComputeEquipmentBonus();
	TestEqual(TEXT("Potential physical attack percent applies after enhance"), Bonus.PhysAtk, 132.0f);
	TestEqual(TEXT("Potential HP percent applies after enhance"), Bonus.Hp, 64.8f);
	TestEqual(TEXT("Potential crit rate percent adds flat derived crit"), Bonus.CritRate, 0.02f);
	TestEqual(TEXT("Potential contributes to PowerScore"), FItemPowerScore::Compute(Item), 158);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryLockedAutoEquipTest,
	"IdleProject.Inventory.AutoEquip.LockedItem",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInventoryLockedAutoEquipTest::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inventory = NewObject<UInventoryComponent>();
	Inventory->AddItem(MakeTestItem(TEXT("locked_sword"), EItemSlot::Weapon, EItemRarity::Rare, 10.0f, 0.0f, 0.0f));
	TestTrue(TEXT("Equipped item can be locked"), Inventory->SetItemLocked(EItemSlot::Weapon, true));

	Inventory->AddItem(MakeTestItem(TEXT("better_sword"), EItemSlot::Weapon, EItemRarity::Rare, 100.0f, 0.0f, 0.0f));
	const FItemInstance* Equipped = Inventory->GetEquippedItem(EItemSlot::Weapon);
	TestNotNull(TEXT("Weapon remains equipped"), Equipped);
	TestEqual(TEXT("Locked item blocks auto-equip replacement"), Equipped ? Equipped->ItemId : NAME_None, FName(TEXT("locked_sword")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryUnequipReducesBonusTest,
	"IdleProject.Inventory.Bonus.Unequip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInventoryUnequipReducesBonusTest::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inventory = NewObject<UInventoryComponent>();
	Inventory->AddItem(MakeTestItem(TEXT("rare_sword"), EItemSlot::Weapon, EItemRarity::Rare, 12.0f, 0.0f, 0.0f));
	Inventory->AddItem(MakeTestItem(TEXT("common_helmet"), EItemSlot::Helmet, EItemRarity::Common, 0.0f, 4.0f, 20.0f));

	Inventory->UnequipSlot(EItemSlot::Helmet);
	const FDerivedStats Bonus = Inventory->ComputeEquipmentBonus();
	TestEqual(TEXT("무기 공격력 유지"), Bonus.PhysAtk, 12.0f);
	TestEqual(TEXT("투구 방어력 제거"), Bonus.PhysDef, 0.0f);
	TestEqual(TEXT("투구 체력 제거"), Bonus.Hp, 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FItemFactoryGuaranteedDropForLevelTest,
	"IdleProject.Inventory.ItemFactory.GuaranteedDropForLevel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FItemFactoryGuaranteedDropForLevelTest::RunTest(const FString& Parameters)
{
	for (int32 Index = 0; Index < 100; ++Index)
	{
		const FItemInstance Drop = FItemFactory::GuaranteedDropForLevel(1);
		TestTrue(TEXT("Guaranteed drop never returns None rarity"), Drop.Rarity >= EItemRarity::Common);
		TestTrue(TEXT("Guaranteed drop has an equipment slot"), Drop.Slot >= EItemSlot::Weapon && Drop.Slot <= EItemSlot::Accessory);
		TestTrue(TEXT("Guaranteed drop has stats"), FItemPowerScore::Compute(Drop) > 0);
	}

	const FItemInstance LowLevelDrop = FItemFactory::GuaranteedDropForLevel(-20);
	const FItemInstance HighLevelDrop = FItemFactory::GuaranteedDropForLevel(20);
	TestTrue(TEXT("Guaranteed drop clamps invalid levels to playable stats"), FItemPowerScore::Compute(LowLevelDrop) > 0);
	TestTrue(TEXT("Guaranteed high-level drop clamps to playable stats"), FItemPowerScore::Compute(HighLevelDrop) > 0);

	const FItemInstance LowFormulaItem = FDropFormula::ComputeItemBonus(EItemSlot::Weapon, -20, EItemRarity::Common, 1.0f);
	const FItemInstance HighFormulaItem = FDropFormula::ComputeItemBonus(EItemSlot::Weapon, 20, EItemRarity::Common, 1.0f);
	TestTrue(TEXT("Level affects deterministic item stats"), FItemPowerScore::Compute(HighFormulaItem) > FItemPowerScore::Compute(LowFormulaItem));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FItemFactoryRandomDropRangeTest,
	"IdleProject.Inventory.ItemFactory.RandomDropRange",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FItemFactoryRandomDropRangeTest::RunTest(const FString& Parameters)
{
	for (int32 Index = 0; Index < 100; ++Index)
	{
		const FItemInstance Drop = FItemFactory::RandomDropFromMonster(1);
		TestTrue(TEXT("등급 범위"), Drop.Rarity >= EItemRarity::None && Drop.Rarity <= EItemRarity::Mythic);

		if (Drop.Rarity == EItemRarity::None)
		{
			TestEqual(TEXT("드롭 없음 슬롯"), static_cast<int32>(Drop.Slot), static_cast<int32>(EItemSlot::None));
			continue;
		}

		TestTrue(TEXT("슬롯 범위"), Drop.Slot >= EItemSlot::Weapon && Drop.Slot <= EItemSlot::Accessory);
		TestTrue(TEXT("보너스 중 하나 이상"), Drop.BonusAtk > 0.0f || Drop.BonusDef > 0.0f || Drop.BonusHp > 0.0f);
		TestEqual(TEXT("드롭 강화 기본값"), Drop.EnhanceLevel, 0);
		TestAffixCountForRarity(*this, TEXT("RandomDropFromMonster"), Drop);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FItemFactoryHighLevelDropIncludesExpandedRarityTest,
	"IdleProject.Inventory.ItemFactory.HighLevelExpandedRarity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FItemFactoryHighLevelDropIncludesExpandedRarityTest::RunTest(const FString& Parameters)
{
	bool bFoundEpicOrHigher = false;
	for (int32 Index = 0; Index < 2000; ++Index)
	{
		const FItemInstance Drop = FItemFactory::RandomDropFromMonster(100);
		if (Drop.Rarity >= EItemRarity::Epic)
		{
			bFoundEpicOrHigher = true;
			TestTrue(TEXT("Epic+ drop carries scaled stats"), FItemPowerScore::Compute(Drop) > 0);
			TestAffixCountForRarity(*this, TEXT("High-level random drop"), Drop);
			break;
		}
	}

	TestTrue(TEXT("High-level monster drops can produce Epic, Legendary, or Mythic items"), bFoundEpicOrHigher);

	return true;
}

#endif
