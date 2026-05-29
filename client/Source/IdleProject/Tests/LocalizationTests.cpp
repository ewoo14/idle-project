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
				TEXT("DUNGEON_PANEL_TITLE"),
				TEXT("DUNGEON_GOLD"),
				TEXT("DUNGEON_EXP"),
				TEXT("DUNGEON_ESSENCE"),
				TEXT("DUNGEON_ENTRIES_FORMAT"),
				TEXT("DUNGEON_TIER_FORMAT"),
				TEXT("DUNGEON_CP_FORMAT"),
				TEXT("DUNGEON_NEXT_TIER_CP_FORMAT"),
				TEXT("DUNGEON_REWARD_FORMAT"),
				TEXT("DUNGEON_ENTER"),
				TEXT("DUNGEON_STATUS_SOLD_OUT"),
				TEXT("DUNGEON_STATUS_NEED_CP"),
				TEXT("CONSUMABLE_PANEL_TITLE"),
				TEXT("CONSUMABLE_ACTIVE_BUFF_TITLE"),
				TEXT("CONSUMABLE_ACTIVE_BUFF_EMPTY"),
				TEXT("CONSUMABLE_COUNT_FORMAT"),
				TEXT("CONSUMABLE_GRADE_LESSER"),
				TEXT("CONSUMABLE_GRADE_STANDARD"),
				TEXT("CONSUMABLE_GRADE_GREATER"),
				TEXT("CONSUMABLE_NAME_WITH_GRADE_FORMAT"),
				TEXT("ACTION_USE"),
				TEXT("CONSUMABLE_ATTACK_TONIC_NAME"),
				TEXT("CONSUMABLE_GUARD_TONIC_NAME"),
				TEXT("CONSUMABLE_ALL_STAT_ELIXIR_NAME"),
				TEXT("CONSUMABLE_FORTUNE_SCROLL_NAME"),
				TEXT("CONSUMABLE_GOLD_FEAST_NAME"),
				TEXT("CONSUMABLE_WISDOM_BOOSTER_NAME"),
				TEXT("CONSUMABLE_ATTACK_TONIC_EFFECT"),
				TEXT("CONSUMABLE_GUARD_TONIC_EFFECT"),
				TEXT("CONSUMABLE_ALL_STAT_ELIXIR_EFFECT"),
				TEXT("CONSUMABLE_FORTUNE_SCROLL_EFFECT"),
				TEXT("CONSUMABLE_GOLD_FEAST_EFFECT"),
				TEXT("CONSUMABLE_WISDOM_BOOSTER_EFFECT"),
				TEXT("STAGE_INDICATOR_TITLE"),
				TEXT("STAGE_CHAPTER_FORMAT"),
				TEXT("STAGE_PROGRESS_FORMAT"),
				TEXT("STAGE_BOSS_BADGE"),
				TEXT("STAGE_ELITE_BADGE"),
				TEXT("STAGE_WEAKNESS_FORMAT"),
				TEXT("STAGE_CHAPTER_ENTRY_FORMAT"),
				TEXT("STAGE_CHAPTER_CLEAR_FORMAT"),
				TEXT("ELEMENT_DARK"),
				TEXT("PET_NAME_DOG"),
				TEXT("PET_NAME_BIRD"),
				TEXT("PET_NAME_CAT"),
				TEXT("PET_NAME_WOLF"),
				TEXT("PET_NAME_OWL"),
				TEXT("PET_NAME_BEAR"),
				TEXT("PET_NAME_TURTLE"),
				TEXT("PET_NAME_FOX"),
				TEXT("PET_NAME_RABBIT"),
				TEXT("PET_NAME_DRAGON"),
				TEXT("PET_BONUS_EXP_FORMAT"),
				TEXT("PET_BONUS_PHYS_ATK_FORMAT"),
				TEXT("PET_BONUS_MAGIC_ATK_FORMAT"),
				TEXT("PET_BONUS_HP_FORMAT"),
				TEXT("PET_BONUS_DEF_FORMAT"),
				TEXT("PET_BONUS_ALL_STAT_FORMAT"),
				TEXT("PET_STATUS_LOCKED"),
				TEXT("MASTERY_PANEL_TITLE"),
				TEXT("MASTERY_WORLD_POWER_FORMAT"),
				TEXT("MASTERY_TRACK_COMBAT"),
				TEXT("MASTERY_TRACK_EQUIPMENT"),
				TEXT("MASTERY_TRACK_ABYSS"),
				TEXT("MASTERY_TRACK_RUNE"),
				TEXT("MASTERY_TRACK_BEAST"),
				TEXT("MASTERY_TRACK_EXPLORE"),
				TEXT("MASTERY_TRACK_LEVEL_FORMAT"),
				TEXT("MASTERY_TRACK_XP_FORMAT"),
				TEXT("MASTERY_TRACK_PROGRESS_FORMAT"),
				TEXT("MASTERY_BONUS_CORE_FORMAT"),
				TEXT("MASTERY_BONUS_CRIT_FORMAT"),
				TEXT("MASTERY_BONUS_DROP_FORMAT"),
				TEXT("MASTERY_BONUS_GOLD_FORMAT"),
				TEXT("MASTERY_BONUS_EXP_FORMAT"),
				TEXT("MASTERY_TOOLTIP_LOCKED"),
				TEXT("MASTERY_TOOLTIP_WORLD_POWER"),
				TEXT("MASTERY_LOCAL_BONUS_COMBAT_FORMAT"),
				TEXT("MASTERY_LOCAL_BONUS_EQUIPMENT_FORMAT"),
				TEXT("MASTERY_LOCAL_BONUS_ABYSS_FORMAT"),
				TEXT("MASTERY_LOCAL_BONUS_RUNE_FORMAT"),
				TEXT("MASTERY_LOCAL_BONUS_BEAST_FORMAT"),
				TEXT("MASTERY_LOCAL_BONUS_EXPLORE_FORMAT"),
				TEXT("MASTERY_LOCAL_BONUS_COMBAT_TOOLTIP"),
				TEXT("MASTERY_LOCAL_BONUS_EQUIPMENT_TOOLTIP"),
				TEXT("MASTERY_LOCAL_BONUS_ABYSS_TOOLTIP"),
				TEXT("MASTERY_LOCAL_BONUS_RUNE_TOOLTIP"),
				TEXT("MASTERY_LOCAL_BONUS_BEAST_TOOLTIP"),
				TEXT("MASTERY_LOCAL_BONUS_EXPLORE_TOOLTIP"),
				TEXT("MASTERY_LOCAL_BONUS2_COMBAT_FORMAT"),
				TEXT("MASTERY_LOCAL_BONUS2_EQUIPMENT_FORMAT"),
				TEXT("MASTERY_LOCAL_BONUS2_ABYSS_FORMAT"),
				TEXT("MASTERY_LOCAL_BONUS2_RUNE_FORMAT"),
				TEXT("MASTERY_LOCAL_BONUS2_BEAST_FORMAT"),
				TEXT("MASTERY_LOCAL_BONUS2_EXPLORE_FORMAT"),
				TEXT("LEADERBOARD_PANEL_TITLE"),
				TEXT("LEADERBOARD_TAB_POWER"),
				TEXT("LEADERBOARD_TAB_REBIRTH"),
				TEXT("LEADERBOARD_TAB_WEEKLY"),
				TEXT("LEADERBOARD_WEEK_FORMAT"),
				TEXT("LEADERBOARD_SEASON_FORMAT"),
				TEXT("LEADERBOARD_MY_RANK"),
				TEXT("LEADERBOARD_SCORE_FORMAT"),
				TEXT("LEADERBOARD_EMPTY"),
				TEXT("LEADERBOARD_LOADING"),
				TEXT("LEADERBOARD_OFFLINE"),
				TEXT("ACTION_REFRESH"),
				TEXT("WEEKLY_BOSS_PANEL_TITLE"),
				TEXT("WEEKLY_BOSS_WEEK_FORMAT"),
				TEXT("WEEKLY_BOSS_DAMAGE_FORMAT"),
				TEXT("WEEKLY_BOSS_REMAINING_FORMAT"),
				TEXT("WEEKLY_BOSS_MILESTONE_SUMMARY_FORMAT"),
				TEXT("WEEKLY_BOSS_RESET_WEEKLY"),
				TEXT("WEEKLY_BOSS_CHALLENGE"),
				TEXT("WEEKLY_BOSS_CLAIM"),
				TEXT("WEEKLY_BOSS_MILESTONE_FORMAT"),
				TEXT("WEEKLY_BOSS_THRESHOLD_FORMAT"),
				TEXT("WEEKLY_BOSS_REWARD_FORMAT"),
				TEXT("WEEKLY_BOSS_STATUS_CLAIMED"),
				TEXT("WEEKLY_BOSS_STATUS_REACHED"),
				TEXT("WEEKLY_BOSS_STATUS_LOCKED"),
				TEXT("WEEKLY_BOSS_CHALLENGE_RESULT_FORMAT"),
				TEXT("WEEKLY_BOSS_CHALLENGE_BLOCKED"),
				TEXT("WEEKLY_BOSS_CLAIM_RESULT_FORMAT"),
				TEXT("WEEKLY_BOSS_CLAIM_BLOCKED"),
			};

			for (const FString& RequiredKey : RequiredCloudSyncKeys)
			{
				TestTrue(*FString::Printf(TEXT("UI Korean contains required cloud sync key %s"), *RequiredKey), KoreanKeys.Contains(RequiredKey));
				TestTrue(*FString::Printf(TEXT("UI English contains required cloud sync key %s"), *RequiredKey), EnglishKeys.Contains(RequiredKey));
			}
		}

		if (TableName == TEXT("Story"))
		{
			const TArray<FString> RequiredChapter5StoryKeys = {
				TEXT("STORY_CHAPTER_05_TITLE"),
				TEXT("STORY_CHAPTER_05_SUMMARY"),
				TEXT("STORY_MAP_C05_M01_NAME"),
				TEXT("STORY_MAP_C05_M01_SUMMARY"),
				TEXT("STORY_MAP_C05_M05_NAME"),
				TEXT("STORY_MAP_C05_M05_SUMMARY"),
				TEXT("STORY_MAP_C05_M10_NAME"),
				TEXT("STORY_MAP_C05_M10_SUMMARY"),
				TEXT("STORY_ELITE_C05_VOID_HERALD_NAME"),
				TEXT("STORY_ELITE_C05_VOID_HERALD_SUMMARY"),
				TEXT("STORY_BOSS_C05_VERKAS_TRUE_NAME"),
				TEXT("STORY_BOSS_C05_VERKAS_TRUE_SUMMARY"),
				TEXT("STORY_VOIDHEART_SUMMARY"),
			};

			for (const FString& RequiredKey : RequiredChapter5StoryKeys)
			{
				TestTrue(*FString::Printf(TEXT("Story Korean contains required chapter 5 key %s"), *RequiredKey), KoreanKeys.Contains(RequiredKey));
				TestTrue(*FString::Printf(TEXT("Story English contains required chapter 5 key %s"), *RequiredKey), EnglishKeys.Contains(RequiredKey));
			}
		}

		if (TableName == TEXT("StoryText"))
		{
			const TArray<FString> RequiredChapter5StoryTextKeys = {
				TEXT("STORY_C05_M01_INTRO_01"),
				TEXT("STORY_C05_M01_CLEAR_01"),
				TEXT("STORY_C05_M05_ELITE_01"),
				TEXT("STORY_C05_M05_ELITE_02"),
				TEXT("STORY_C05_M05_CLEAR_01"),
				TEXT("STORY_C05_M10_BOSS_01"),
				TEXT("STORY_C05_M10_BOSS_02"),
				TEXT("STORY_C05_M10_CLEAR_01"),
				TEXT("STORY_C05_M10_CLEAR_02"),
			};

			for (const FString& RequiredKey : RequiredChapter5StoryTextKeys)
			{
				TestTrue(*FString::Printf(TEXT("StoryText Korean contains required chapter 5 key %s"), *RequiredKey), KoreanKeys.Contains(RequiredKey));
				TestTrue(*FString::Printf(TEXT("StoryText English contains required chapter 5 key %s"), *RequiredKey), EnglishKeys.Contains(RequiredKey));
			}
		}
	}

	return true;
}

#endif
