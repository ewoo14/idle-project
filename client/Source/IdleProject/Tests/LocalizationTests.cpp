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
				TEXT("ACTION_EVOLVE"),
				TEXT("PET_STAR_FORMAT"),
				TEXT("PET_EVOLVE_COST_FORMAT"),
				TEXT("PET_EVOLVE_EFFECT_FORMAT"),
				TEXT("PET_EVOLVE_FEEDBACK_SUCCESS_FORMAT"),
				TEXT("PET_EVOLVE_FEEDBACK_BLOCKED"),
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
				TEXT("ATTENDANCE_PANEL_TITLE"),
				TEXT("ATTENDANCE_TOTAL_FORMAT"),
				TEXT("ATTENDANCE_CHECKIN_DONE"),
				TEXT("ATTENDANCE_CHECKIN_AVAILABLE"),
				TEXT("ATTENDANCE_MILESTONE_SUMMARY_FORMAT"),
				TEXT("ATTENDANCE_CLAIM"),
				TEXT("ATTENDANCE_MILESTONE_FORMAT"),
				TEXT("ATTENDANCE_THRESHOLD_FORMAT"),
				TEXT("ATTENDANCE_REWARD_FORMAT"),
				TEXT("ATTENDANCE_REWARD_GOLD"),
				TEXT("ATTENDANCE_REWARD_ESSENCE"),
				TEXT("ATTENDANCE_REWARD_CONSUMABLE"),
				TEXT("ATTENDANCE_STATUS_CLAIMED"),
				TEXT("ATTENDANCE_STATUS_REACHED"),
				TEXT("ATTENDANCE_STATUS_PROGRESS_FORMAT"),
				TEXT("ATTENDANCE_CLAIM_RESULT_FORMAT"),
				TEXT("ATTENDANCE_CLAIM_BLOCKED"),
				TEXT("TREASURE_PANEL_TITLE"),
				TEXT("TREASURE_STATUS_AVAILABLE"),
				TEXT("TREASURE_STATUS_DONE"),
				TEXT("TREASURE_DRAW"),
				TEXT("TREASURE_TOTAL_DRAWS_FORMAT"),
				TEXT("TREASURE_REWARD_GOLD"),
				TEXT("TREASURE_REWARD_ESSENCE"),
				TEXT("TREASURE_REWARD_CONSUMABLE"),
				TEXT("TREASURE_REWARD_PROTECTION_SCROLL"),
				TEXT("TREASURE_REWARD_RESET_CUBE"),
				TEXT("TREASURE_REWARD_RANK_CUBE"),
				TEXT("TREASURE_DRAW_RESULT_FORMAT"),
				TEXT("TREASURE_DRAW_BLOCKED"),
				TEXT("GUILD_PANEL_TITLE"),
				TEXT("GUILD_REFRESH"),
				TEXT("GUILD_OFFLINE"),
				TEXT("GUILD_LOADING"),
				TEXT("GUILD_NONE_TITLE"),
				TEXT("GUILD_LIST_EMPTY"),
				TEXT("GUILD_JOIN"),
				TEXT("GUILD_JOIN_REQUEST"),
				TEXT("GUILD_LIST_ROW_FORMAT"),
				TEXT("GUILD_CREATE_TITLE"),
				TEXT("GUILD_CREATE"),
				TEXT("GUILD_CREATE_NAME_CYCLE"),
				TEXT("GUILD_CREATE_NAME_FORMAT"),
				TEXT("GUILD_PRESET_NAME_1"),
				TEXT("GUILD_PRESET_NAME_2"),
				TEXT("GUILD_PRESET_NAME_3"),
				TEXT("GUILD_PRESET_NAME_4"),
				TEXT("GUILD_PRESET_NAME_5"),
				TEXT("GUILD_PRESET_NAME_6"),
				TEXT("GUILD_PRESET_NAME_7"),
				TEXT("GUILD_PRESET_NAME_8"),
				TEXT("GUILD_MY_TITLE"),
				TEXT("GUILD_SUMMARY_FORMAT"),
				TEXT("GUILD_JOINMODE_OPEN"),
				TEXT("GUILD_JOINMODE_APPROVAL"),
				TEXT("GUILD_LEAVE"),
				TEXT("GUILD_MEMBER_LIST_TITLE"),
				TEXT("GUILD_RANK_MASTER"),
				TEXT("GUILD_RANK_VICE"),
				TEXT("GUILD_RANK_OFFICER"),
				TEXT("GUILD_RANK_MEMBER"),
				TEXT("GUILD_MANAGE_TITLE"),
				TEXT("GUILD_SETTINGS_TOGGLE_JOINMODE"),
				TEXT("GUILD_REQUESTS_TITLE"),
				TEXT("GUILD_REQUESTS_EMPTY"),
				TEXT("GUILD_APPROVE"),
				TEXT("GUILD_REJECT"),
				TEXT("GUILD_PROMOTE"),
				TEXT("GUILD_DEMOTE"),
				TEXT("GUILD_RANK_LOCKED"),
				TEXT("GUILD_RANK_FULL"),
				TEXT("GUILD_FEEDBACK_CREATED"),
				TEXT("GUILD_FEEDBACK_JOINED"),
				TEXT("GUILD_FEEDBACK_REQUESTED"),
				TEXT("GUILD_FEEDBACK_LEFT"),
				TEXT("GUILD_FEEDBACK_UPDATED"),
				TEXT("GUILD_FEEDBACK_APPROVED"),
				TEXT("GUILD_FEEDBACK_REJECTED"),
				TEXT("GUILD_FEEDBACK_RANK_CHANGED"),
				TEXT("GUILD_FEEDBACK_FAILED"),
				TEXT("GUILD_LEVEL_FORMAT"),
				TEXT("GUILD_EXP_FORMAT"),
				TEXT("GUILD_BUFF_FORMAT"),
				TEXT("GUILD_CONTRIBUTION_FORMAT"),
				TEXT("GUILD_WEEKLY_CONTRIBUTION_FORMAT"),
				TEXT("GUILD_ATTEND"),
				TEXT("GUILD_ATTEND_DONE"),
				TEXT("GUILD_DONATE_FORMAT"),
				TEXT("GUILD_DONATE_CYCLE"),
				TEXT("GUILD_SHOP_TITLE"),
				TEXT("GUILD_SHOP_EMPTY"),
				TEXT("GUILD_SHOP_PRICE_FORMAT"),
				TEXT("GUILD_SHOP_BUY"),
				TEXT("GUILD_SHOP_INSUFFICIENT"),
				TEXT("GUILD_SHOP_ITEM_PROTECTION_SCROLL"),
				TEXT("GUILD_SHOP_ITEM_RESET_CUBE"),
				TEXT("GUILD_SHOP_ITEM_GOLD_POUCH"),
				TEXT("GUILD_SHOP_ITEM_EXP_POTION"),
				TEXT("GUILD_SHOP_ITEM_RANK_CUBE"),
				TEXT("GUILD_SHOP_ITEM_ESSENCE"),
				TEXT("GUILD_FEEDBACK_ATTENDED"),
				TEXT("GUILD_FEEDBACK_DONATED"),
				TEXT("GUILD_FEEDBACK_PURCHASED"),
				TEXT("GUILD_BOSS_TITLE"),
				TEXT("GUILD_BOSS_HP_FORMAT"),
				TEXT("GUILD_BOSS_DEFEATED_FORMAT"),
				TEXT("GUILD_BOSS_CHALLENGE_FORMAT"),
				TEXT("GUILD_BOSS_CLAIM_FORMAT"),
				TEXT("GUILD_TAB_MY"),
				TEXT("GUILD_TAB_RANKINGS"),
				TEXT("GUILD_RANKINGS_TITLE"),
				TEXT("GUILD_RANKINGS_EMPTY"),
				TEXT("GUILD_RANKINGS_LOADING"),
				TEXT("GUILD_RANKINGS_MY_TITLE"),
				TEXT("GUILD_RANKINGS_MY_FORMAT"),
				TEXT("GUILD_RANKINGS_MY_UNRANKED"),
				TEXT("GUILD_RANKINGS_RANK_FORMAT"),
				TEXT("GUILD_RANKINGS_INFO_FORMAT"),
				TEXT("GUILD_FEEDBACK_BOSS_CHALLENGED"),
				TEXT("GUILD_FEEDBACK_BOSS_CLAIMED"),
				TEXT("TITLE_PANEL_TITLE"),
				TEXT("TITLE_EQUIPPED_NONE"),
				TEXT("TITLE_EQUIPPED_FORMAT"),
				TEXT("TITLE_STATUS_LOCKED"),
				TEXT("TITLE_STATUS_UNLOCKED"),
				TEXT("TITLE_ACTION_UNEQUIP"),
				TEXT("TITLE_UNLOCK_CONDITION_FORMAT"),
				TEXT("TITLE_PROGRESS_FORMAT"),
				TEXT("TITLE_BONUS_ALL_STAT_FORMAT"),
				TEXT("TITLE_BONUS_GOLD_FORMAT"),
				TEXT("TITLE_BONUS_EXP_FORMAT"),
				TEXT("TITLE_BONUS_CRIT_DMG_FORMAT"),
				TEXT("TITLE_METRIC_MONSTERS_KILLED"),
				TEXT("TITLE_METRIC_BOSSES_KILLED"),
				TEXT("TITLE_METRIC_HIGHEST_LEVEL"),
				TEXT("TITLE_METRIC_STAGES_CLEARED"),
				TEXT("TITLE_METRIC_REBIRTH_COUNT"),
				TEXT("TITLE_METRIC_TRANSCEND_COUNT"),
				TEXT("TITLE_METRIC_TOWER_FLOOR"),
				TEXT("TITLE_METRIC_HIGHEST_ENHANCE"),
				TEXT("TITLE_METRIC_ITEMS_COLLECTED"),
				TEXT("TITLE_METRIC_UNIQUE_ITEMS"),
				TEXT("TITLE_METRIC_GOLD_EARNED"),
				TEXT("TITLE_METRIC_HIGHEST_PET_LEVEL"),
				TEXT("TITLE_METRIC_QUESTS_COMPLETED"),
				TEXT("TITLE_METRIC_DAYS_PLAYED"),
				TEXT("TITLE_NAME_MONSTER_HUNTER"),
				TEXT("TITLE_NAME_BOSS_SLAYER"),
				TEXT("TITLE_NAME_MONSTER_ANNIHILATOR"),
				TEXT("TITLE_NAME_BOSS_EXECUTIONER"),
				TEXT("TITLE_NAME_REBIRTH_MASTER"),
				TEXT("TITLE_NAME_TRANSCENDENT"),
				TEXT("TITLE_NAME_STAGE_CONQUEROR"),
				TEXT("TITLE_NAME_LEVEL_LEGEND"),
				TEXT("TITLE_NAME_TOWER_CONQUEROR"),
				TEXT("TITLE_NAME_TOWER_OVERLORD"),
				TEXT("TITLE_NAME_COLLECTOR"),
				TEXT("TITLE_NAME_UNIQUE_SEEKER"),
				TEXT("TITLE_NAME_GOLD_KING"),
				TEXT("TITLE_NAME_PET_WHISPERER"),
				TEXT("TITLE_NAME_QUEST_CHAMPION"),
				TEXT("TITLE_NAME_ENHANCE_ARTISAN"),
				TEXT("TITLE_NAME_ENHANCE_GRANDMASTER"),
				TEXT("TITLE_NAME_VETERAN"),
				TEXT("MISSION_PANEL_TITLE"),
				TEXT("MISSION_TAB_DAILY"),
				TEXT("MISSION_TAB_WEEKLY"),
				TEXT("MISSION_EMPTY"),
				TEXT("MISSION_OBJECTIVE_FORMAT"),
				TEXT("MISSION_PROGRESS_FORMAT"),
				TEXT("MISSION_ACTION_CLAIM"),
				TEXT("MISSION_ACTION_CLAIMED"),
				TEXT("MISSION_METRIC_MONSTERS_KILLED"),
				TEXT("MISSION_METRIC_BOSSES_KILLED"),
				TEXT("MISSION_METRIC_STAGES_CLEARED"),
				TEXT("MISSION_METRIC_DUNGEON_RUNS"),
				TEXT("MISSION_METRIC_GEAR_ENHANCED"),
				TEXT("MISSION_METRIC_GOLD_EARNED"),
				TEXT("MISSION_REWARD_GOLD_FORMAT"),
				TEXT("MISSION_REWARD_ESSENCE_FORMAT"),
				TEXT("MISSION_REWARD_CONSUMABLE_FORMAT"),
				TEXT("POTENTIAL_STAT_ALL_STAT"),
				TEXT("POTENTIAL_STAT_GOLD_FIND"),
				TEXT("POTENTIAL_STAT_DROP_RATE"),
				TEXT("RARITY_TRANSCENDENT"),
				TEXT("REBIRTH_PERK_PANEL_TITLE"),
				TEXT("REBIRTH_PERK_POINTS_FORMAT"),
				TEXT("REBIRTH_PERK_DETAIL_FORMAT"),
				TEXT("REBIRTH_PERK_RESET"),
				TEXT("REBIRTH_PERK_NAME_GOLD"),
				TEXT("REBIRTH_PERK_NAME_DROP"),
				TEXT("REBIRTH_PERK_NAME_CRIT_DMG"),
				TEXT("REBIRTH_PERK_NAME_ALL_STAT"),
				TEXT("REBIRTH_PERK_NAME_EXP"),
				TEXT("REBIRTH_PERK_NAME_OFFLINE"),
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

			const TArray<FString> RequiredChapter6StoryKeys = {
				TEXT("STORY_CHAPTER_06_TITLE"),
				TEXT("STORY_CHAPTER_06_SUMMARY"),
				TEXT("STORY_MAP_C06_M01_NAME"),
				TEXT("STORY_MAP_C06_M01_SUMMARY"),
				TEXT("STORY_MAP_C06_M02_NAME"),
				TEXT("STORY_MAP_C06_M02_SUMMARY"),
				TEXT("STORY_MAP_C06_M03_NAME"),
				TEXT("STORY_MAP_C06_M03_SUMMARY"),
				TEXT("STORY_MAP_C06_M04_NAME"),
				TEXT("STORY_MAP_C06_M04_SUMMARY"),
				TEXT("STORY_MAP_C06_M05_NAME"),
				TEXT("STORY_MAP_C06_M05_SUMMARY"),
				TEXT("STORY_MAP_C06_M06_NAME"),
				TEXT("STORY_MAP_C06_M06_SUMMARY"),
				TEXT("STORY_MAP_C06_M07_NAME"),
				TEXT("STORY_MAP_C06_M07_SUMMARY"),
				TEXT("STORY_MAP_C06_M08_NAME"),
				TEXT("STORY_MAP_C06_M08_SUMMARY"),
				TEXT("STORY_MAP_C06_M09_NAME"),
				TEXT("STORY_MAP_C06_M09_SUMMARY"),
				TEXT("STORY_MAP_C06_M10_NAME"),
				TEXT("STORY_MAP_C06_M10_SUMMARY"),
				TEXT("STORY_ELITE_C06_COLLAPSE_ECHO_NAME"),
				TEXT("STORY_ELITE_C06_COLLAPSE_ECHO_SUMMARY"),
				TEXT("STORY_BOSS_C06_END_VOID_NAME"),
				TEXT("STORY_BOSS_C06_END_VOID_SUMMARY"),
			};

			for (const FString& RequiredKey : RequiredChapter6StoryKeys)
			{
				TestTrue(*FString::Printf(TEXT("Story Korean contains required chapter 6 key %s"), *RequiredKey), KoreanKeys.Contains(RequiredKey));
				TestTrue(*FString::Printf(TEXT("Story English contains required chapter 6 key %s"), *RequiredKey), EnglishKeys.Contains(RequiredKey));
			}

			const TArray<FString> RequiredChapter7StoryKeys = {
				TEXT("STORY_CHAPTER_07_TITLE"),
				TEXT("STORY_CHAPTER_07_SUMMARY"),
				TEXT("STORY_MAP_C07_M01_NAME"),
				TEXT("STORY_MAP_C07_M01_SUMMARY"),
				TEXT("STORY_MAP_C07_M02_NAME"),
				TEXT("STORY_MAP_C07_M02_SUMMARY"),
				TEXT("STORY_MAP_C07_M03_NAME"),
				TEXT("STORY_MAP_C07_M03_SUMMARY"),
				TEXT("STORY_MAP_C07_M04_NAME"),
				TEXT("STORY_MAP_C07_M04_SUMMARY"),
				TEXT("STORY_MAP_C07_M05_NAME"),
				TEXT("STORY_MAP_C07_M05_SUMMARY"),
				TEXT("STORY_MAP_C07_M06_NAME"),
				TEXT("STORY_MAP_C07_M06_SUMMARY"),
				TEXT("STORY_MAP_C07_M07_NAME"),
				TEXT("STORY_MAP_C07_M07_SUMMARY"),
				TEXT("STORY_MAP_C07_M08_NAME"),
				TEXT("STORY_MAP_C07_M08_SUMMARY"),
				TEXT("STORY_MAP_C07_M09_NAME"),
				TEXT("STORY_MAP_C07_M09_SUMMARY"),
				TEXT("STORY_MAP_C07_M10_NAME"),
				TEXT("STORY_MAP_C07_M10_SUMMARY"),
				TEXT("STORY_ELITE_C07_UNRETURNED_NAME"),
				TEXT("STORY_ELITE_C07_UNRETURNED_SUMMARY"),
				TEXT("STORY_BOSS_C07_ETERNAL_THRESHOLD_NAME"),
				TEXT("STORY_BOSS_C07_ETERNAL_THRESHOLD_SUMMARY"),
			};

			for (const FString& RequiredKey : RequiredChapter7StoryKeys)
			{
				TestTrue(*FString::Printf(TEXT("Story Korean contains required chapter 7 key %s"), *RequiredKey), KoreanKeys.Contains(RequiredKey));
				TestTrue(*FString::Printf(TEXT("Story English contains required chapter 7 key %s"), *RequiredKey), EnglishKeys.Contains(RequiredKey));
			}

			const TArray<FString> RequiredChapter8StoryKeys = {
				TEXT("STORY_CHAPTER_08_TITLE"),
				TEXT("STORY_CHAPTER_08_SUMMARY"),
				TEXT("STORY_MAP_C08_M01_NAME"),
				TEXT("STORY_MAP_C08_M01_SUMMARY"),
				TEXT("STORY_MAP_C08_M02_NAME"),
				TEXT("STORY_MAP_C08_M02_SUMMARY"),
				TEXT("STORY_MAP_C08_M03_NAME"),
				TEXT("STORY_MAP_C08_M03_SUMMARY"),
				TEXT("STORY_MAP_C08_M04_NAME"),
				TEXT("STORY_MAP_C08_M04_SUMMARY"),
				TEXT("STORY_MAP_C08_M05_NAME"),
				TEXT("STORY_MAP_C08_M05_SUMMARY"),
				TEXT("STORY_MAP_C08_M06_NAME"),
				TEXT("STORY_MAP_C08_M06_SUMMARY"),
				TEXT("STORY_MAP_C08_M07_NAME"),
				TEXT("STORY_MAP_C08_M07_SUMMARY"),
				TEXT("STORY_MAP_C08_M08_NAME"),
				TEXT("STORY_MAP_C08_M08_SUMMARY"),
				TEXT("STORY_MAP_C08_M09_NAME"),
				TEXT("STORY_MAP_C08_M09_SUMMARY"),
				TEXT("STORY_MAP_C08_M10_NAME"),
				TEXT("STORY_MAP_C08_M10_SUMMARY"),
				TEXT("STORY_ELITE_C08_RISEN_REMNANT_NAME"),
				TEXT("STORY_ELITE_C08_RISEN_REMNANT_SUMMARY"),
				TEXT("STORY_BOSS_C08_SEALED_APEX_NAME"),
				TEXT("STORY_BOSS_C08_SEALED_APEX_SUMMARY"),
			};

			for (const FString& RequiredKey : RequiredChapter8StoryKeys)
			{
				TestTrue(*FString::Printf(TEXT("Story Korean contains required chapter 8 key %s"), *RequiredKey), KoreanKeys.Contains(RequiredKey));
				TestTrue(*FString::Printf(TEXT("Story English contains required chapter 8 key %s"), *RequiredKey), EnglishKeys.Contains(RequiredKey));
			}

			const TArray<FString> RequiredChapter9StoryKeys = {
				TEXT("STORY_CHAPTER_09_TITLE"),
				TEXT("STORY_CHAPTER_09_SUMMARY"),
				TEXT("STORY_MAP_C09_M01_NAME"),
				TEXT("STORY_MAP_C09_M01_SUMMARY"),
				TEXT("STORY_MAP_C09_M02_NAME"),
				TEXT("STORY_MAP_C09_M02_SUMMARY"),
				TEXT("STORY_MAP_C09_M03_NAME"),
				TEXT("STORY_MAP_C09_M03_SUMMARY"),
				TEXT("STORY_MAP_C09_M04_NAME"),
				TEXT("STORY_MAP_C09_M04_SUMMARY"),
				TEXT("STORY_MAP_C09_M05_NAME"),
				TEXT("STORY_MAP_C09_M05_SUMMARY"),
				TEXT("STORY_MAP_C09_M06_NAME"),
				TEXT("STORY_MAP_C09_M06_SUMMARY"),
				TEXT("STORY_MAP_C09_M07_NAME"),
				TEXT("STORY_MAP_C09_M07_SUMMARY"),
				TEXT("STORY_MAP_C09_M08_NAME"),
				TEXT("STORY_MAP_C09_M08_SUMMARY"),
				TEXT("STORY_MAP_C09_M09_NAME"),
				TEXT("STORY_MAP_C09_M09_SUMMARY"),
				TEXT("STORY_MAP_C09_M10_NAME"),
				TEXT("STORY_MAP_C09_M10_SUMMARY"),
				TEXT("STORY_ELITE_C09_RESONANT_REMNANT_NAME"),
				TEXT("STORY_ELITE_C09_RESONANT_REMNANT_SUMMARY"),
				TEXT("STORY_BOSS_C09_RESONANT_APEX_NAME"),
				TEXT("STORY_BOSS_C09_RESONANT_APEX_SUMMARY"),
			};

			for (const FString& RequiredKey : RequiredChapter9StoryKeys)
			{
				TestTrue(*FString::Printf(TEXT("Story Korean contains required chapter 9 key %s"), *RequiredKey), KoreanKeys.Contains(RequiredKey));
				TestTrue(*FString::Printf(TEXT("Story English contains required chapter 9 key %s"), *RequiredKey), EnglishKeys.Contains(RequiredKey));
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

			const TArray<FString> RequiredChapter6StoryTextKeys = {
				TEXT("STORY_C06_M01_INTRO_01"),
				TEXT("STORY_C06_M01_CLEAR_01"),
				TEXT("STORY_C06_M02_INTRO_01"),
				TEXT("STORY_C06_M02_CLEAR_01"),
				TEXT("STORY_C06_M03_INTRO_01"),
				TEXT("STORY_C06_M03_CLEAR_01"),
				TEXT("STORY_C06_M04_INTRO_01"),
				TEXT("STORY_C06_M04_CLEAR_01"),
				TEXT("STORY_C06_M05_ELITE_01"),
				TEXT("STORY_C06_M05_ELITE_02"),
				TEXT("STORY_C06_M05_CLEAR_01"),
				TEXT("STORY_C06_M06_INTRO_01"),
				TEXT("STORY_C06_M06_CLEAR_01"),
				TEXT("STORY_C06_M07_INTRO_01"),
				TEXT("STORY_C06_M07_CLEAR_01"),
				TEXT("STORY_C06_M08_INTRO_01"),
				TEXT("STORY_C06_M08_CLEAR_01"),
				TEXT("STORY_C06_M09_INTRO_01"),
				TEXT("STORY_C06_M09_CLEAR_01"),
				TEXT("STORY_C06_M10_BOSS_01"),
				TEXT("STORY_C06_M10_BOSS_02"),
				TEXT("STORY_C06_M10_CLEAR_01"),
				TEXT("STORY_C06_M10_CLEAR_02"),
			};

			for (const FString& RequiredKey : RequiredChapter6StoryTextKeys)
			{
				TestTrue(*FString::Printf(TEXT("StoryText Korean contains required chapter 6 key %s"), *RequiredKey), KoreanKeys.Contains(RequiredKey));
				TestTrue(*FString::Printf(TEXT("StoryText English contains required chapter 6 key %s"), *RequiredKey), EnglishKeys.Contains(RequiredKey));
			}

			const TArray<FString> RequiredChapter7StoryTextKeys = {
				TEXT("STORY_C07_M01_INTRO_01"),
				TEXT("STORY_C07_M01_CLEAR_01"),
				TEXT("STORY_C07_M02_INTRO_01"),
				TEXT("STORY_C07_M02_CLEAR_01"),
				TEXT("STORY_C07_M03_INTRO_01"),
				TEXT("STORY_C07_M03_CLEAR_01"),
				TEXT("STORY_C07_M04_INTRO_01"),
				TEXT("STORY_C07_M04_CLEAR_01"),
				TEXT("STORY_C07_M05_ELITE_01"),
				TEXT("STORY_C07_M05_ELITE_02"),
				TEXT("STORY_C07_M05_CLEAR_01"),
				TEXT("STORY_C07_M06_INTRO_01"),
				TEXT("STORY_C07_M06_CLEAR_01"),
				TEXT("STORY_C07_M07_INTRO_01"),
				TEXT("STORY_C07_M07_CLEAR_01"),
				TEXT("STORY_C07_M08_INTRO_01"),
				TEXT("STORY_C07_M08_CLEAR_01"),
				TEXT("STORY_C07_M09_INTRO_01"),
				TEXT("STORY_C07_M09_CLEAR_01"),
				TEXT("STORY_C07_M10_BOSS_01"),
				TEXT("STORY_C07_M10_BOSS_02"),
				TEXT("STORY_C07_M10_CLEAR_01"),
				TEXT("STORY_C07_M10_CLEAR_02"),
			};

			for (const FString& RequiredKey : RequiredChapter7StoryTextKeys)
			{
				TestTrue(*FString::Printf(TEXT("StoryText Korean contains required chapter 7 key %s"), *RequiredKey), KoreanKeys.Contains(RequiredKey));
				TestTrue(*FString::Printf(TEXT("StoryText English contains required chapter 7 key %s"), *RequiredKey), EnglishKeys.Contains(RequiredKey));
			}

			const TArray<FString> RequiredChapter8StoryTextKeys = {
				TEXT("STORY_C08_M01_INTRO_01"),
				TEXT("STORY_C08_M01_CLEAR_01"),
				TEXT("STORY_C08_M02_INTRO_01"),
				TEXT("STORY_C08_M02_CLEAR_01"),
				TEXT("STORY_C08_M03_INTRO_01"),
				TEXT("STORY_C08_M03_CLEAR_01"),
				TEXT("STORY_C08_M04_INTRO_01"),
				TEXT("STORY_C08_M04_CLEAR_01"),
				TEXT("STORY_C08_M05_ELITE_01"),
				TEXT("STORY_C08_M05_ELITE_02"),
				TEXT("STORY_C08_M05_CLEAR_01"),
				TEXT("STORY_C08_M06_INTRO_01"),
				TEXT("STORY_C08_M06_CLEAR_01"),
				TEXT("STORY_C08_M07_INTRO_01"),
				TEXT("STORY_C08_M07_CLEAR_01"),
				TEXT("STORY_C08_M08_INTRO_01"),
				TEXT("STORY_C08_M08_CLEAR_01"),
				TEXT("STORY_C08_M09_INTRO_01"),
				TEXT("STORY_C08_M09_CLEAR_01"),
				TEXT("STORY_C08_M10_BOSS_01"),
				TEXT("STORY_C08_M10_BOSS_02"),
				TEXT("STORY_C08_M10_CLEAR_01"),
				TEXT("STORY_C08_M10_CLEAR_02"),
			};

			for (const FString& RequiredKey : RequiredChapter8StoryTextKeys)
			{
				TestTrue(*FString::Printf(TEXT("StoryText Korean contains required chapter 8 key %s"), *RequiredKey), KoreanKeys.Contains(RequiredKey));
				TestTrue(*FString::Printf(TEXT("StoryText English contains required chapter 8 key %s"), *RequiredKey), EnglishKeys.Contains(RequiredKey));
			}

			const TArray<FString> RequiredChapter9StoryTextKeys = {
				TEXT("STORY_C09_M01_INTRO_01"),
				TEXT("STORY_C09_M01_CLEAR_01"),
				TEXT("STORY_C09_M02_INTRO_01"),
				TEXT("STORY_C09_M02_CLEAR_01"),
				TEXT("STORY_C09_M03_INTRO_01"),
				TEXT("STORY_C09_M03_CLEAR_01"),
				TEXT("STORY_C09_M04_INTRO_01"),
				TEXT("STORY_C09_M04_CLEAR_01"),
				TEXT("STORY_C09_M05_ELITE_01"),
				TEXT("STORY_C09_M05_ELITE_02"),
				TEXT("STORY_C09_M05_CLEAR_01"),
				TEXT("STORY_C09_M06_INTRO_01"),
				TEXT("STORY_C09_M06_CLEAR_01"),
				TEXT("STORY_C09_M07_INTRO_01"),
				TEXT("STORY_C09_M07_CLEAR_01"),
				TEXT("STORY_C09_M08_INTRO_01"),
				TEXT("STORY_C09_M08_CLEAR_01"),
				TEXT("STORY_C09_M09_INTRO_01"),
				TEXT("STORY_C09_M09_CLEAR_01"),
				TEXT("STORY_C09_M10_BOSS_01"),
				TEXT("STORY_C09_M10_BOSS_02"),
				TEXT("STORY_C09_M10_CLEAR_01"),
				TEXT("STORY_C09_M10_CLEAR_02"),
			};

			for (const FString& RequiredKey : RequiredChapter9StoryTextKeys)
			{
				TestTrue(*FString::Printf(TEXT("StoryText Korean contains required chapter 9 key %s"), *RequiredKey), KoreanKeys.Contains(RequiredKey));
				TestTrue(*FString::Printf(TEXT("StoryText English contains required chapter 9 key %s"), *RequiredKey), EnglishKeys.Contains(RequiredKey));
			}
		}

		if (TableName == TEXT("Rune"))
		{
			const TArray<FString> RequiredRuneActionKeys = {
				TEXT("RUNE_ACTION_SECTION_TITLE"),
				TEXT("RUNE_ACTION_NO_SELECTION"),
				TEXT("RUNE_ACTION_REROLL_SET"),
				TEXT("RUNE_ACTION_UPGRADE_RARITY"),
				TEXT("RUNE_ACTION_TRANSFER"),
				TEXT("RUNE_TRANSFER_CYCLE"),
				TEXT("RUNE_CURRENT_SET_FORMAT"),
				TEXT("RUNE_REROLL_COST_FORMAT"),
				TEXT("RUNE_UPGRADE_INFO_FORMAT"),
				TEXT("RUNE_UPGRADE_MAX"),
				TEXT("RUNE_TRANSFER_TARGET_FORMAT"),
				TEXT("RUNE_TRANSFER_NO_TARGET"),
				TEXT("RUNE_TRANSFER_COST_FORMAT"),
				TEXT("RUNE_FEEDBACK_REROLL_DONE"),
				TEXT("RUNE_FEEDBACK_UPGRADE_SUCCESS"),
				TEXT("RUNE_FEEDBACK_UPGRADE_FAIL"),
				TEXT("RUNE_FEEDBACK_TRANSFER_DONE"),
				TEXT("RUNE_FEEDBACK_FAILED"),
			};

			for (const FString& RequiredKey : RequiredRuneActionKeys)
			{
				TestTrue(*FString::Printf(TEXT("Rune Korean contains required rune action key %s"), *RequiredKey), KoreanKeys.Contains(RequiredKey));
				TestTrue(*FString::Printf(TEXT("Rune English contains required rune action key %s"), *RequiredKey), EnglishKeys.Contains(RequiredKey));
			}
		}

		if (TableName == TEXT("Quest"))
		{
			const TArray<FString> RequiredChapter6QuestKeys = {
				TEXT("main_ch6_001"),
				TEXT("main_ch6_002"),
				TEXT("main_ch6_003"),
				TEXT("main_ch6_004"),
				TEXT("main_ch6_005"),
				TEXT("main_ch6_006"),
			};

			for (const FString& RequiredKey : RequiredChapter6QuestKeys)
			{
				TestTrue(*FString::Printf(TEXT("Quest Korean contains required chapter 6 key %s"), *RequiredKey), KoreanKeys.Contains(RequiredKey));
				TestTrue(*FString::Printf(TEXT("Quest English contains required chapter 6 key %s"), *RequiredKey), EnglishKeys.Contains(RequiredKey));
			}

			const TArray<FString> RequiredChapter7QuestKeys = {
				TEXT("main_ch7_001"),
				TEXT("main_ch7_002"),
				TEXT("main_ch7_003"),
				TEXT("main_ch7_004"),
				TEXT("main_ch7_005"),
				TEXT("main_ch7_006"),
			};

			for (const FString& RequiredKey : RequiredChapter7QuestKeys)
			{
				TestTrue(*FString::Printf(TEXT("Quest Korean contains required chapter 7 key %s"), *RequiredKey), KoreanKeys.Contains(RequiredKey));
				TestTrue(*FString::Printf(TEXT("Quest English contains required chapter 7 key %s"), *RequiredKey), EnglishKeys.Contains(RequiredKey));
			}

			const TArray<FString> RequiredChapter8QuestKeys = {
				TEXT("main_ch8_001"),
				TEXT("main_ch8_002"),
				TEXT("main_ch8_003"),
				TEXT("main_ch8_004"),
				TEXT("main_ch8_005"),
				TEXT("main_ch8_006"),
			};

			for (const FString& RequiredKey : RequiredChapter8QuestKeys)
			{
				TestTrue(*FString::Printf(TEXT("Quest Korean contains required chapter 8 key %s"), *RequiredKey), KoreanKeys.Contains(RequiredKey));
				TestTrue(*FString::Printf(TEXT("Quest English contains required chapter 8 key %s"), *RequiredKey), EnglishKeys.Contains(RequiredKey));
			}

			const TArray<FString> RequiredChapter9QuestKeys = {
				TEXT("main_ch9_001"),
				TEXT("main_ch9_002"),
				TEXT("main_ch9_003"),
				TEXT("main_ch9_004"),
				TEXT("main_ch9_005"),
				TEXT("main_ch9_006"),
			};

			for (const FString& RequiredKey : RequiredChapter9QuestKeys)
			{
				TestTrue(*FString::Printf(TEXT("Quest Korean contains required chapter 9 key %s"), *RequiredKey), KoreanKeys.Contains(RequiredKey));
				TestTrue(*FString::Printf(TEXT("Quest English contains required chapter 9 key %s"), *RequiredKey), EnglishKeys.Contains(RequiredKey));
			}
		}
	}

	return true;
}

#endif
