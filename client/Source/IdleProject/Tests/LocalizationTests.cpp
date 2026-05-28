#include "Misc/AutomationTest.h"

#include "Internationalization/IdleLocalization.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleLocalizationLookupTest,
	"IdleProject.Localization.LookupAndCultureSwitch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleLocalizationLookupTest::RunTest(const FString& Parameters)
{
	FFormatNamedArguments GoldArgs;
	GoldArgs.Add(TEXT("Amount"), FText::AsNumber(1200));

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	TestEqual(TEXT("Korean UI lookup returns approved copy"),
		IdleProject::Localization::UI(TEXT("HUD_GOLD_FORMAT"), GoldArgs).ToString(),
		FString(TEXT("골드 1,200")));

	IdleProject::Localization::SetLanguageForTests(TEXT("en"));
	TestEqual(TEXT("English UI lookup returns approved copy"),
		IdleProject::Localization::UI(TEXT("HUD_GOLD_FORMAT"), GoldArgs).ToString(),
		FString(TEXT("Gold 1,200")));

	FFormatNamedArguments CombatPowerArgs;
	CombatPowerArgs.Add(TEXT("Amount"), FText::FromString(TEXT("1,234,567")));
	TestEqual(TEXT("English combat power label uses approved copy"),
		IdleProject::Localization::UI(TEXT("HUD_COMBAT_POWER_FORMAT"), CombatPowerArgs).ToString(),
		FString(TEXT("Combat Power 1,234,567")));

	TestEqual(TEXT("English rarity lookup includes Legendary"),
		IdleProject::Localization::UI(TEXT("RARITY_LEGENDARY")).ToString(),
		FString(TEXT("Legendary")));
	TestEqual(TEXT("English rarity lookup includes Mythic"),
		IdleProject::Localization::UI(TEXT("RARITY_MYTHIC")).ToString(),
		FString(TEXT("Mythic")));
	TestEqual(TEXT("English dark element lookup uses approved copy"),
		IdleProject::Localization::UI(TEXT("ELEMENT_DARK")).ToString(),
		FString(TEXT("Dark")));
	TestEqual(TEXT("English elite stage badge lookup uses approved copy"),
		IdleProject::Localization::UI(TEXT("STAGE_ELITE_BADGE")).ToString(),
		FString(TEXT("Elite")));

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	TestEqual(TEXT("Korean rarity lookup includes Legendary"),
		IdleProject::Localization::UI(TEXT("RARITY_LEGENDARY")).ToString(),
		FString(TEXT("전설")));

	TestEqual(TEXT("Korean rarity lookup includes Mythic"),
		IdleProject::Localization::UI(TEXT("RARITY_MYTHIC")).ToString(),
		FString(TEXT("신화")));

	TestEqual(TEXT("Korean dark element lookup uses approved copy"),
		IdleProject::Localization::UI(TEXT("ELEMENT_DARK")).ToString(),
		FString(TEXT("암흑")));
	TestEqual(TEXT("Korean elite stage badge lookup uses approved copy"),
		IdleProject::Localization::UI(TEXT("STAGE_ELITE_BADGE")).ToString(),
		FString(TEXT("정예")));

	TestEqual(TEXT("Unsupported language falls back to Korean"),
		IdleProject::Localization::NormalizeLanguage(TEXT("ja")),
		FString(TEXT("ko")));

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleLocalizationCsvIntegrityTest,
	"IdleProject.Localization.CsvIntegrity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleLocalizationCsvIntegrityTest::RunTest(const FString& Parameters)
{
	const TArray<FName> TableNames = {
		TEXT("UI"),
		TEXT("Story"),
		TEXT("StoryText"),
		TEXT("Skill"),
		TEXT("Quest"),
		TEXT("Achievement"),
		TEXT("Rune"),
	};

	for (const FName TableName : TableNames)
	{
		TSet<FString> KoreanKeys;
		TSet<FString> EnglishKeys;
		TArray<FString> KoreanErrors;
		TArray<FString> EnglishErrors;

		const bool bLoadedKorean = IdleProject::Localization::LoadTableKeysForTests(TEXT("ko"), TableName, KoreanKeys, KoreanErrors);
		const bool bLoadedEnglish = IdleProject::Localization::LoadTableKeysForTests(TEXT("en"), TableName, EnglishKeys, EnglishErrors);
		TestTrue(*FString::Printf(TEXT("%s Korean CSV loads"), *TableName.ToString()), bLoadedKorean);
		TestTrue(*FString::Printf(TEXT("%s English CSV loads"), *TableName.ToString()), bLoadedEnglish);
		TestEqual(*FString::Printf(TEXT("%s Korean CSV has no empty or duplicate rows"), *TableName.ToString()), KoreanErrors.Num(), 0);
		TestEqual(*FString::Printf(TEXT("%s English CSV has no empty or duplicate rows"), *TableName.ToString()), EnglishErrors.Num(), 0);
		TestEqual(*FString::Printf(TEXT("%s ko/en key count matches"), *TableName.ToString()), KoreanKeys.Num(), EnglishKeys.Num());

		for (const FString& KoreanKey : KoreanKeys)
		{
			TestTrue(*FString::Printf(TEXT("%s English contains key %s"), *TableName.ToString(), *KoreanKey), EnglishKeys.Contains(KoreanKey));
		}
		for (const FString& EnglishKey : EnglishKeys)
		{
			TestTrue(*FString::Printf(TEXT("%s Korean contains key %s"), *TableName.ToString(), *EnglishKey), KoreanKeys.Contains(EnglishKey));
		}

		if (TableName == TEXT("UI"))
		{
			const TArray<FString> RequiredCloudSyncKeys = {
				TEXT("CLOUD_SYNC_SYNCING"),
				TEXT("CLOUD_SYNC_SYNCED"),
				TEXT("CLOUD_SYNC_OFFLINE"),
				TEXT("CLOUD_SYNC_CONFLICT"),
				TEXT("UNIQUE_TRAIT_ALL_STAT_SURGE"),
				TEXT("UNIQUE_TRAIT_CRIT_DAMAGE_SURGE"),
				TEXT("UNIQUE_TRAIT_CRIT_RATE_SURGE"),
				TEXT("UNIQUE_TRAIT_LIFE_SURGE"),
				TEXT("UNIQUE_TRAIT_SWIFT_SURGE"),
				TEXT("UNIQUE_TRAIT_PHYS_MASTERY"),
				TEXT("UNIQUE_TRAIT_MAGIC_MASTERY"),
				TEXT("UNIQUE_TRAIT_GUARD_MASTERY"),
				TEXT("UNIQUE_TRAIT_PERCENT_FORMAT"),
				TEXT("UNIQUE_TRAIT_FLAT_FORMAT"),
			};

			for (const FString& RequiredKey : RequiredCloudSyncKeys)
			{
				TestTrue(*FString::Printf(TEXT("UI Korean contains required cloud sync key %s"), *RequiredKey), KoreanKeys.Contains(RequiredKey));
				TestTrue(*FString::Printf(TEXT("UI English contains required cloud sync key %s"), *RequiredKey), EnglishKeys.Contains(RequiredKey));
			}
		}
	}

	return true;
}

#endif
