#include "UI/IdleHUD.h"

#include "CombatSystem/BattleAIComponent.h"
#include "CombatSystem/BossPhaseFormula.h"
#include "CombatSystem/CombatComponent.h"
#include "CombatSystem/SkillComponent.h"
#include "CharacterSystem/IdleCharacter.h"
#include "CharacterSystem/IdleMonster.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/GameViewportClient.h"
#include "EngineUtils.h"
#include "GameCore/BuffService.h"
#include "GameCore/ConsumableFormula.h"
#include "GameCore/DungeonFormula.h"
#include "GameCore/GuildBossFormula.h"
#include "GameCore/GuildFormula.h"
#include "GameCore/GuildService.h"
#include "GameCore/IdleGameInstance.h"
#include "GameCore/LeaderboardService.h"
#include "GameCore/MasteryService.h"
#include "GameCore/PetLevelFormula.h"
#include "GameCore/QuestService.h"
#include "GameCore/TowerMilestoneFormula.h"
#include "GameCore/TranscendFormula.h"
#include "GameCore/AttendanceService.h"
#include "GameCore/WeeklyBossFormula.h"
#include "GameCore/WeeklyBossService.h"
#include "Internationalization/IdleLocalization.h"
#include "ItemSystem/EnhanceFormula.h"
#include "ItemSystem/InventoryComponent.h"
#include "ItemSystem/PotentialFormula.h"
#include "ItemSystem/SetBonusFormula.h"
#include "ItemSystem/ShopFormula.h"
#include "ItemSystem/UniqueTraitFormula.h"
#include "RuneSystem/ClassRuneFormula.h"
#include "RuneSystem/RuneCodexFormula.h"
#include "RuneSystem/RuneFormula.h"
#include "RuneSystem/RuneService.h"
#include "RuneSystem/RuneSetFormula.h"
#include "UI/IdleHUDWidget.h"
#include "UI/UIThemeTokens.h"

namespace
{
const FName OfflineRewardClaimHitBoxName(TEXT("OfflineRewardClaim"));
const FName RebirthHitBoxName(TEXT("RebirthAction"));
const FName TranscendHitBoxName(TEXT("TranscendAction"));
const FName TowerClimbHitBoxName(TEXT("TowerClimb"));
const FString ClassSelectionHitBoxPrefix(TEXT("ClassSelect_"));
const FString QuestClaimHitBoxPrefix(TEXT("QuestClaim_"));
const FString PetEquipHitBoxPrefix(TEXT("PetEquip_"));
const FString PetFeedHitBoxPrefix(TEXT("PetFeed_"));
const FString PetEvolveHitBoxPrefix(TEXT("PetEvolve_"));
// 칭호 패널(칭호 시스템) — 고유 Title~ prefix 로 jumbo ODR 회피.
const FString TitleEquipHitBoxPrefix(TEXT("TitleEquip_"));
const FName TitleUnequipHitBoxName(TEXT("TitleUnequip"));
// 미션 패널(일일/주간 미션) — 고유 Mission~ prefix 로 jumbo ODR 회피.
const FString MissionClaimHitBoxPrefix(TEXT("MissionClaim_"));
const FName MissionDailyTabHitBoxName(TEXT("MissionTabDaily"));
const FName MissionWeeklyTabHitBoxName(TEXT("MissionTabWeekly"));
const FString SeasonClaimHitBoxPrefix(TEXT("SeasonClaim_"));
const FString SkillRankHitBoxPrefix(TEXT("SkillRank_"));
const FString EnhanceSlotHitBoxPrefix(TEXT("EnhanceSlot_"));
const FString EnhanceProtectHitBoxPrefix(TEXT("EnhanceProtect_"));
const FString PotentialResetHitBoxPrefix(TEXT("PotentialReset_"));
const FString PotentialRankHitBoxPrefix(TEXT("PotentialRank_"));
const FString PotentialLockHitBoxPrefix(TEXT("PotentialLock_"));
const FString RuneSelectHitBoxPrefix(TEXT("RuneSelect_"));
const FString RuneEquipHitBoxPrefix(TEXT("RuneEquip_"));
const FString RuneUnequipHitBoxPrefix(TEXT("RuneUnequip_"));
const FString RuneEnhanceHitBoxPrefix(TEXT("RuneEnhance_"));
const FString RuneDisenchantHitBoxPrefix(TEXT("RuneDisenchant_"));
const FString StatAllocationHitBoxPrefix(TEXT("StatAlloc_"));
const FString DungeonEnterHitBoxPrefix(TEXT("DungeonEnter_"));
const FString ConsumableUseHitBoxPrefix(TEXT("ConsumableUse_"));
const FString WeeklyBossClaimHitBoxPrefix(TEXT("WeeklyBossClaim_"));
// 출석 보상 패널 — 고유 prefix(Attendance~)로 jumbo ODR 회피.
const FString AttendanceClaimHitBoxPrefix(TEXT("AttendanceClaim_"));
const FName LeaderboardPowerTabHitBoxName(TEXT("LeaderboardTabPower"));
const FName LeaderboardRebirthTabHitBoxName(TEXT("LeaderboardTabRebirth"));
const FName LeaderboardWeeklyTabHitBoxName(TEXT("LeaderboardTabWeekly"));
const FName LeaderboardRefreshHitBoxName(TEXT("LeaderboardRefresh"));
const FName WeeklyBossChallengeHitBoxName(TEXT("WeeklyBossChallenge"));
const FName ShopGearRollHitBoxName(TEXT("ShopGearRoll"));
const FName ShopProtectionScrollHitBoxName(TEXT("ShopProtectionScroll"));
const FName ShopResetCubeHitBoxName(TEXT("ShopResetCube"));
const FName ShopRankCubeHitBoxName(TEXT("ShopRankCube"));
const FName ShopRuneRollHitBoxName(TEXT("ShopRuneRoll"));
const FName CraftClassRuneHitBoxName(TEXT("CraftClassRune"));
const FName RuneRerollSetHitBoxName(TEXT("RuneRerollSet"));
const FName RuneUpgradeRarityHitBoxName(TEXT("RuneUpgradeRarity"));
const FName RuneTransferHitBoxName(TEXT("RuneTransfer"));
const FName RuneTransferCycleHitBoxName(TEXT("RuneTransferCycle"));
const FName StatResetHitBoxName(TEXT("StatReset"));
const FName StatInfoToggleHitBoxName(TEXT("StatInfoToggle"));
// 길드 패널(PR-G1) — 고유 prefix(Guild~)로 jumbo ODR 회피.
const FName GuildRefreshListHitBoxName(TEXT("GuildRefreshList"));
const FName GuildCreateNameCycleHitBoxName(TEXT("GuildCreateNameCycle"));
const FName GuildCreateHitBoxName(TEXT("GuildCreate"));
const FName GuildLeaveHitBoxName(TEXT("GuildLeave"));
const FName GuildToggleJoinModeHitBoxName(TEXT("GuildToggleJoinMode"));
const FString GuildJoinHitBoxPrefix(TEXT("GuildJoin_"));
const FString GuildApproveHitBoxPrefix(TEXT("GuildApprove_"));
const FString GuildRejectHitBoxPrefix(TEXT("GuildReject_"));
const FString GuildPromoteHitBoxPrefix(TEXT("GuildPromote_"));
const FString GuildDemoteHitBoxPrefix(TEXT("GuildDemote_"));
// 길드 기여/상점(PR-G2) — 동일 Guild~ prefix 로 jumbo ODR 회피.
const FName GuildAttendHitBoxName(TEXT("GuildAttend"));
const FName GuildDonateHitBoxName(TEXT("GuildDonate"));
const FName GuildDonateCycleHitBoxName(TEXT("GuildDonateCycle"));
const FString GuildShopBuyHitBoxPrefix(TEXT("GuildShopBuy_"));
// 길드 보스/주간 랭킹(PR-G3) — 동일 Guild~ prefix 로 jumbo ODR 회피.
const FName GuildBossChallengeHitBoxName(TEXT("GuildBossChallenge"));
const FName GuildBossClaimHitBoxName(TEXT("GuildBossClaim"));
const FName GuildTabMyHitBoxName(TEXT("GuildTabMy"));
const FName GuildTabRankingsHitBoxName(TEXT("GuildTabRankings"));
// 헌납 금액 프리셋(순환) — 보유 골드로 가능한 만큼만 활성.
const int64 GuildDonatePresets[] = { 1000, 5000, 10000 };
constexpr int32 GuildDonatePresetCount = 3;
constexpr int32 GuildCreateNamePresetCount = 8;
constexpr float GuildFeedbackDurationSeconds = 2.4f;
constexpr int32 RebirthRequiredLevel = 100;
constexpr float FloatingDamageLifetimeSeconds = 1.0f;
constexpr float FloatingDamageRisePixels = 32.0f;
constexpr float FloatingDamageHeadOffsetZ = 120.0f;
constexpr float StatusIndicatorHeadOffsetZ = 152.0f;
constexpr float BossSpecialWarningDurationSeconds = 1.6f;
constexpr float StageFeedbackDurationSeconds = 2.2f;
constexpr float PetFeedbackDurationSeconds = 2.2f;
constexpr float TranscendFeedbackDurationSeconds = 2.4f;
constexpr float TowerFeedbackDurationSeconds = 2.4f;
constexpr float DungeonFeedbackDurationSeconds = 2.4f;
constexpr float AchievementFeedbackDurationSeconds = 2.4f;
constexpr float WeeklyBossFeedbackDurationSeconds = 2.4f;
constexpr float AttendanceFeedbackDurationSeconds = 2.4f;
constexpr float ProgressSavedFeedbackDurationSeconds = 1.6f;
constexpr float CloudSyncFeedbackDurationSeconds = 2.4f;

FString FormatIntegerWithCommas(int64 Value)
{
	FString Digits = FString::Printf(TEXT("%lld"), FMath::Abs(Value));
	FString Formatted;
	int32 DigitCount = 0;
	for (int32 Index = Digits.Len() - 1; Index >= 0; --Index)
	{
		if (DigitCount > 0 && DigitCount % 3 == 0)
		{
			Formatted.InsertAt(0, TEXT(","));
		}
		Formatted.InsertAt(0, Digits[Index]);
		++DigitCount;
	}

	return Value < 0 ? FString::Printf(TEXT("-%s"), *Formatted) : Formatted;
}

FString FormatElapsedHoursMinutes(int64 CappedSeconds)
{
	const int64 ClampedSeconds = FMath::Max<int64>(0, CappedSeconds);
	const int64 Hours = ClampedSeconds / 3600;
	const int64 Minutes = (ClampedSeconds % 3600) / 60;
	return FString::Printf(TEXT("%lld:%02lld"), Hours, Minutes);
}

FString FormatMinutesSeconds(int64 Seconds)
{
	const int64 ClampedSeconds = FMath::Max<int64>(0, Seconds);
	return FString::Printf(TEXT("%lld:%02lld"), ClampedSeconds / 60, ClampedSeconds % 60);
}

FText FormatLocalizedUI(const TCHAR* Key, TFunctionRef<void(FFormatNamedArguments&)> BuildArgs)
{
	FFormatNamedArguments Args;
	BuildArgs(Args);
	return IdleProject::Localization::UI(Key, Args);
}

FText FormatLocalizedUIWithInt64(const TCHAR* Key, const TCHAR* ArgName, int64 Value)
{
	return FormatLocalizedUI(Key, [ArgName, Value](FFormatNamedArguments& Args)
	{
		Args.Add(ArgName, FText::FromString(FormatIntegerWithCommas(Value)));
	});
}

FText FormatLeaderboardRankLabel(int32 Rank)
{
	return Rank > 0 ? FText::FromString(FString::Printf(TEXT("#%d"), Rank)) : IdleProject::Localization::UI(TEXT("NONE_DASH"));
}

FText FormatLeaderboardScoreLabel(int64 Score)
{
	return FormatLocalizedUIWithInt64(TEXT("LEADERBOARD_SCORE_FORMAT"), TEXT("Score"), Score);
}

FText FormatLeaderboardCharacterLabel(const FString& CharacterId)
{
	if (CharacterId.IsEmpty())
	{
		return IdleProject::Localization::UI(TEXT("NONE_DASH"));
	}
	if (CharacterId.Len() <= 10)
	{
		return FText::FromString(CharacterId);
	}
	return FText::FromString(FString::Printf(TEXT("%s...%s"), *CharacterId.Left(6), *CharacterId.Right(4)));
}

FIdleHUDLeaderboardRowViewModel BuildLeaderboardRow(const FLeaderboardEntry& Entry, const FString& MyCharacterId)
{
	FIdleHUDLeaderboardRowViewModel Row;
	Row.CharacterId = Entry.CharacterId;
	Row.CharacterLabel = FormatLeaderboardCharacterLabel(Entry.CharacterId);
	Row.RankLabel = FormatLeaderboardRankLabel(Entry.Rank);
	Row.ScoreLabel = FormatLeaderboardScoreLabel(Entry.Score);
	Row.bSelf = !MyCharacterId.IsEmpty() && Entry.CharacterId == MyCharacterId;
	return Row;
}

// ── 길드 패널 헬퍼(PR-G1) — 고유 prefix(Guild~)로 jumbo ODR 회피 ─────────────
const TCHAR* GuildRankToLocalizationKey(EGuildRank Rank)
{
	switch (Rank)
	{
	case EGuildRank::Master:
		return TEXT("GUILD_RANK_MASTER");
	case EGuildRank::Vice:
		return TEXT("GUILD_RANK_VICE");
	case EGuildRank::Officer:
		return TEXT("GUILD_RANK_OFFICER");
	default:
		return TEXT("GUILD_RANK_MEMBER");
	}
}

FText GuildJoinModeLabel(EGuildJoinMode JoinMode)
{
	return IdleProject::Localization::UI(JoinMode == EGuildJoinMode::Approval ? TEXT("GUILD_JOINMODE_APPROVAL") : TEXT("GUILD_JOINMODE_OPEN"));
}

FText GuildPresetCreateName(int32 PresetIndex)
{
	const int32 Clamped = ((PresetIndex % GuildCreateNamePresetCount) + GuildCreateNamePresetCount) % GuildCreateNamePresetCount;
	const FString Key = FString::Printf(TEXT("GUILD_PRESET_NAME_%d"), Clamped + 1);
	return IdleProject::Localization::UI(*Key);
}

FText GuildShortCharacterLabel(const FString& CharacterId)
{
	if (CharacterId.IsEmpty())
	{
		return IdleProject::Localization::UI(TEXT("NONE_DASH"));
	}
	if (CharacterId.Len() <= 10)
	{
		return FText::FromString(CharacterId);
	}
	return FText::FromString(FString::Printf(TEXT("%s...%s"), *CharacterId.Left(6), *CharacterId.Right(4)));
}

// ── 길드 기여/상점 헬퍼(PR-G2) ───────────────────────────────────────────────
/** 상점 아이템 id → 로컬라이즈 표기명. 알 수 없는 id 는 서버 제공 name 폴백. */
FText GuildShopItemNameLabel(const FGuildShopItemInfo& Item)
{
	static const TMap<FString, const TCHAR*> IdToKey = {
		{ TEXT("protection_scroll"), TEXT("GUILD_SHOP_ITEM_PROTECTION_SCROLL") },
		{ TEXT("reset_cube"), TEXT("GUILD_SHOP_ITEM_RESET_CUBE") },
		{ TEXT("gold_pouch"), TEXT("GUILD_SHOP_ITEM_GOLD_POUCH") },
		{ TEXT("exp_potion"), TEXT("GUILD_SHOP_ITEM_EXP_POTION") },
		{ TEXT("rank_cube"), TEXT("GUILD_SHOP_ITEM_RANK_CUBE") },
		{ TEXT("essence"), TEXT("GUILD_SHOP_ITEM_ESSENCE") },
	};
	if (const TCHAR* const* KeyPtr = IdToKey.Find(Item.Id))
	{
		return IdleProject::Localization::UI(*KeyPtr);
	}
	return Item.Name.IsEmpty() ? IdleProject::Localization::UI(TEXT("NONE_DASH")) : FText::FromString(Item.Name);
}

/**
 * 현재 EXP 의 "현재 레벨 내 진행" 산출 — 다음 레벨까지 채운 양/필요 양·비율.
 * 서버 parity 상수(FGuildFormula::GUILD_LEVEL_BASE/GROWTH)로 누적 임계를 재현한다.
 * 진행 표시 전용(레벨 자체는 GetGuildLevel 권위).
 */
void GuildComputeExpProgress(int64 Exp, int32 Level, int64& OutInto, int64& OutSpan, float& OutRatio)
{
	OutInto = 0;
	OutSpan = 0;
	OutRatio = 0.0f;
	if (Exp < 0 || Level < 1)
	{
		return;
	}

	// 레벨 L 도달 누적 임계 = Σ_{i=1}^{L-1} floor(BASE*GROWTH^(i-1)).
	int64 Cumulative = 0;
	for (int32 i = 1; i < Level; ++i)
	{
		const double StepDouble = FMath::Floor(static_cast<double>(FGuildFormula::GUILD_LEVEL_BASE) * FMath::Pow(FGuildFormula::GUILD_LEVEL_GROWTH, static_cast<double>(i - 1)));
		if (StepDouble >= static_cast<double>(MAX_int64))
		{
			return;
		}
		Cumulative += static_cast<int64>(StepDouble);
	}

	const double NextStepDouble = FMath::Floor(static_cast<double>(FGuildFormula::GUILD_LEVEL_BASE) * FMath::Pow(FGuildFormula::GUILD_LEVEL_GROWTH, static_cast<double>(Level - 1)));
	if (NextStepDouble >= static_cast<double>(MAX_int64))
	{
		return; // 캡 레벨 — 진행 표시 생략.
	}
	OutSpan = static_cast<int64>(NextStepDouble);
	OutInto = FMath::Clamp<int64>(Exp - Cumulative, 0, OutSpan);
	OutRatio = OutSpan > 0 ? FMath::Clamp(static_cast<float>(OutInto) / static_cast<float>(OutSpan), 0.0f, 1.0f) : 0.0f;
}

/** 헌납 프리셋 금액(순환) — 보유 골드 검증은 호출부. */
int64 GuildDonatePresetAmount(int32 PresetIndex)
{
	const int32 Clamped = ((PresetIndex % GuildDonatePresetCount) + GuildDonatePresetCount) % GuildDonatePresetCount;
	return GuildDonatePresets[Clamped];
}

FText FormatPercentLabel(const TCHAR* Key, float Rate)
{
	const int32 Percent = FMath::RoundToInt(FMath::Clamp(Rate, 0.0f, 1.0f) * 100.0f);
	return FormatLocalizedUI(Key, [Percent](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Percent"), FText::AsNumber(Percent));
	});
}

FText FormatAffixRateLabel(const TCHAR* Key, float Rate)
{
	const int32 Percent = FMath::RoundToInt(FMath::Max(0.0f, Rate) * 100.0f);
	return FormatLocalizedUI(Key, [Percent](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Percent"), FText::AsNumber(Percent));
	});
}

FText FormatAffixFloatLabel(const TCHAR* Key, float Value)
{
	const FString FormattedValue = FString::Printf(TEXT("%.2f"), FMath::Max(0.0f, Value));
	return FormatLocalizedUI(Key, [&FormattedValue](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Value"), FText::FromString(FormattedValue));
	});
}

FText FormatAffixFlatLabel(const TCHAR* Key, float Value)
{
	const int32 RoundedValue = FMath::RoundToInt(FMath::Max(0.0f, Value));
	return FormatLocalizedUI(Key, [RoundedValue](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Value"), FText::AsNumber(RoundedValue));
	});
}

FText FormatIntegerLabel(float Value)
{
	return FText::AsNumber(FMath::RoundToInt(Value));
}

FText FormatStatInfoPercentLabel(float Rate)
{
	return FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(FMath::Clamp(Rate, 0.0f, 1.0f) * 100.0f)));
}

FText FormatStatInfoScalarPercentLabel(float Value)
{
	return FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(FMath::Max(0.0f, Value) * 100.0f)));
}

FText FormatMultiplierLabel(float Value)
{
	return FText::FromString(FString::Printf(TEXT("x%.2f"), FMath::Max(0.0f, Value)));
}

FText FormatLocalizedUIWithNumber(const TCHAR* Key, const TCHAR* ArgName, int32 Value)
{
	return FormatLocalizedUI(Key, [ArgName, Value](FFormatNamedArguments& Args)
	{
		Args.Add(ArgName, FText::AsNumber(Value));
	});
}

FName MakeWeeklyBossClaimHitBoxName(int32 Milestone)
{
	return FName(*FString::Printf(TEXT("%s%d"), *WeeklyBossClaimHitBoxPrefix, Milestone));
}

int32 WeeklyBossMilestoneFromHitBoxName(FName BoxName)
{
	FString MilestoneText = BoxName.ToString();
	MilestoneText.RemoveFromStart(WeeklyBossClaimHitBoxPrefix);
	return FCString::Atoi(*MilestoneText);
}

FName MakeAttendanceClaimHitBoxName(int32 Milestone)
{
	return FName(*FString::Printf(TEXT("%s%d"), *AttendanceClaimHitBoxPrefix, Milestone));
}

int32 AttendanceMilestoneFromHitBoxName(FName BoxName)
{
	FString MilestoneText = BoxName.ToString();
	MilestoneText.RemoveFromStart(AttendanceClaimHitBoxPrefix);
	return FCString::Atoi(*MilestoneText);
}

// 출석 마일스톤 보상 종류 → 로컬라이즈 표시명(골드/룬정수/소비).
FText AttendanceRewardTypeLabel(EAttendanceReward RewardType)
{
	switch (RewardType)
	{
	case EAttendanceReward::Essence:
		return IdleProject::Localization::UI(TEXT("ATTENDANCE_REWARD_ESSENCE"));
	case EAttendanceReward::Consumable:
		return IdleProject::Localization::UI(TEXT("ATTENDANCE_REWARD_CONSUMABLE"));
	case EAttendanceReward::Gold:
	default:
		return IdleProject::Localization::UI(TEXT("ATTENDANCE_REWARD_GOLD"));
	}
}

const EConsumableType* GetConsumableDisplayOrder()
{
	static const EConsumableType Order[] = {
		EConsumableType::AttackTonic,
		EConsumableType::GuardTonic,
		EConsumableType::AllStatElixir,
		EConsumableType::FortuneScroll,
		EConsumableType::GoldFeast,
		EConsumableType::WisdomBooster,
	};
	return Order;
}

constexpr int32 GetConsumableDisplayCount()
{
	return 6;
}

const TCHAR* ConsumableNameKey(EConsumableType Type)
{
	switch (Type)
	{
	case EConsumableType::AttackTonic:
		return TEXT("CONSUMABLE_ATTACK_TONIC_NAME");
	case EConsumableType::GuardTonic:
		return TEXT("CONSUMABLE_GUARD_TONIC_NAME");
	case EConsumableType::AllStatElixir:
		return TEXT("CONSUMABLE_ALL_STAT_ELIXIR_NAME");
	case EConsumableType::FortuneScroll:
		return TEXT("CONSUMABLE_FORTUNE_SCROLL_NAME");
	case EConsumableType::GoldFeast:
		return TEXT("CONSUMABLE_GOLD_FEAST_NAME");
	case EConsumableType::WisdomBooster:
		return TEXT("CONSUMABLE_WISDOM_BOOSTER_NAME");
	default:
		return TEXT("NONE_DASH");
	}
}

const TCHAR* ConsumableEffectKey(EConsumableType Type)
{
	switch (Type)
	{
	case EConsumableType::AttackTonic:
		return TEXT("CONSUMABLE_ATTACK_TONIC_EFFECT");
	case EConsumableType::GuardTonic:
		return TEXT("CONSUMABLE_GUARD_TONIC_EFFECT");
	case EConsumableType::AllStatElixir:
		return TEXT("CONSUMABLE_ALL_STAT_ELIXIR_EFFECT");
	case EConsumableType::FortuneScroll:
		return TEXT("CONSUMABLE_FORTUNE_SCROLL_EFFECT");
	case EConsumableType::GoldFeast:
		return TEXT("CONSUMABLE_GOLD_FEAST_EFFECT");
	case EConsumableType::WisdomBooster:
		return TEXT("CONSUMABLE_WISDOM_BOOSTER_EFFECT");
	default:
		return TEXT("NONE_DASH");
	}
}

const TCHAR* ConsumableGradeLabelKey(EConsumableGrade Grade)
{
	switch (Grade)
	{
	case EConsumableGrade::Lesser:
		return TEXT("CONSUMABLE_GRADE_LESSER");
	case EConsumableGrade::Standard:
		return TEXT("CONSUMABLE_GRADE_STANDARD");
	case EConsumableGrade::Greater:
		return TEXT("CONSUMABLE_GRADE_GREATER");
	default:
		return TEXT("NONE_DASH");
	}
}

const EMasteryTrack* GetMasteryTrackDisplayOrder()
{
	static const EMasteryTrack Order[] = {
		EMasteryTrack::Combat,
		EMasteryTrack::Equipment,
		EMasteryTrack::Abyss,
		EMasteryTrack::Rune,
		EMasteryTrack::Beast,
		EMasteryTrack::Explore,
	};
	return Order;
}

const TCHAR* MasteryTrackToLocalizationKey(EMasteryTrack Track)
{
	switch (Track)
	{
	case EMasteryTrack::Combat:
		return TEXT("MASTERY_TRACK_COMBAT");
	case EMasteryTrack::Equipment:
		return TEXT("MASTERY_TRACK_EQUIPMENT");
	case EMasteryTrack::Abyss:
		return TEXT("MASTERY_TRACK_ABYSS");
	case EMasteryTrack::Rune:
		return TEXT("MASTERY_TRACK_RUNE");
	case EMasteryTrack::Beast:
		return TEXT("MASTERY_TRACK_BEAST");
	case EMasteryTrack::Explore:
		return TEXT("MASTERY_TRACK_EXPLORE");
	default:
		return TEXT("NONE_DASH");
	}
}

const TCHAR* MasteryLocalBonusFormatKey(EMasteryTrack Track)
{
	switch (Track)
	{
	case EMasteryTrack::Combat:
		return TEXT("MASTERY_LOCAL_BONUS_COMBAT_FORMAT");
	case EMasteryTrack::Equipment:
		return TEXT("MASTERY_LOCAL_BONUS_EQUIPMENT_FORMAT");
	case EMasteryTrack::Abyss:
		return TEXT("MASTERY_LOCAL_BONUS_ABYSS_FORMAT");
	case EMasteryTrack::Rune:
		return TEXT("MASTERY_LOCAL_BONUS_RUNE_FORMAT");
	case EMasteryTrack::Beast:
		return TEXT("MASTERY_LOCAL_BONUS_BEAST_FORMAT");
	case EMasteryTrack::Explore:
		return TEXT("MASTERY_LOCAL_BONUS_EXPLORE_FORMAT");
	default:
		return TEXT("NONE_DASH");
	}
}

const TCHAR* MasteryLocalBonusTooltipKey(EMasteryTrack Track)
{
	switch (Track)
	{
	case EMasteryTrack::Combat:
		return TEXT("MASTERY_LOCAL_BONUS_COMBAT_TOOLTIP");
	case EMasteryTrack::Equipment:
		return TEXT("MASTERY_LOCAL_BONUS_EQUIPMENT_TOOLTIP");
	case EMasteryTrack::Abyss:
		return TEXT("MASTERY_LOCAL_BONUS_ABYSS_TOOLTIP");
	case EMasteryTrack::Rune:
		return TEXT("MASTERY_LOCAL_BONUS_RUNE_TOOLTIP");
	case EMasteryTrack::Beast:
		return TEXT("MASTERY_LOCAL_BONUS_BEAST_TOOLTIP");
	case EMasteryTrack::Explore:
		return TEXT("MASTERY_LOCAL_BONUS_EXPLORE_TOOLTIP");
	default:
		return TEXT("MASTERY_TOOLTIP_LOCKED");
	}
}

// V2(2종) 로컬 보너스 포맷 키. 심연은 던전 입장 +N(정수), 나머지는 % 포맷.
const TCHAR* MasteryLocalBonus2FormatKey(EMasteryTrack Track)
{
	switch (Track)
	{
	case EMasteryTrack::Combat:
		return TEXT("MASTERY_LOCAL_BONUS2_COMBAT_FORMAT");
	case EMasteryTrack::Equipment:
		return TEXT("MASTERY_LOCAL_BONUS2_EQUIPMENT_FORMAT");
	case EMasteryTrack::Abyss:
		return TEXT("MASTERY_LOCAL_BONUS2_ABYSS_FORMAT");
	case EMasteryTrack::Rune:
		return TEXT("MASTERY_LOCAL_BONUS2_RUNE_FORMAT");
	case EMasteryTrack::Beast:
		return TEXT("MASTERY_LOCAL_BONUS2_BEAST_FORMAT");
	case EMasteryTrack::Explore:
		return TEXT("MASTERY_LOCAL_BONUS2_EXPLORE_FORMAT");
	default:
		return TEXT("NONE_DASH");
	}
}

FName MakeConsumableUseHitBoxName(EConsumableType Type, EConsumableGrade Grade)
{
	return FName(*FString::Printf(TEXT("%s%d_%d"), *ConsumableUseHitBoxPrefix, static_cast<int32>(Type), static_cast<int32>(Grade)));
}

bool ParseConsumableUseHitBoxName(FName BoxName, EConsumableType& OutType, EConsumableGrade& OutGrade)
{
	const FString Raw = BoxName.ToString();
	if (!Raw.StartsWith(ConsumableUseHitBoxPrefix))
	{
		return false;
	}

	const FString Payload = Raw.RightChop(ConsumableUseHitBoxPrefix.Len());
	FString TypeText;
	FString GradeText;
	if (!Payload.Split(TEXT("_"), &TypeText, &GradeText))
	{
		return false;
	}

	int32 ParsedType = INDEX_NONE;
	int32 ParsedGrade = INDEX_NONE;
	if (!LexTryParseString(ParsedType, *TypeText) || !LexTryParseString(ParsedGrade, *GradeText))
	{
		return false;
	}

	OutType = static_cast<EConsumableType>(ParsedType);
	OutGrade = static_cast<EConsumableGrade>(ParsedGrade);
	return true;
}

const TCHAR* RarityToLocalizationKey(EItemRarity Rarity)
{
	switch (Rarity)
	{
	case EItemRarity::Common:
		return TEXT("RARITY_COMMON");
	case EItemRarity::Rare:
		return TEXT("RARITY_RARE");
	case EItemRarity::Epic:
		return TEXT("RARITY_EPIC");
	case EItemRarity::Unique:
		return TEXT("RARITY_UNIQUE");
	case EItemRarity::Legendary:
		return TEXT("RARITY_LEGENDARY");
	case EItemRarity::Transcendent:
		return TEXT("RARITY_TRANSCENDENT");
	case EItemRarity::Mythic:
		return TEXT("RARITY_MYTHIC");
	case EItemRarity::None:
	default:
		return TEXT("RARITY_NONE");
	}
}

const TCHAR* PotentialGradeToLocalizationKey(EPotentialGrade Grade)
{
	switch (Grade)
	{
	case EPotentialGrade::Rare:
		return TEXT("RARITY_RARE");
	case EPotentialGrade::Epic:
		return TEXT("RARITY_EPIC");
	case EPotentialGrade::Unique:
		return TEXT("RARITY_UNIQUE");
	case EPotentialGrade::Legendary:
		return TEXT("RARITY_LEGENDARY");
	case EPotentialGrade::Transcendent:
		return TEXT("RARITY_TRANSCENDENT");
	case EPotentialGrade::None:
	default:
		return TEXT("RARITY_NONE");
	}
}

FText PotentialGradeToLabel(EPotentialGrade Grade)
{
	return IdleProject::Localization::UI(PotentialGradeToLocalizationKey(Grade));
}

FLinearColor PotentialGradeToColor(EPotentialGrade Grade)
{
	using namespace IdleProject::UI::Theme;

	switch (Grade)
	{
	case EPotentialGrade::Rare:
		return RarityRare;
	case EPotentialGrade::Epic:
		return RarityEpic;
	case EPotentialGrade::Unique:
		return RarityUnique;
	case EPotentialGrade::Legendary:
		return RarityLegendary;
	case EPotentialGrade::Transcendent:
		return RarityTranscendent;
	case EPotentialGrade::None:
	default:
		return TextMuted;
	}
}

const TCHAR* PotentialStatToLocalizationKey(EPotentialStat Stat)
{
	switch (Stat)
	{
	case EPotentialStat::PhysAtkPercent:
		return TEXT("POTENTIAL_STAT_PHYS_ATK");
	case EPotentialStat::MagicAtkPercent:
		return TEXT("POTENTIAL_STAT_MAGIC_ATK");
	case EPotentialStat::HpPercent:
		return TEXT("POTENTIAL_STAT_HP");
	case EPotentialStat::PhysDefPercent:
		return TEXT("POTENTIAL_STAT_PHYS_DEF");
	case EPotentialStat::MagicDefPercent:
		return TEXT("POTENTIAL_STAT_MAGIC_DEF");
	case EPotentialStat::CritRatePercent:
		return TEXT("POTENTIAL_STAT_CRIT_RATE");
	case EPotentialStat::AtkSpeedPercent:
		return TEXT("POTENTIAL_STAT_ATK_SPEED");
	case EPotentialStat::CritDmgPercent:
		return TEXT("POTENTIAL_STAT_CRIT_DMG");
	case EPotentialStat::AllStatPercent:
		return TEXT("POTENTIAL_STAT_ALL_STAT");
	case EPotentialStat::GoldFindPercent:
		return TEXT("POTENTIAL_STAT_GOLD_FIND");
	case EPotentialStat::DropRatePercent:
		return TEXT("POTENTIAL_STAT_DROP_RATE");
	case EPotentialStat::None:
	default:
		return TEXT("NONE_DASH");
	}
}

FText FormatPotentialLineLabel(const FPotentialLine& Line)
{
	if (Line.Stat == EPotentialStat::None || Line.Value <= 0.0f)
	{
		return FText::GetEmpty();
	}

	return FormatLocalizedUI(TEXT("POTENTIAL_LINE_PERCENT_FORMAT"), [&Line](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Stat"), IdleProject::Localization::UI(PotentialStatToLocalizationKey(Line.Stat)));
		Args.Add(TEXT("Percent"), FText::AsNumber(FMath::RoundToInt(Line.Value * 100.0f)));
	});
}

FText BuildPotentialLineSummary(const FItemInstance& Item)
{
	TArray<FString> Parts;
	for (const FPotentialLine& Line : { Item.PotentialLine1, Item.PotentialLine2, Item.PotentialLine3, Item.PotentialLine4 })
	{
		const FText Label = FormatPotentialLineLabel(Line);
		if (!Label.IsEmpty())
		{
			Parts.Add(Label.ToString());
		}
	}

	return Parts.Num() > 0
		? FText::FromString(FString::Join(Parts, TEXT(" / ")))
		: IdleProject::Localization::UI(TEXT("NONE_DASH"));
}

const TCHAR* UniqueTraitToLocalizationKey(EUniqueTrait Trait)
{
	switch (Trait)
	{
	case EUniqueTrait::AllStatSurge:
		return TEXT("UNIQUE_TRAIT_ALL_STAT_SURGE");
	case EUniqueTrait::CritDamageSurge:
		return TEXT("UNIQUE_TRAIT_CRIT_DAMAGE_SURGE");
	case EUniqueTrait::CritRateSurge:
		return TEXT("UNIQUE_TRAIT_CRIT_RATE_SURGE");
	case EUniqueTrait::LifeSurge:
		return TEXT("UNIQUE_TRAIT_LIFE_SURGE");
	case EUniqueTrait::SwiftSurge:
		return TEXT("UNIQUE_TRAIT_SWIFT_SURGE");
	case EUniqueTrait::PhysMastery:
		return TEXT("UNIQUE_TRAIT_PHYS_MASTERY");
	case EUniqueTrait::MagicMastery:
		return TEXT("UNIQUE_TRAIT_MAGIC_MASTERY");
	case EUniqueTrait::GuardMastery:
		return TEXT("UNIQUE_TRAIT_GUARD_MASTERY");
	case EUniqueTrait::None:
	default:
		return TEXT("NONE_DASH");
	}
}

bool UniqueTraitUsesFlatValue(EUniqueTrait Trait)
{
	return Trait == EUniqueTrait::SwiftSurge;
}

FText FormatUniqueTraitLabel(EUniqueTrait Trait, EItemRarity Rarity)
{
	const float Value = FUniqueTraitFormula::GetTraitValue(Trait, Rarity);
	if (Value <= 0.0f)
	{
		return FText::GetEmpty();
	}

	const FText TraitLabel = IdleProject::Localization::UI(UniqueTraitToLocalizationKey(Trait));
	if (UniqueTraitUsesFlatValue(Trait))
	{
		const FString FormattedValue = FString::Printf(TEXT("%.2f"), Value);
		return FormatLocalizedUI(TEXT("UNIQUE_TRAIT_FLAT_FORMAT"), [&TraitLabel, &FormattedValue](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Trait"), TraitLabel);
			Args.Add(TEXT("Value"), FText::FromString(FormattedValue));
		});
	}

	const int32 Percent = FMath::RoundToInt(Value * 100.0f);
	return FormatLocalizedUI(TEXT("UNIQUE_TRAIT_PERCENT_FORMAT"), [&TraitLabel, Percent](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Trait"), TraitLabel);
		Args.Add(TEXT("Percent"), FText::AsNumber(Percent));
	});
}

FText RuneText(const TCHAR* Key)
{
	return IdleProject::Localization::Text(TEXT("Rune"), Key);
}

FText FormatLocalizedRune(const TCHAR* Key, TFunctionRef<void(FFormatNamedArguments&)> BuildArgs)
{
	FFormatNamedArguments Args;
	BuildArgs(Args);
	return IdleProject::Localization::Text(TEXT("Rune"), Key, Args);
}

const TCHAR* RuneTypeToLocalizationKey(ERuneType Type)
{
	switch (Type)
	{
	case ERuneType::PhysAtk:
		return TEXT("RUNE_TYPE_PHYS_ATK");
	case ERuneType::MagicAtk:
		return TEXT("RUNE_TYPE_MAGIC_ATK");
	case ERuneType::PhysDef:
		return TEXT("RUNE_TYPE_PHYS_DEF");
	case ERuneType::MagicDef:
		return TEXT("RUNE_TYPE_MAGIC_DEF");
	case ERuneType::Hp:
		return TEXT("RUNE_TYPE_HP");
	case ERuneType::CritDamage:
		return TEXT("RUNE_TYPE_CRIT_DAMAGE");
	case ERuneType::GoldFind:
		return TEXT("RUNE_TYPE_GOLD_FIND");
	case ERuneType::ExpBoost:
		return TEXT("RUNE_TYPE_EXP_BOOST");
	case ERuneType::OfflineEff:
		return TEXT("RUNE_TYPE_OFFLINE_EFF");
	case ERuneType::ClassMastery:
		return TEXT("RUNE_TYPE_CLASS_MASTERY");
	case ERuneType::None:
	default:
		return TEXT("RUNE_TYPE_NONE");
	}
}

const TCHAR* RuneSetToLocalizationKey(ERuneSet RuneSet)
{
	switch (RuneSet)
	{
	case ERuneSet::Offense:
		return TEXT("RUNE_SET_OFFENSE");
	case ERuneSet::Bastion:
		return TEXT("RUNE_SET_BASTION");
	case ERuneSet::Vitality:
		return TEXT("RUNE_SET_VITALITY");
	case ERuneSet::Fortune:
		return TEXT("RUNE_SET_FORTUNE");
	case ERuneSet::None:
	default:
		return TEXT("RUNE_SET_NONE");
	}
}

FText RuneTypeToLabel(ERuneType Type)
{
	return RuneText(RuneTypeToLocalizationKey(Type));
}

FText RuneSetToLabel(ERuneSet RuneSet)
{
	return RuneText(RuneSetToLocalizationKey(RuneSet));
}

FText FormatRuneSetTierLabel(int32 Count)
{
	if (Count < FRuneSetFormula::Tier1Count)
	{
		return RuneText(TEXT("RUNE_SET_NONE"));
	}

	const int32 ActiveTier = Count >= FRuneSetFormula::Tier3Count
		? FRuneSetFormula::Tier3Count
		: (Count >= FRuneSetFormula::Tier2Count ? FRuneSetFormula::Tier2Count : FRuneSetFormula::Tier1Count);
	return FormatLocalizedRune(TEXT("RUNE_SET_TIER_FORMAT"), [ActiveTier](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Tier"), FText::AsNumber(ActiveTier));
	});
}

FText FormatRuneSetNextTierLabel(int32 Count)
{
	if (Count >= FRuneSetFormula::Tier3Count)
	{
		return RuneText(TEXT("RUNE_SET_COMPLETE"));
	}

	const int32 NextTier = Count >= FRuneSetFormula::Tier2Count
		? FRuneSetFormula::Tier3Count
		: (Count >= FRuneSetFormula::Tier1Count ? FRuneSetFormula::Tier2Count : FRuneSetFormula::Tier1Count);
	const int32 Needed = FMath::Max(0, NextTier - Count);
	return FormatLocalizedRune(TEXT("RUNE_SET_NEXT_FORMAT"), [NextTier, Needed](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Tier"), FText::AsNumber(NextTier));
		Args.Add(TEXT("Needed"), FText::AsNumber(Needed));
	});
}

FText FormatRuneSetBonusLabel(ERuneSet RuneSet, int32 Count)
{
	const int32 Percent = FMath::RoundToInt(FRuneSetFormula::GetSetTierBonus(Count) * 100.0f);
	if (Percent <= 0)
	{
		return RuneText(TEXT("RUNE_SET_BONUS_NONE"));
	}

	switch (RuneSet)
	{
	case ERuneSet::Offense:
		return FormatLocalizedRune(TEXT("RUNE_SET_BONUS_DUAL_FORMAT"), [Percent](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("StatA"), RuneTypeToLabel(ERuneType::PhysAtk));
			Args.Add(TEXT("StatB"), RuneTypeToLabel(ERuneType::MagicAtk));
			Args.Add(TEXT("Percent"), FText::AsNumber(Percent));
		});
	case ERuneSet::Bastion:
		return FormatLocalizedRune(TEXT("RUNE_SET_BONUS_DUAL_FORMAT"), [Percent](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("StatA"), RuneTypeToLabel(ERuneType::PhysDef));
			Args.Add(TEXT("StatB"), RuneTypeToLabel(ERuneType::MagicDef));
			Args.Add(TEXT("Percent"), FText::AsNumber(Percent));
		});
	case ERuneSet::Vitality:
		return FormatLocalizedRune(TEXT("RUNE_SET_BONUS_DUAL_FORMAT"), [Percent](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("StatA"), RuneTypeToLabel(ERuneType::Hp));
			Args.Add(TEXT("StatB"), RuneTypeToLabel(ERuneType::OfflineEff));
			Args.Add(TEXT("Percent"), FText::AsNumber(Percent));
		});
	case ERuneSet::Fortune:
		return FormatLocalizedRune(TEXT("RUNE_SET_BONUS_TRIPLE_FORMAT"), [Percent](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("StatA"), RuneTypeToLabel(ERuneType::GoldFind));
			Args.Add(TEXT("StatB"), RuneTypeToLabel(ERuneType::ExpBoost));
			Args.Add(TEXT("StatC"), RuneTypeToLabel(ERuneType::CritDamage));
			Args.Add(TEXT("Percent"), FText::AsNumber(Percent));
		});
	case ERuneSet::None:
	default:
		return RuneText(TEXT("RUNE_SET_BONUS_NONE"));
	}
}

const TCHAR* ClassToLocalizationKey(EClassId ClassId);

FText ClassRuneTypeToLabel(const FRuneInstance& Rune)
{
	if (Rune.RuneType != ERuneType::ClassMastery)
	{
		return RuneTypeToLabel(Rune.RuneType);
	}

	return FormatLocalizedRune(TEXT("RUNE_CLASS_MASTERY_FORMAT"), [&Rune](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Class"), IdleProject::Localization::UI(ClassToLocalizationKey(Rune.ClassRestriction)));
	});
}

bool CanEquipRuneIntoSlot(const TArray<FRuneInstance>& OwnedRunes, int32 OwnedIndex, int32 SlotIndex, EClassId OwnerClassId)
{
	if (!OwnedRunes.IsValidIndex(OwnedIndex))
	{
		return false;
	}

	const FRuneInstance& Rune = OwnedRunes[OwnedIndex];
	if (SlotIndex == FClassRuneFormula::ClassRuneSlotIndex)
	{
		return Rune.RuneType == ERuneType::ClassMastery
			&& OwnerClassId != EClassId::None
			&& Rune.ClassRestriction == OwnerClassId;
	}

	return Rune.RuneType != ERuneType::ClassMastery;
}

EItemRarity RarityFromCodexRow(int32 RowIndex)
{
	return static_cast<EItemRarity>(static_cast<int32>(EItemRarity::Common) + RowIndex);
}

bool IsCodexRuneType(ERuneType Type)
{
	return FRuneFormula::IsCoreType(Type) || FRuneFormula::IsUtilType(Type);
}

int32 GetCodexColumnIndex(ERuneType Type)
{
	return static_cast<int32>(Type) - static_cast<int32>(ERuneType::PhysAtk);
}

int32 GetCodexCellKey(ERuneType Type, EItemRarity Rarity)
{
	return GetCodexColumnIndex(Type) * 7 + (static_cast<int32>(Rarity) - static_cast<int32>(EItemRarity::Common));
}

FText FormatRuneCodexPercentLabel(const TCHAR* Key, float Value)
{
	const int32 Percent = FMath::RoundToInt(FMath::Max(0.0f, Value) * 100.0f);
	return FormatLocalizedRune(Key, [Percent](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Percent"), FText::AsNumber(Percent));
	});
}

float GetRuneDisplayValue(const FRuneInstance& Rune)
{
	if (FRuneFormula::IsCoreType(Rune.RuneType))
	{
		return FRuneFormula::GetCoreRuneMultiplier(Rune.Rarity, Rune.EnhanceLevel);
	}
	if (FRuneFormula::IsUtilType(Rune.RuneType))
	{
		return FRuneFormula::GetUtilRuneValue(Rune.RuneType, Rune.Rarity, Rune.EnhanceLevel);
	}
	if (Rune.RuneType == ERuneType::ClassMastery)
	{
		const FRuneCoreMultipliers Multipliers = FClassRuneFormula::GetClassMasteryMultipliers(Rune.ClassRestriction, Rune.Rarity, Rune.EnhanceLevel);
		return FMath::Max(FMath::Max(Multipliers.PhysAtk, Multipliers.MagicAtk), FMath::Max(Multipliers.PhysDef, Multipliers.Hp));
	}
	return 0.0f;
}

FText FormatRuneValueLabel(const FRuneInstance& Rune)
{
	const int32 Percent = FMath::RoundToInt(GetRuneDisplayValue(Rune) * 100.0f);
	return FormatLocalizedRune(TEXT("RUNE_VALUE_PERCENT_FORMAT"), [Percent](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Percent"), FText::AsNumber(Percent));
	});
}

FText FormatRuneEnhancePreviewLabel(const FRuneInstance& Rune)
{
	FRuneInstance NextRune = Rune;
	NextRune.EnhanceLevel = FMath::Max(0, Rune.EnhanceLevel) + 1;
	const int32 CurrentPercent = FMath::RoundToInt(GetRuneDisplayValue(Rune) * 100.0f);
	const int32 NextPercent = FMath::RoundToInt(GetRuneDisplayValue(NextRune) * 100.0f);
	return FormatLocalizedRune(TEXT("RUNE_ENHANCE_PREVIEW_FORMAT"), [CurrentPercent, NextPercent](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("CurrentPercent"), FText::AsNumber(CurrentPercent));
		Args.Add(TEXT("NextPercent"), FText::AsNumber(NextPercent));
	});
}

FText QuestTypeToLabel(EQuestType Type)
{
	switch (Type)
	{
	case EQuestType::Main:
		return IdleProject::Localization::UI(TEXT("QUEST_TYPE_MAIN"));
	case EQuestType::Weekly:
		return IdleProject::Localization::UI(TEXT("QUEST_TYPE_WEEKLY"));
	case EQuestType::Daily:
	default:
		return IdleProject::Localization::UI(TEXT("QUEST_TYPE_DAILY"));
	}
}

FLinearColor QuestTypeToAccentColor(EQuestType Type)
{
	switch (Type)
	{
	case EQuestType::Main:
		return IdleProject::UI::Theme::AccentBlue;
	case EQuestType::Weekly:
		return IdleProject::UI::Theme::AccentGold;
	case EQuestType::Daily:
	default:
		return IdleProject::UI::Theme::TextMuted;
	}
}

const TCHAR* PetIdToNameLocalizationKey(const FString& PetId)
{
	if (PetId == TEXT("dog"))
	{
		return TEXT("PET_NAME_DOG");
	}
	if (PetId == TEXT("bird"))
	{
		return TEXT("PET_NAME_BIRD");
	}
	if (PetId == TEXT("cat"))
	{
		return TEXT("PET_NAME_CAT");
	}
	if (PetId == TEXT("wolf"))
	{
		return TEXT("PET_NAME_WOLF");
	}
	if (PetId == TEXT("owl"))
	{
		return TEXT("PET_NAME_OWL");
	}
	if (PetId == TEXT("bear"))
	{
		return TEXT("PET_NAME_BEAR");
	}
	if (PetId == TEXT("turtle"))
	{
		return TEXT("PET_NAME_TURTLE");
	}
	if (PetId == TEXT("fox"))
	{
		return TEXT("PET_NAME_FOX");
	}
	if (PetId == TEXT("rabbit"))
	{
		return TEXT("PET_NAME_RABBIT");
	}
	if (PetId == TEXT("dragon"))
	{
		return TEXT("PET_NAME_DRAGON");
	}
	return TEXT("NONE_DASH");
}

FText PetIdToNameLabel(const FString& PetId)
{
	return IdleProject::Localization::UI(PetIdToNameLocalizationKey(PetId));
}

FText PetBonusTypeToLabel(EPetBonusType Type, float BonusPercent)
{
	const FString Percent = FString::Printf(TEXT("%.0f%%"), BonusPercent);
	auto FormatPetBonus = [&Percent](const TCHAR* Key)
	{
		return FormatLocalizedUI(Key, [&Percent](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Percent"), FText::FromString(Percent));
		});
	};

	switch (Type)
	{
	case EPetBonusType::Gold:
		return FormatPetBonus(TEXT("PET_BONUS_GOLD_FORMAT"));
	case EPetBonusType::Drop:
		return FormatPetBonus(TEXT("PET_BONUS_DROP_FORMAT"));
	case EPetBonusType::Exp:
		return FormatPetBonus(TEXT("PET_BONUS_EXP_FORMAT"));
	case EPetBonusType::PhysAtk:
		return FormatPetBonus(TEXT("PET_BONUS_PHYS_ATK_FORMAT"));
	case EPetBonusType::MagicAtk:
		return FormatPetBonus(TEXT("PET_BONUS_MAGIC_ATK_FORMAT"));
	case EPetBonusType::Hp:
		return FormatPetBonus(TEXT("PET_BONUS_HP_FORMAT"));
	case EPetBonusType::Def:
		return FormatPetBonus(TEXT("PET_BONUS_DEF_FORMAT"));
	case EPetBonusType::AllStat:
		return FormatPetBonus(TEXT("PET_BONUS_ALL_STAT_FORMAT"));
	case EPetBonusType::None:
	default:
		return IdleProject::Localization::UI(TEXT("NONE_DASH"));
	}
}

// 칭호 id → 이름 로컬키 매핑(서버 카탈로그 18종). TitleService 의 Name 은 id 미러이므로 UI 가 키로 변환한다.
const TCHAR* TitleIdToNameLocalizationKey(const FString& TitleId)
{
	static const TMap<FString, const TCHAR*> Map = {
		{ TEXT("monster_hunter"), TEXT("TITLE_NAME_MONSTER_HUNTER") },
		{ TEXT("boss_slayer"), TEXT("TITLE_NAME_BOSS_SLAYER") },
		{ TEXT("monster_annihilator"), TEXT("TITLE_NAME_MONSTER_ANNIHILATOR") },
		{ TEXT("boss_executioner"), TEXT("TITLE_NAME_BOSS_EXECUTIONER") },
		{ TEXT("rebirth_master"), TEXT("TITLE_NAME_REBIRTH_MASTER") },
		{ TEXT("transcendent"), TEXT("TITLE_NAME_TRANSCENDENT") },
		{ TEXT("stage_conqueror"), TEXT("TITLE_NAME_STAGE_CONQUEROR") },
		{ TEXT("level_legend"), TEXT("TITLE_NAME_LEVEL_LEGEND") },
		{ TEXT("tower_conqueror"), TEXT("TITLE_NAME_TOWER_CONQUEROR") },
		{ TEXT("tower_overlord"), TEXT("TITLE_NAME_TOWER_OVERLORD") },
		{ TEXT("collector"), TEXT("TITLE_NAME_COLLECTOR") },
		{ TEXT("unique_seeker"), TEXT("TITLE_NAME_UNIQUE_SEEKER") },
		{ TEXT("gold_king"), TEXT("TITLE_NAME_GOLD_KING") },
		{ TEXT("pet_whisperer"), TEXT("TITLE_NAME_PET_WHISPERER") },
		{ TEXT("quest_champion"), TEXT("TITLE_NAME_QUEST_CHAMPION") },
		{ TEXT("enhance_artisan"), TEXT("TITLE_NAME_ENHANCE_ARTISAN") },
		{ TEXT("enhance_grandmaster"), TEXT("TITLE_NAME_ENHANCE_GRANDMASTER") },
		{ TEXT("veteran"), TEXT("TITLE_NAME_VETERAN") },
	};
	if (const TCHAR* const* Found = Map.Find(TitleId))
	{
		return *Found;
	}
	return TEXT("NONE_DASH");
}

FText TitleIdToNameLabel(const FString& TitleId)
{
	return IdleProject::Localization::UI(TitleIdToNameLocalizationKey(TitleId));
}

// 칭호 해금 메트릭 → 표시명 로컬키. 카탈로그에서 실제 사용하는 메트릭만 매핑(그 외는 대시).
const TCHAR* TitleMetricToLocalizationKey(EAchievementMetric Metric)
{
	switch (Metric)
	{
	case EAchievementMetric::MonstersKilled:
		return TEXT("TITLE_METRIC_MONSTERS_KILLED");
	case EAchievementMetric::BossesKilled:
		return TEXT("TITLE_METRIC_BOSSES_KILLED");
	case EAchievementMetric::HighestLevelReached:
		return TEXT("TITLE_METRIC_HIGHEST_LEVEL");
	case EAchievementMetric::StagesCleared:
		return TEXT("TITLE_METRIC_STAGES_CLEARED");
	case EAchievementMetric::RebirthCount:
		return TEXT("TITLE_METRIC_REBIRTH_COUNT");
	case EAchievementMetric::TranscendCount:
		return TEXT("TITLE_METRIC_TRANSCEND_COUNT");
	case EAchievementMetric::TowerHighestFloor:
		return TEXT("TITLE_METRIC_TOWER_FLOOR");
	case EAchievementMetric::HighestEnhanceLevel:
		return TEXT("TITLE_METRIC_HIGHEST_ENHANCE");
	case EAchievementMetric::ItemsCollected:
		return TEXT("TITLE_METRIC_ITEMS_COLLECTED");
	case EAchievementMetric::UniqueItemsFound:
		return TEXT("TITLE_METRIC_UNIQUE_ITEMS");
	case EAchievementMetric::GoldEarned:
		return TEXT("TITLE_METRIC_GOLD_EARNED");
	case EAchievementMetric::HighestPetLevel:
		return TEXT("TITLE_METRIC_HIGHEST_PET_LEVEL");
	case EAchievementMetric::QuestsCompleted:
		return TEXT("TITLE_METRIC_QUESTS_COMPLETED");
	case EAchievementMetric::DaysPlayed:
		return TEXT("TITLE_METRIC_DAYS_PLAYED");
	default:
		return TEXT("NONE_DASH");
	}
}

// 칭호 보너스(타입+비율) → 표시 라벨. BonusValue 는 비율(0.03)이므로 ×100 해 %로 표기.
FText TitleBonusToLabel(ETitleBonus Type, float BonusValue)
{
	const FString Percent = FString::Printf(TEXT("%.0f%%"), BonusValue * 100.0f);
	auto FormatTitleBonus = [&Percent](const TCHAR* Key)
	{
		return FormatLocalizedUI(Key, [&Percent](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Percent"), FText::FromString(Percent));
		});
	};

	switch (Type)
	{
	case ETitleBonus::AllStatPct:
		return FormatTitleBonus(TEXT("TITLE_BONUS_ALL_STAT_FORMAT"));
	case ETitleBonus::GoldPct:
		return FormatTitleBonus(TEXT("TITLE_BONUS_GOLD_FORMAT"));
	case ETitleBonus::ExpPct:
		return FormatTitleBonus(TEXT("TITLE_BONUS_EXP_FORMAT"));
	case ETitleBonus::CritDmgPct:
		return FormatTitleBonus(TEXT("TITLE_BONUS_CRIT_DMG_FORMAT"));
	case ETitleBonus::None:
	default:
		return IdleProject::Localization::UI(TEXT("NONE_DASH"));
	}
}

FText BuildPetLevelLabel(int32 Level)
{
	return FormatLocalizedUI(TEXT("PET_LEVEL_FORMAT"), [Level](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Level"), FText::AsNumber(Level));
		Args.Add(TEXT("MaxLevel"), FText::AsNumber(FPetLevelFormula::MaxPetLevel));
	});
}

FText BuildPetFeedCostLabel(int64 FeedCost)
{
	if (FeedCost <= 0)
	{
		return IdleProject::Localization::UI(TEXT("PET_FEED_COST_MAX"));
	}

	return FormatLocalizedUI(TEXT("PET_FEED_COST_FORMAT"), [FeedCost](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Gold"), FText::FromString(FormatIntegerWithCommas(FeedCost)));
	});
}

FText BuildPetStarLabel(int32 Star)
{
	return FormatLocalizedUI(TEXT("PET_STAR_FORMAT"), [Star](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Star"), FText::AsNumber(FMath::Max(0, Star)));
	});
}

FText BuildPetEvolveCostLabel(int64 EvolveCost)
{
	return FormatLocalizedUI(TEXT("PET_EVOLVE_COST_FORMAT"), [EvolveCost](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Gold"), FText::FromString(FormatIntegerWithCommas(EvolveCost)));
	});
}

FText BuildPetEvolveEffectLabel(int32 NextStar)
{
	const float NextBonusPercent = (FPetLevelFormula::GetPetStarMultiplier(NextStar) - 1.0f) * 100.0f;
	return FormatLocalizedUI(TEXT("PET_EVOLVE_EFFECT_FORMAT"), [NextBonusPercent](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Percent"), FText::FromString(FString::Printf(TEXT("%.0f%%"), NextBonusPercent)));
	});
}

FText SeasonRewardToLabel(ESeasonRewardType Type, int64 Amount)
{
	switch (Type)
	{
	case ESeasonRewardType::Gold:
		return FormatLocalizedUI(TEXT("SEASON_REWARD_GOLD_FORMAT"), [Amount](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Amount"), FText::FromString(FormatIntegerWithCommas(Amount)));
		});
	case ESeasonRewardType::Exp:
		return FormatLocalizedUI(TEXT("SEASON_REWARD_EXP_FORMAT"), [Amount](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Amount"), FText::FromString(FormatIntegerWithCommas(Amount)));
		});
	case ESeasonRewardType::None:
	default:
		return IdleProject::Localization::UI(TEXT("NONE_DASH"));
	}
}

FText StageWeakElementToLabel(ESkillElement Element)
{
	switch (Element)
	{
	case ESkillElement::Fire:
		return IdleProject::Localization::UI(TEXT("ELEMENT_FIRE"));
	case ESkillElement::Ice:
		return IdleProject::Localization::UI(TEXT("ELEMENT_ICE"));
	case ESkillElement::Lightning:
		return IdleProject::Localization::UI(TEXT("ELEMENT_LIGHTNING"));
	case ESkillElement::Holy:
		return IdleProject::Localization::UI(TEXT("ELEMENT_HOLY"));
	case ESkillElement::Dark:
		return IdleProject::Localization::UI(TEXT("ELEMENT_DARK"));
	case ESkillElement::None:
	default:
		return IdleProject::Localization::UI(TEXT("ELEMENT_NONE"));
	}
}

FText StageWeakElementToIconLabel(ESkillElement Element)
{
	switch (Element)
	{
	case ESkillElement::Fire:
		return FText::FromString(TEXT("F"));
	case ESkillElement::Ice:
		return FText::FromString(TEXT("I"));
	case ESkillElement::Lightning:
		return FText::FromString(TEXT("L"));
	case ESkillElement::Holy:
		return FText::FromString(TEXT("H"));
	case ESkillElement::Dark:
		return FText::FromString(TEXT("D"));
	case ESkillElement::None:
	default:
		return FText::FromString(TEXT("-"));
	}
}

FLinearColor StageWeakElementToColor(ESkillElement Element)
{
	using namespace IdleProject::UI::Theme;

	switch (Element)
	{
	case ESkillElement::Fire:
		return AccentRed;
	case ESkillElement::Ice:
		return AccentBlue;
	case ESkillElement::Lightning:
		return Warn;
	case ESkillElement::Holy:
		return AccentGold;
	case ESkillElement::Dark:
		return ElementDark;
	case ESkillElement::None:
	default:
		return TextMuted;
	}
}

FLinearColor BossPhaseToColor(int32 Phase)
{
	using namespace IdleProject::UI::Theme;

	if (Phase >= 3)
	{
		return AccentRed;
	}
	if (Phase == 2)
	{
		return Warn;
	}
	return AccentGold;
}

FName MakeQuestClaimHitBoxName(const FString& QuestId)
{
	return FName(*(QuestClaimHitBoxPrefix + QuestId));
}

FName MakeClassSelectionHitBoxName(EClassId ClassId)
{
	return FName(*(ClassSelectionHitBoxPrefix + FString::FromInt(static_cast<int32>(ClassId))));
}

FName MakePetEquipHitBoxName(const FString& PetId)
{
	return FName(*(PetEquipHitBoxPrefix + PetId));
}

FName MakePetFeedHitBoxName(const FString& PetId)
{
	return FName(*(PetFeedHitBoxPrefix + PetId));
}

FName MakePetEvolveHitBoxName(const FString& PetId)
{
	return FName(*(PetEvolveHitBoxPrefix + PetId));
}

FName MakeTitleEquipHitBoxName(const FString& TitleId)
{
	return FName(*(TitleEquipHitBoxPrefix + TitleId));
}

FName MakeMissionClaimHitBoxName(const FString& MissionId)
{
	return FName(*(MissionClaimHitBoxPrefix + MissionId));
}

// 미션 메트릭(처치/보스/스테이지/던전/강화/골드) → 표시명 로컬 키.
const TCHAR* MissionMetricToLocalizationKey(EMissionMetric Metric)
{
	switch (Metric)
	{
	case EMissionMetric::MonstersKilled: return TEXT("MISSION_METRIC_MONSTERS_KILLED");
	case EMissionMetric::BossesKilled:   return TEXT("MISSION_METRIC_BOSSES_KILLED");
	case EMissionMetric::StagesCleared:  return TEXT("MISSION_METRIC_STAGES_CLEARED");
	case EMissionMetric::DungeonRuns:    return TEXT("MISSION_METRIC_DUNGEON_RUNS");
	case EMissionMetric::GearEnhanced:   return TEXT("MISSION_METRIC_GEAR_ENHANCED");
	case EMissionMetric::GoldEarned:     return TEXT("MISSION_METRIC_GOLD_EARNED");
	default:                             return TEXT("MISSION_METRIC_MONSTERS_KILLED");
	}
}

// 미션 보상 타입(골드/룬정수/소비) → 보상 라벨 포맷 로컬 키({Amount} 인자).
const TCHAR* MissionRewardToLocalizationKey(EMissionReward RewardType)
{
	switch (RewardType)
	{
	case EMissionReward::Gold:       return TEXT("MISSION_REWARD_GOLD_FORMAT");
	case EMissionReward::Essence:    return TEXT("MISSION_REWARD_ESSENCE_FORMAT");
	case EMissionReward::Consumable: return TEXT("MISSION_REWARD_CONSUMABLE_FORMAT");
	default:                         return TEXT("MISSION_REWARD_GOLD_FORMAT");
	}
}

FName MakeSeasonClaimHitBoxName(int32 Tier)
{
	return FName(*(SeasonClaimHitBoxPrefix + FString::FromInt(Tier)));
}

FName MakeSkillRankHitBoxName(FName SkillId)
{
	return FName(*(SkillRankHitBoxPrefix + SkillId.ToString()));
}

FName MakeEnhanceSlotHitBoxName(EItemSlot Slot)
{
	return FName(*(EnhanceSlotHitBoxPrefix + FString::FromInt(static_cast<int32>(Slot))));
}

FName MakeEnhanceProtectHitBoxName(EItemSlot Slot)
{
	return FName(*(EnhanceProtectHitBoxPrefix + FString::FromInt(static_cast<int32>(Slot))));
}

FName MakePotentialResetHitBoxName(EItemSlot Slot)
{
	return FName(*(PotentialResetHitBoxPrefix + FString::FromInt(static_cast<int32>(Slot))));
}

FName MakePotentialRankHitBoxName(EItemSlot Slot)
{
	return FName(*(PotentialRankHitBoxPrefix + FString::FromInt(static_cast<int32>(Slot))));
}

FName MakePotentialLockHitBoxName(EItemSlot Slot)
{
	return FName(*(PotentialLockHitBoxPrefix + FString::FromInt(static_cast<int32>(Slot))));
}

FName MakeRuneSelectHitBoxName(int32 OwnedIndex)
{
	return FName(*(RuneSelectHitBoxPrefix + FString::FromInt(OwnedIndex)));
}

FName MakeRuneEquipHitBoxName(int32 SlotIndex)
{
	return FName(*(RuneEquipHitBoxPrefix + FString::FromInt(SlotIndex)));
}

FName MakeRuneUnequipHitBoxName(int32 SlotIndex)
{
	return FName(*(RuneUnequipHitBoxPrefix + FString::FromInt(SlotIndex)));
}

FName MakeRuneEnhanceHitBoxName(int32 OwnedIndex)
{
	return FName(*(RuneEnhanceHitBoxPrefix + FString::FromInt(OwnedIndex)));
}

FName MakeRuneDisenchantHitBoxName(int32 OwnedIndex)
{
	return FName(*(RuneDisenchantHitBoxPrefix + FString::FromInt(OwnedIndex)));
}

FName MakeStatAllocationHitBoxName(EPrimaryStat Stat)
{
	return FName(*(StatAllocationHitBoxPrefix + FString::FromInt(static_cast<int32>(Stat))));
}

FName MakeDungeonEnterHitBoxName(EDungeonType Type)
{
	return FName(*(DungeonEnterHitBoxPrefix + FString::FromInt(static_cast<int32>(Type))));
}

FName MakeDungeonEnterHitBoxName(EDungeonType Type, int32 Tier)
{
	return FName(*(DungeonEnterHitBoxPrefix + FString::FromInt(static_cast<int32>(Type)) + TEXT("_") + FString::FromInt(FMath::Max(1, Tier))));
}

EDungeonType DungeonTypeFromHitBoxName(FName BoxName)
{
	FString RawType = BoxName.ToString();
	RawType.RightChopInline(DungeonEnterHitBoxPrefix.Len());
	int32 DelimiterIndex = INDEX_NONE;
	if (RawType.FindChar(TEXT('_'), DelimiterIndex))
	{
		RawType.LeftInline(DelimiterIndex);
	}
	return static_cast<EDungeonType>(FCString::Atoi(*RawType));
}

int32 DungeonTierFromHitBoxName(FName BoxName)
{
	FString RawTier = BoxName.ToString();
	RawTier.RightChopInline(DungeonEnterHitBoxPrefix.Len());
	int32 DelimiterIndex = INDEX_NONE;
	if (!RawTier.FindChar(TEXT('_'), DelimiterIndex))
	{
		return 1;
	}
	RawTier.RightChopInline(DelimiterIndex + 1);
	return FMath::Max(1, FCString::Atoi(*RawTier));
}

const TCHAR* DungeonTypeToLocalizationKey(EDungeonType Type)
{
	switch (Type)
	{
	case EDungeonType::Gold:
		return TEXT("DUNGEON_GOLD");
	case EDungeonType::Exp:
		return TEXT("DUNGEON_EXP");
	case EDungeonType::Essence:
		return TEXT("DUNGEON_ESSENCE");
	case EDungeonType::None:
	default:
		return TEXT("NONE_DASH");
	}
}

FText BuildDungeonRewardLabel(const FDungeonRunResult& Reward)
{
	if (Reward.GoldReward > 0)
	{
		return FormatLocalizedUI(TEXT("DUNGEON_REWARD_FORMAT"), [&Reward](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Reward"), FormatLocalizedUIWithInt64(TEXT("REWARD_GOLD_PLUS_FORMAT"), TEXT("Amount"), Reward.GoldReward));
		});
	}
	if (Reward.ExpReward > 0)
	{
		return FormatLocalizedUI(TEXT("DUNGEON_REWARD_FORMAT"), [&Reward](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Reward"), FormatLocalizedUIWithInt64(TEXT("REWARD_EXP_PLUS_FORMAT"), TEXT("Amount"), Reward.ExpReward));
		});
	}
	if (Reward.EssenceReward > 0)
	{
		return FormatLocalizedUI(TEXT("DUNGEON_REWARD_FORMAT"), [&Reward](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Reward"), FormatLocalizedRune(TEXT("RUNE_ESSENCE_FORMAT"), [&Reward](FFormatNamedArguments& RuneArgs)
			{
				RuneArgs.Add(TEXT("Amount"), FText::FromString(FormatIntegerWithCommas(Reward.EssenceReward)));
			}));
		});
	}
	return IdleProject::Localization::UI(TEXT("NONE_DASH"));
}

const TCHAR* AchievementCategoryToLocalizationKey(EAchievementCategory Category)
{
	switch (Category)
	{
	case EAchievementCategory::Combat:
		return TEXT("ACHIEVEMENT_CATEGORY_COMBAT");
	case EAchievementCategory::Progression:
		return TEXT("ACHIEVEMENT_CATEGORY_PROGRESSION");
	case EAchievementCategory::Gear:
		return TEXT("ACHIEVEMENT_CATEGORY_GEAR");
	case EAchievementCategory::Economy:
		return TEXT("ACHIEVEMENT_CATEGORY_ECONOMY");
	case EAchievementCategory::Skill:
		return TEXT("ACHIEVEMENT_CATEGORY_SKILL");
	case EAchievementCategory::Pet:
		return TEXT("ACHIEVEMENT_CATEGORY_PET");
	case EAchievementCategory::Quest:
		return TEXT("ACHIEVEMENT_CATEGORY_QUEST");
	case EAchievementCategory::Collection:
		return TEXT("ACHIEVEMENT_CATEGORY_COLLECTION");
	case EAchievementCategory::Misc:
	default:
		return TEXT("ACHIEVEMENT_CATEGORY_MISC");
	}
}

int64 GetAchievementThresholdForTier(const FAchievementDefinition& Definition, int32 Tier)
{
	if (Tier <= 0)
	{
		return Definition.BaseThreshold;
	}

	double Threshold = static_cast<double>(Definition.BaseThreshold);
	for (int32 Index = 0; Index < Tier; ++Index)
	{
		Threshold *= static_cast<double>(Definition.Growth);
		if (Threshold >= static_cast<double>(MAX_int64))
		{
			return MAX_int64;
		}
	}
	return FMath::Max<int64>(Definition.BaseThreshold, FMath::RoundToInt64(Threshold));
}

const FAchievementDefinition* FindAchievementDefinitionById(const FString& AchievementId)
{
	return FAchievementFormula::GetDefinitions().FindByPredicate([&AchievementId](const FAchievementDefinition& Definition)
	{
		return Definition.AchievementId == AchievementId;
	});
}

FText GetAchievementDisplayName(const FAchievementDefinition& Definition)
{
	if (!Definition.DisplayNameKey.IsEmpty())
	{
		const FText Localized = IdleProject::Localization::Text(TEXT("Achievement"), *Definition.DisplayNameKey);
		if (Localized.ToString() != Definition.DisplayNameKey)
		{
			return Localized;
		}
	}
	return Definition.DisplayName;
}

FText BuildShopResultLabel(const FShopPurchaseResult& Result)
{
	if (!Result.bPurchased)
	{
		return IdleProject::Localization::UI(TEXT("SHOP_RESULT_BLOCKED"));
	}

	return FormatLocalizedUI(TEXT("SHOP_RESULT_SUCCESS_FORMAT"), [&Result](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Rarity"), IdleProject::Localization::UI(RarityToLocalizationKey(Result.Rarity)));
		Args.Add(TEXT("ItemName"), Result.ItemName);
	});
}

const TCHAR* PrimaryStatToLabel(EPrimaryStat Stat)
{
	switch (Stat)
	{
	case EPrimaryStat::Str:
		return TEXT("STR");
	case EPrimaryStat::Dex:
		return TEXT("DEX");
	case EPrimaryStat::Int:
		return TEXT("INT");
	case EPrimaryStat::Wis:
		return TEXT("WIS");
	case EPrimaryStat::Con:
		return TEXT("CON");
	case EPrimaryStat::Luk:
		return TEXT("LUK");
	default:
		return TEXT("-");
	}
}

float GetPrimaryStatValue(const FPrimaryStats& Stats, EPrimaryStat Stat)
{
	switch (Stat)
	{
	case EPrimaryStat::Str:
		return Stats.Str;
	case EPrimaryStat::Dex:
		return Stats.Dex;
	case EPrimaryStat::Int:
		return Stats.Int_;
	case EPrimaryStat::Wis:
		return Stats.Wis;
	case EPrimaryStat::Con:
		return Stats.Con;
	case EPrimaryStat::Luk:
		return Stats.Luk;
	default:
		return 0.0f;
	}
}

const TCHAR* ClassToLocalizationKey(EClassId ClassId)
{
	switch (ClassId)
	{
	case EClassId::Warrior:
		return TEXT("CLASS_WARRIOR_NAME");
	case EClassId::Mage:
		return TEXT("CLASS_MAGE_NAME");
	case EClassId::Archer:
		return TEXT("CLASS_ARCHER_NAME");
	case EClassId::Thief:
		return TEXT("CLASS_THIEF_NAME");
	case EClassId::Cleric:
		return TEXT("CLASS_CLERIC_NAME");
	case EClassId::Paladin:
		return TEXT("CLASS_PALADIN_NAME");
	case EClassId::Berserker:
		return TEXT("CLASS_BERSERKER_NAME");
	case EClassId::Summoner:
		return TEXT("CLASS_SUMMONER_NAME");
	default:
		return TEXT("NONE_DASH");
	}
}

const TCHAR* ItemSetToLocalizationKey(EItemSet ItemSet)
{
	switch (ItemSet)
	{
	case EItemSet::Warrior:
		return TEXT("ITEM_SET_WARRIOR");
	case EItemSet::Guardian:
		return TEXT("ITEM_SET_GUARDIAN");
	case EItemSet::Arcane:
		return TEXT("ITEM_SET_ARCANE");
	case EItemSet::Assassin:
		return TEXT("ITEM_SET_ASSASSIN");
	case EItemSet::Hunter:
		return TEXT("ITEM_SET_HUNTER");
	case EItemSet::Holy:
		return TEXT("ITEM_SET_HOLY");
	case EItemSet::Berserker:
		return TEXT("ITEM_SET_BERSERKER");
	case EItemSet::None:
	default:
		return TEXT("NONE_DASH");
	}
}

void AddSetBonusPart(TArray<FString>& Parts, const TCHAR* LabelKey, float Value)
{
	if (Value <= 0.0f)
	{
		return;
	}

	Parts.Add(FormatLocalizedUI(TEXT("SET_BONUS_FLAT_FORMAT"), [LabelKey, Value](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Stat"), IdleProject::Localization::UI(LabelKey));
		Args.Add(TEXT("Value"), FText::AsNumber(FMath::RoundToInt(Value)));
	}).ToString());
}

void AddSetBonusPercentPart(TArray<FString>& Parts, const TCHAR* LabelKey, float Rate)
{
	if (Rate <= 0.0f)
	{
		return;
	}

	Parts.Add(FormatLocalizedUI(TEXT("SET_BONUS_PERCENT_FORMAT"), [LabelKey, Rate](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Stat"), IdleProject::Localization::UI(LabelKey));
		Args.Add(TEXT("Percent"), FText::AsNumber(FMath::RoundToInt(Rate * 100.0f)));
	}).ToString());
}

FText BuildSetBonusLabel(EItemSet ItemSet, int32 PieceCount)
{
	if (PieceCount < FSetBonusFormula::GetSetPieceThreshold(0))
	{
		return IdleProject::Localization::UI(TEXT("SET_BONUS_INACTIVE"));
	}

	TArray<FItemInstance> Items;
	for (int32 Index = 0; Index < PieceCount; ++Index)
	{
		FItemInstance Item;
		Item.ItemId = FName(*(FString(TEXT("set_piece_")) + FString::FromInt(Index)));
		Item.Slot = static_cast<EItemSlot>(static_cast<int32>(EItemSlot::Weapon) + FMath::Clamp(Index, 0, 7));
		Item.Rarity = EItemRarity::Rare;
		Item.ItemSet = ItemSet;
		Items.Add(Item);
	}

	const FDerivedStats Bonus = FSetBonusFormula::ComputeSetBonus(Items);
	TArray<FString> Parts;
	AddSetBonusPart(Parts, TEXT("STAT_INFO_HP"), Bonus.Hp);
	AddSetBonusPart(Parts, TEXT("STAT_INFO_PHYS_ATK"), Bonus.PhysAtk);
	AddSetBonusPart(Parts, TEXT("STAT_INFO_MAGIC_ATK"), Bonus.MagicAtk);
	AddSetBonusPart(Parts, TEXT("STAT_INFO_PHYS_DEF"), Bonus.PhysDef);
	AddSetBonusPart(Parts, TEXT("STAT_INFO_MAGIC_DEF"), Bonus.MagicDef);
	AddSetBonusPercentPart(Parts, TEXT("STAT_INFO_ATK_SPEED"), Bonus.AtkSpeed);
	AddSetBonusPercentPart(Parts, TEXT("STAT_INFO_CRIT_RATE"), Bonus.CritRate);
	AddSetBonusPercentPart(Parts, TEXT("STAT_INFO_CRIT_DMG"), Bonus.CritDmg);

	const FString BonusText = Parts.IsEmpty()
		? IdleProject::Localization::UI(TEXT("NONE_DASH")).ToString()
		: FString::Join(Parts, TEXT(" / "));
	return FormatLocalizedUI(TEXT("SET_BONUS_SUMMARY_FORMAT"), [&BonusText](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Bonus"), FText::FromString(BonusText));
	});
}

const EItemSlot* GetEnhanceSlotOrder()
{
	static const EItemSlot Slots[] = {
		EItemSlot::Weapon,
		EItemSlot::Helmet,
		EItemSlot::Top,
		EItemSlot::Bottom,
		EItemSlot::Shoes,
		EItemSlot::Gloves,
		EItemSlot::Cloak,
		EItemSlot::Accessory
	};
	return Slots;
}

int32 GetEnhanceSlotCount()
{
	return 8;
}

const TCHAR* SlotToLocalizationKey(EItemSlot Slot)
{
	switch (Slot)
	{
	case EItemSlot::Weapon:
		return TEXT("ENHANCE_SLOT_WEAPON");
	case EItemSlot::Helmet:
		return TEXT("ENHANCE_SLOT_HELMET");
	case EItemSlot::Top:
		return TEXT("ENHANCE_SLOT_TOP");
	case EItemSlot::Bottom:
		return TEXT("ENHANCE_SLOT_BOTTOM");
	case EItemSlot::Shoes:
		return TEXT("ENHANCE_SLOT_SHOES");
	case EItemSlot::Gloves:
		return TEXT("ENHANCE_SLOT_GLOVES");
	case EItemSlot::Cloak:
		return TEXT("ENHANCE_SLOT_CLOAK");
	case EItemSlot::Accessory:
		return TEXT("ENHANCE_SLOT_ACCESSORY");
	case EItemSlot::None:
	default:
		return TEXT("NONE_DASH");
	}
}

bool TryBuildStatusIndicator(ESkillStatusEffect Type, const TArray<FActiveSkillStatus>& Statuses, float Now, float HudScale, FIdleHUDStatusIndicatorViewModel& OutIndicator)
{
	const FActiveSkillStatus* Status = Statuses.FindByPredicate([Type, Now](const FActiveSkillStatus& Candidate)
	{
		return Candidate.Type == Type && Candidate.EndTime > Now;
	});
	if (!Status)
	{
		return false;
	}

	OutIndicator.Type = Type;
	OutIndicator.RemainingSeconds = FMath::Max(0.0f, Status->EndTime - Now);
	OutIndicator.Size = 22.0f * HudScale;

	switch (Type)
	{
	case ESkillStatusEffect::Poison:
		OutIndicator.Label = TEXT("P");
		OutIndicator.Color = IdleProject::UI::Theme::RarityUnique;
		return true;
	case ESkillStatusEffect::Burn:
		OutIndicator.Label = TEXT("B");
		OutIndicator.Color = IdleProject::UI::Theme::RarityLegendary;
		return true;
	case ESkillStatusEffect::Freeze:
		OutIndicator.Label = TEXT("F");
		OutIndicator.Color = IdleProject::UI::Theme::AccentBlue;
		return true;
	case ESkillStatusEffect::Curse:
		OutIndicator.Label = TEXT("C");
		OutIndicator.Color = IdleProject::UI::Theme::ElementDark;
		return true;
	case ESkillStatusEffect::None:
	default:
		return false;
	}
}
}

FText IdleProject::UI::RarityToLabel(EItemRarity Rarity)
{
	return IdleProject::Localization::UI(RarityToLocalizationKey(Rarity));
}

FLinearColor IdleProject::UI::RarityToColor(EItemRarity Rarity)
{
	using namespace IdleProject::UI::Theme;

	switch (Rarity)
	{
	case EItemRarity::Common:
		return RarityCommon;
	case EItemRarity::Rare:
		return RarityRare;
	case EItemRarity::Epic:
		return RarityEpic;
	case EItemRarity::Unique:
		return RarityUnique;
	case EItemRarity::Legendary:
		return RarityLegendary;
	case EItemRarity::Transcendent:
		return RarityTranscendent;
	case EItemRarity::Mythic:
		return RarityMythicStart;
	case EItemRarity::None:
	default:
		return TextMuted;
	}
}

FText IdleProject::UI::BuildAffixSummary(const FItemInstance& Item)
{
	TArray<FString> Parts;
	if (Item.BonusCritRate > 0.0f)
	{
		Parts.Add(FormatAffixRateLabel(TEXT("AFFIX_CRIT_RATE_FORMAT"), Item.BonusCritRate).ToString());
	}
	if (Item.BonusAtkSpeed > 0.0f)
	{
		Parts.Add(FormatAffixFloatLabel(TEXT("AFFIX_ATK_SPEED_FORMAT"), Item.BonusAtkSpeed).ToString());
	}
	if (Item.BonusMagicAtk > 0.0f)
	{
		Parts.Add(FormatAffixFlatLabel(TEXT("AFFIX_MAGIC_ATK_FORMAT"), Item.BonusMagicAtk).ToString());
	}
	if (Item.BonusPhysDef > 0.0f)
	{
		Parts.Add(FormatAffixFlatLabel(TEXT("AFFIX_PHYS_DEF_FORMAT"), Item.BonusPhysDef).ToString());
	}
	if (Item.BonusMagicDef > 0.0f)
	{
		Parts.Add(FormatAffixFlatLabel(TEXT("AFFIX_MAGIC_DEF_FORMAT"), Item.BonusMagicDef).ToString());
	}
	if (Item.BonusAffixHp > 0.0f)
	{
		Parts.Add(FormatAffixFlatLabel(TEXT("AFFIX_HP_FORMAT"), Item.BonusAffixHp).ToString());
	}
	if (Item.BonusCritDmg > 0.0f)
	{
		Parts.Add(FormatAffixRateLabel(TEXT("AFFIX_CRIT_DMG_FORMAT"), Item.BonusCritDmg).ToString());
	}

	return Parts.IsEmpty()
		? FText::GetEmpty()
		: FText::FromString(FString::Join(Parts, TEXT(" / ")));
}

FText IdleProject::UI::BuildUniqueTraitSummary(const FItemInstance& Item)
{
	TArray<FString> Parts;
	const FText Trait1 = FormatUniqueTraitLabel(Item.UniqueTrait1, Item.Rarity);
	if (!Trait1.IsEmpty())
	{
		Parts.Add(Trait1.ToString());
	}

	const FText Trait2 = FormatUniqueTraitLabel(Item.UniqueTrait2, Item.Rarity);
	if (!Trait2.IsEmpty())
	{
		Parts.Add(Trait2.ToString());
	}

	return Parts.IsEmpty()
		? FText::GetEmpty()
		: FText::FromString(FString::Join(Parts, TEXT(" / ")));
}

FIdleHUDSetSummaryViewModel IdleProject::UI::BuildSetSummaryViewModel(const TArray<FItemInstance>& EquippedItems)
{
	FIdleHUDSetSummaryViewModel ViewModel;

	TMap<EItemSet, int32> PieceCounts;
	for (const FItemInstance& Item : EquippedItems)
	{
		if (Item.ItemSet == EItemSet::None || Item.Slot == EItemSlot::None || Item.Rarity == EItemRarity::None)
		{
			continue;
		}
		PieceCounts.FindOrAdd(Item.ItemSet) += 1;
	}

	const EItemSet SetOrder[] = {
		EItemSet::Warrior,
		EItemSet::Guardian,
		EItemSet::Arcane,
		EItemSet::Assassin,
		EItemSet::Hunter,
		EItemSet::Holy,
		EItemSet::Berserker
	};

	for (const EItemSet ItemSet : SetOrder)
	{
		const int32* CountPtr = PieceCounts.Find(ItemSet);
		if (!CountPtr || *CountPtr <= 0)
		{
			continue;
		}

		FIdleHUDSetSummaryRowViewModel Row;
		Row.ItemSet = ItemSet;
		Row.PieceCount = FMath::Clamp(*CountPtr, 0, FSetBonusFormula::GetSetPieceThreshold(1));
		Row.SetLabel = IdleProject::Localization::UI(ItemSetToLocalizationKey(ItemSet));
		Row.bTwoPieceActive = Row.PieceCount >= FSetBonusFormula::GetSetPieceThreshold(0);
		Row.bFourPieceActive = Row.PieceCount >= FSetBonusFormula::GetSetPieceThreshold(1);
		Row.TierLabel = Row.bFourPieceActive
			? IdleProject::Localization::UI(TEXT("SET_TIER_FOUR_ACTIVE"))
			: (Row.bTwoPieceActive ? IdleProject::Localization::UI(TEXT("SET_TIER_TWO_ACTIVE")) : IdleProject::Localization::UI(TEXT("SET_TIER_INACTIVE")));

		if (Row.bFourPieceActive)
		{
			Row.NextTierLabel = IdleProject::Localization::UI(TEXT("SET_NEXT_COMPLETE"));
		}
		else
		{
			const int32 NextThreshold = Row.bTwoPieceActive
				? FSetBonusFormula::GetSetPieceThreshold(1)
				: FSetBonusFormula::GetSetPieceThreshold(0);
			const int32 Needed = FMath::Max(0, NextThreshold - Row.PieceCount);
			Row.NextTierLabel = FormatLocalizedUI(TEXT("SET_NEXT_TIER_FORMAT"), [NextThreshold, Needed](FFormatNamedArguments& Args)
			{
				Args.Add(TEXT("Piece"), FText::AsNumber(NextThreshold));
				Args.Add(TEXT("Needed"), FText::AsNumber(Needed));
			});
		}

		Row.BonusLabel = BuildSetBonusLabel(ItemSet, Row.PieceCount);
		Row.SummaryLabel = FormatLocalizedUI(TEXT("SET_SUMMARY_FORMAT"), [&Row](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("SetName"), Row.SetLabel);
			Args.Add(TEXT("Count"), FText::AsNumber(Row.PieceCount));
			Args.Add(TEXT("Tier"), Row.TierLabel);
		});
		ViewModel.Rows.Add(Row);
	}

	return ViewModel;
}

TArray<FIdleHUDSkillSlotViewModel> IdleProject::UI::BuildSkillSlotViewModels(const USkillComponent& SkillComponent, float Now)
{
	TArray<FIdleHUDSkillSlotViewModel> Slots;
	for (const FSkillDefinition& Skill : SkillComponent.Skills)
	{
		if (Skill.Type != ESkillType::Active)
		{
			continue;
		}

		FIdleHUDSkillSlotViewModel Slot;
		Slot.SkillId = Skill.SkillId;
		Slot.DisplayName = Skill.DisplayName;
		Slot.CooldownRatio = SkillComponent.GetCooldownRatio(Skill.SkillId, Now);
		Slot.CooldownRemaining = SkillComponent.GetCooldownRemaining(Skill.SkillId, Now);
		Slot.AvailableSkillPoints = SkillComponent.GetSkillPoints();
		Slot.Rank = SkillComponent.GetSkillRank(Skill.SkillId);
		Slot.MaxRank = SkillComponent.MaxRank;
		Slot.bReady = Slot.CooldownRemaining <= 0.0f;
		Slot.bCanRankUp = SkillComponent.CanRankUp(Skill.SkillId);
		Slots.Add(Slot);
	}

	return Slots;
}

FIdleHUDUltimateViewModel IdleProject::UI::BuildUltimateViewModel(const USkillComponent& SkillComponent)
{
	FIdleHUDUltimateViewModel Ultimate;
	Ultimate.GaugePercent = FMath::Clamp(SkillComponent.GetCurrentGauge(), 0.0f, 100.0f);
	Ultimate.GaugeRatio = Ultimate.GaugePercent / 100.0f;
	Ultimate.bReady = SkillComponent.IsUltimateReady();
	return Ultimate;
}

FIdleHUDStageViewModel IdleProject::UI::BuildStageViewModel(const FStageInfo& StageInfo)
{
	FIdleHUDStageViewModel ViewModel;
	const int32 SafeChapter = FMath::Max(1, StageInfo.Chapter);
	const int32 SafeStage = FMath::Max(1, StageInfo.Stage);
	const int32 SafeCurrent = FMath::Max(0, StageInfo.KillsThisStage);
	const int32 SafeTarget = FMath::Max(0, StageInfo.KillsToAdvance);

	ViewModel.TitleLabel = IdleProject::Localization::UI(TEXT("STAGE_INDICATOR_TITLE"));
	ViewModel.ChapterLabel = FormatLocalizedUI(TEXT("STAGE_CHAPTER_FORMAT"), [SafeChapter](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Chapter"), FText::AsNumber(SafeChapter));
	});
	ViewModel.ProgressLabel = FormatLocalizedUI(TEXT("STAGE_PROGRESS_FORMAT"), [SafeChapter, SafeStage, SafeCurrent, SafeTarget](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Chapter"), FText::AsNumber(SafeChapter));
		Args.Add(TEXT("Stage"), FText::AsNumber(SafeStage));
		Args.Add(TEXT("Current"), FText::AsNumber(SafeCurrent));
		Args.Add(TEXT("Target"), FText::AsNumber(SafeTarget));
	});
	ViewModel.WeaknessLabel = FormatLocalizedUI(TEXT("STAGE_WEAKNESS_FORMAT"), [&StageInfo](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Element"), StageWeakElementToLabel(StageInfo.WeakElement));
	});
	ViewModel.WeaknessIconLabel = StageWeakElementToIconLabel(StageInfo.WeakElement);
	ViewModel.ProgressRatio = SafeTarget > 0
		? FMath::Clamp(static_cast<float>(SafeCurrent) / static_cast<float>(SafeTarget), 0.0f, 1.0f)
		: 0.0f;
	ViewModel.bBossStage = StageInfo.bBossStage;
	ViewModel.bEliteStage = StageInfo.bEliteStage && !StageInfo.bBossStage;
	ViewModel.BossBadgeLabel = StageInfo.bBossStage ? IdleProject::Localization::UI(TEXT("STAGE_BOSS_BADGE")) : FText::GetEmpty();
	ViewModel.EliteBadgeLabel = ViewModel.bEliteStage ? IdleProject::Localization::UI(TEXT("STAGE_ELITE_BADGE")) : FText::GetEmpty();
	ViewModel.BorderColor = StageInfo.bBossStage
		? Theme::AccentGold
		: (ViewModel.bEliteStage ? Theme::ElementDark : Theme::AccentBlue);
	ViewModel.WeaknessColor = StageWeakElementToColor(StageInfo.WeakElement);
	return ViewModel;
}

FText IdleProject::UI::BuildChapterEntryFeedbackLabel(int32 Chapter)
{
	const int32 SafeChapter = FMath::Max(1, Chapter);
	return FormatLocalizedUI(TEXT("STAGE_CHAPTER_ENTRY_FORMAT"), [SafeChapter](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Chapter"), FText::AsNumber(SafeChapter));
	});
}

FText IdleProject::UI::BuildChapterClearFeedbackLabel(int32 Chapter)
{
	const int32 SafeChapter = FMath::Max(1, Chapter);
	return FormatLocalizedUI(TEXT("STAGE_CHAPTER_CLEAR_FORMAT"), [SafeChapter](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Chapter"), FText::AsNumber(SafeChapter));
	});
}

FIdleHUDBossViewModel IdleProject::UI::BuildBossViewModel(float CurrentHp, float MaxHp)
{
	FIdleHUDBossViewModel ViewModel;
	if (MaxHp <= 0.0f)
	{
		return ViewModel;
	}

	const float SafeCurrentHp = FMath::Clamp(CurrentHp, 0.0f, MaxHp);
	ViewModel.bVisible = true;
	ViewModel.HpRatio = FMath::Clamp(SafeCurrentHp / MaxHp, 0.0f, 1.0f);
	ViewModel.HpPercent = ViewModel.HpRatio * 100.0f;
	ViewModel.Phase = FBossPhaseFormula::GetBossPhase(ViewModel.HpRatio);
	ViewModel.PhaseColor = BossPhaseToColor(ViewModel.Phase);
	ViewModel.TitleLabel = IdleProject::Localization::UI(TEXT("BOSS_HUD_TITLE"));
	ViewModel.HpLabel = FormatLocalizedUI(TEXT("BOSS_HP_FORMAT"), [SafeCurrentHp, MaxHp](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Current"), FText::AsNumber(FMath::RoundToInt(SafeCurrentHp)));
		Args.Add(TEXT("Max"), FText::AsNumber(FMath::RoundToInt(MaxHp)));
	});
	ViewModel.PhaseLabel = FormatLocalizedUIWithNumber(TEXT("BOSS_PHASE_FORMAT"), TEXT("Phase"), ViewModel.Phase);
	return ViewModel;
}

FIdleHUDEnhancePanelViewModel IdleProject::UI::BuildEnhancePanelViewModel(const UInventoryComponent& Inventory, int64 Gold, FText FeedbackLabel, bool bFeedbackSuccess)
{
	return BuildEnhancePanelViewModel(Inventory, Gold, 0, MoveTemp(FeedbackLabel), bFeedbackSuccess);
}

FIdleHUDEnhancePanelViewModel IdleProject::UI::BuildEnhancePanelViewModel(const UInventoryComponent& Inventory, int64 Gold, int64 ProtectionScrolls, FText FeedbackLabel, bool bFeedbackSuccess)
{
	FIdleHUDEnhancePanelViewModel ViewModel;
	ViewModel.Title = IdleProject::Localization::UI(TEXT("ENHANCE_PANEL_TITLE"));
	ViewModel.GoldLabel = FormatLocalizedUIWithInt64(TEXT("HUD_GOLD_FORMAT"), TEXT("Amount"), Gold);
	ViewModel.ProtectionLabel = FormatLocalizedUIWithInt64(TEXT("ENHANCE_PROTECTION_COUNT_FORMAT"), TEXT("Count"), FMath::Max<int64>(0, ProtectionScrolls));
	ViewModel.FeedbackLabel = MoveTemp(FeedbackLabel);
	ViewModel.bFeedbackSuccess = bFeedbackSuccess;

	const EItemSlot* Slots = GetEnhanceSlotOrder();
	for (int32 Index = 0; Index < GetEnhanceSlotCount(); ++Index)
	{
		const EItemSlot Slot = Slots[Index];
		const FItemInstance* Item = Inventory.GetEquippedItem(Slot);

		FIdleHUDEnhanceSlotViewModel Row;
		Row.Slot = Slot;
		Row.SlotLabel = IdleProject::Localization::UI(SlotToLocalizationKey(Slot));
		Row.ItemName = Item ? Item->DisplayName : IdleProject::Localization::UI(TEXT("NONE_DASH"));
		Row.RarityLabel = Item ? RarityToLabel(Item->Rarity) : IdleProject::Localization::UI(TEXT("NONE_DASH"));
		Row.RarityColor = Item ? RarityToColor(Item->Rarity) : IdleProject::UI::Theme::TextMuted;
		Row.bEquipped = Item != nullptr;
		Row.ButtonLabel = IdleProject::Localization::UI(TEXT("ACTION_ENHANCE"));

		if (!Item)
		{
			Row.LevelLabel = IdleProject::Localization::UI(TEXT("NONE_DASH"));
			Row.CostLabel = IdleProject::Localization::UI(TEXT("NONE_DASH"));
			Row.SuccessRateLabel = IdleProject::Localization::UI(TEXT("NONE_DASH"));
			Row.RiskLabel = IdleProject::Localization::UI(TEXT("NONE_DASH"));
			Row.FailStreakLabel = IdleProject::Localization::UI(TEXT("NONE_DASH"));
			Row.ProtectionButtonLabel = IdleProject::Localization::UI(TEXT("ACTION_PROTECT"));
			Row.StatusLabel = IdleProject::Localization::UI(TEXT("ENHANCE_STATUS_EMPTY"));
			ViewModel.Rows.Add(Row);
			continue;
		}

		Row.EnhanceLevel = FMath::Clamp(Item->EnhanceLevel, 0, FEnhanceFormula::MaxEnhanceLevel);
		Row.bMaxLevel = Row.EnhanceLevel >= FEnhanceFormula::MaxEnhanceLevel;
		Row.Cost = FEnhanceFormula::GetEnhanceCost(Row.EnhanceLevel, Item->Rarity);
		Row.SuccessRate = FEnhanceFormula::GetEnhanceSuccessRate(Row.EnhanceLevel);
		Row.bGoldEnough = Gold >= Row.Cost;
		Row.bCanEnhance = !Row.bMaxLevel && Row.Cost > 0 && Row.bGoldEnough;
		Row.FailStreak = FMath::Max(0, Item->EnhanceFailStreak);
		Row.bRiskLevel = FEnhanceFormula::IsRiskLevel(Row.EnhanceLevel);
		Row.bCanUseProtection = Row.bCanEnhance && Row.bRiskLevel && ProtectionScrolls > 0;
		Row.LevelLabel = FormatLocalizedUI(TEXT("ENHANCE_LEVEL_FORMAT"), [&Row](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Level"), FText::AsNumber(Row.EnhanceLevel));
			Args.Add(TEXT("MaxLevel"), FText::AsNumber(FEnhanceFormula::MaxEnhanceLevel));
		});
		Row.RiskLabel = Row.bRiskLevel
			? IdleProject::Localization::UI(TEXT("ENHANCE_RISK_DOWNGRADE"))
			: IdleProject::Localization::UI(TEXT("ENHANCE_RISK_SAFE"));
		Row.FailStreakLabel = FormatLocalizedUI(TEXT("ENHANCE_FAIL_STREAK_FORMAT"), [&Row](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Count"), FText::AsNumber(Row.FailStreak));
			Args.Add(TEXT("Threshold"), FText::AsNumber(FEnhanceFormula::PityThreshold));
		});
		Row.ProtectionButtonLabel = IdleProject::Localization::UI(TEXT("ACTION_PROTECT"));
		Row.CostLabel = Row.bMaxLevel
			? IdleProject::Localization::UI(TEXT("ENHANCE_STATUS_MAX"))
			: FormatLocalizedUIWithInt64(TEXT("ENHANCE_COST_FORMAT"), TEXT("Cost"), Row.Cost);
		Row.SuccessRateLabel = Row.bMaxLevel
			? IdleProject::Localization::UI(TEXT("ENHANCE_STATUS_MAX"))
			: FormatPercentLabel(TEXT("ENHANCE_SUCCESS_FORMAT"), Row.SuccessRate);

		if (Row.bMaxLevel)
		{
			Row.StatusLabel = IdleProject::Localization::UI(TEXT("ENHANCE_STATUS_MAX"));
		}
		else if (!Row.bGoldEnough)
		{
			Row.StatusLabel = IdleProject::Localization::UI(TEXT("ENHANCE_STATUS_NEED_GOLD"));
		}
		else
		{
			Row.StatusLabel = IdleProject::Localization::UI(TEXT("ENHANCE_STATUS_READY"));
		}

		ViewModel.Rows.Add(Row);
	}

	return ViewModel;
}

FIdleHUDPotentialPanelViewModel IdleProject::UI::BuildPotentialPanelViewModel(const UInventoryComponent& Inventory, int64 ResetCubes, int64 RankCubes)
{
	FIdleHUDPotentialPanelViewModel ViewModel;
	ViewModel.Title = IdleProject::Localization::UI(TEXT("POTENTIAL_PANEL_TITLE"));
	ViewModel.ResetCubeLabel = FormatLocalizedUIWithInt64(TEXT("POTENTIAL_RESET_CUBE_COUNT_FORMAT"), TEXT("Count"), FMath::Max<int64>(0, ResetCubes));
	ViewModel.RankCubeLabel = FormatLocalizedUIWithInt64(TEXT("POTENTIAL_RANK_CUBE_COUNT_FORMAT"), TEXT("Count"), FMath::Max<int64>(0, RankCubes));

	const EItemSlot* Slots = GetEnhanceSlotOrder();
	for (int32 Index = 0; Index < GetEnhanceSlotCount(); ++Index)
	{
		const EItemSlot Slot = Slots[Index];
		const FItemInstance* Item = Inventory.GetEquippedItem(Slot);

		FIdleHUDPotentialSlotViewModel Row;
		Row.Slot = Slot;
		Row.SlotLabel = IdleProject::Localization::UI(SlotToLocalizationKey(Slot));
		Row.ItemName = Item ? Item->DisplayName : IdleProject::Localization::UI(TEXT("NONE_DASH"));
		Row.ResetActionLabel = IdleProject::Localization::UI(TEXT("ACTION_POTENTIAL_RESET"));
		Row.RankActionLabel = IdleProject::Localization::UI(TEXT("ACTION_POTENTIAL_RANK"));
		Row.LockActionLabel = IdleProject::Localization::UI(TEXT("ACTION_LOCK"));
		Row.ResetHitBoxName = MakePotentialResetHitBoxName(Slot);
		Row.RankHitBoxName = MakePotentialRankHitBoxName(Slot);
		Row.LockHitBoxName = MakePotentialLockHitBoxName(Slot);

		if (!Item)
		{
			Row.GradeLabel = IdleProject::Localization::UI(TEXT("NONE_DASH"));
			Row.LineSummaryLabel = IdleProject::Localization::UI(TEXT("NONE_DASH"));
			Row.StatusLabel = IdleProject::Localization::UI(TEXT("ENHANCE_STATUS_EMPTY"));
			Row.GradeColor = Theme::TextMuted;
			ViewModel.Rows.Add(Row);
			continue;
		}

		Row.bEquipped = true;
		Row.bLocked = Item->bLocked;
		Row.Grade = Item->PotentialGrade;
		Row.MaxGrade = FPotentialFormula::GetMaxPotentialGrade(Item->Rarity);
		Row.bHasPotential = Row.Grade != EPotentialGrade::None;
		Row.bCanResetPotential = Row.bHasPotential && ResetCubes > 0;
		Row.bCanRankPotential = Row.bHasPotential
			&& RankCubes > 0
			&& Row.MaxGrade != EPotentialGrade::None
			&& static_cast<uint8>(Row.Grade) < static_cast<uint8>(Row.MaxGrade);
		Row.LockActionLabel = IdleProject::Localization::UI(Row.bLocked ? TEXT("ACTION_UNLOCK") : TEXT("ACTION_LOCK"));
		Row.GradeColor = PotentialGradeToColor(Row.Grade);
		Row.GradeLabel = FormatLocalizedUI(TEXT("POTENTIAL_GRADE_CAP_FORMAT"), [&Row](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Grade"), PotentialGradeToLabel(Row.Grade));
			Args.Add(TEXT("MaxGrade"), PotentialGradeToLabel(Row.MaxGrade));
		});
		Row.LineSummaryLabel = BuildPotentialLineSummary(*Item);
		Row.StatusLabel = Row.bHasPotential
			? IdleProject::Localization::UI(TEXT("POTENTIAL_STATUS_READY"))
			: IdleProject::Localization::UI(TEXT("POTENTIAL_STATUS_NONE"));
		ViewModel.Rows.Add(Row);
	}

	return ViewModel;
}

FIdleHUDShopPanelViewModel IdleProject::UI::BuildShopPanelViewModel(int64 GearRollCost, int64 ProtectionScrollCost, int64 ResetCubeCost, int64 RankCubeCost, int64 Gold, const FShopPurchaseResult& LastResult)
{
	FIdleHUDShopPanelViewModel ViewModel;
	ViewModel.Title = IdleProject::Localization::UI(TEXT("SHOP_PANEL_TITLE"));
	ViewModel.Gold = FMath::Max<int64>(0, Gold);
	ViewModel.GearRollCost = FMath::Max<int64>(0, GearRollCost);
	ViewModel.ProtectionScrollCost = FMath::Max<int64>(0, ProtectionScrollCost);
	ViewModel.ResetCubeCost = FMath::Max<int64>(0, ResetCubeCost);
	ViewModel.RankCubeCost = FMath::Max<int64>(0, RankCubeCost);
	ViewModel.GearRollHitBoxName = ShopGearRollHitBoxName;
	ViewModel.ProtectionScrollHitBoxName = ShopProtectionScrollHitBoxName;
	ViewModel.ResetCubeHitBoxName = ShopResetCubeHitBoxName;
	ViewModel.RankCubeHitBoxName = ShopRankCubeHitBoxName;
	ViewModel.GoldLabel = FormatLocalizedUIWithInt64(TEXT("HUD_GOLD_FORMAT"), TEXT("Amount"), ViewModel.Gold);
	ViewModel.CostLabel = FormatLocalizedUIWithInt64(TEXT("SHOP_GEAR_ROLL_COST_FORMAT"), TEXT("Cost"), ViewModel.GearRollCost);
	ViewModel.ProtectionScrollCostLabel = FormatLocalizedUIWithInt64(TEXT("SHOP_PROTECTION_SCROLL_COST_FORMAT"), TEXT("Cost"), ViewModel.ProtectionScrollCost);
	ViewModel.ResetCubeCostLabel = FormatLocalizedUIWithInt64(TEXT("SHOP_RESET_CUBE_COST_FORMAT"), TEXT("Cost"), ViewModel.ResetCubeCost);
	ViewModel.RankCubeCostLabel = FormatLocalizedUIWithInt64(TEXT("SHOP_RANK_CUBE_COST_FORMAT"), TEXT("Cost"), ViewModel.RankCubeCost);
	ViewModel.ButtonLabel = IdleProject::Localization::UI(TEXT("SHOP_GEAR_ROLL_BUTTON"));
	ViewModel.ProtectionScrollButtonLabel = IdleProject::Localization::UI(TEXT("SHOP_PROTECTION_SCROLL_BUTTON"));
	ViewModel.ResetCubeButtonLabel = IdleProject::Localization::UI(TEXT("SHOP_RESET_CUBE_BUTTON"));
	ViewModel.RankCubeButtonLabel = IdleProject::Localization::UI(TEXT("SHOP_RANK_CUBE_BUTTON"));
	ViewModel.bCanBuyGearRoll = ViewModel.GearRollCost > 0 && ViewModel.Gold >= ViewModel.GearRollCost;
	ViewModel.bCanBuyProtectionScroll = ViewModel.ProtectionScrollCost > 0 && ViewModel.Gold >= ViewModel.ProtectionScrollCost;
	ViewModel.bCanBuyResetCube = ViewModel.ResetCubeCost > 0 && ViewModel.Gold >= ViewModel.ResetCubeCost;
	ViewModel.bCanBuyRankCube = ViewModel.RankCubeCost > 0 && ViewModel.Gold >= ViewModel.RankCubeCost;
	ViewModel.StatusLabel = ViewModel.bCanBuyGearRoll
		? IdleProject::Localization::UI(TEXT("SHOP_STATUS_READY"))
		: IdleProject::Localization::UI(TEXT("SHOP_STATUS_NEED_GOLD"));

	ViewModel.bHasLastResult = LastResult.bPurchased || LastResult.GoldSpent > 0 || !LastResult.ItemName.IsEmpty();
	ViewModel.bLastResultError = ViewModel.bHasLastResult && !LastResult.bPurchased;
	ViewModel.LastResultRarity = LastResult.Rarity;
	if (ViewModel.bHasLastResult)
	{
		ViewModel.LastResultLabel = BuildShopResultLabel(LastResult);
	}

	return ViewModel;
}

FIdleHUDConsumablePanelViewModel IdleProject::UI::BuildConsumablePanelViewModel(const UBuffService& BuffService, int64 NowUnixSec)
{
	FIdleHUDConsumablePanelViewModel ViewModel;
	ViewModel.Title = IdleProject::Localization::UI(TEXT("CONSUMABLE_PANEL_TITLE"));
	ViewModel.ActiveBuffTitle = IdleProject::Localization::UI(TEXT("CONSUMABLE_ACTIVE_BUFF_TITLE"));
	ViewModel.EmptyActiveBuffLabel = IdleProject::Localization::UI(TEXT("CONSUMABLE_ACTIVE_BUFF_EMPTY"));

	// 등급은 소(Lesser)→중(Standard)→대(Greater) 순으로 표시합니다.
	static const EConsumableGrade GradeOrder[] = {
		EConsumableGrade::Lesser,
		EConsumableGrade::Standard,
		EConsumableGrade::Greater,
	};

	const EConsumableType* Order = GetConsumableDisplayOrder();
	for (int32 Index = 0; Index < GetConsumableDisplayCount(); ++Index)
	{
		const EConsumableType Type = Order[Index];
		const FText TypeName = IdleProject::Localization::UI(ConsumableNameKey(Type));
		const FText EffectLabel = IdleProject::Localization::UI(ConsumableEffectKey(Type));
		const bool bTypeActive = BuffService.IsBuffActive(Type, NowUnixSec);
		const EConsumableGrade ActiveGrade = BuffService.GetActiveGrade(Type, NowUnixSec);
		const int64 RemainingSec = BuffService.GetBuffRemainingSec(Type, NowUnixSec);

		// 등급별 보유 행을 생성합니다. 보유 등급이 하나도 없으면 Standard 행만 비활성으로 노출해
		// 6종 타입이 항상 패널에 보이도록 유지합니다(#73 회귀 방지).
		const int32 TotalCount = BuffService.GetTotalCount(Type);
		// 재고가 모두 소진됐지만 현재 활성 등급이 있으면 그 등급 행을 유지해 활성 표시가 사라지지 않게 합니다.
		const bool bShowActivePlaceholder = TotalCount == 0 && bTypeActive;
		for (const EConsumableGrade Grade : GradeOrder)
		{
			const int32 GradeCount = BuffService.GetCount(Type, Grade);
			const bool bIsActiveGrade = bTypeActive && Grade == ActiveGrade;
			// 보유 등급은 항상 노출하고, 무재고 타입은 (활성 등급 / 없으면 Standard) 1행만 노출합니다.
			const bool bPlaceholderRow = TotalCount == 0
				&& (bShowActivePlaceholder ? bIsActiveGrade : Grade == EConsumableGrade::Standard);
			if (GradeCount <= 0 && !bPlaceholderRow)
			{
				continue;
			}

			FIdleHUDConsumableRowViewModel Row;
			Row.Type = Type;
			Row.Grade = Grade;
			Row.GradeLabel = IdleProject::Localization::UI(ConsumableGradeLabelKey(Grade));
			// 이름에 등급을 함께 표기합니다(예: "공격 토닉 (대)").
			Row.NameLabel = FormatLocalizedUI(TEXT("CONSUMABLE_NAME_WITH_GRADE_FORMAT"), [&TypeName, &Row](FFormatNamedArguments& Args)
			{
				Args.Add(TEXT("Name"), TypeName);
				Args.Add(TEXT("Grade"), Row.GradeLabel);
			});
			Row.EffectLabel = EffectLabel;
			Row.Count = GradeCount;
			Row.CountLabel = FormatLocalizedUIWithNumber(TEXT("CONSUMABLE_COUNT_FORMAT"), TEXT("Count"), Row.Count);
			Row.ActionLabel = IdleProject::Localization::UI(TEXT("ACTION_USE"));
			Row.UseHitBoxName = MakeConsumableUseHitBoxName(Type, Grade);
			Row.RemainingSec = RemainingSec;
			Row.RemainingLabel = FText::FromString(FormatMinutesSeconds(RemainingSec));
			Row.bCanUse = GradeCount > 0;
			// 활성 표시는 현재 활성 등급 행에만 부여합니다.
			Row.bActive = bTypeActive && Grade == ActiveGrade;
			ViewModel.Rows.Add(Row);
			if (Row.bActive)
			{
				ViewModel.ActiveBuffRows.Add(Row);
			}
		}
	}

	return ViewModel;
}

FIdleHUDLeaderboardPanelViewModel IdleProject::UI::BuildLeaderboardPanelViewModel(const ULeaderboardService& LeaderboardService, ELeaderboardKind Kind, int32 SeasonId, const FString& WeekId, bool bLoading, bool bOffline)
{
	FIdleHUDLeaderboardPanelViewModel ViewModel;
	ViewModel.ActiveKind = Kind;
	ViewModel.bLoading = bLoading;
	ViewModel.bOffline = bOffline;
	ViewModel.Title = IdleProject::Localization::UI(TEXT("LEADERBOARD_PANEL_TITLE"));
	ViewModel.SeasonLabel = FormatLocalizedUIWithNumber(TEXT("LEADERBOARD_SEASON_FORMAT"), TEXT("Season"), SeasonId);
	ViewModel.PowerTabLabel = IdleProject::Localization::UI(TEXT("LEADERBOARD_TAB_POWER"));
	ViewModel.RebirthTabLabel = IdleProject::Localization::UI(TEXT("LEADERBOARD_TAB_REBIRTH"));
	ViewModel.WeeklyTabLabel = IdleProject::Localization::UI(TEXT("LEADERBOARD_TAB_WEEKLY"));
	ViewModel.WeekLabel = FormatLocalizedUI(TEXT("LEADERBOARD_WEEK_FORMAT"), [&WeekId](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Week"), FText::FromString(WeekId));
	});
	ViewModel.MyRankTitle = IdleProject::Localization::UI(TEXT("LEADERBOARD_MY_RANK"));
	ViewModel.EmptyLabel = IdleProject::Localization::UI(TEXT("LEADERBOARD_EMPTY"));
	ViewModel.OfflineLabel = IdleProject::Localization::UI(TEXT("LEADERBOARD_OFFLINE"));
	ViewModel.LoadingLabel = IdleProject::Localization::UI(TEXT("LEADERBOARD_LOADING"));
	ViewModel.RefreshLabel = IdleProject::Localization::UI(TEXT("ACTION_REFRESH"));

	const FLeaderboardEntry MyEntry = LeaderboardService.GetMyEntry(Kind);
	ViewModel.MyEntry = BuildLeaderboardRow(MyEntry, MyEntry.CharacterId);
	ViewModel.MyEntry.bSelf = MyEntry.Rank > 0 || !MyEntry.CharacterId.IsEmpty();

	const TArray<FLeaderboardEntry> Entries = LeaderboardService.GetEntries(Kind);
	for (const FLeaderboardEntry& Entry : Entries)
	{
		ViewModel.Rows.Add(BuildLeaderboardRow(Entry, MyEntry.CharacterId));
	}

	return ViewModel;
}

FIdleHUDGuildPanelViewModel IdleProject::UI::BuildGuildPanelViewModel(const UGuildService& GuildService, const TArray<FGuildSummary>& BrowseList, const FString& PendingCreateName, bool bLoading, bool bOffline, int64 PlayerGold, int64 DonateAmount, const TArray<FGuildShopItemInfo>& ShopItems, bool bRankingsView, bool bRankingsLoading, const TArray<FGuildRankingEntry>& Rankings, const FGuildRankingEntry& MyRanking)
{
	FIdleHUDGuildPanelViewModel ViewModel;
	ViewModel.Title = IdleProject::Localization::UI(TEXT("GUILD_PANEL_TITLE"));
	ViewModel.RefreshLabel = IdleProject::Localization::UI(TEXT("GUILD_REFRESH"));
	ViewModel.bOffline = bOffline;
	ViewModel.bLoading = bLoading;
	ViewModel.StateLabel = bLoading
		? IdleProject::Localization::UI(TEXT("GUILD_LOADING"))
		: (bOffline ? IdleProject::Localization::UI(TEXT("GUILD_OFFLINE")) : FText::GetEmpty());
	ViewModel.bHasGuild = GuildService.HasGuild();

	if (!ViewModel.bHasGuild)
	{
		// ── 무소속 화면: 길드 목록 + 생성 ──
		ViewModel.NoneTitle = IdleProject::Localization::UI(TEXT("GUILD_NONE_TITLE"));
		ViewModel.ListEmptyLabel = IdleProject::Localization::UI(TEXT("GUILD_LIST_EMPTY"));
		for (const FGuildSummary& Summary : BrowseList)
		{
			FIdleHUDGuildListRowViewModel Row;
			Row.GuildId = Summary.Id;
			Row.NameLabel = FText::FromString(Summary.Name);
			Row.InfoLabel = FormatLocalizedUI(TEXT("GUILD_LIST_ROW_FORMAT"), [&Summary](FFormatNamedArguments& Args)
			{
				Args.Add(TEXT("Level"), FText::AsNumber(Summary.Level));
				Args.Add(TEXT("Count"), FText::AsNumber(Summary.MemberCount));
			});
			Row.bApproval = Summary.JoinMode == EGuildJoinMode::Approval;
			Row.JoinLabel = IdleProject::Localization::UI(Row.bApproval ? TEXT("GUILD_JOIN_REQUEST") : TEXT("GUILD_JOIN"));
			Row.JoinHitBoxName = FName(*(GuildJoinHitBoxPrefix + Summary.Id));
			ViewModel.ListRows.Add(MoveTemp(Row));
		}

		ViewModel.CreateTitle = IdleProject::Localization::UI(TEXT("GUILD_CREATE_TITLE"));
		const FText PendingName = PendingCreateName.IsEmpty() ? IdleProject::Localization::UI(TEXT("NONE_DASH")) : FText::FromString(PendingCreateName);
		ViewModel.CreateNameLabel = FormatLocalizedUI(TEXT("GUILD_CREATE_NAME_FORMAT"), [&PendingName](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Name"), PendingName);
		});
		ViewModel.CreateNameCycleLabel = IdleProject::Localization::UI(TEXT("GUILD_CREATE_NAME_CYCLE"));
		ViewModel.CreateLabel = IdleProject::Localization::UI(TEXT("GUILD_CREATE"));
		ViewModel.CreateNameCycleHitBoxName = GuildCreateNameCycleHitBoxName;
		ViewModel.CreateHitBoxName = GuildCreateHitBoxName;
		return ViewModel;
	}

	// ── 내 길드 화면 ──
	const FGuildSummary Summary = GuildService.GetGuildSummary();
	const EGuildRank MyRank = GuildService.GetMyRank();
	ViewModel.MyTitle = IdleProject::Localization::UI(TEXT("GUILD_MY_TITLE"));
	ViewModel.GuildNameLabel = FText::FromString(Summary.Name);
	ViewModel.SummaryLabel = FormatLocalizedUI(TEXT("GUILD_SUMMARY_FORMAT"), [&Summary](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Level"), FText::AsNumber(Summary.Level));
		Args.Add(TEXT("Count"), FText::AsNumber(Summary.MemberCount));
		Args.Add(TEXT("Mode"), GuildJoinModeLabel(Summary.JoinMode));
	});
	ViewModel.MyRankBadgeLabel = IdleProject::Localization::UI(GuildRankToLocalizationKey(MyRank));
	ViewModel.LeaveLabel = IdleProject::Localization::UI(TEXT("GUILD_LEAVE"));
	ViewModel.LeaveHitBoxName = GuildLeaveHitBoxName;

	// ── 길드 레벨/EXP/버프/기여(PR-G2) ──
	const FGuildSnapshot& Snapshot = GuildService.GetSnapshot();
	const int32 GuildLevel = GuildService.GetGuildLevel();
	const FGuildBuff Buff = GuildService.GetGuildBuff();
	const int64 ContributionPoints = GuildService.GetContributionPoints();
	ViewModel.LevelLabel = FormatLocalizedUI(TEXT("GUILD_LEVEL_FORMAT"), [GuildLevel](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Level"), FText::AsNumber(GuildLevel));
	});
	int64 ExpInto = 0;
	int64 ExpSpan = 0;
	GuildComputeExpProgress(Snapshot.GuildExp, GuildLevel, ExpInto, ExpSpan, ViewModel.ExpProgressRatio);
	ViewModel.ExpLabel = FormatLocalizedUI(TEXT("GUILD_EXP_FORMAT"), [ExpInto, ExpSpan](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Into"), FText::AsNumber(ExpInto));
		Args.Add(TEXT("Span"), FText::AsNumber(ExpSpan));
	});
	// 버프는 비율(0.04=+4%) → %.1f%% 로 표기.
	const float AttackPercent = Buff.AttackPct * 100.0f;
	const float GoldPercent = Buff.GoldPct * 100.0f;
	ViewModel.BuffLabel = FormatLocalizedUI(TEXT("GUILD_BUFF_FORMAT"), [AttackPercent, GoldPercent](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Attack"), FText::FromString(FString::Printf(TEXT("%.1f"), AttackPercent)));
		Args.Add(TEXT("Gold"), FText::FromString(FString::Printf(TEXT("%.1f"), GoldPercent)));
	});
	ViewModel.ContributionLabel = FormatLocalizedUIWithInt64(TEXT("GUILD_CONTRIBUTION_FORMAT"), TEXT("Points"), ContributionPoints);
	ViewModel.WeeklyContributionLabel = FormatLocalizedUIWithInt64(TEXT("GUILD_WEEKLY_CONTRIBUTION_FORMAT"), TEXT("Weekly"), Snapshot.WeeklyContribution);

	// 일일 출석 — 가능 시 활성(오프라인/액션 중 비활성은 Draw 단계 가드).
	ViewModel.bCanAttend = Snapshot.bCanAttendToday;
	ViewModel.AttendLabel = IdleProject::Localization::UI(Snapshot.bCanAttendToday ? TEXT("GUILD_ATTEND") : TEXT("GUILD_ATTEND_DONE"));
	ViewModel.AttendHitBoxName = GuildAttendHitBoxName;

	// 헌납 — 프리셋 금액 순환, 보유 골드+일일 상한 모두 충족 시 활성.
	const int64 DonateClamped = FMath::Max<int64>(0, DonateAmount);
	const bool bAffordDonate = PlayerGold >= DonateClamped && DonateClamped > 0;
	ViewModel.bCanDonate = Snapshot.bCanDonateToday && bAffordDonate;
	ViewModel.DonateLabel = FormatLocalizedUIWithInt64(TEXT("GUILD_DONATE_FORMAT"), TEXT("Amount"), DonateClamped);
	ViewModel.DonateHitBoxName = GuildDonateHitBoxName;
	ViewModel.DonateCycleLabel = IdleProject::Localization::UI(TEXT("GUILD_DONATE_CYCLE"));
	ViewModel.DonateCycleHitBoxName = GuildDonateCycleHitBoxName;

	// 길드 상점 — 별도 fetch 한 카탈로그(없으면 빈 안내). 포인트 부족 시 비활성.
	ViewModel.ShopTitle = IdleProject::Localization::UI(TEXT("GUILD_SHOP_TITLE"));
	ViewModel.ShopEmptyLabel = IdleProject::Localization::UI(TEXT("GUILD_SHOP_EMPTY"));
	for (const FGuildShopItemInfo& Item : ShopItems)
	{
		FIdleHUDGuildShopRowViewModel Row;
		Row.ItemId = Item.Id;
		Row.NameLabel = GuildShopItemNameLabel(Item);
		Row.PriceLabel = FormatLocalizedUIWithInt64(TEXT("GUILD_SHOP_PRICE_FORMAT"), TEXT("Price"), Item.Price);
		const bool bAfford = ContributionPoints >= Item.Price;
		Row.bCanBuy = bAfford;
		Row.BuyLabel = IdleProject::Localization::UI(bAfford ? TEXT("GUILD_SHOP_BUY") : TEXT("GUILD_SHOP_INSUFFICIENT"));
		Row.BuyHitBoxName = FName(*(GuildShopBuyHitBoxPrefix + Item.Id));
		ViewModel.ShopRows.Add(MoveTemp(Row));
	}

	// ── 길드 보스(PR-G3, 공유 HP 풀 — 서버 권위 표시) ──
	// HP = getGuildBossHp(defeated)(서버 snapshot.boss.hp 우선, 0 이면 공식으로 폴백).
	const int64 BossDefeated = Snapshot.BossDefeatedCount;
	const int64 BossAccum = FMath::Max<int64>(0, Snapshot.BossAccumDamage);
	const int64 BossHp = Snapshot.BossHp > 0 ? Snapshot.BossHp : FGuildBossFormula::GetGuildBossHp(Snapshot.BossDefeatedCount);
	ViewModel.BossTitle = IdleProject::Localization::UI(TEXT("GUILD_BOSS_TITLE"));
	ViewModel.BossHpRatio = BossHp > 0 ? FMath::Clamp(static_cast<float>(static_cast<double>(BossAccum) / static_cast<double>(BossHp)), 0.0f, 1.0f) : 0.0f;
	ViewModel.BossHpLabel = FormatLocalizedUI(TEXT("GUILD_BOSS_HP_FORMAT"), [BossAccum, BossHp](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Accum"), FText::FromString(FormatIntegerWithCommas(BossAccum)));
		Args.Add(TEXT("Hp"), FText::FromString(FormatIntegerWithCommas(BossHp)));
	});
	ViewModel.BossDefeatedLabel = FormatLocalizedUI(TEXT("GUILD_BOSS_DEFEATED_FORMAT"), [BossDefeated](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Count"), FText::AsNumber(BossDefeated));
	});
	const int32 BossRemaining = FMath::Max(0, Snapshot.BossChallengesRemaining);
	ViewModel.bCanChallengeBoss = BossRemaining > 0;
	ViewModel.BossChallengeLabel = FormatLocalizedUI(TEXT("GUILD_BOSS_CHALLENGE_FORMAT"), [BossRemaining](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Remaining"), FText::AsNumber(BossRemaining));
	});
	const int32 BossUnclaimed = FMath::Max(0, Snapshot.BossUnclaimedDefeats);
	ViewModel.bCanClaimBoss = BossUnclaimed > 0;
	ViewModel.BossClaimLabel = FormatLocalizedUI(TEXT("GUILD_BOSS_CLAIM_FORMAT"), [BossUnclaimed](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Count"), FText::AsNumber(BossUnclaimed));
	});

	// ── 주간 길드 랭킹 탭(PR-G3) ──
	ViewModel.bRankingsView = bRankingsView;
	ViewModel.MyTabLabel = IdleProject::Localization::UI(TEXT("GUILD_TAB_MY"));
	ViewModel.RankingsTabLabel = IdleProject::Localization::UI(TEXT("GUILD_TAB_RANKINGS"));
	if (bRankingsView)
	{
		ViewModel.RankingsTitle = IdleProject::Localization::UI(TEXT("GUILD_RANKINGS_TITLE"));
		ViewModel.RankingsEmptyLabel = IdleProject::Localization::UI(TEXT("GUILD_RANKINGS_EMPTY"));
		ViewModel.RankingsLoadingLabel = IdleProject::Localization::UI(TEXT("GUILD_RANKINGS_LOADING"));
		ViewModel.MyRankingTitle = IdleProject::Localization::UI(TEXT("GUILD_RANKINGS_MY_TITLE"));
		const FString SelfGuildId = Summary.Id;
		ViewModel.MyRankingLabel = MyRanking.Rank > 0
			? FormatLocalizedUI(TEXT("GUILD_RANKINGS_MY_FORMAT"), [&MyRanking](FFormatNamedArguments& Args)
			{
				Args.Add(TEXT("Rank"), FText::AsNumber(MyRanking.Rank));
				Args.Add(TEXT("Weekly"), FText::FromString(FormatIntegerWithCommas(MyRanking.WeeklyContribution)));
			})
			: IdleProject::Localization::UI(TEXT("GUILD_RANKINGS_MY_UNRANKED"));
		for (const FGuildRankingEntry& Entry : Rankings)
		{
			FIdleHUDGuildRankingRowViewModel Row;
			Row.GuildId = Entry.GuildId;
			Row.bSelf = !SelfGuildId.IsEmpty() && Entry.GuildId == SelfGuildId;
			Row.RankLabel = FormatLocalizedUI(TEXT("GUILD_RANKINGS_RANK_FORMAT"), [&Entry](FFormatNamedArguments& Args)
			{
				Args.Add(TEXT("Rank"), FText::AsNumber(Entry.Rank));
			});
			Row.NameLabel = FText::FromString(Entry.Name);
			Row.InfoLabel = FormatLocalizedUI(TEXT("GUILD_RANKINGS_INFO_FORMAT"), [&Entry](FFormatNamedArguments& Args)
			{
				Args.Add(TEXT("Level"), FText::AsNumber(Entry.Level));
			});
			Row.ContributionLabel = FormatLocalizedUIWithInt64(TEXT("GUILD_WEEKLY_CONTRIBUTION_FORMAT"), TEXT("Weekly"), Entry.WeeklyContribution);
			ViewModel.RankingRows.Add(MoveTemp(Row));
		}
	}

	ViewModel.MemberListTitle = IdleProject::Localization::UI(TEXT("GUILD_MEMBER_LIST_TITLE"));

	const TArray<FGuildMemberInfo>& Members = GuildService.GetMembers();
	const int32 MemberCount = Summary.MemberCount > 0 ? Summary.MemberCount : Members.Num();
	const bool bManage = MyRank == EGuildRank::Master;

	// 현재 계급별 인원(승급 정원 검증용).
	int32 ViceCount = 0;
	int32 OfficerCount = 0;
	for (const FGuildMemberInfo& Member : Members)
	{
		if (Member.Rank == EGuildRank::Vice)
		{
			++ViceCount;
		}
		else if (Member.Rank == EGuildRank::Officer)
		{
			++OfficerCount;
		}
	}

	for (const FGuildMemberInfo& Member : Members)
	{
		FIdleHUDGuildMemberRowViewModel Row;
		Row.CharacterId = Member.CharacterId;
		Row.NicknameLabel = Member.Nickname.IsEmpty() ? GuildShortCharacterLabel(Member.CharacterId) : FText::FromString(Member.Nickname);
		Row.Rank = Member.Rank;
		Row.RankBadgeLabel = IdleProject::Localization::UI(GuildRankToLocalizationKey(Member.Rank));

		// 길드장 관리: 본인/길드장 외 멤버에 승급/강등 노출.
		if (bManage && Member.Rank != EGuildRank::Master)
		{
			// 승급 대상 계급: Member→Officer, Officer→Vice.
			if (Member.Rank == EGuildRank::Member || Member.Rank == EGuildRank::Officer)
			{
				Row.bShowPromote = true;
				Row.PromoteTargetRank = Member.Rank == EGuildRank::Member ? EGuildRank::Officer : EGuildRank::Vice;
				Row.PromoteHitBoxName = FName(*(GuildPromoteHitBoxPrefix + Member.CharacterId));

				const bool bUnlocked = FGuildFormula::IsRankUnlocked(Row.PromoteTargetRank, MemberCount);
				const int32 TargetCap = FGuildFormula::GetRankSlotCap(Row.PromoteTargetRank);
				const int32 TargetCount = Row.PromoteTargetRank == EGuildRank::Vice ? ViceCount : OfficerCount;
				const bool bSlotFree = TargetCount < TargetCap;
				Row.bCanPromote = bUnlocked && bSlotFree;
				Row.PromoteLabel = !bUnlocked
					? IdleProject::Localization::UI(TEXT("GUILD_RANK_LOCKED"))
					: (!bSlotFree ? IdleProject::Localization::UI(TEXT("GUILD_RANK_FULL")) : IdleProject::Localization::UI(TEXT("GUILD_PROMOTE")));
			}

			// 강등: Vice/Officer → Member.
			if (Member.Rank == EGuildRank::Vice || Member.Rank == EGuildRank::Officer)
			{
				Row.bShowDemote = true;
				Row.DemoteHitBoxName = FName(*(GuildDemoteHitBoxPrefix + Member.CharacterId));
			}
		}
		ViewModel.MemberRows.Add(MoveTemp(Row));
	}

	// ── 길드장 관리(설정 토글 + 승인 큐) ──
	ViewModel.bShowManage = bManage;
	if (bManage)
	{
		ViewModel.ManageTitle = IdleProject::Localization::UI(TEXT("GUILD_MANAGE_TITLE"));
		ViewModel.ToggleJoinModeLabel = IdleProject::Localization::UI(TEXT("GUILD_SETTINGS_TOGGLE_JOINMODE"));
		ViewModel.ToggleJoinModeHitBoxName = GuildToggleJoinModeHitBoxName;
		ViewModel.RequestsTitle = IdleProject::Localization::UI(TEXT("GUILD_REQUESTS_TITLE"));
		ViewModel.RequestsEmptyLabel = IdleProject::Localization::UI(TEXT("GUILD_REQUESTS_EMPTY"));

		for (const FGuildJoinRequestInfo& Request : GuildService.GetRequests())
		{
			FIdleHUDGuildRequestRowViewModel Row;
			Row.CharacterId = Request.CharacterId;
			Row.CharacterLabel = GuildShortCharacterLabel(Request.CharacterId);
			Row.ApproveHitBoxName = FName(*(GuildApproveHitBoxPrefix + Request.CharacterId));
			Row.RejectHitBoxName = FName(*(GuildRejectHitBoxPrefix + Request.CharacterId));
			ViewModel.RequestRows.Add(MoveTemp(Row));
		}
	}

	return ViewModel;
}

FIdleHUDRuneViewModel IdleProject::UI::BuildRuneViewModel(const URuneService& RuneService, int64 RuneEssence, int64 Gold, int32 ProgressIndex, int32 SelectedOwnedIndex, int32 TransferTargetOwnedIndex)
{
	FIdleHUDRuneViewModel ViewModel;
	ViewModel.Title = RuneText(TEXT("RUNE_PANEL_TITLE"));
	ViewModel.RuneEssence = FMath::Max<int64>(0, RuneEssence);
	ViewModel.Gold = FMath::Max<int64>(0, Gold);
	ViewModel.SelectedOwnedIndex = SelectedOwnedIndex;
	ViewModel.ShopCost = FRuneFormula::GetShopRuneRollCost(ProgressIndex);
	ViewModel.ClassCraftCost = FClassRuneFormula::GetClassRuneCraftCost(EItemRarity::Common);
	ViewModel.ShopHitBoxName = ShopRuneRollHitBoxName;
	ViewModel.ClassCraftHitBoxName = CraftClassRuneHitBoxName;
	ViewModel.EssenceLabel = FormatLocalizedRune(TEXT("RUNE_ESSENCE_FORMAT"), [&ViewModel](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Amount"), FText::FromString(FormatIntegerWithCommas(ViewModel.RuneEssence)));
	});
	ViewModel.GoldLabel = FormatLocalizedUIWithInt64(TEXT("HUD_GOLD_FORMAT"), TEXT("Amount"), ViewModel.Gold);
	ViewModel.ShopCostLabel = FormatLocalizedRune(TEXT("RUNE_SHOP_COST_FORMAT"), [&ViewModel](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Cost"), FText::FromString(FormatIntegerWithCommas(ViewModel.ShopCost)));
	});
	ViewModel.ShopButtonLabel = RuneText(TEXT("RUNE_SHOP_ROLL_BUTTON"));
	ViewModel.bCanBuyRuneRoll = ViewModel.ShopCost > 0 && ViewModel.Gold >= ViewModel.ShopCost;
	ViewModel.ShopStatusLabel = ViewModel.bCanBuyRuneRoll ? RuneText(TEXT("RUNE_STATUS_READY")) : RuneText(TEXT("RUNE_STATUS_NEED_GOLD"));
	ViewModel.EmptyOwnedLabel = RuneText(TEXT("RUNE_OWNED_EMPTY"));
	ViewModel.ClassCraftCostLabel = FormatLocalizedRune(TEXT("RUNE_CLASS_CRAFT_COST_FORMAT"), [&ViewModel](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Essence"), FText::FromString(FormatIntegerWithCommas(ViewModel.ClassCraftCost)));
	});
	ViewModel.ClassCraftButtonLabel = RuneText(TEXT("RUNE_CLASS_CRAFT_BUTTON"));
	const EClassId OwnerClassId = RuneService.GetOwnerClassId();
	ViewModel.bCanCraftClassRune = OwnerClassId != EClassId::None
		&& ViewModel.ClassCraftCost > 0
		&& ViewModel.RuneEssence >= ViewModel.ClassCraftCost;

	const TArray<FRuneInstance>& OwnedRunes = RuneService.GetOwnedRunes();
	TSet<int32> EquippedOwnedIndexes;
	for (int32 SlotIndex = 0; SlotIndex < FRuneFormula::RuneSlotCount; ++SlotIndex)
	{
		const int32 OwnedIndex = RuneService.GetEquippedOwnedIndex(SlotIndex);
		if (OwnedRunes.IsValidIndex(OwnedIndex))
		{
			EquippedOwnedIndexes.Add(OwnedIndex);
		}
	}

	TMap<ERuneSet, int32> RuneSetCounts;
	const int32 RegularRuneSlotCount = FMath::Min(FClassRuneFormula::ClassRuneSlotIndex, FRuneFormula::RuneSlotCount);
	for (int32 SlotIndex = 0; SlotIndex < RegularRuneSlotCount; ++SlotIndex)
	{
		const int32 OwnedIndex = RuneService.GetEquippedOwnedIndex(SlotIndex);
		if (!OwnedRunes.IsValidIndex(OwnedIndex))
		{
			continue;
		}

		const FRuneInstance& Rune = OwnedRunes[OwnedIndex];
		if (Rune.RuneSet == ERuneSet::None || Rune.RuneType == ERuneType::ClassMastery)
		{
			continue;
		}
		RuneSetCounts.FindOrAdd(Rune.RuneSet) += 1;
	}

	ViewModel.SetTitle = RuneText(TEXT("RUNE_SET_TITLE"));
	const ERuneSet RuneSetOrder[] = { ERuneSet::Offense, ERuneSet::Bastion, ERuneSet::Vitality, ERuneSet::Fortune };
	for (const ERuneSet RuneSet : RuneSetOrder)
	{
		FIdleHUDRuneSetRowViewModel Row;
		Row.RuneSet = RuneSet;
		Row.SetLabel = RuneSetToLabel(RuneSet);
		Row.Count = FMath::Clamp(RuneSetCounts.FindRef(RuneSet), 0, RegularRuneSlotCount);
		Row.CountLabel = FormatLocalizedRune(TEXT("RUNE_SET_COUNT_FORMAT"), [&Row](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Set"), Row.SetLabel);
			Args.Add(TEXT("Count"), FText::AsNumber(Row.Count));
		});
		Row.TierLabel = FormatRuneSetTierLabel(Row.Count);
		Row.BonusLabel = FormatRuneSetBonusLabel(RuneSet, Row.Count);
		Row.NextTierLabel = FormatRuneSetNextTierLabel(Row.Count);
		Row.bTwoSetActive = Row.Count >= FRuneSetFormula::Tier1Count;
		Row.bFourSetActive = Row.Count >= FRuneSetFormula::Tier2Count;
		Row.bSixSetActive = Row.Count >= FRuneSetFormula::Tier3Count;
		Row.bActive = Row.bTwoSetActive;
		ViewModel.SetRows.Add(Row);
	}

	for (int32 SlotIndex = 0; SlotIndex < FRuneFormula::RuneSlotCount; ++SlotIndex)
	{
		FIdleHUDRuneSlotViewModel Slot;
		Slot.SlotIndex = SlotIndex;
		Slot.SlotLabel = SlotIndex == FClassRuneFormula::ClassRuneSlotIndex
			? RuneText(TEXT("RUNE_CLASS_SLOT_TITLE"))
			: FormatLocalizedRune(TEXT("RUNE_SLOT_FORMAT"), [SlotIndex](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Slot"), FText::AsNumber(SlotIndex + 1));
		});

		Slot.OwnedIndex = RuneService.GetEquippedOwnedIndex(SlotIndex);
		if (OwnedRunes.IsValidIndex(Slot.OwnedIndex))
		{
			const FRuneInstance& Rune = OwnedRunes[Slot.OwnedIndex];
			Slot.bEquipped = true;
			Slot.TypeLabel = ClassRuneTypeToLabel(Rune);
			Slot.RarityLabel = IdleProject::UI::RarityToLabel(Rune.Rarity);
			Slot.RarityColor = IdleProject::UI::RarityToColor(Rune.Rarity);
			Slot.LevelLabel = FormatLocalizedRune(TEXT("RUNE_LEVEL_FORMAT"), [&Rune](FFormatNamedArguments& Args)
			{
				Args.Add(TEXT("Level"), FText::AsNumber(FMath::Max(0, Rune.EnhanceLevel)));
			});
			Slot.ValueLabel = FormatRuneValueLabel(Rune);
			Slot.StatusLabel = RuneText(TEXT("RUNE_STATUS_EQUIPPED"));
			Slot.ActionLabel = RuneText(TEXT("RUNE_ACTION_UNEQUIP"));
			Slot.ActionHitBoxName = MakeRuneUnequipHitBoxName(SlotIndex);
			Slot.bCanUnequip = true;
		}
		else
		{
			Slot.TypeLabel = SlotIndex == FClassRuneFormula::ClassRuneSlotIndex ? RuneText(TEXT("RUNE_CLASS_SLOT_EMPTY")) : RuneText(TEXT("RUNE_SLOT_EMPTY"));
			Slot.RarityLabel = IdleProject::Localization::UI(TEXT("NONE_DASH"));
			Slot.RarityColor = IdleProject::UI::Theme::TextMuted;
			Slot.LevelLabel = IdleProject::Localization::UI(TEXT("NONE_DASH"));
			Slot.ValueLabel = IdleProject::Localization::UI(TEXT("NONE_DASH"));
			Slot.bCanEquipSelected = CanEquipRuneIntoSlot(OwnedRunes, SelectedOwnedIndex, SlotIndex, OwnerClassId);
			if (OwnedRunes.IsValidIndex(SelectedOwnedIndex) && !Slot.bCanEquipSelected)
			{
				Slot.StatusLabel = RuneText(TEXT("RUNE_CLASS_MISMATCH"));
			}
			else
			{
				Slot.StatusLabel = Slot.bCanEquipSelected ? RuneText(TEXT("RUNE_STATUS_SELECT_READY")) : RuneText(TEXT("RUNE_STATUS_SELECT_REQUIRED"));
			}
			Slot.ActionLabel = RuneText(TEXT("RUNE_ACTION_EQUIP"));
			Slot.ActionHitBoxName = MakeRuneEquipHitBoxName(SlotIndex);
		}
		ViewModel.Slots.Add(Slot);
	}

	for (int32 OwnedIndex = 0; OwnedIndex < OwnedRunes.Num(); ++OwnedIndex)
	{
		const FRuneInstance& Rune = OwnedRunes[OwnedIndex];
		FIdleHUDRuneOwnedRowViewModel Row;
		Row.OwnedIndex = OwnedIndex;
		Row.RuneType = Rune.RuneType;
		Row.Rarity = Rune.Rarity;
		Row.TypeLabel = ClassRuneTypeToLabel(Rune);
		Row.RarityLabel = IdleProject::UI::RarityToLabel(Rune.Rarity);
		Row.RarityColor = IdleProject::UI::RarityToColor(Rune.Rarity);
		Row.LevelLabel = FormatLocalizedRune(TEXT("RUNE_LEVEL_FORMAT"), [&Rune](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Level"), FText::AsNumber(FMath::Max(0, Rune.EnhanceLevel)));
		});
		Row.ValueLabel = FormatRuneValueLabel(Rune);
		Row.EnhanceEssenceCost = FRuneFormula::GetEnhanceEssenceCost(Rune.EnhanceLevel);
		Row.EnhanceGoldCost = FRuneFormula::GetEnhanceGoldCost(Rune.EnhanceLevel);
		Row.DisenchantEssence = FRuneFormula::GetDisenchantEssence(Rune.Rarity, Rune.EnhanceLevel);
		Row.EnhanceCostLabel = FormatLocalizedRune(TEXT("RUNE_ENHANCE_COST_FORMAT"), [&Row](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Essence"), FText::FromString(FormatIntegerWithCommas(Row.EnhanceEssenceCost)));
			Args.Add(TEXT("Gold"), FText::FromString(FormatIntegerWithCommas(Row.EnhanceGoldCost)));
		});
		Row.EnhancePreviewLabel = FormatRuneEnhancePreviewLabel(Rune);
		Row.DisenchantLabel = FormatLocalizedRune(TEXT("RUNE_DISENCHANT_REFUND_FORMAT"), [&Row](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Essence"), FText::FromString(FormatIntegerWithCommas(Row.DisenchantEssence)));
		});
		Row.SelectActionLabel = RuneText(OwnedIndex == SelectedOwnedIndex ? TEXT("RUNE_ACTION_SELECTED") : TEXT("RUNE_ACTION_SELECT"));
		Row.EnhanceActionLabel = RuneText(TEXT("RUNE_ACTION_ENHANCE"));
		Row.DisenchantActionLabel = RuneText(TEXT("RUNE_ACTION_DISENCHANT"));
		Row.SelectHitBoxName = MakeRuneSelectHitBoxName(OwnedIndex);
		Row.EnhanceHitBoxName = MakeRuneEnhanceHitBoxName(OwnedIndex);
		Row.DisenchantHitBoxName = MakeRuneDisenchantHitBoxName(OwnedIndex);
		Row.bEquipped = EquippedOwnedIndexes.Contains(OwnedIndex);
		Row.bSelected = OwnedIndex == SelectedOwnedIndex;
		Row.bCanEnhance = ViewModel.RuneEssence >= Row.EnhanceEssenceCost && ViewModel.Gold >= Row.EnhanceGoldCost;
		Row.bCanDisenchant = !Row.bEquipped && Row.DisenchantEssence > 0;
		ViewModel.OwnedRows.Add(Row);
	}

	// 선택 룬 액션(세트 리롤 / 등급 상승 / 강화 전송) — 룬 확장4.
	FIdleHUDRuneActionViewModel& Action = ViewModel.Action;
	Action.TitleLabel = RuneText(TEXT("RUNE_ACTION_SECTION_TITLE"));
	Action.EmptyLabel = RuneText(TEXT("RUNE_ACTION_NO_SELECTION"));
	Action.RerollHitBoxName = RuneRerollSetHitBoxName;
	Action.UpgradeHitBoxName = RuneUpgradeRarityHitBoxName;
	Action.TransferHitBoxName = RuneTransferHitBoxName;
	Action.TransferCycleHitBoxName = RuneTransferCycleHitBoxName;
	Action.RerollActionLabel = RuneText(TEXT("RUNE_ACTION_REROLL_SET"));
	Action.UpgradeActionLabel = RuneText(TEXT("RUNE_ACTION_UPGRADE_RARITY"));
	Action.TransferActionLabel = RuneText(TEXT("RUNE_ACTION_TRANSFER"));
	Action.TransferCycleLabel = RuneText(TEXT("RUNE_TRANSFER_CYCLE"));
	Action.SourceOwnedIndex = SelectedOwnedIndex;
	Action.bHasSelection = OwnedRunes.IsValidIndex(SelectedOwnedIndex);

	if (Action.bHasSelection)
	{
		const FRuneInstance& Source = OwnedRunes[SelectedOwnedIndex];

		// 세트 리롤
		Action.CurrentSetLabel = FormatLocalizedRune(TEXT("RUNE_CURRENT_SET_FORMAT"), [&Source](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Set"), RuneSetToLabel(Source.RuneSet));
		});
		Action.RerollEssenceCost = FRuneFormula::GetRerollSetEssenceCost(Source.Rarity);
		Action.RerollCostLabel = FormatLocalizedRune(TEXT("RUNE_REROLL_COST_FORMAT"), [&Action](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Essence"), FText::FromString(FormatIntegerWithCommas(Action.RerollEssenceCost)));
		});
		Action.bCanReroll = Action.RerollEssenceCost > 0 && ViewModel.RuneEssence >= Action.RerollEssenceCost;

		// 등급 상승 시도
		Action.bIsMythic = Source.Rarity >= EItemRarity::Mythic;
		Action.UpgradeEssenceCost = FRuneFormula::GetRarityUpgradeEssenceCost(Source.Rarity);
		Action.UpgradeGoldCost = FRuneFormula::GetRarityUpgradeGoldCost(Source.Rarity);
		Action.UpgradeChance = FRuneFormula::GetRarityUpgradeChance(Source.Rarity);
		if (Action.bIsMythic)
		{
			Action.UpgradeInfoLabel = RuneText(TEXT("RUNE_UPGRADE_MAX"));
			Action.bCanUpgrade = false;
		}
		else
		{
			const EItemRarity NextRarity = static_cast<EItemRarity>(static_cast<int32>(Source.Rarity) + 1);
			const int32 ChancePercent = FMath::RoundToInt(Action.UpgradeChance * 100.0f);
			Action.UpgradeInfoLabel = FormatLocalizedRune(TEXT("RUNE_UPGRADE_INFO_FORMAT"), [&Action, NextRarity, ChancePercent](FFormatNamedArguments& Args)
			{
				Args.Add(TEXT("Next"), RarityToLabel(NextRarity));
				Args.Add(TEXT("Essence"), FText::FromString(FormatIntegerWithCommas(Action.UpgradeEssenceCost)));
				Args.Add(TEXT("Gold"), FText::FromString(FormatIntegerWithCommas(Action.UpgradeGoldCost)));
				Args.Add(TEXT("Chance"), FText::AsNumber(ChancePercent));
			});
			Action.bCanUpgrade = Action.UpgradeEssenceCost > 0
				&& ViewModel.RuneEssence >= Action.UpgradeEssenceCost
				&& ViewModel.Gold >= Action.UpgradeGoldCost;
		}

		// 강화 전송 — target = 선택 룬과 다른 유효 인덱스(없으면 INDEX_NONE).
		int32 ResolvedTarget = INDEX_NONE;
		if (OwnedRunes.IsValidIndex(TransferTargetOwnedIndex) && TransferTargetOwnedIndex != SelectedOwnedIndex)
		{
			ResolvedTarget = TransferTargetOwnedIndex;
		}
		else
		{
			for (int32 Candidate = 0; Candidate < OwnedRunes.Num(); ++Candidate)
			{
				if (Candidate != SelectedOwnedIndex)
				{
					ResolvedTarget = Candidate;
					break;
				}
			}
		}
		Action.TransferTargetOwnedIndex = ResolvedTarget;

		int32 OtherCount = 0;
		for (int32 Candidate = 0; Candidate < OwnedRunes.Num(); ++Candidate)
		{
			if (Candidate != SelectedOwnedIndex)
			{
				++OtherCount;
			}
		}
		Action.bCanCycleTarget = OtherCount >= 2;

		Action.TransferEssenceCost = FRuneFormula::GetTransferEssenceCost(Source.EnhanceLevel);
		Action.TransferCostLabel = FormatLocalizedRune(TEXT("RUNE_TRANSFER_COST_FORMAT"), [&Action](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Essence"), FText::FromString(FormatIntegerWithCommas(Action.TransferEssenceCost)));
		});

		if (OwnedRunes.IsValidIndex(ResolvedTarget))
		{
			const FRuneInstance& Target = OwnedRunes[ResolvedTarget];
			Action.TransferTargetLabel = FormatLocalizedRune(TEXT("RUNE_TRANSFER_TARGET_FORMAT"), [&Target](FFormatNamedArguments& Args)
			{
				Args.Add(TEXT("Type"), ClassRuneTypeToLabel(Target));
				Args.Add(TEXT("Rarity"), RarityToLabel(Target.Rarity));
				Args.Add(TEXT("Level"), FText::AsNumber(FMath::Max(0, Target.EnhanceLevel)));
			});
			Action.bCanTransfer = ViewModel.RuneEssence >= Action.TransferEssenceCost;
		}
		else
		{
			Action.TransferTargetLabel = RuneText(TEXT("RUNE_TRANSFER_NO_TARGET"));
			Action.bCanTransfer = false;
		}
	}

	return ViewModel;
}

FIdleHUDRuneCodexViewModel IdleProject::UI::BuildRuneCodexViewModel(const URuneService& RuneService)
{
	FIdleHUDRuneCodexViewModel ViewModel;
	ViewModel.Title = RuneText(TEXT("RUNE_CODEX_TITLE"));

	const FRuneCodexCompletion Completion = RuneService.GetCodexCompletion();
	const FRuneCodexBonus Bonus = RuneService.GetCodexBonus();
	ViewModel.UnlockedCells = FMath::Clamp(Completion.UnlockedCells, 0, FRuneCodexFormula::TotalCells);
	ViewModel.TotalCells = FRuneCodexFormula::TotalCells;
	ViewModel.ProgressRatio = static_cast<float>(ViewModel.UnlockedCells) / static_cast<float>(ViewModel.TotalCells);
	ViewModel.bCoreCategoryComplete = Completion.bCoreCategoryComplete;
	ViewModel.bUtilCategoryComplete = Completion.bUtilCategoryComplete;
	ViewModel.ProgressLabel = FormatLocalizedRune(TEXT("RUNE_CODEX_PROGRESS_FORMAT"), [&ViewModel](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Unlocked"), FText::AsNumber(ViewModel.UnlockedCells));
		Args.Add(TEXT("Total"), FText::AsNumber(ViewModel.TotalCells));
	});
	ViewModel.CoreBonusLabel = FormatRuneCodexPercentLabel(TEXT("RUNE_CODEX_CORE_BONUS_FORMAT"), Bonus.CoreStatAdd);
	ViewModel.UtilCapLabel = FormatRuneCodexPercentLabel(TEXT("RUNE_CODEX_UTIL_CAP_FORMAT"), Bonus.UtilCapExtension);

	TMap<int32, bool> UnlockedByCellKey;
	for (const FRuneCodexEntry& Entry : RuneService.GetOwnedCodex())
	{
		if (!Entry.bUnlocked || !IsCodexRuneType(Entry.RuneType) || Entry.Rarity < EItemRarity::Common || Entry.Rarity > EItemRarity::Mythic)
		{
			continue;
		}
		UnlockedByCellKey.Add(GetCodexCellKey(Entry.RuneType, Entry.Rarity), true);
	}

	for (int32 TypeValue = static_cast<int32>(ERuneType::PhysAtk); TypeValue <= static_cast<int32>(ERuneType::OfflineEff); ++TypeValue)
	{
		ViewModel.ColumnLabels.Add(RuneTypeToLabel(static_cast<ERuneType>(TypeValue)));
	}

	TArray<int32> RowUnlocked;
	RowUnlocked.Init(0, 7);
	for (int32 RowIndex = 0; RowIndex < 7; ++RowIndex)
	{
		const EItemRarity Rarity = RarityFromCodexRow(RowIndex);
		for (int32 TypeValue = static_cast<int32>(ERuneType::PhysAtk); TypeValue <= static_cast<int32>(ERuneType::OfflineEff); ++TypeValue)
		{
			const ERuneType RuneType = static_cast<ERuneType>(TypeValue);
			const bool bUnlocked = UnlockedByCellKey.Contains(GetCodexCellKey(RuneType, Rarity));
			if (bUnlocked)
			{
				++RowUnlocked[RowIndex];
				if (FRuneFormula::IsCoreType(RuneType))
				{
					++ViewModel.CoreUnlockedCells;
				}
				else
				{
					++ViewModel.UtilUnlockedCells;
				}
			}

			FIdleHUDRuneCodexCellViewModel Cell;
			Cell.RuneType = RuneType;
			Cell.Rarity = Rarity;
			Cell.RowIndex = RowIndex;
			Cell.ColumnIndex = GetCodexColumnIndex(RuneType);
			Cell.bUnlocked = bUnlocked;
			Cell.bCoreType = FRuneFormula::IsCoreType(RuneType);
			Cell.TypeLabel = RuneTypeToLabel(RuneType);
			Cell.RarityLabel = RarityToLabel(Rarity);
			Cell.StatusLabel = RuneText(bUnlocked ? TEXT("RUNE_CODEX_CELL_UNLOCKED") : TEXT("RUNE_CODEX_CELL_LOCKED"));
			Cell.AccentColor = bUnlocked ? RarityToColor(Rarity) : Theme::TextMuted.CopyWithNewOpacity(0.42f);
			ViewModel.Cells.Add(Cell);
		}
	}

	for (int32 RowIndex = 0; RowIndex < 7; ++RowIndex)
	{
		const EItemRarity Rarity = RarityFromCodexRow(RowIndex);
		FIdleHUDRuneCodexRowViewModel Row;
		Row.Rarity = Rarity;
		Row.RowIndex = RowIndex;
		Row.RarityLabel = RarityToLabel(Rarity);
		Row.AccentColor = RarityToColor(Rarity);
		Row.UnlockedCount = RowUnlocked[RowIndex];
		Row.bComplete = Completion.RowComplete.IsValidIndex(RowIndex) && Completion.RowComplete[RowIndex];
		Row.BonusLabel = FormatLocalizedRune(TEXT("RUNE_CODEX_ROW_BONUS_FORMAT"), [Rarity](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Rarity"), RarityToLabel(Rarity));
			Args.Add(TEXT("Percent"), FText::AsNumber(FMath::RoundToInt(FRuneCodexFormula::GetRowCompletionBonus(Rarity) * 100.0f)));
		});
		ViewModel.Rows.Add(Row);
	}

	ViewModel.CoreCategoryLabel = ViewModel.bCoreCategoryComplete
		? RuneText(TEXT("RUNE_CODEX_CORE_CATEGORY_COMPLETE"))
		: FormatLocalizedRune(TEXT("RUNE_CODEX_CORE_CATEGORY_PROGRESS_FORMAT"), [&ViewModel](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Unlocked"), FText::AsNumber(ViewModel.CoreUnlockedCells));
			Args.Add(TEXT("Total"), FText::AsNumber(FRuneCodexFormula::CoreCategoryCells));
		});
	ViewModel.UtilCategoryLabel = ViewModel.bUtilCategoryComplete
		? RuneText(TEXT("RUNE_CODEX_UTIL_CATEGORY_COMPLETE"))
		: FormatLocalizedRune(TEXT("RUNE_CODEX_UTIL_CATEGORY_PROGRESS_FORMAT"), [&ViewModel](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Unlocked"), FText::AsNumber(ViewModel.UtilUnlockedCells));
			Args.Add(TEXT("Total"), FText::AsNumber(FRuneCodexFormula::UtilCategoryCells));
		});

	return ViewModel;
}

FIdleHUDStatPanelViewModel IdleProject::UI::BuildStatPanelViewModel(const FPrimaryStats& BaseStats, const FPrimaryStats& AllocatedStats, int32 AvailablePoints)
{
	FIdleHUDStatPanelViewModel ViewModel;
	ViewModel.Title = IdleProject::Localization::UI(TEXT("STAT_PANEL_TITLE"));
	ViewModel.AvailablePoints = FMath::Max(0, AvailablePoints);
	ViewModel.AvailableLabel = FormatLocalizedUI(TEXT("STAT_AVAILABLE_FORMAT"), [&ViewModel](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Points"), FText::AsNumber(ViewModel.AvailablePoints));
	});
	ViewModel.ResetLabel = IdleProject::Localization::UI(TEXT("STAT_RESET"));
	ViewModel.ResetHitBoxName = StatResetHitBoxName;

	const EPrimaryStat StatOrder[] = {
		EPrimaryStat::Str,
		EPrimaryStat::Dex,
		EPrimaryStat::Int,
		EPrimaryStat::Wis,
		EPrimaryStat::Con,
		EPrimaryStat::Luk
	};

	for (const EPrimaryStat Stat : StatOrder)
	{
		FIdleHUDStatRowViewModel Row;
		Row.Stat = Stat;
		Row.StatLabel = FText::FromString(PrimaryStatToLabel(Stat));
		Row.AllocationHitBoxName = MakeStatAllocationHitBoxName(Stat);
		Row.BaseValue = FMath::RoundToInt(GetPrimaryStatValue(BaseStats, Stat));
		Row.AllocatedValue = FMath::Max(0, FMath::RoundToInt(GetPrimaryStatValue(AllocatedStats, Stat)));
		Row.TotalValue = Row.BaseValue + Row.AllocatedValue;
		Row.bCanAllocate = ViewModel.AvailablePoints > 0;
		Row.ValueLabel = FormatLocalizedUI(TEXT("STAT_VALUE_FORMAT"), [&Row](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Total"), FText::AsNumber(Row.TotalValue));
			Args.Add(TEXT("Allocated"), FText::AsNumber(Row.AllocatedValue));
		});

		ViewModel.bCanReset = ViewModel.bCanReset || Row.AllocatedValue > 0;
		ViewModel.Rows.Add(Row);
	}

	return ViewModel;
}

FIdleHUDStatInfoViewModel IdleProject::UI::BuildStatInfoViewModel(const FPrimaryStats& PrimaryStats, const FDerivedStats& DerivedStats, int32 Level, EClassId ClassId, int32 RebirthCount, int64 CombatPower)
{
	FIdleHUDStatInfoViewModel ViewModel;
	ViewModel.Title = IdleProject::Localization::UI(TEXT("STAT_INFO_TITLE"));
	ViewModel.ToggleLabel = IdleProject::Localization::UI(TEXT("STAT_INFO_TOGGLE"));
	ViewModel.ToggleHitBoxName = StatInfoToggleHitBoxName;
	ViewModel.HeaderLabel = FormatLocalizedUI(TEXT("STAT_INFO_HEADER_FORMAT"), [ClassId, Level, RebirthCount](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Class"), IdleProject::Localization::UI(ClassToLocalizationKey(ClassId)));
		Args.Add(TEXT("Level"), FText::AsNumber(FMath::Max(1, Level)));
		Args.Add(TEXT("Rebirth"), FText::AsNumber(FMath::Max(0, RebirthCount)));
	});
	ViewModel.CombatPowerLabel = FormatLocalizedUIWithInt64(TEXT("HUD_COMBAT_POWER_FORMAT"), TEXT("Amount"), FMath::Max<int64>(0, CombatPower));

	const EPrimaryStat StatOrder[] = {
		EPrimaryStat::Str,
		EPrimaryStat::Dex,
		EPrimaryStat::Int,
		EPrimaryStat::Wis,
		EPrimaryStat::Con,
		EPrimaryStat::Luk
	};

	for (const EPrimaryStat Stat : StatOrder)
	{
		FIdleHUDStatInfoRowViewModel Row;
		Row.StatLabel = FText::FromString(PrimaryStatToLabel(Stat));
		Row.ValueLabel = FormatIntegerLabel(GetPrimaryStatValue(PrimaryStats, Stat));
		ViewModel.PrimaryRows.Add(Row);
	}

	auto AddDerivedRow = [&ViewModel](const TCHAR* LabelKey, FText ValueLabel)
	{
		FIdleHUDStatInfoRowViewModel Row;
		Row.StatLabel = IdleProject::Localization::UI(LabelKey);
		Row.ValueLabel = MoveTemp(ValueLabel);
		ViewModel.DerivedRows.Add(Row);
	};

	AddDerivedRow(TEXT("STAT_INFO_HP"), FormatIntegerLabel(DerivedStats.Hp));
	AddDerivedRow(TEXT("STAT_INFO_MP"), FormatIntegerLabel(DerivedStats.Mp));
	AddDerivedRow(TEXT("STAT_INFO_PHYS_ATK"), FormatIntegerLabel(DerivedStats.PhysAtk));
	AddDerivedRow(TEXT("STAT_INFO_MAGIC_ATK"), FormatIntegerLabel(DerivedStats.MagicAtk));
	AddDerivedRow(TEXT("STAT_INFO_PHYS_DEF"), FormatIntegerLabel(DerivedStats.PhysDef));
	AddDerivedRow(TEXT("STAT_INFO_MAGIC_DEF"), FormatIntegerLabel(DerivedStats.MagicDef));
	AddDerivedRow(TEXT("STAT_INFO_ATK_SPEED"), FormatStatInfoScalarPercentLabel(DerivedStats.AtkSpeed));
	AddDerivedRow(TEXT("STAT_INFO_CRIT_RATE"), FormatStatInfoPercentLabel(DerivedStats.CritRate));
	AddDerivedRow(TEXT("STAT_INFO_CRIT_DMG"), FormatMultiplierLabel(DerivedStats.CritDmg));
	AddDerivedRow(TEXT("STAT_INFO_DODGE"), FormatStatInfoPercentLabel(DerivedStats.Dodge));
	AddDerivedRow(TEXT("STAT_INFO_ACCURACY"), FormatStatInfoPercentLabel(DerivedStats.Accuracy));

	return ViewModel;
}

FIdleHUDTowerViewModel IdleProject::UI::BuildTowerViewModel(int32 HighestFloor, int64 NextRequiredPower, int64 CombatPower, float MilestoneMultiplier)
{
	FIdleHUDTowerViewModel ViewModel;
	ViewModel.HighestFloor = FMath::Max(0, HighestFloor);
	ViewModel.NextMilestoneFloor = ((ViewModel.HighestFloor / FTowerMilestoneFormula::MilestoneStep) + 1) * FTowerMilestoneFormula::MilestoneStep;
	ViewModel.NextRequiredPower = FMath::Max<int64>(0, NextRequiredPower);
	ViewModel.CombatPower = FMath::Max<int64>(0, CombatPower);
	ViewModel.MilestoneMultiplier = MilestoneMultiplier > 0.0f
		? MilestoneMultiplier
		: FTowerMilestoneFormula::GetTowerMilestoneMultiplier(ViewModel.HighestFloor);
	ViewModel.bCanClimb = ViewModel.NextRequiredPower > 0 && ViewModel.CombatPower >= ViewModel.NextRequiredPower;
	ViewModel.ClimbHitBoxName = TowerClimbHitBoxName;
	ViewModel.Title = IdleProject::Localization::UI(TEXT("TOWER_PANEL_TITLE"));
	ViewModel.HighestFloorLabel = FormatLocalizedUI(TEXT("TOWER_HIGHEST_FLOOR_FORMAT"), [&ViewModel](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Floor"), FText::AsNumber(ViewModel.HighestFloor));
	});
	ViewModel.NextRequiredPowerLabel = FormatLocalizedUI(TEXT("TOWER_NEXT_CP_FORMAT"), [&ViewModel](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Amount"), FText::FromString(FormatIntegerWithCommas(ViewModel.NextRequiredPower)));
	});
	ViewModel.CombatPowerLabel = FormatLocalizedUIWithInt64(TEXT("HUD_COMBAT_POWER_FORMAT"), TEXT("Amount"), ViewModel.CombatPower);
	ViewModel.MilestoneMultiplierLabel = FormatLocalizedUI(TEXT("TOWER_MILESTONE_MULTIPLIER_FORMAT"), [&ViewModel](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Multiplier"), FormatMultiplierLabel(ViewModel.MilestoneMultiplier));
	});
	ViewModel.NextMilestoneLabel = FormatLocalizedUI(TEXT("TOWER_NEXT_MILESTONE_FORMAT"), [&ViewModel](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Floor"), FText::AsNumber(ViewModel.NextMilestoneFloor));
	});
	ViewModel.StatusLabel = IdleProject::Localization::UI(ViewModel.bCanClimb ? TEXT("TOWER_STATUS_READY") : TEXT("TOWER_STATUS_NEED_CP"));
	ViewModel.ButtonLabel = IdleProject::Localization::UI(TEXT("TOWER_CLIMB_BUTTON"));
	return ViewModel;
}

FText IdleProject::UI::BuildTowerClimbFeedbackLabel(int32 NewHighestFloor, int64 TotalReward)
{
	const int32 SafeFloor = FMath::Max(0, NewHighestFloor);
	const int64 SafeReward = FMath::Max<int64>(0, TotalReward);
	return FormatLocalizedUI(TEXT("TOWER_CLIMB_FEEDBACK_FORMAT"), [SafeFloor, SafeReward](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Floor"), FText::AsNumber(SafeFloor));
		Args.Add(TEXT("Gold"), FText::FromString(FormatIntegerWithCommas(SafeReward)));
	});
}

FIdleHUDDungeonPanelViewModel IdleProject::UI::BuildDungeonPanelViewModel(const UDungeonService& DungeonService, int64 CombatPower, const FString& TodayUtc)
{
	FIdleHUDDungeonPanelViewModel ViewModel;
	ViewModel.Title = IdleProject::Localization::UI(TEXT("DUNGEON_PANEL_TITLE"));

	const EDungeonType Types[] = { EDungeonType::Gold, EDungeonType::Exp, EDungeonType::Essence };
	for (const EDungeonType Type : Types)
	{
		FIdleHUDDungeonRowViewModel Row;
		Row.Type = Type;
		Row.NameLabel = IdleProject::Localization::UI(DungeonTypeToLocalizationKey(Type));
		Row.EntryLimit = FDungeonFormula::GetDailyEntryLimit(Type);
		Row.RemainingEntries = DungeonService.GetRemainingEntries(Type, TodayUtc);
		Row.CombatPower = FMath::Max<int64>(0, CombatPower);
		Row.MaxAccessibleTier = DungeonService.GetMaxAccessibleTier(Type, Row.CombatPower);
		Row.SelectedTier = FMath::Max(1, Row.MaxAccessibleTier);
		Row.RequiredPower = DungeonService.GetTierCpRequirement(Type, Row.SelectedTier);
		Row.NextTierRequirement = DungeonService.GetTierCpRequirement(Type, Row.SelectedTier + 1);
		Row.EnterHitBoxName = MakeDungeonEnterHitBoxName(Type, Row.SelectedTier);
		Row.ActionLabel = IdleProject::Localization::UI(TEXT("DUNGEON_ENTER"));
		Row.bSoldOut = Row.EntryLimit > 0 && Row.RemainingEntries <= 0;
		Row.bNeedsPower = Row.CombatPower < Row.RequiredPower;
		Row.bCanEnter = Row.EntryLimit > 0 && Row.RemainingEntries > 0 && !Row.bNeedsPower;

		Row.EntriesLabel = FormatLocalizedUI(TEXT("DUNGEON_ENTRIES_FORMAT"), [&Row](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Remaining"), FText::AsNumber(Row.RemainingEntries));
			Args.Add(TEXT("Limit"), FText::AsNumber(Row.EntryLimit));
		});
		Row.TierLabel = FormatLocalizedUI(TEXT("DUNGEON_TIER_FORMAT"), [&Row](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Tier"), FText::AsNumber(Row.SelectedTier));
			Args.Add(TEXT("MaxTier"), FText::AsNumber(Row.MaxAccessibleTier));
		});
		Row.RequiredPowerLabel = FormatLocalizedUI(TEXT("DUNGEON_CP_FORMAT"), [&Row](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Current"), FText::FromString(FormatIntegerWithCommas(Row.CombatPower)));
			Args.Add(TEXT("Required"), FText::FromString(FormatIntegerWithCommas(Row.RequiredPower)));
		});
		Row.NextTierLabel = FormatLocalizedUI(TEXT("DUNGEON_NEXT_TIER_CP_FORMAT"), [&Row](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Required"), FText::FromString(FormatIntegerWithCommas(Row.NextTierRequirement)));
		});
		Row.RewardLabel = BuildDungeonRewardLabel(FDungeonFormula::GetRewardForCp(Type, Row.CombatPower, Row.SelectedTier));
		Row.StatusLabel = Row.bSoldOut
			? IdleProject::Localization::UI(TEXT("DUNGEON_STATUS_SOLD_OUT"))
			: (Row.bNeedsPower ? IdleProject::Localization::UI(TEXT("DUNGEON_STATUS_NEED_CP")) : IdleProject::Localization::UI(TEXT("DUNGEON_ENTER")));

		ViewModel.Rows.Add(Row);
	}

	return ViewModel;
}

FIdleHUDWeeklyBossPanelViewModel IdleProject::UI::BuildWeeklyBossPanelViewModel(const UWeeklyBossService& WeeklyBossService)
{
	FIdleHUDWeeklyBossPanelViewModel ViewModel;
	ViewModel.Title = IdleProject::Localization::UI(TEXT("WEEKLY_BOSS_PANEL_TITLE"));
	ViewModel.ChallengeLabel = IdleProject::Localization::UI(TEXT("WEEKLY_BOSS_CHALLENGE"));
	ViewModel.ChallengeHitBoxName = WeeklyBossChallengeHitBoxName;
	ViewModel.Damage = WeeklyBossService.GetDamage();
	ViewModel.RemainingChallenges = WeeklyBossService.GetRemainingChallenges();
	ViewModel.ReachedMilestones = WeeklyBossService.GetReachedMilestones();
	ViewModel.ClaimedMilestones = WeeklyBossService.GetClaimedMilestones();
	ViewModel.bCanChallenge = ViewModel.RemainingChallenges > 0;

	const int32 NextMilestone = FMath::Max(1, ViewModel.ReachedMilestones + 1);
	ViewModel.NextMilestoneThreshold = FWeeklyBossFormula::MilestoneThreshold(NextMilestone);
	ViewModel.ProgressRatio = ViewModel.NextMilestoneThreshold > 0
		? FMath::Clamp(static_cast<float>(ViewModel.Damage) / static_cast<float>(ViewModel.NextMilestoneThreshold), 0.0f, 1.0f)
		: 0.0f;

	ViewModel.WeekLabel = FormatLocalizedUI(TEXT("WEEKLY_BOSS_WEEK_FORMAT"), [&WeeklyBossService](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Week"), FText::FromString(WeeklyBossService.GetWeekId()));
	});
	ViewModel.DamageLabel = FormatLocalizedUI(TEXT("WEEKLY_BOSS_DAMAGE_FORMAT"), [&ViewModel](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Damage"), FText::FromString(FormatIntegerWithCommas(ViewModel.Damage)));
		Args.Add(TEXT("Threshold"), FText::FromString(FormatIntegerWithCommas(ViewModel.NextMilestoneThreshold)));
	});
	ViewModel.RemainingLabel = FormatLocalizedUI(TEXT("WEEKLY_BOSS_REMAINING_FORMAT"), [&ViewModel](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Remaining"), FText::AsNumber(ViewModel.RemainingChallenges));
		Args.Add(TEXT("Limit"), FText::AsNumber(FWeeklyBossFormula::WeeklyChallengeLimit));
	});
	ViewModel.MilestoneSummaryLabel = FormatLocalizedUI(TEXT("WEEKLY_BOSS_MILESTONE_SUMMARY_FORMAT"), [&ViewModel](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Reached"), FText::AsNumber(ViewModel.ReachedMilestones));
		Args.Add(TEXT("Claimed"), FText::AsNumber(ViewModel.ClaimedMilestones));
	});
	ViewModel.ResetLabel = IdleProject::Localization::UI(TEXT("WEEKLY_BOSS_RESET_WEEKLY"));

	const int32 FirstDisplayedMilestone = FMath::Max(1, ViewModel.ClaimedMilestones + 1);
	constexpr int32 DisplayMilestoneCount = 5;
	for (int32 Offset = 0; Offset < DisplayMilestoneCount; ++Offset)
	{
		FIdleHUDWeeklyBossMilestoneRowViewModel Row;
		Row.Milestone = FirstDisplayedMilestone + Offset;
		Row.Threshold = FWeeklyBossFormula::MilestoneThreshold(Row.Milestone);
		Row.GoldReward = FWeeklyBossFormula::MilestoneGoldReward(Row.Milestone);
		Row.EssenceReward = FWeeklyBossFormula::MilestoneEssenceReward(Row.Milestone);
		Row.bReached = Row.Milestone <= ViewModel.ReachedMilestones;
		Row.bClaimed = Row.Milestone <= ViewModel.ClaimedMilestones;
		Row.bCanClaim = Row.bReached && !Row.bClaimed;
		Row.ClaimHitBoxName = MakeWeeklyBossClaimHitBoxName(Row.Milestone);
		Row.ActionLabel = IdleProject::Localization::UI(TEXT("WEEKLY_BOSS_CLAIM"));
		Row.MilestoneLabel = FormatLocalizedUI(TEXT("WEEKLY_BOSS_MILESTONE_FORMAT"), [&Row](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Milestone"), FText::AsNumber(Row.Milestone));
		});
		Row.ThresholdLabel = FormatLocalizedUI(TEXT("WEEKLY_BOSS_THRESHOLD_FORMAT"), [&Row](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Threshold"), FText::FromString(FormatIntegerWithCommas(Row.Threshold)));
		});
		Row.RewardLabel = FormatLocalizedUI(TEXT("WEEKLY_BOSS_REWARD_FORMAT"), [&Row](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Gold"), FText::FromString(FormatIntegerWithCommas(Row.GoldReward)));
			Args.Add(TEXT("Essence"), FText::FromString(FormatIntegerWithCommas(Row.EssenceReward)));
		});
		Row.StatusLabel = Row.bClaimed
			? IdleProject::Localization::UI(TEXT("WEEKLY_BOSS_STATUS_CLAIMED"))
			: (Row.bReached ? IdleProject::Localization::UI(TEXT("WEEKLY_BOSS_STATUS_REACHED")) : IdleProject::Localization::UI(TEXT("WEEKLY_BOSS_STATUS_LOCKED")));
		ViewModel.Rows.Add(Row);
	}

	return ViewModel;
}

FIdleHUDAttendancePanelViewModel IdleProject::UI::BuildAttendancePanelViewModel(const UAttendanceService& AttendanceService, const FString& TodayUtcDate)
{
	FIdleHUDAttendancePanelViewModel ViewModel;
	ViewModel.Title = IdleProject::Localization::UI(TEXT("ATTENDANCE_PANEL_TITLE"));
	ViewModel.TotalAttendance = AttendanceService.GetTotalAttendance();
	ViewModel.ReachedMilestones = AttendanceService.GetReachedMilestones();
	ViewModel.ClaimedMilestones = AttendanceService.GetClaimedMilestones().Num();
	// 오늘 출석 처리 여부: 마지막 출석일이 오늘(UTC date)과 같으면 완료(보통 로그인 시 자동 체크인).
	ViewModel.bCheckedInToday = !TodayUtcDate.IsEmpty() && AttendanceService.GetLastAttendanceDate() == TodayUtcDate;

	ViewModel.TotalLabel = FormatLocalizedUI(TEXT("ATTENDANCE_TOTAL_FORMAT"), [&ViewModel](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Total"), FText::FromString(FormatIntegerWithCommas(ViewModel.TotalAttendance)));
	});
	ViewModel.CheckInLabel = ViewModel.bCheckedInToday
		? IdleProject::Localization::UI(TEXT("ATTENDANCE_CHECKIN_DONE"))
		: IdleProject::Localization::UI(TEXT("ATTENDANCE_CHECKIN_AVAILABLE"));
	ViewModel.MilestoneSummaryLabel = FormatLocalizedUI(TEXT("ATTENDANCE_MILESTONE_SUMMARY_FORMAT"), [&ViewModel](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Reached"), FText::AsNumber(ViewModel.ReachedMilestones));
		Args.Add(TEXT("Claimed"), FText::AsNumber(ViewModel.ClaimedMilestones));
	});

	// 무한 마일스톤: 수령분은 접고(수령+1부터) 도달 미수령 + 다음 미도달 일부를 합리적 범위로 표시.
	const int32 FirstDisplayedMilestone = FMath::Max(1, ViewModel.ClaimedMilestones + 1);
	constexpr int32 DisplayMilestoneCount = 5;
	for (int32 Offset = 0; Offset < DisplayMilestoneCount; ++Offset)
	{
		const int32 MilestoneN = FirstDisplayedMilestone + Offset;
		const FAttendanceMilestone Milestone = AttendanceService.GetMilestone(MilestoneN);

		FIdleHUDAttendanceMilestoneRowViewModel Row;
		Row.Milestone = MilestoneN;
		Row.Threshold = Milestone.Threshold;
		Row.bReached = MilestoneN <= ViewModel.ReachedMilestones;
		Row.bClaimed = AttendanceService.IsMilestoneClaimed(MilestoneN);
		Row.bCanClaim = Row.bReached && !Row.bClaimed;
		Row.ClaimHitBoxName = MakeAttendanceClaimHitBoxName(MilestoneN);
		Row.ActionLabel = IdleProject::Localization::UI(TEXT("ATTENDANCE_CLAIM"));
		Row.MilestoneLabel = FormatLocalizedUI(TEXT("ATTENDANCE_MILESTONE_FORMAT"), [MilestoneN](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Milestone"), FText::AsNumber(MilestoneN));
		});
		Row.ThresholdLabel = FormatLocalizedUI(TEXT("ATTENDANCE_THRESHOLD_FORMAT"), [&Row](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Threshold"), FText::FromString(FormatIntegerWithCommas(Row.Threshold)));
		});
		Row.RewardLabel = FormatLocalizedUI(TEXT("ATTENDANCE_REWARD_FORMAT"), [&Milestone](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Type"), AttendanceRewardTypeLabel(Milestone.RewardType));
			Args.Add(TEXT("Value"), FText::FromString(FormatIntegerWithCommas(Milestone.RewardValue)));
		});
		if (Row.bClaimed)
		{
			Row.StatusLabel = IdleProject::Localization::UI(TEXT("ATTENDANCE_STATUS_CLAIMED"));
		}
		else if (Row.bReached)
		{
			Row.StatusLabel = IdleProject::Localization::UI(TEXT("ATTENDANCE_STATUS_REACHED"));
		}
		else
		{
			// 미달 진행: 누적 출석일 / 필요 임계.
			Row.StatusLabel = FormatLocalizedUI(TEXT("ATTENDANCE_STATUS_PROGRESS_FORMAT"), [&ViewModel, &Row](FFormatNamedArguments& Args)
			{
				Args.Add(TEXT("Total"), FText::FromString(FormatIntegerWithCommas(ViewModel.TotalAttendance)));
				Args.Add(TEXT("Threshold"), FText::FromString(FormatIntegerWithCommas(Row.Threshold)));
			});
		}
		ViewModel.Rows.Add(Row);
	}

	return ViewModel;
}

FIdleHUDAchievementViewModel IdleProject::UI::BuildAchievementViewModel(const UAchievementService& AchievementService)
{
	FIdleHUDAchievementViewModel ViewModel;
	ViewModel.TotalPoints = AchievementService.GetTotalPoints();
	ViewModel.StatMultiplier = AchievementService.GetStatMultiplier();
	ViewModel.Title = IdleProject::Localization::UI(TEXT("ACHIEVEMENT_PANEL_TITLE"));
	ViewModel.TotalPointsLabel = FormatLocalizedUI(TEXT("ACHIEVEMENT_TOTAL_POINTS_FORMAT"), [&ViewModel](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Points"), FText::AsNumber(ViewModel.TotalPoints));
	});
	ViewModel.StatMultiplierLabel = FormatLocalizedUI(TEXT("ACHIEVEMENT_STAT_MULTIPLIER_FORMAT"), [&ViewModel](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Multiplier"), FormatMultiplierLabel(ViewModel.StatMultiplier));
	});

	TMap<EAchievementCategory, FAchievementCategoryProgress> ProgressByCategory;
	for (const FAchievementCategoryProgress& Progress : AchievementService.GetCategoryProgress())
	{
		ProgressByCategory.Add(Progress.Category, Progress);
	}

	const EAchievementCategory CategoryOrder[] = {
		EAchievementCategory::Combat,
		EAchievementCategory::Progression,
		EAchievementCategory::Gear,
		EAchievementCategory::Economy,
		EAchievementCategory::Skill,
		EAchievementCategory::Pet,
		EAchievementCategory::Quest,
		EAchievementCategory::Collection,
		EAchievementCategory::Misc
	};

	for (const EAchievementCategory Category : CategoryOrder)
	{
		const FAchievementCategoryProgress Progress = ProgressByCategory.FindRef(Category);

		int64 NextThreshold = MAX_int64;
		for (const FAchievementDefinition& Definition : FAchievementFormula::GetDefinitions())
		{
			if (Definition.Category != Category)
			{
				continue;
			}

			const int64 MetricValue = AchievementService.GetMetricValue(Definition.Metric);
			const int32 CurrentTier = FAchievementFormula::GetTierForValue(Definition, MetricValue);
			NextThreshold = FMath::Min(NextThreshold, GetAchievementThresholdForTier(Definition, CurrentTier));
		}
		if (NextThreshold == MAX_int64)
		{
			NextThreshold = 0;
		}

		FIdleHUDAchievementRowViewModel Row;
		Row.Category = Category;
		Row.CategoryLabel = IdleProject::Localization::UI(AchievementCategoryToLocalizationKey(Category));
		Row.Tier = Progress.HighestUnlockedTier;
		Row.Points = Progress.UnlockedPoints;
		Row.CurrentValue = Progress.CurrentValue;
		Row.NextThreshold = NextThreshold;
		Row.ProgressRatio = NextThreshold > 0
			? FMath::Clamp(static_cast<float>(Row.CurrentValue) / static_cast<float>(NextThreshold), 0.0f, 1.0f)
			: 0.0f;
		Row.TierLabel = FormatLocalizedUI(TEXT("ACHIEVEMENT_TIER_FORMAT"), [&Row](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Tier"), FText::AsNumber(Row.Tier));
		});
		Row.PointsLabel = FormatLocalizedUI(TEXT("ACHIEVEMENT_POINTS_FORMAT"), [&Row](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Points"), FText::AsNumber(Row.Points));
		});
		Row.ValueLabel = FormatLocalizedUI(TEXT("ACHIEVEMENT_VALUE_FORMAT"), [&Row](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Value"), FText::FromString(FormatIntegerWithCommas(Row.CurrentValue)));
		});
		Row.NextThresholdLabel = FormatLocalizedUI(TEXT("ACHIEVEMENT_NEXT_THRESHOLD_FORMAT"), [&Row](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Value"), FText::FromString(FormatIntegerWithCommas(Row.NextThreshold)));
		});
		ViewModel.Rows.Add(Row);
	}

	return ViewModel;
}

FIdleHUDMasteryPanelViewModel IdleProject::UI::BuildMasteryPanelViewModel(const UMasteryService& MasteryService)
{
	FIdleHUDMasteryPanelViewModel ViewModel;
	ViewModel.Title = IdleProject::Localization::UI(TEXT("MASTERY_PANEL_TITLE"));
	ViewModel.WorldPowerLabel = FormatLocalizedUIWithInt64(TEXT("MASTERY_WORLD_POWER_FORMAT"), TEXT("Amount"), MasteryService.GetWorldPower());
	ViewModel.Rows.Reserve(FMasteryFormula::TrackCount);

	const EMasteryTrack* Tracks = GetMasteryTrackDisplayOrder();
	for (int32 Index = 0; Index < FMasteryFormula::TrackCount; ++Index)
	{
		const EMasteryTrack Track = Tracks[Index];
		const FMasteryLevelInfo Info = MasteryService.GetTrackLevelInfo(Track);
		const float LocalBonus = MasteryService.GetLocalBonus(Track);
		const int32 LocalBonusPercent = FMath::RoundToInt(FMath::Max(0.0f, LocalBonus) * 100.0f);
		const float LocalBonus2 = MasteryService.GetLocalBonus2(Track);
		const int32 LocalBonus2Percent = FMath::RoundToInt(FMath::Max(0.0f, LocalBonus2) * 100.0f);
		const int32 AbyssBonusEntries = Track == EMasteryTrack::Abyss ? MasteryService.GetAbyssBonusEntries() : 0;
		const float ProgressRatio = Info.XpToNext > 0
			? FMath::Clamp(static_cast<float>(Info.XpIntoLevel) / static_cast<float>(Info.XpToNext), 0.0f, 1.0f)
			: 0.0f;

		FIdleHUDMasteryTrackRowViewModel Row;
		Row.Track = Track;
		Row.TrackLabel = IdleProject::Localization::UI(MasteryTrackToLocalizationKey(Track));
		Row.LevelLabel = FormatLocalizedUI(TEXT("MASTERY_TRACK_LEVEL_FORMAT"), [&Info](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Level"), FText::AsNumber(Info.Level));
		});
		Row.XpLabel = FormatLocalizedUI(TEXT("MASTERY_TRACK_XP_FORMAT"), [&Info](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Current"), FText::FromString(FormatIntegerWithCommas(Info.XpIntoLevel)));
			Args.Add(TEXT("Next"), FText::FromString(FormatIntegerWithCommas(Info.XpToNext)));
		});
		Row.ProgressLabel = FormatLocalizedUI(TEXT("MASTERY_TRACK_PROGRESS_FORMAT"), [ProgressRatio](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Percent"), FText::AsNumber(FMath::RoundToInt(ProgressRatio * 100.0f)));
		});
		Row.LocalBonusLabel = FormatLocalizedUI(MasteryLocalBonusFormatKey(Track), [LocalBonusPercent](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Percent"), FText::AsNumber(LocalBonusPercent));
		});
		if (Track == EMasteryTrack::Abyss)
		{
			Row.LocalBonus2Label = FormatLocalizedUI(MasteryLocalBonus2FormatKey(Track), [AbyssBonusEntries](FFormatNamedArguments& Args)
			{
				Args.Add(TEXT("Entries"), FText::AsNumber(AbyssBonusEntries));
			});
		}
		else
		{
			Row.LocalBonus2Label = FormatLocalizedUI(MasteryLocalBonus2FormatKey(Track), [LocalBonus2Percent](FFormatNamedArguments& Args)
			{
				Args.Add(TEXT("Percent"), FText::AsNumber(LocalBonus2Percent));
			});
		}
		Row.TooltipLabel = IdleProject::Localization::UI(MasteryLocalBonusTooltipKey(Track));
		Row.ProgressRatio = ProgressRatio;
		Row.LocalBonusPercent = LocalBonusPercent;
		Row.LocalBonus2Percent = LocalBonus2Percent;
		Row.LocalBonus2AbyssEntries = AbyssBonusEntries;
		ViewModel.Rows.Add(Row);
	}

	return ViewModel;
}

FIdleHUDTitlePanelViewModel IdleProject::UI::BuildTitlePanelViewModel(const UTitleService& TitleService, const UAchievementService& AchievementService)
{
	FIdleHUDTitlePanelViewModel ViewModel;
	ViewModel.Title = IdleProject::Localization::UI(TEXT("TITLE_PANEL_TITLE"));

	const FString& EquippedId = TitleService.GetEquippedTitleId();
	ViewModel.EquippedLabel = EquippedId.IsEmpty()
		? IdleProject::Localization::UI(TEXT("TITLE_EQUIPPED_NONE"))
		: FormatLocalizedUI(TEXT("TITLE_EQUIPPED_FORMAT"), [&EquippedId](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("TitleName"), TitleIdToNameLabel(EquippedId));
		});

	const TArray<FTitleDefinition>& Definitions = TitleService.GetDefinitions();
	ViewModel.Rows.Reserve(Definitions.Num());
	for (const FTitleDefinition& Definition : Definitions)
	{
		const bool bUnlocked = TitleService.IsUnlocked(Definition.TitleId);
		const bool bEquipped = Definition.TitleId == EquippedId;
		const int64 CurrentValue = AchievementService.GetMetricValue(Definition.Metric);

		FIdleHUDTitleRowViewModel Row;
		Row.TitleId = Definition.TitleId;
		Row.Name = TitleIdToNameLabel(Definition.TitleId);
		Row.BonusLabel = TitleBonusToLabel(Definition.BonusType, Definition.BonusValue);
		Row.bUnlocked = bUnlocked;
		Row.bEquipped = bEquipped;
		Row.bCanEquip = bUnlocked && !bEquipped;
		Row.StatusLabel = bUnlocked
			? IdleProject::Localization::UI(TEXT("TITLE_STATUS_UNLOCKED"))
			: IdleProject::Localization::UI(TEXT("TITLE_STATUS_LOCKED"));

		// 미해금 행: 해금 조건(메트릭+임계) + 진행(현재/임계). 해금 행은 진행 표시 생략.
		const FText MetricLabel = IdleProject::Localization::UI(TitleMetricToLocalizationKey(Definition.Metric));
		Row.ConditionLabel = FormatLocalizedUI(TEXT("TITLE_UNLOCK_CONDITION_FORMAT"), [&MetricLabel, &Definition](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Metric"), MetricLabel);
			Args.Add(TEXT("Threshold"), FText::FromString(FormatIntegerWithCommas(Definition.Threshold)));
		});
		Row.ProgressLabel = FormatLocalizedUI(TEXT("TITLE_PROGRESS_FORMAT"), [CurrentValue, &Definition](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Current"), FText::FromString(FormatIntegerWithCommas(CurrentValue)));
			Args.Add(TEXT("Threshold"), FText::FromString(FormatIntegerWithCommas(Definition.Threshold)));
		});
		Row.ProgressRatio = Definition.Threshold > 0
			? FMath::Clamp(static_cast<float>(CurrentValue) / static_cast<float>(Definition.Threshold), 0.0f, 1.0f)
			: (bUnlocked ? 1.0f : 0.0f);

		// 액션 라벨: 장착중=해제, 해금=장착, 미해금=잠김.
		Row.ActionLabel = bEquipped
			? IdleProject::Localization::UI(TEXT("TITLE_ACTION_UNEQUIP"))
			: (bUnlocked
				? IdleProject::Localization::UI(TEXT("ACTION_EQUIP"))
				: IdleProject::Localization::UI(TEXT("TITLE_STATUS_LOCKED")));

		ViewModel.Rows.Add(Row);
	}

	return ViewModel;
}

FIdleHUDMissionPanelViewModel IdleProject::UI::BuildMissionPanelViewModel(const UMissionService& MissionService, EMissionPeriod ActivePeriod)
{
	FIdleHUDMissionPanelViewModel ViewModel;
	ViewModel.Title = IdleProject::Localization::UI(TEXT("MISSION_PANEL_TITLE"));
	ViewModel.DailyTabLabel = IdleProject::Localization::UI(TEXT("MISSION_TAB_DAILY"));
	ViewModel.WeeklyTabLabel = IdleProject::Localization::UI(TEXT("MISSION_TAB_WEEKLY"));
	ViewModel.EmptyLabel = IdleProject::Localization::UI(TEXT("MISSION_EMPTY"));
	ViewModel.ActivePeriod = ActivePeriod;

	const TArray<FMissionProgressView> Views = MissionService.GetProgressViews();
	ViewModel.Rows.Reserve(Views.Num());
	for (const FMissionProgressView& View : Views)
	{
		// 선택된 기간(일일/주간)에 해당하는 미션만 표시한다.
		if (View.Period != ActivePeriod)
		{
			continue;
		}

		FIdleHUDMissionRowViewModel Row;
		Row.MissionId = View.Id;
		Row.bCompleted = View.bCompleted;
		Row.bClaimed = View.bClaimed;
		Row.bCanClaim = View.bCompleted && !View.bClaimed;

		// 목표 설명: 메트릭 표시명 + 목표치.
		const FText MetricLabel = IdleProject::Localization::UI(MissionMetricToLocalizationKey(View.Metric));
		Row.ObjectiveLabel = FormatLocalizedUI(TEXT("MISSION_OBJECTIVE_FORMAT"), [&MetricLabel, &View](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Metric"), MetricLabel);
			Args.Add(TEXT("Target"), FText::FromString(FormatIntegerWithCommas(View.Target)));
		});

		// 진행: 현재/목표(현재는 목표로 clamp 해 과표시 방지).
		const int64 DisplayProgress = View.Target > 0 ? FMath::Min(View.Progress, View.Target) : View.Progress;
		Row.ProgressLabel = FormatLocalizedUI(TEXT("MISSION_PROGRESS_FORMAT"), [DisplayProgress, &View](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Current"), FText::FromString(FormatIntegerWithCommas(DisplayProgress)));
			Args.Add(TEXT("Target"), FText::FromString(FormatIntegerWithCommas(View.Target)));
		});
		Row.ProgressRatio = View.Target > 0
			? FMath::Clamp(static_cast<float>(View.Progress) / static_cast<float>(View.Target), 0.0f, 1.0f)
			: (View.bCompleted ? 1.0f : 0.0f);

		// 보상: 타입별 포맷 + 값.
		Row.RewardLabel = FormatLocalizedUIWithInt64(MissionRewardToLocalizationKey(View.RewardType), TEXT("Amount"), View.RewardValue);

		// 수령 버튼: 완료&미수령=수령, 수령됨=수령완료, 미완료=진행(라벨 비표시 — 버튼 비활성).
		Row.ActionLabel = View.bClaimed
			? IdleProject::Localization::UI(TEXT("MISSION_ACTION_CLAIMED"))
			: IdleProject::Localization::UI(TEXT("MISSION_ACTION_CLAIM"));

		ViewModel.Rows.Add(Row);
	}

	return ViewModel;
}

FText IdleProject::UI::BuildAchievementUnlockedFeedbackLabel(const FString& AchievementId, int32 Tier)
{
	const FAchievementDefinition* Definition = FindAchievementDefinitionById(AchievementId);
	const FText AchievementName = Definition ? GetAchievementDisplayName(*Definition) : FText::FromString(AchievementId);
	const int32 SafeTier = FMath::Max(1, Tier);
	return FormatLocalizedUI(TEXT("ACHIEVEMENT_UNLOCKED_FORMAT"), [AchievementName, SafeTier](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Name"), AchievementName);
		Args.Add(TEXT("Tier"), FText::AsNumber(SafeTier));
	});
}

FText IdleProject::UI::BuildProgressSavedFeedbackLabel()
{
	return IdleProject::Localization::UI(TEXT("SAVE_PROGRESS_SAVED"));
}

FIdleHUDCloudSyncViewModel IdleProject::UI::BuildCloudSyncViewModel(ECloudSyncState State)
{
	FIdleHUDCloudSyncViewModel ViewModel;
	ViewModel.State = State;

	switch (State)
	{
	case ECloudSyncState::Syncing:
		ViewModel.Label = IdleProject::Localization::UI(TEXT("CLOUD_SYNC_SYNCING"));
		ViewModel.bVisible = true;
		break;
	case ECloudSyncState::Synced:
		ViewModel.Label = IdleProject::Localization::UI(TEXT("CLOUD_SYNC_SYNCED"));
		ViewModel.bVisible = true;
		ViewModel.bTransient = true;
		break;
	case ECloudSyncState::Offline:
		ViewModel.Label = IdleProject::Localization::UI(TEXT("CLOUD_SYNC_OFFLINE"));
		ViewModel.bVisible = true;
		ViewModel.bError = true;
		break;
	case ECloudSyncState::Conflict:
		ViewModel.Label = IdleProject::Localization::UI(TEXT("CLOUD_SYNC_CONFLICT"));
		ViewModel.bVisible = true;
		ViewModel.bError = true;
		break;
	case ECloudSyncState::Idle:
	default:
		ViewModel.bVisible = false;
		break;
	}

	return ViewModel;
}

FIdleHUDOfflineRewardViewModel IdleProject::UI::BuildOfflineRewardViewModel(const FOfflineRewardResult& Reward)
{
	FIdleHUDOfflineRewardViewModel ViewModel;
	ViewModel.bVisible = Reward.Gold > 0 || Reward.Exp > 0;
	ViewModel.Title = IdleProject::Localization::UI(TEXT("OFFLINE_REWARD_TITLE"));
	ViewModel.ElapsedLabel = FormatLocalizedUI(TEXT("OFFLINE_REWARD_ELAPSED_FORMAT"), [&Reward](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Elapsed"), FText::FromString(FormatElapsedHoursMinutes(Reward.CappedSeconds)));
	});
	ViewModel.GoldLabel = FormatLocalizedUI(TEXT("REWARD_GOLD_PLUS_FORMAT"), [&Reward](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Amount"), FText::FromString(FormatIntegerWithCommas(Reward.Gold)));
	});
	ViewModel.ExpLabel = FormatLocalizedUI(TEXT("REWARD_EXP_PLUS_FORMAT"), [&Reward](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Amount"), FText::FromString(FormatIntegerWithCommas(Reward.Exp)));
	});
	ViewModel.ClaimLabel = IdleProject::Localization::UI(TEXT("ACTION_CLAIM"));
	return ViewModel;
}

FIdleHUDQuestLogViewModel IdleProject::UI::BuildQuestLogViewModel(const TArray<FQuestState>& QuestStates)
{
	FIdleHUDQuestLogViewModel ViewModel;
	ViewModel.Title = IdleProject::Localization::UI(TEXT("QUEST_LOG_TITLE"));
	ViewModel.ShortcutLabel = IdleProject::Localization::UI(TEXT("QUEST_LOG_SHORTCUT_CLOSE"));
	ViewModel.EmptyLabel = IdleProject::Localization::UI(TEXT("QUEST_LOG_EMPTY"));

	auto FindOrAddSection = [&ViewModel](EQuestType Type) -> FIdleHUDQuestLogSectionViewModel&
	{
		for (FIdleHUDQuestLogSectionViewModel& Section : ViewModel.Sections)
		{
			if (Section.Type == Type)
			{
				return Section;
			}
		}

		FIdleHUDQuestLogSectionViewModel& Section = ViewModel.Sections.AddDefaulted_GetRef();
		Section.Type = Type;
		Section.TypeLabel = QuestTypeToLabel(Type);
		return Section;
	};

	FindOrAddSection(EQuestType::Main);
	FindOrAddSection(EQuestType::Daily);
	FindOrAddSection(EQuestType::Weekly);

	for (const FQuestState& State : QuestStates)
	{
		FIdleHUDQuestLogRowViewModel Row;
		Row.QuestId = State.QuestId;
		Row.TypeLabel = QuestTypeToLabel(State.Type);
		Row.Title = State.Title;
		Row.ProgressRatio = State.TargetCount > 0
			? FMath::Clamp(static_cast<float>(State.Progress) / static_cast<float>(State.TargetCount), 0.0f, 1.0f)
			: 1.0f;
		Row.ProgressLabel = FormatLocalizedUI(TEXT("QUEST_PROGRESS_FORMAT"), [&State](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Current"), FText::AsNumber(State.Progress));
			Args.Add(TEXT("Target"), FText::AsNumber(State.TargetCount));
		});
		Row.RewardLabel = FormatLocalizedUI(TEXT("QUEST_REWARD_FORMAT"), [&State](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Gold"), FText::AsNumber(State.RewardGold));
			Args.Add(TEXT("Exp"), FText::AsNumber(State.RewardExp));
		});
		Row.bCanClaim = State.bCompleted && !State.bClaimed;
		Row.bClaimed = State.bClaimed;
		if (Row.bCanClaim)
		{
			Row.ActionLabel = IdleProject::Localization::UI(TEXT("ACTION_CLAIM"));
		}
		else if (Row.bClaimed)
		{
			Row.ActionLabel = IdleProject::Localization::UI(TEXT("ACTION_CLAIMED"));
		}
		else
		{
			Row.ActionLabel = IdleProject::Localization::UI(TEXT("ACTION_IN_PROGRESS"));
		}
		ViewModel.Rows.Add(Row);
		FindOrAddSection(State.Type).Rows.Add(Row);
	}

	ViewModel.Sections.RemoveAll([](const FIdleHUDQuestLogSectionViewModel& Section)
	{
		return Section.Rows.IsEmpty();
	});

	return ViewModel;
}

FIdleHUDRebirthViewModel IdleProject::UI::BuildRebirthViewModel(bool bCanRebirth, bool bBossDefeated, int32 CharacterLevel, int32 RebirthCount, int32 RebirthBonusPoints, int32 PreviewRebirthReward)
{
	FIdleHUDRebirthViewModel ViewModel;
	const int32 SafeLevel = FMath::Max(1, CharacterLevel);
	const int32 SafeRebirthCount = FMath::Max(0, RebirthCount);
	const int32 SafeBonusPoints = FMath::Max(0, RebirthBonusPoints);
	const int32 SafePreviewReward = FMath::Max(0, PreviewRebirthReward);

	ViewModel.bCanRebirth = bCanRebirth;
	ViewModel.bBossDefeated = bBossDefeated;
	ViewModel.bLevelReady = SafeLevel >= RebirthRequiredLevel;
	ViewModel.Title = IdleProject::Localization::UI(TEXT("REBIRTH_TITLE"));
	ViewModel.StatusLabel = IdleProject::Localization::UI(bCanRebirth ? TEXT("REBIRTH_STATUS_READY") : TEXT("REBIRTH_STATUS_LOCKED"));
	ViewModel.BossLabel = IdleProject::Localization::UI(bBossDefeated ? TEXT("REBIRTH_BOSS_DONE") : TEXT("REBIRTH_BOSS_REQUIRED"));
	ViewModel.LevelLabel = ViewModel.bLevelReady
		? IdleProject::Localization::UI(TEXT("REBIRTH_LEVEL_DONE"))
		: FormatLocalizedUI(TEXT("REBIRTH_LEVEL_FORMAT"), [SafeLevel](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Level"), FText::AsNumber(SafeLevel));
		});
	ViewModel.CountLabel = FormatLocalizedUI(TEXT("REBIRTH_COUNT_FORMAT"), [SafeRebirthCount](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Count"), FText::AsNumber(SafeRebirthCount));
	});
	ViewModel.BonusLabel = FormatLocalizedUI(TEXT("REBIRTH_BONUS_FORMAT"), [SafeBonusPoints](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Points"), FText::AsNumber(SafeBonusPoints));
	});
	ViewModel.NextBonusLabel = FormatLocalizedUI(TEXT("REBIRTH_PREVIEW_REWARD_FORMAT"), [SafePreviewReward](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Points"), FText::AsNumber(SafePreviewReward));
	});
	ViewModel.ButtonLabel = IdleProject::Localization::UI(TEXT("REBIRTH_BUTTON"));
	return ViewModel;
}

FIdleHUDTranscendViewModel IdleProject::UI::BuildTranscendViewModel(bool bCanTranscend, int32 RebirthCount, int32 Threshold, int32 TranscendCount, float CurrentMultiplier, float PreviewMultiplier)
{
	FIdleHUDTranscendViewModel ViewModel;
	const int32 SafeThreshold = FMath::Max(1, Threshold);
	const int32 SafeRebirthCount = FMath::Max(0, RebirthCount);
	const int32 SafeTranscendCount = FMath::Max(0, TranscendCount);
	const FString CurrentMultiplierText = FString::Printf(TEXT("x%.2f"), FMath::Max(0.0f, CurrentMultiplier));
	const FString PreviewMultiplierText = FString::Printf(TEXT("x%.2f"), FMath::Max(0.0f, PreviewMultiplier));

	ViewModel.bCanTranscend = bCanTranscend;
	ViewModel.bThresholdReady = SafeRebirthCount >= SafeThreshold;
	ViewModel.Title = IdleProject::Localization::UI(TEXT("TRANSCEND_TITLE"));
	ViewModel.StatusLabel = IdleProject::Localization::UI(bCanTranscend ? TEXT("TRANSCEND_STATUS_READY") : TEXT("TRANSCEND_STATUS_LOCKED"));
	ViewModel.RequirementLabel = FormatLocalizedUI(TEXT("TRANSCEND_REQUIREMENT_FORMAT"), [SafeRebirthCount, SafeThreshold](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Rebirth"), FText::AsNumber(SafeRebirthCount));
		Args.Add(TEXT("Threshold"), FText::AsNumber(SafeThreshold));
	});
	ViewModel.CurrentMultiplierLabel = FormatLocalizedUI(TEXT("TRANSCEND_CURRENT_MULTIPLIER_FORMAT"), [&CurrentMultiplierText](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Multiplier"), FText::FromString(CurrentMultiplierText));
	});
	ViewModel.PreviewMultiplierLabel = FormatLocalizedUI(TEXT("TRANSCEND_PREVIEW_MULTIPLIER_FORMAT"), [&PreviewMultiplierText](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Multiplier"), FText::FromString(PreviewMultiplierText));
	});
	ViewModel.CountLabel = FormatLocalizedUI(TEXT("TRANSCEND_COUNT_FORMAT"), [SafeTranscendCount](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Count"), FText::AsNumber(SafeTranscendCount));
	});
	ViewModel.ButtonLabel = IdleProject::Localization::UI(TEXT("TRANSCEND_BUTTON"));
	return ViewModel;
}

TArray<FIdleHUDClassSelectionOptionViewModel> IdleProject::UI::BuildClassSelectionOptions(EClassId CurrentClassId)
{
	struct FClassOptionSeed
	{
		EClassId ClassId;
		const TCHAR* DisplayNameKey;
		const TCHAR* RoleLabelKey;
		const TCHAR* StatSummary;
	};

	const FClassOptionSeed Seeds[] = {
		{ EClassId::Warrior, TEXT("CLASS_WARRIOR_NAME"), TEXT("CLASS_WARRIOR_ROLE"), TEXT("STR/CON") },
		{ EClassId::Mage, TEXT("CLASS_MAGE_NAME"), TEXT("CLASS_MAGE_ROLE"), TEXT("INT/WIS") },
		{ EClassId::Archer, TEXT("CLASS_ARCHER_NAME"), TEXT("CLASS_ARCHER_ROLE"), TEXT("DEX/LUK") },
		{ EClassId::Thief, TEXT("CLASS_THIEF_NAME"), TEXT("CLASS_THIEF_ROLE"), TEXT("DEX/LUK") },
		{ EClassId::Cleric, TEXT("CLASS_CLERIC_NAME"), TEXT("CLASS_CLERIC_ROLE"), TEXT("WIS/INT") },
		{ EClassId::Paladin, TEXT("CLASS_PALADIN_NAME"), TEXT("CLASS_PALADIN_ROLE"), TEXT("CON/STR") },
		{ EClassId::Berserker, TEXT("CLASS_BERSERKER_NAME"), TEXT("CLASS_BERSERKER_ROLE"), TEXT("STR/LUK") },
		{ EClassId::Summoner, TEXT("CLASS_SUMMONER_NAME"), TEXT("CLASS_SUMMONER_ROLE"), TEXT("INT/WIS") }
	};

	TArray<FIdleHUDClassSelectionOptionViewModel> Options;
	for (const FClassOptionSeed& Seed : Seeds)
	{
		FIdleHUDClassSelectionOptionViewModel Option;
		Option.ClassId = Seed.ClassId;
		Option.DisplayName = IdleProject::Localization::UI(Seed.DisplayNameKey);
		Option.RoleLabel = IdleProject::Localization::UI(Seed.RoleLabelKey);
		Option.StatSummary = FText::FromString(Seed.StatSummary);
		Option.bSelected = Seed.ClassId == CurrentClassId;
		Options.Add(Option);
	}
	return Options;
}

FIdleHUDPetPanelViewModel IdleProject::UI::BuildPetPanelViewModel(const TArray<FPetDefinition>& PetDefinitions, const FString& EquippedPetId, float GoldBonusPercent, float DropBonusPercent, int64 Gold, TFunctionRef<int32(const FString&)> GetPetLevel, TFunctionRef<bool(const FString&)> IsPetOwned, TFunctionRef<int32(const FString&)> GetPetStar)
{
	FIdleHUDPetPanelViewModel ViewModel;
	ViewModel.Title = IdleProject::Localization::UI(TEXT("PET_PANEL_TITLE"));
	ViewModel.EquippedLabel = EquippedPetId.IsEmpty()
		? IdleProject::Localization::UI(TEXT("PET_EQUIPPED_NONE"))
		: FormatLocalizedUI(TEXT("PET_EQUIPPED_FORMAT"), [&EquippedPetId](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("PetId"), PetIdToNameLabel(EquippedPetId));
		});
	ViewModel.GoldBonusLabel = FormatLocalizedUI(TEXT("PET_BONUS_GOLD_FORMAT"), [GoldBonusPercent](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Percent"), FText::FromString(FString::Printf(TEXT("%.0f%%"), GoldBonusPercent)));
	});
	ViewModel.DropBonusLabel = FormatLocalizedUI(TEXT("PET_BONUS_DROP_FORMAT"), [DropBonusPercent](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Percent"), FText::FromString(FString::Printf(TEXT("%.0f%%"), DropBonusPercent)));
	});

	for (const FPetDefinition& Definition : PetDefinitions)
	{
		const int32 Level = FMath::Clamp(GetPetLevel(Definition.PetId), 0, FPetLevelFormula::MaxPetLevel);
		const bool bOwned = IsPetOwned(Definition.PetId);
		const int64 FeedCost = FPetLevelFormula::GetFeedCost(Level);
		const bool bMaxLevel = Level >= FPetLevelFormula::MaxPetLevel;
		const bool bCanFeed = bOwned && !bMaxLevel && FeedCost > 0 && Gold >= FeedCost;

		FIdleHUDPetRowViewModel Row;
		Row.PetId = Definition.PetId;
		Row.Name = PetIdToNameLabel(Definition.PetId);
		Row.BonusLabel = PetBonusTypeToLabel(Definition.BonusType, Definition.BonusPercent * FPetLevelFormula::GetBonusMultiplier(Level));
		Row.LevelLabel = BuildPetLevelLabel(Level);
		Row.FeedCostLabel = BuildPetFeedCostLabel(FeedCost);
		Row.bEquipped = Definition.PetId == EquippedPetId;
		Row.bCanEquip = bOwned && !Row.bEquipped;
		Row.ActionLabel = Row.bEquipped
			? IdleProject::Localization::UI(TEXT("ACTION_EQUIPPED"))
			: (bOwned ? IdleProject::Localization::UI(TEXT("ACTION_EQUIP")) : IdleProject::Localization::UI(TEXT("PET_STATUS_LOCKED")));
		Row.FeedActionLabel = IdleProject::Localization::UI(TEXT("ACTION_FEED"));
		Row.bCanFeed = bCanFeed;
		Row.bFeedDisabled = !bCanFeed;
		Row.bMaxLevel = bMaxLevel;

		const int32 Star = FMath::Max(0, GetPetStar(Definition.PetId));
		const int64 EvolveCost = FPetLevelFormula::GetPetEvolveCost(Star);
		const bool bCanEvolve = bOwned && EvolveCost > 0 && Gold >= EvolveCost;
		Row.StarLabel = BuildPetStarLabel(Star);
		Row.EvolveActionLabel = IdleProject::Localization::UI(TEXT("ACTION_EVOLVE"));
		Row.EvolveCostLabel = BuildPetEvolveCostLabel(EvolveCost);
		Row.EvolveEffectLabel = BuildPetEvolveEffectLabel(Star + 1);
		Row.bCanEvolve = bCanEvolve;

		Row.StatusLabel = !bOwned
			? IdleProject::Localization::UI(TEXT("PET_STATUS_LOCKED"))
			: (bMaxLevel
			? IdleProject::Localization::UI(TEXT("PET_FEED_STATUS_MAX"))
			: (bCanFeed
				? IdleProject::Localization::UI(TEXT("PET_FEED_STATUS_READY"))
				: IdleProject::Localization::UI(TEXT("PET_FEED_STATUS_NEED_GOLD"))));
		ViewModel.Rows.Add(Row);
	}

	return ViewModel;
}

FIdleHUDSeasonPassViewModel IdleProject::UI::BuildSeasonPassViewModel(const TArray<FSeasonTierDefinition>& Tiers, int32 SeasonTokens, int32 ReachedTier, TFunctionRef<bool(int32)> IsTierClaimed)
{
	FIdleHUDSeasonPassViewModel ViewModel;
	ViewModel.Title = IdleProject::Localization::UI(TEXT("SEASON_PASS_TITLE"));

	int32 MaxRequiredTokens = 0;
	for (const FSeasonTierDefinition& Tier : Tiers)
	{
		MaxRequiredTokens = FMath::Max(MaxRequiredTokens, Tier.RequiredTokens);
	}

	ViewModel.TokenLabel = FText::FromString(FString::Printf(TEXT("%d / %d"), SeasonTokens, MaxRequiredTokens));
	ViewModel.ProgressRatio = MaxRequiredTokens > 0
		? FMath::Clamp(static_cast<float>(SeasonTokens) / static_cast<float>(MaxRequiredTokens), 0.0f, 1.0f)
		: 0.0f;
	ViewModel.ProgressLabel = FormatLocalizedUI(TEXT("SEASON_PROGRESS_FORMAT"), [ReachedTier, &Tiers](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Reached"), FText::AsNumber(ReachedTier));
		Args.Add(TEXT("Total"), FText::AsNumber(Tiers.Num()));
	});

	for (const FSeasonTierDefinition& Tier : Tiers)
	{
		FIdleHUDSeasonTierRowViewModel Row;
		Row.Tier = Tier.Tier;
		Row.TierLabel = FormatLocalizedUI(TEXT("SEASON_TIER_FORMAT"), [&Tier](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Tier"), FText::AsNumber(Tier.Tier));
		});
		Row.RequirementLabel = FormatLocalizedUI(TEXT("SEASON_REQUIREMENT_FORMAT"), [&Tier](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Tokens"), FText::AsNumber(Tier.RequiredTokens));
		});
		Row.RewardLabel = SeasonRewardToLabel(Tier.RewardType, Tier.RewardAmount);
		Row.ProgressRatio = Tier.RequiredTokens > 0
			? FMath::Clamp(static_cast<float>(SeasonTokens) / static_cast<float>(Tier.RequiredTokens), 0.0f, 1.0f)
			: 1.0f;
		Row.bReached = ReachedTier >= Tier.Tier;
		Row.bClaimed = IsTierClaimed(Tier.Tier);
		Row.bCanClaim = Row.bReached && !Row.bClaimed;
		Row.ActionLabel = IdleProject::Localization::UI(Row.bClaimed ? TEXT("SEASON_CLAIMED") : (Row.bCanClaim ? TEXT("ACTION_RECEIVE") : TEXT("ACTION_LOCKED")));
		ViewModel.Rows.Add(Row);
	}

	return ViewModel;
}

FIdleHUDFloatingDamageViewModel IdleProject::UI::BuildFloatingDamageViewModel(const FIdleHUDFloatingDamageEntry& Entry, float Now, FVector2D ProjectedScreenPosition, float HudScale)
{
	using namespace IdleProject::UI;

	FIdleHUDFloatingDamageViewModel ViewModel;
	const float Age = Now - Entry.StartTime;
	if (Age < 0.0f || Age > FloatingDamageLifetimeSeconds)
	{
		return ViewModel;
	}

	const float Progress = FMath::Clamp(Age / FloatingDamageLifetimeSeconds, 0.0f, 1.0f);
	const int64 RoundedAmount = FMath::Max<int64>(0, FMath::RoundToInt64(Entry.Amount));
	ViewModel.bVisible = true;
	ViewModel.Label = FormatIntegerWithCommas(RoundedAmount);
	if (Entry.bWasCrit)
	{
		ViewModel.Label += TEXT("!");
	}

	if (Entry.bWasCrit)
	{
		ViewModel.Color = Theme::AccentGold;
		ViewModel.TextScale = 1.25f * HudScale;
	}
	else if (Entry.Kind == EDamageKind::Magic)
	{
		ViewModel.Color = Theme::AccentBlue;
		ViewModel.TextScale = 1.0f * HudScale;
	}
	else
	{
		ViewModel.Color = Theme::TextPrimary;
		ViewModel.TextScale = 1.0f * HudScale;
	}

	ViewModel.Color.A = 1.0f - Progress;
	ViewModel.ScreenPosition = ProjectedScreenPosition + FVector2D(0.0f, -FloatingDamageRisePixels * HudScale * Progress);
	return ViewModel;
}

TArray<FIdleHUDStatusIndicatorViewModel> IdleProject::UI::BuildStatusIndicatorViewModels(const TArray<FActiveSkillStatus>& Statuses, float Now, float HudScale)
{
	TArray<FIdleHUDStatusIndicatorViewModel> Indicators;
	const ESkillStatusEffect DisplayOrder[] = {
		ESkillStatusEffect::Poison,
		ESkillStatusEffect::Burn,
		ESkillStatusEffect::Freeze,
		ESkillStatusEffect::Curse
	};

	for (const ESkillStatusEffect Type : DisplayOrder)
	{
		FIdleHUDStatusIndicatorViewModel Indicator;
		if (TryBuildStatusIndicator(Type, Statuses, Now, HudScale, Indicator))
		{
			Indicators.Add(Indicator);
		}
	}

	return Indicators;
}

void AIdleHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	RootWidget = SNew(SIdleHUDWidget);
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->AddViewportWidgetContent(RootWidget.ToSharedRef());
	}

	IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	if (!IdleGameInstance)
	{
		return;
	}

	IdleGameInstance->OnGoldChanged.AddDynamic(this, &AIdleHUD::HandleGoldChanged);
	IdleGameInstance->OnExpChanged.AddDynamic(this, &AIdleHUD::HandleExpChanged);
	IdleGameInstance->OnLevelUp.AddDynamic(this, &AIdleHUD::HandleLevelUp);
	IdleGameInstance->OnEnhanceResult.AddDynamic(this, &AIdleHUD::HandleEnhanceResult);
	IdleGameInstance->OnShopPurchase.AddDynamic(this, &AIdleHUD::HandleShopPurchase);
	IdleGameInstance->OnPetFed.AddDynamic(this, &AIdleHUD::HandlePetFed);
	IdleGameInstance->OnStatPointsChanged.AddDynamic(this, &AIdleHUD::HandleStatPointsChanged);
	IdleGameInstance->OnTranscend.AddDynamic(this, &AIdleHUD::HandleTranscend);
	IdleGameInstance->OnProgressSaved.AddDynamic(this, &AIdleHUD::HandleProgressSaved);
	IdleGameInstance->OnCloudSyncStateChanged.AddDynamic(this, &AIdleHUD::HandleCloudSyncStateChanged);
	HandleCloudSyncStateChanged(IdleGameInstance->GetCloudSyncState());

	RootWidget->UpdateGold(IdleGameInstance->GetGold());
	RootWidget->UpdateExp(IdleGameInstance->GetCurrentExp(), IdleGameInstance->GetNextExp());
	RootWidget->UpdateLevel(IdleGameInstance->GetCharacterLevel());

	if (!AnyDamageReceivedHandle.IsValid())
	{
		AnyDamageReceivedHandle = UCombatComponent::OnAnyDamageReceived.AddUObject(this, &AIdleHUD::HandleDamageReceived);
	}

	BindPlayerCombat();
	BindPlayerInventory();
	BindStageService();
	BindTowerService();
	BindAchievementService();
}

void AIdleHUD::BeginPlay()
{
	Super::BeginPlay();
	BindPlayerCombat();
	BindPlayerInventory();
	BindStageService();
	BindTowerService();
	BindAchievementService();
	PreviewOfflineRewardModal();
}

void AIdleHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AnyDamageReceivedHandle.IsValid())
	{
		UCombatComponent::OnAnyDamageReceived.Remove(AnyDamageReceivedHandle);
		AnyDamageReceivedHandle.Reset();
	}

	UnbindPlayerCombat();
	UnbindBossSpecialAttack();
	UnbindStageService();
	UnbindTowerService();
	UnbindAchievementService();

	if (IdleGameInstance)
	{
		IdleGameInstance->OnGoldChanged.RemoveDynamic(this, &AIdleHUD::HandleGoldChanged);
		IdleGameInstance->OnExpChanged.RemoveDynamic(this, &AIdleHUD::HandleExpChanged);
		IdleGameInstance->OnLevelUp.RemoveDynamic(this, &AIdleHUD::HandleLevelUp);
		IdleGameInstance->OnEnhanceResult.RemoveDynamic(this, &AIdleHUD::HandleEnhanceResult);
		IdleGameInstance->OnShopPurchase.RemoveDynamic(this, &AIdleHUD::HandleShopPurchase);
		IdleGameInstance->OnPetFed.RemoveDynamic(this, &AIdleHUD::HandlePetFed);
		IdleGameInstance->OnStatPointsChanged.RemoveDynamic(this, &AIdleHUD::HandleStatPointsChanged);
		IdleGameInstance->OnTranscend.RemoveDynamic(this, &AIdleHUD::HandleTranscend);
		IdleGameInstance->OnProgressSaved.RemoveDynamic(this, &AIdleHUD::HandleProgressSaved);
		IdleGameInstance->OnCloudSyncStateChanged.RemoveDynamic(this, &AIdleHUD::HandleCloudSyncStateChanged);
	}

	if (GEngine && GEngine->GameViewport && RootWidget)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(RootWidget.ToSharedRef());
	}

	RootWidget.Reset();
	PlayerInventory = nullptr;
	BoundStageService.Reset();

	Super::EndPlay(EndPlayReason);
}

void AIdleHUD::DrawHUD()
{
	Super::DrawHUD();

	const UWorld* World = GetWorld();
	BindPlayerCombat();

	const USkillComponent* PlayerSkills = ResolvePlayerSkills();
	if (World && PlayerSkills)
	{
		DrawSkillHud(*PlayerSkills, World->GetTimeSeconds());
	}

	DrawStageIndicator();
	DrawBossBar();
	DrawRebirthPanel();
	DrawTranscendPanel();
	DrawTowerPanel();
	DrawDungeonPanel();
	DrawWeeklyBossPanel();
	DrawAttendancePanel();
	DrawAchievementPanel();
	DrawMasteryPanel();
	DrawStatAllocationPanel();
	DrawStatInfoPanel();
	DrawShopPanel();
	DrawConsumablePanel();
	DrawLeaderboardPanel();
	DrawGuildPanel();
	DrawRunePanel();
	DrawRuneCodexPanel();
	DrawEnhancePanel();
	DrawPotentialPanel();
	DrawClassSelectionPanel();
	DrawPetPanel();
	DrawTitlePanel();
	DrawMissionPanel();
	DrawSeasonPassPanel();
	DrawQuestLog();
	DrawOfflineRewardModal();
	if (World)
	{
		DrawBossSpecialWarning(World->GetTimeSeconds());
		DrawFloatingDamageTexts(World->GetTimeSeconds());
		DrawStatusIndicators(World->GetTimeSeconds());
		DrawProgressSavedIndicator(World->GetTimeSeconds());
		DrawCloudSyncIndicator(World->GetTimeSeconds());
	}
}

void AIdleHUD::NotifyHitBoxClick(FName BoxName)
{
	if (BoxName == OfflineRewardClaimHitBoxName)
	{
		ClaimOfflineRewardModal();
		return;
	}
	if (BoxName.ToString().StartsWith(QuestClaimHitBoxPrefix))
	{
		ClaimQuestFromHitBox(BoxName);
		return;
	}
	if (BoxName.ToString().StartsWith(PetEquipHitBoxPrefix))
	{
		EquipPetFromHitBox(BoxName);
		return;
	}
	if (BoxName.ToString().StartsWith(PetFeedHitBoxPrefix))
	{
		TryFeedPetFromHitBox(BoxName);
		return;
	}
	if (BoxName.ToString().StartsWith(PetEvolveHitBoxPrefix))
	{
		TryEvolvePetFromHitBox(BoxName);
		return;
	}
	if (BoxName == TitleUnequipHitBoxName)
	{
		UnequipTitleAction();
		return;
	}
	if (BoxName.ToString().StartsWith(TitleEquipHitBoxPrefix))
	{
		EquipTitleFromHitBox(BoxName);
		return;
	}
	if (BoxName == MissionDailyTabHitBoxName)
	{
		SelectMissionPeriod(EMissionPeriod::Daily);
		return;
	}
	if (BoxName == MissionWeeklyTabHitBoxName)
	{
		SelectMissionPeriod(EMissionPeriod::Weekly);
		return;
	}
	if (BoxName.ToString().StartsWith(MissionClaimHitBoxPrefix))
	{
		ClaimMissionFromHitBox(BoxName);
		return;
	}
	if (BoxName.ToString().StartsWith(SeasonClaimHitBoxPrefix))
	{
		ClaimSeasonTierFromHitBox(BoxName);
		return;
	}
	if (BoxName.ToString().StartsWith(SkillRankHitBoxPrefix))
	{
		RankUpSkillFromHitBox(BoxName);
		return;
	}
	if (BoxName.ToString().StartsWith(EnhanceSlotHitBoxPrefix))
	{
		TryEnhanceFromHitBox(BoxName);
		return;
	}
	if (BoxName.ToString().StartsWith(EnhanceProtectHitBoxPrefix))
	{
		TryEnhanceFromHitBox(BoxName);
		return;
	}
	if (BoxName.ToString().StartsWith(PotentialResetHitBoxPrefix))
	{
		TryRerollPotentialFromHitBox(BoxName, EPotentialCubeType::Reset);
		return;
	}
	if (BoxName.ToString().StartsWith(PotentialRankHitBoxPrefix))
	{
		TryRerollPotentialFromHitBox(BoxName, EPotentialCubeType::Rank);
		return;
	}
	if (BoxName.ToString().StartsWith(PotentialLockHitBoxPrefix))
	{
		ToggleItemLockFromHitBox(BoxName);
		return;
	}
	if (BoxName.ToString().StartsWith(RuneSelectHitBoxPrefix))
	{
		SelectRuneFromHitBox(BoxName);
		return;
	}
	if (BoxName.ToString().StartsWith(RuneEquipHitBoxPrefix))
	{
		EquipSelectedRuneFromHitBox(BoxName);
		return;
	}
	if (BoxName.ToString().StartsWith(RuneUnequipHitBoxPrefix))
	{
		UnequipRuneFromHitBox(BoxName);
		return;
	}
	if (BoxName.ToString().StartsWith(RuneEnhanceHitBoxPrefix))
	{
		TryEnhanceRuneFromHitBox(BoxName);
		return;
	}
	if (BoxName.ToString().StartsWith(RuneDisenchantHitBoxPrefix))
	{
		TryDisenchantRuneFromHitBox(BoxName);
		return;
	}
	if (BoxName == RuneRerollSetHitBoxName)
	{
		TryRerollRuneSetAction();
		return;
	}
	if (BoxName == RuneUpgradeRarityHitBoxName)
	{
		TryUpgradeRuneRarityAction();
		return;
	}
	if (BoxName == RuneTransferHitBoxName)
	{
		TryTransferRuneEnhancementAction();
		return;
	}
	if (BoxName == RuneTransferCycleHitBoxName)
	{
		CycleRuneTransferTarget();
		return;
	}
	if (BoxName.ToString().StartsWith(StatAllocationHitBoxPrefix))
	{
		AllocateStatFromHitBox(BoxName);
		return;
	}
	if (BoxName == StatResetHitBoxName)
	{
		ResetStatAllocation();
		return;
	}
	if (BoxName == StatInfoToggleHitBoxName)
	{
		ToggleStatInfoPanel();
		return;
	}
	if (BoxName == RebirthHitBoxName)
	{
		TryRebirth();
		return;
	}
	if (BoxName == TranscendHitBoxName)
	{
		TryTranscend();
		return;
	}
	if (BoxName == TowerClimbHitBoxName)
	{
		TryClimbTower();
		return;
	}
	if (BoxName.ToString().StartsWith(DungeonEnterHitBoxPrefix))
	{
		TryRunDungeonFromHitBox(BoxName);
		return;
	}
	if (BoxName == WeeklyBossChallengeHitBoxName)
	{
		TryChallengeWeeklyBoss();
		return;
	}
	if (BoxName.ToString().StartsWith(WeeklyBossClaimHitBoxPrefix))
	{
		ClaimWeeklyBossFromHitBox(BoxName);
		return;
	}
	if (BoxName.ToString().StartsWith(AttendanceClaimHitBoxPrefix))
	{
		ClaimAttendanceFromHitBox(BoxName);
		return;
	}
	if (BoxName.ToString().StartsWith(ConsumableUseHitBoxPrefix))
	{
		TryUseConsumableFromHitBox(BoxName);
		return;
	}
	if (BoxName == LeaderboardPowerTabHitBoxName)
	{
		SelectLeaderboardKind(ELeaderboardKind::Power);
		return;
	}
	if (BoxName == LeaderboardWeeklyTabHitBoxName)
	{
		SelectLeaderboardKind(ELeaderboardKind::WeeklyDamage);
		return;
	}
	if (BoxName == LeaderboardRebirthTabHitBoxName)
	{
		SelectLeaderboardKind(ELeaderboardKind::Rebirth);
		return;
	}
	if (BoxName == LeaderboardRefreshHitBoxName)
	{
		RefreshSelectedLeaderboard();
		return;
	}
	if (BoxName == GuildRefreshListHitBoxName)
	{
		RefreshGuildBrowseList();
		return;
	}
	if (BoxName == GuildCreateNameCycleHitBoxName)
	{
		CycleGuildCreateName();
		return;
	}
	if (BoxName == GuildCreateHitBoxName)
	{
		TryCreateGuild();
		return;
	}
	if (BoxName == GuildLeaveHitBoxName)
	{
		TryLeaveGuild();
		return;
	}
	if (BoxName == GuildToggleJoinModeHitBoxName)
	{
		ToggleGuildJoinMode();
		return;
	}
	if (BoxName.ToString().StartsWith(GuildJoinHitBoxPrefix))
	{
		JoinGuildFromHitBox(BoxName);
		return;
	}
	if (BoxName.ToString().StartsWith(GuildApproveHitBoxPrefix))
	{
		ApproveGuildRequestFromHitBox(BoxName);
		return;
	}
	if (BoxName.ToString().StartsWith(GuildRejectHitBoxPrefix))
	{
		RejectGuildRequestFromHitBox(BoxName);
		return;
	}
	if (BoxName.ToString().StartsWith(GuildPromoteHitBoxPrefix))
	{
		SetGuildMemberRankFromHitBox(BoxName, true);
		return;
	}
	if (BoxName.ToString().StartsWith(GuildDemoteHitBoxPrefix))
	{
		SetGuildMemberRankFromHitBox(BoxName, false);
		return;
	}
	if (BoxName == GuildAttendHitBoxName)
	{
		TryGuildAttendance();
		return;
	}
	if (BoxName == GuildDonateCycleHitBoxName)
	{
		CycleGuildDonateAmount();
		return;
	}
	if (BoxName == GuildDonateHitBoxName)
	{
		TryGuildDonate();
		return;
	}
	if (BoxName.ToString().StartsWith(GuildShopBuyHitBoxPrefix))
	{
		BuyGuildShopItemFromHitBox(BoxName);
		return;
	}
	if (BoxName == GuildTabMyHitBoxName)
	{
		SetGuildRankingsView(false);
		return;
	}
	if (BoxName == GuildTabRankingsHitBoxName)
	{
		SetGuildRankingsView(true);
		return;
	}
	if (BoxName == GuildBossChallengeHitBoxName)
	{
		TryChallengeGuildBoss();
		return;
	}
	if (BoxName == GuildBossClaimHitBoxName)
	{
		TryClaimGuildBossReward();
		return;
	}
	if (BoxName == ShopGearRollHitBoxName)
	{
		TryBuyGearRoll();
		return;
	}
	if (BoxName == ShopProtectionScrollHitBoxName)
	{
		TryBuyProtectionScroll();
		return;
	}
	if (BoxName == ShopResetCubeHitBoxName)
	{
		TryBuyResetCube();
		return;
	}
	if (BoxName == ShopRankCubeHitBoxName)
	{
		TryBuyRankCube();
		return;
	}
	if (BoxName == ShopRuneRollHitBoxName)
	{
		TryBuyRuneRoll();
		return;
	}
	if (BoxName == CraftClassRuneHitBoxName)
	{
		TryCraftClassRune();
		return;
	}
	if (BoxName.ToString().StartsWith(ClassSelectionHitBoxPrefix))
	{
		SelectClassFromHitBox(BoxName);
		return;
	}

	Super::NotifyHitBoxClick(BoxName);
}

void AIdleHUD::ToggleQuestLog()
{
	bQuestLogVisible = !bQuestLogVisible;
	if (PlayerOwner)
	{
		RefreshMouseInteraction();
	}
}

void AIdleHUD::HandleGoldChanged(int64 NewGold)
{
	UE_LOG(LogTemp, Display, TEXT("[IdleHUD] Gold=%lld"), NewGold);
	if (RootWidget)
	{
		RootWidget->UpdateGold(NewGold);
	}
}

void AIdleHUD::HandleExpChanged(int64 CurrentExp, int64 NextExp)
{
	UE_LOG(LogTemp, Display, TEXT("[IdleHUD] EXP=%lld/%lld"), CurrentExp, NextExp);
	if (RootWidget)
	{
		RootWidget->UpdateExp(CurrentExp, NextExp);
	}
}

void AIdleHUD::HandleLevelUp(int32 NewLevel)
{
	UE_LOG(LogTemp, Display, TEXT("[IdleHUD] Level=%d"), NewLevel);
	if (RootWidget)
	{
		RootWidget->UpdateLevel(NewLevel);
	}
}

void AIdleHUD::HandleHpChanged(float NewHp)
{
	if (RootWidget && PlayerCombat)
	{
		RootWidget->UpdateHp(NewHp, PlayerCombat->MaxHp);
	}
}

void AIdleHUD::HandleDamageReceived(AActor* DamagedActor, float Amount, bool bWasCrit, EDamageKind Kind)
{
	const UWorld* World = GetWorld();
	if (!World || !DamagedActor)
	{
		return;
	}

	FIdleHUDFloatingDamageEntry Entry;
	Entry.WorldLocation = DamagedActor->GetActorLocation() + FVector(0.0f, 0.0f, FloatingDamageHeadOffsetZ);
	Entry.Amount = Amount;
	Entry.bWasCrit = bWasCrit;
	Entry.Kind = Kind;
	Entry.StartTime = World->GetTimeSeconds();
	FloatingDamageEntries.Add(Entry);
}

void AIdleHUD::HandleEquippedChanged(EItemSlot Slot)
{
	RefreshEquipmentSummary();
}

void AIdleHUD::HandleEnhanceResult(const FEnhanceAttemptResult& Result)
{
	bEnhanceFeedbackSuccess = Result.bAttempted && Result.bSuccess;
	if (!Result.bAttempted)
	{
		EnhanceFeedbackLabel = IdleProject::Localization::UI(TEXT("ENHANCE_FEEDBACK_BLOCKED"));
		return;
	}

	if (Result.bSuccess)
	{
		EnhanceFeedbackLabel = FormatLocalizedUI(TEXT("ENHANCE_FEEDBACK_SUCCESS_FORMAT"), [&Result](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Level"), FText::AsNumber(Result.NewLevel));
			Args.Add(TEXT("Gold"), FText::FromString(FormatIntegerWithCommas(Result.GoldSpent)));
		});
	}
	else
	{
		EnhanceFeedbackLabel = FormatLocalizedUI(TEXT("ENHANCE_FEEDBACK_FAIL_FORMAT"), [&Result](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Gold"), FText::FromString(FormatIntegerWithCommas(Result.GoldSpent)));
		});
	}

	RefreshMouseInteraction();
}

void AIdleHUD::HandleShopPurchase(const FShopPurchaseResult& Result)
{
	LastShopPurchaseResult = Result;
	if (!LastShopPurchaseResult.bPurchased && LastShopPurchaseResult.GoldSpent <= 0 && IdleGameInstance)
	{
		const UStageService* StageService = IdleGameInstance->GetStageService();
		const int32 GlobalStageIndex = StageService ? StageService->GetGlobalStageIndex() : 0;
		LastShopPurchaseResult.GoldSpent = FShopFormula::GetGearRollCost(GlobalStageIndex);
	}

	RefreshEquipmentSummary();
	RefreshMouseInteraction();
}

void AIdleHUD::HandlePetFed(const FPetFeedResult& Result)
{
	bPetFeedbackSuccess = Result.bFed;
	if (Result.bFed)
	{
		PetFeedbackLabel = FormatLocalizedUI(TEXT("PET_FEED_FEEDBACK_SUCCESS_FORMAT"), [&Result](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Level"), FText::AsNumber(Result.NewLevel));
			Args.Add(TEXT("Gold"), FText::FromString(FormatIntegerWithCommas(Result.GoldSpent)));
		});
	}
	else
	{
		PetFeedbackLabel = IdleProject::Localization::UI(TEXT("PET_FEED_FEEDBACK_BLOCKED"));
	}

	const UWorld* World = GetWorld();
	PetFeedbackStartTime = World ? World->GetTimeSeconds() : 0.0f;
	RefreshMouseInteraction();
}

void AIdleHUD::HandleStatPointsChanged()
{
	RefreshMouseInteraction();
}

void AIdleHUD::HandleTranscend()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	const float CurrentMultiplier = IdleGameInstance ? IdleGameInstance->GetTranscendStatMultiplier() : 1.0f;
	const FString MultiplierText = FString::Printf(TEXT("x%.2f"), FMath::Max(0.0f, CurrentMultiplier));
	TranscendFeedbackLabel = FormatLocalizedUI(TEXT("TRANSCEND_FEEDBACK_FORMAT"), [&MultiplierText](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Multiplier"), FText::FromString(MultiplierText));
	});
	const UWorld* World = GetWorld();
	TranscendFeedbackStartTime = World ? World->GetTimeSeconds() : 0.0f;
	RefreshMouseInteraction();
}

void AIdleHUD::HandleBossSpecialAttack(AActor* Boss)
{
	const UWorld* World = GetWorld();
	if (!World || !Boss)
	{
		return;
	}

	BossSpecialAttackActor = Boss;
	BossSpecialAttackStartTime = World->GetTimeSeconds();
}

void AIdleHUD::HandleStageChanged(FStageInfo NewStageInfo)
{
	if (NewStageInfo.Chapter > 1 && NewStageInfo.Stage == 1 && NewStageInfo.KillsThisStage == 0)
	{
		StageFeedbackLabel = IdleProject::UI::BuildChapterEntryFeedbackLabel(NewStageInfo.Chapter);
		if (const UWorld* World = GetWorld())
		{
			StageFeedbackStartTime = World->GetTimeSeconds();
		}
	}
}

void AIdleHUD::HandleChapterBossDefeated(int32 ClearedChapter)
{
	StageFeedbackLabel = IdleProject::UI::BuildChapterClearFeedbackLabel(ClearedChapter);
	if (const UWorld* World = GetWorld())
	{
		StageFeedbackStartTime = World->GetTimeSeconds();
	}
}

void AIdleHUD::HandleTowerClimbed(int32 NewHighestFloor, int64 TotalReward)
{
	TowerFeedbackLabel = IdleProject::UI::BuildTowerClimbFeedbackLabel(NewHighestFloor, TotalReward);
	if (const UWorld* World = GetWorld())
	{
		TowerFeedbackStartTime = World->GetTimeSeconds();
	}
	RefreshMouseInteraction();
}

void AIdleHUD::HandleAchievementUnlocked(const FString& AchievementId, int32 Tier)
{
	AchievementFeedbackLabel = IdleProject::UI::BuildAchievementUnlockedFeedbackLabel(AchievementId, Tier);
	if (const UWorld* World = GetWorld())
	{
		AchievementFeedbackStartTime = World->GetTimeSeconds();
	}
}

void AIdleHUD::HandleProgressSaved()
{
	ProgressSavedFeedbackLabel = IdleProject::UI::BuildProgressSavedFeedbackLabel();
	if (const UWorld* World = GetWorld())
	{
		ProgressSavedFeedbackStartTime = World->GetTimeSeconds();
	}
}

void AIdleHUD::HandleCloudSyncStateChanged(ECloudSyncState NewState)
{
	CloudSyncViewModel = IdleProject::UI::BuildCloudSyncViewModel(NewState);
	if (const UWorld* World = GetWorld())
	{
		CloudSyncFeedbackStartTime = World->GetTimeSeconds();
	}
}

void AIdleHUD::BindStageService()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}

	UStageService* StageService = IdleGameInstance ? IdleGameInstance->GetStageService() : nullptr;
	if (!StageService)
	{
		UnbindStageService();
		return;
	}

	if (BoundStageService.Get() == StageService)
	{
		return;
	}

	UnbindStageService();
	BoundStageService = StageService;
	StageService->OnStageChanged.AddUniqueDynamic(this, &AIdleHUD::HandleStageChanged);
	StageService->OnChapterBossDefeated.AddUniqueDynamic(this, &AIdleHUD::HandleChapterBossDefeated);
}

void AIdleHUD::UnbindStageService()
{
	if (UStageService* StageService = BoundStageService.Get())
	{
		StageService->OnStageChanged.RemoveDynamic(this, &AIdleHUD::HandleStageChanged);
		StageService->OnChapterBossDefeated.RemoveDynamic(this, &AIdleHUD::HandleChapterBossDefeated);
	}
	BoundStageService.Reset();
}

void AIdleHUD::BindTowerService()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}

	UTowerService* TowerService = IdleGameInstance ? IdleGameInstance->GetTowerService() : nullptr;
	if (!TowerService)
	{
		UnbindTowerService();
		return;
	}

	if (BoundTowerService.Get() == TowerService)
	{
		return;
	}

	UnbindTowerService();
	BoundTowerService = TowerService;
	TowerService->OnTowerClimbed.AddUniqueDynamic(this, &AIdleHUD::HandleTowerClimbed);
}

void AIdleHUD::UnbindTowerService()
{
	if (UTowerService* TowerService = BoundTowerService.Get())
	{
		TowerService->OnTowerClimbed.RemoveDynamic(this, &AIdleHUD::HandleTowerClimbed);
	}
	BoundTowerService.Reset();
}

void AIdleHUD::BindAchievementService()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}

	UAchievementService* AchievementService = IdleGameInstance ? IdleGameInstance->GetAchievementService() : nullptr;
	if (!AchievementService)
	{
		UnbindAchievementService();
		return;
	}

	if (BoundAchievementService.Get() == AchievementService)
	{
		return;
	}

	UnbindAchievementService();
	BoundAchievementService = AchievementService;
	AchievementService->OnAchievementUnlocked.AddUniqueDynamic(this, &AIdleHUD::HandleAchievementUnlocked);
}

void AIdleHUD::UnbindAchievementService()
{
	if (UAchievementService* AchievementService = BoundAchievementService.Get())
	{
		AchievementService->OnAchievementUnlocked.RemoveDynamic(this, &AIdleHUD::HandleAchievementUnlocked);
	}
	BoundAchievementService.Reset();
}

void AIdleHUD::BindPlayerCombat()
{
	APawn* Pawn = PlayerOwner ? PlayerOwner->GetPawn() : nullptr;
	if (!Pawn)
	{
		UnbindPlayerCombat();
		return;
	}

	if (PlayerCombat && PlayerCombatPawn == Pawn)
	{
		return;
	}

	UnbindPlayerCombat();
	PlayerCombat = Pawn->FindComponentByClass<UCombatComponent>();
	if (!PlayerCombat)
	{
		return;
	}

	PlayerCombatPawn = Pawn;
	PlayerCombat->OnHpChanged.AddDynamic(this, &AIdleHUD::HandleHpChanged);
	if (RootWidget)
	{
		RootWidget->UpdateHp(PlayerCombat->CurrentHp, PlayerCombat->MaxHp);
	}
}

void AIdleHUD::UnbindPlayerCombat()
{
	if (PlayerCombat)
	{
		PlayerCombat->OnHpChanged.RemoveDynamic(this, &AIdleHUD::HandleHpChanged);
	}

	PlayerCombat = nullptr;
	PlayerCombatPawn = nullptr;
}

void AIdleHUD::BindBossSpecialAttack(AIdleMonster* Boss)
{
	UBattleAIComponent* BattleAI = Boss ? Boss->FindComponentByClass<UBattleAIComponent>() : nullptr;
	if (!BattleAI)
	{
		UnbindBossSpecialAttack();
		return;
	}

	if (BoundBossBattleAI.Get() == BattleAI)
	{
		return;
	}

	UnbindBossSpecialAttack();
	BattleAI->OnBossSpecialAttack.AddDynamic(this, &AIdleHUD::HandleBossSpecialAttack);
	BoundBossBattleAI = BattleAI;
}

void AIdleHUD::UnbindBossSpecialAttack()
{
	if (UBattleAIComponent* BattleAI = BoundBossBattleAI.Get())
	{
		BattleAI->OnBossSpecialAttack.RemoveDynamic(this, &AIdleHUD::HandleBossSpecialAttack);
	}
	BoundBossBattleAI.Reset();
}

void AIdleHUD::BindPlayerInventory()
{
	if (PlayerInventory)
	{
		return;
	}

	APawn* Pawn = PlayerOwner ? PlayerOwner->GetPawn() : nullptr;
	if (!Pawn)
	{
		return;
	}

	PlayerInventory = Pawn->FindComponentByClass<UInventoryComponent>();
	if (!PlayerInventory)
	{
		return;
	}

	PlayerInventory->OnEquippedChanged.AddDynamic(this, &AIdleHUD::HandleEquippedChanged);
	RefreshEquipmentSummary();
}

void AIdleHUD::RefreshEquipmentSummary()
{
	if (!RootWidget || !PlayerInventory)
	{
		return;
	}

	FText WeaponLine = IdleProject::Localization::UI(TEXT("HUD_NO_WEAPON"));
	FLinearColor WeaponColor = IdleProject::UI::Theme::TextMuted;
	TArray<FItemInstance> EquippedItems;
	if (const FItemInstance* Weapon = PlayerInventory->GetEquippedItem(EItemSlot::Weapon))
	{
		EquippedItems.Add(*Weapon);
		WeaponLine = FormatLocalizedUI(TEXT("HUD_WEAPON_FORMAT"), [Weapon](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Name"), Weapon->DisplayName);
			Args.Add(TEXT("Rarity"), IdleProject::UI::RarityToLabel(Weapon->Rarity));
			Args.Add(TEXT("Attack"), FText::AsNumber(FMath::RoundToInt(Weapon->BonusAtk)));
		});
		const FText AffixSummary = IdleProject::UI::BuildAffixSummary(*Weapon);
		if (!AffixSummary.IsEmpty())
		{
			WeaponLine = FText::Format(FText::FromString(TEXT("{0}\n{1}")), WeaponLine, AffixSummary);
		}
		const FText TraitSummary = IdleProject::UI::BuildUniqueTraitSummary(*Weapon);
		if (!TraitSummary.IsEmpty())
		{
			WeaponLine = FText::Format(FText::FromString(TEXT("{0}\n{1}")), WeaponLine, TraitSummary);
		}
		WeaponColor = IdleProject::UI::RarityToColor(Weapon->Rarity);
	}

	const EItemSlot ArmorSlots[] = {
		EItemSlot::Helmet,
		EItemSlot::Top,
		EItemSlot::Bottom,
		EItemSlot::Shoes,
		EItemSlot::Gloves,
		EItemSlot::Cloak,
		EItemSlot::Accessory
	};

	int32 EquippedArmorCount = 0;
	float BonusDef = 0.0f;
	float BonusHp = 0.0f;
	TArray<FString> ArmorTraitLines;

	for (EItemSlot Slot : ArmorSlots)
	{
		if (const FItemInstance* Item = PlayerInventory->GetEquippedItem(Slot))
		{
			EquippedItems.Add(*Item);
			++EquippedArmorCount;
			BonusDef += Item->BonusDef;
			BonusHp += Item->BonusHp;
			const FText TraitSummary = IdleProject::UI::BuildUniqueTraitSummary(*Item);
			if (!TraitSummary.IsEmpty())
			{
				ArmorTraitLines.Add(FString::Printf(TEXT("%s: %s"), *Item->DisplayName.ToString(), *TraitSummary.ToString()));
			}
		}
	}

	FText ArmorLine = FormatLocalizedUI(TEXT("HUD_ARMOR_SUMMARY_FORMAT"), [EquippedArmorCount, BonusDef, BonusHp](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Count"), FText::AsNumber(EquippedArmorCount));
		Args.Add(TEXT("Defense"), FText::AsNumber(FMath::RoundToInt(BonusDef)));
		Args.Add(TEXT("Hp"), FText::AsNumber(FMath::RoundToInt(BonusHp)));
	});
	if (!ArmorTraitLines.IsEmpty())
	{
		ArmorLine = FText::Format(
			FText::FromString(TEXT("{0}\n{1}")),
			ArmorLine,
			FText::FromString(FString::Join(ArmorTraitLines, TEXT("\n"))));
	}

	TArray<FString> SetLines;
	const FIdleHUDSetSummaryViewModel SetViewModel = IdleProject::UI::BuildSetSummaryViewModel(EquippedItems);
	for (const FIdleHUDSetSummaryRowViewModel& Row : SetViewModel.Rows)
	{
		SetLines.Add(Row.SummaryLabel.ToString());
		SetLines.Add(FString::Printf(TEXT("%s / %s"), *Row.BonusLabel.ToString(), *Row.NextTierLabel.ToString()));
	}
	const FText SetLine = SetLines.IsEmpty()
		? FText::GetEmpty()
		: FText::FromString(FString::Join(SetLines, TEXT("\n")));

	RootWidget->UpdateEquipment(WeaponLine, ArmorLine, SetLine, WeaponColor);
}

USkillComponent* AIdleHUD::ResolvePlayerSkills() const
{
	const AIdleCharacter* Character = ResolvePlayerCharacter();
	return Character ? Character->FindComponentByClass<USkillComponent>() : nullptr;
}

AIdleCharacter* AIdleHUD::ResolvePlayerCharacter() const
{
	APawn* Pawn = PlayerOwner ? PlayerOwner->GetPawn() : nullptr;
	return Pawn ? Cast<AIdleCharacter>(Pawn) : nullptr;
}

AIdleMonster* AIdleHUD::ResolveVisibleBoss() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<AIdleMonster> ActorIt(World); ActorIt; ++ActorIt)
	{
		AIdleMonster* Monster = *ActorIt;
		if (!IsValid(Monster) || !Monster->IsBoss())
		{
			continue;
		}

		const UCombatComponent* Combat = Monster->GetCombat();
		if (!Combat || Combat->MaxHp <= 0.0f || Combat->IsDead())
		{
			continue;
		}

		return Monster;
	}

	return nullptr;
}

void AIdleHUD::DrawClassSelectionPanel()
{
	using namespace IdleProject::UI;

	if (!Canvas)
	{
		return;
	}

	AIdleCharacter* IdleCharacter = ResolvePlayerCharacter();
	if (!IdleCharacter)
	{
		return;
	}

	const TArray<FIdleHUDClassSelectionOptionViewModel> Options = BuildClassSelectionOptions(IdleCharacter->GetClassId());
	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelWidth = 322.0f * Scale;
	const float HeaderHeight = 38.0f * Scale;
	const float OptionHeight = 34.0f * Scale;
	const float OptionGap = 6.0f * Scale;
	const float Padding = 10.0f * Scale;
	const float PanelHeight = HeaderHeight + Padding + Options.Num() * OptionHeight + FMath::Max(0, Options.Num() - 1) * OptionGap + Padding;
	const float X = 28.0f * Scale;
	const float Y = 92.0f * Scale;
	const float Border = 2.0f * Scale;

	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.91f), X, Y, PanelWidth, PanelHeight);
	DrawRect(Theme::AccentBlue, X, Y, PanelWidth, Border);
	DrawRect(Theme::AccentBlue, X, Y + PanelHeight - Border, PanelWidth, Border);
	DrawRect(Theme::AccentBlue, X, Y, Border, PanelHeight);
	DrawRect(Theme::AccentBlue, X + PanelWidth - Border, Y, Border, PanelHeight);

	DrawText(IdleProject::Localization::UI(TEXT("CLASS_PANEL_TITLE")).ToString(), Theme::TextPrimary, X + Padding, Y + 10.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 0.88f * Scale);
	DrawText(IdleProject::Localization::UI(TEXT("CLASS_PANEL_SUBTITLE")).ToString(), Theme::TextMuted, X + PanelWidth - 92.0f * Scale, Y + 14.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.68f * Scale);

	float OptionY = Y + HeaderHeight + Padding;
	for (const FIdleHUDClassSelectionOptionViewModel& Option : Options)
	{
		DrawClassSelectionOption(Option, X + Padding, OptionY, PanelWidth - Padding * 2.0f, OptionHeight);
		OptionY += OptionHeight + OptionGap;
	}

	RefreshMouseInteraction();
}

void AIdleHUD::DrawClassSelectionOption(const FIdleHUDClassSelectionOptionViewModel& Option, float X, float Y, float Width, float Height)
{
	using namespace IdleProject::UI;

	const float Scale = Height / 34.0f;
	const FLinearColor StateColor = Option.bSelected ? Theme::AccentGold : Theme::TextMuted.CopyWithNewOpacity(0.56f);
	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.90f), X, Y, Width, Height);
	DrawRect(StateColor, X, Y, 4.0f * Scale, Height);

	DrawText(Option.DisplayName.ToString(), Option.bSelected ? Theme::AccentGold : Theme::TextPrimary, X + 12.0f * Scale, Y + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.78f * Scale);
	DrawText(Option.RoleLabel.ToString(), Theme::TextMuted, X + 70.0f * Scale, Y + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.64f * Scale);
	DrawText(Option.StatSummary.ToString(), Theme::AccentBlue, X + 12.0f * Scale, Y + 20.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.66f * Scale);

	const FString StateLabel = IdleProject::Localization::UI(Option.bSelected ? TEXT("CLASS_SELECTED") : TEXT("CLASS_SELECT")).ToString();
	DrawText(StateLabel, Option.bSelected ? Theme::AccentGold : Theme::TextMuted, X + Width - 62.0f * Scale, Y + 11.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.60f * Scale);
	AddHitBox(FVector2D(X, Y), FVector2D(Width, Height), MakeClassSelectionHitBoxName(Option.ClassId), true, 70);
}

void AIdleHUD::SelectClassFromHitBox(FName BoxName)
{
	FString RawClassId = BoxName.ToString();
	RawClassId.RightChopInline(ClassSelectionHitBoxPrefix.Len());
	const int32 ClassIdValue = FCString::Atoi(*RawClassId);
	if (ClassIdValue < static_cast<int32>(EClassId::Warrior) || ClassIdValue > static_cast<int32>(EClassId::Summoner))
	{
		return;
	}

	AIdleCharacter* IdleCharacter = ResolvePlayerCharacter();
	if (!IdleCharacter)
	{
		return;
	}

	IdleCharacter->SetClassId(static_cast<EClassId>(ClassIdValue));
}

void AIdleHUD::DrawStageIndicator()
{
	using namespace IdleProject::UI;
	using namespace IdleProject::UI::Theme;

	if (!Canvas)
	{
		return;
	}
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	BindStageService();

	const UStageService* StageService = IdleGameInstance ? IdleGameInstance->GetStageService() : nullptr;
	if (!StageService)
	{
		return;
	}

	const FIdleHUDStageViewModel ViewModel = BuildStageViewModel(StageService->GetCurrentStageInfo());
	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelWidth = FMath::Clamp(Canvas->SizeX * 0.26f, 360.0f * Scale, 560.0f * Scale);
	const float PanelHeight = 76.0f * Scale;
	const float X = (Canvas->SizeX - PanelWidth) * 0.5f;
	const float Y = 24.0f * Scale;
	const float Padding = 14.0f * Scale;
	const float Border = 2.0f * Scale;
	const float BarHeight = 6.0f * Scale;

	DrawRect(BgPanel.CopyWithNewOpacity(0.91f), X, Y, PanelWidth, PanelHeight);
	DrawRect(ViewModel.BorderColor, X, Y, PanelWidth, Border);
	DrawRect(ViewModel.BorderColor, X, Y + PanelHeight - Border, PanelWidth, Border);
	DrawRect(ViewModel.BorderColor, X, Y, Border, PanelHeight);
	DrawRect(ViewModel.BorderColor, X + PanelWidth - Border, Y, Border, PanelHeight);

	DrawText(ViewModel.TitleLabel.ToString(), TextMuted, X + Padding, Y + 10.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.82f * Scale);
	DrawText(ViewModel.ChapterLabel.ToString(), AccentGold, X + 82.0f * Scale, Y + 10.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.82f * Scale);
	DrawText(ViewModel.ProgressLabel.ToString(), TextPrimary, X + Padding, Y + 31.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 0.92f * Scale);
	const float WeaknessIconSize = 22.0f * Scale;
	const float WeaknessIconX = X + PanelWidth - 156.0f * Scale;
	const float WeaknessIconY = Y + 12.0f * Scale;
	DrawRect(ViewModel.WeaknessColor.CopyWithNewOpacity(0.92f), WeaknessIconX, WeaknessIconY, WeaknessIconSize, WeaknessIconSize);
	DrawText(ViewModel.WeaknessIconLabel.ToString(), BgPrimary, WeaknessIconX + 7.0f * Scale, WeaknessIconY + 3.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.72f * Scale);
	DrawText(ViewModel.WeaknessLabel.ToString(), ViewModel.WeaknessColor, X + PanelWidth - 128.0f * Scale, Y + 15.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.78f * Scale);
	if (ViewModel.bBossStage || ViewModel.bEliteStage)
	{
		const bool bEliteBadge = ViewModel.bEliteStage && !ViewModel.bBossStage;
		const float BadgeWidth = bEliteBadge ? 62.0f * Scale : 58.0f * Scale;
		const float BadgeHeight = 22.0f * Scale;
		const float BadgeX = X + PanelWidth - Padding - BadgeWidth;
		const float BadgeY = Y + 42.0f * Scale;
		const FLinearColor BadgeColor = bEliteBadge ? ElementDark : AccentGold;
		const FText BadgeLabel = bEliteBadge ? ViewModel.EliteBadgeLabel : ViewModel.BossBadgeLabel;
		DrawRect(BadgeColor, BadgeX, BadgeY, BadgeWidth, BadgeHeight);
		DrawText(BadgeLabel.ToString(), BgPrimary, BadgeX + 11.0f * Scale, BadgeY + 4.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.72f * Scale);
	}

	const float BarX = X + Padding;
	const float BarY = Y + PanelHeight - Padding - BarHeight;
	const float BarWidth = PanelWidth - Padding * 2.0f;
	DrawRect(BgPrimary.CopyWithNewOpacity(0.94f), BarX, BarY, BarWidth, BarHeight);
	DrawRect(ViewModel.BorderColor, BarX, BarY, BarWidth * ViewModel.ProgressRatio, BarHeight);

	const UWorld* World = GetWorld();
	if (!StageFeedbackLabel.IsEmpty() && World)
	{
		const float Elapsed = World->GetTimeSeconds() - StageFeedbackStartTime;
		if (Elapsed <= StageFeedbackDurationSeconds)
		{
			const float Alpha = FMath::Clamp(1.0f - (Elapsed / StageFeedbackDurationSeconds), 0.0f, 1.0f);
			const float FeedbackWidth = 220.0f * Scale;
			const float FeedbackHeight = 30.0f * Scale;
			const float FeedbackX = (Canvas->SizeX - FeedbackWidth) * 0.5f;
			const float FeedbackY = Y + PanelHeight + 8.0f * Scale;
			DrawRect(BgPrimary.CopyWithNewOpacity(0.72f * Alpha), FeedbackX, FeedbackY, FeedbackWidth, FeedbackHeight);
			DrawText(StageFeedbackLabel.ToString(), AccentGold.CopyWithNewOpacity(Alpha), FeedbackX + 18.0f * Scale, FeedbackY + 7.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.86f * Scale);
		}
	}
}

void AIdleHUD::DrawBossBar()
{
	using namespace IdleProject::UI;
	using namespace IdleProject::UI::Theme;

	if (!Canvas)
	{
		return;
	}

	AIdleMonster* Boss = ResolveVisibleBoss();
	BindBossSpecialAttack(Boss);
	if (!Boss)
	{
		return;
	}

	const UCombatComponent* BossCombat = Boss->GetCombat();
	if (!BossCombat)
	{
		return;
	}

	const FIdleHUDBossViewModel ViewModel = BuildBossViewModel(BossCombat->CurrentHp, BossCombat->MaxHp);
	if (!ViewModel.bVisible)
	{
		return;
	}

	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelWidth = FMath::Clamp(Canvas->SizeX * 0.42f, 520.0f * Scale, 900.0f * Scale);
	const float PanelHeight = 58.0f * Scale;
	const float X = (Canvas->SizeX - PanelWidth) * 0.5f;
	const float Y = 108.0f * Scale;
	const float Padding = 14.0f * Scale;
	const float Border = 2.0f * Scale;
	const float BarHeight = 10.0f * Scale;

	DrawRect(BgPanel.CopyWithNewOpacity(0.93f), X, Y, PanelWidth, PanelHeight);
	DrawRect(ViewModel.PhaseColor, X, Y, PanelWidth, Border);
	DrawRect(ViewModel.PhaseColor, X, Y + PanelHeight - Border, PanelWidth, Border);
	DrawRect(ViewModel.PhaseColor, X, Y, Border, PanelHeight);
	DrawRect(ViewModel.PhaseColor, X + PanelWidth - Border, Y, Border, PanelHeight);

	DrawText(ViewModel.TitleLabel.ToString(), ViewModel.PhaseColor, X + Padding, Y + 9.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.82f * Scale);
	DrawText(ViewModel.HpLabel.ToString(), TextPrimary, X + 86.0f * Scale, Y + 8.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.88f * Scale);
	DrawText(ViewModel.PhaseLabel.ToString(), ViewModel.PhaseColor, X + PanelWidth - 100.0f * Scale, Y + 9.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.82f * Scale);

	const float BarX = X + Padding;
	const float BarY = Y + PanelHeight - Padding - BarHeight;
	const float BarWidth = PanelWidth - Padding * 2.0f;
	DrawRect(BgPrimary.CopyWithNewOpacity(0.95f), BarX, BarY, BarWidth, BarHeight);
	DrawRect(ViewModel.PhaseColor, BarX, BarY, BarWidth * ViewModel.HpRatio, BarHeight);
}

void AIdleHUD::DrawBossSpecialWarning(float Now)
{
	using namespace IdleProject::UI::Theme;

	if (!Canvas)
	{
		return;
	}

	const float Elapsed = Now - BossSpecialAttackStartTime;
	if (Elapsed < 0.0f || Elapsed > BossSpecialWarningDurationSeconds || !BossSpecialAttackActor.IsValid())
	{
		return;
	}

	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float Alpha = 1.0f - FMath::Clamp(Elapsed / BossSpecialWarningDurationSeconds, 0.0f, 1.0f);
	const FString Warning = IdleProject::Localization::UI(TEXT("BOSS_SPECIAL_WARNING")).ToString();
	const float Width = 210.0f * Scale;
	const float Height = 34.0f * Scale;
	const float X = (Canvas->SizeX - Width) * 0.5f;
	const float Y = 174.0f * Scale;

	DrawRect(BgPrimary.CopyWithNewOpacity(0.82f * Alpha), X, Y, Width, Height);
	DrawRect(AccentRed.CopyWithNewOpacity(0.96f * Alpha), X, Y, 5.0f * Scale, Height);
	DrawText(Warning, AccentRed.CopyWithNewOpacity(Alpha), X + 18.0f * Scale, Y + 7.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 0.82f * Scale);
}

void AIdleHUD::DrawShopPanel()
{
	using namespace IdleProject::UI;

	if (!Canvas)
	{
		return;
	}
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	BindPlayerInventory();
	if (!IdleGameInstance || !PlayerInventory)
	{
		return;
	}

	const UStageService* StageService = IdleGameInstance->GetStageService();
	const int32 GlobalStageIndex = StageService ? StageService->GetGlobalStageIndex() : 0;
	const FIdleHUDShopPanelViewModel ViewModel = BuildShopPanelViewModel(
		FShopFormula::GetGearRollCost(GlobalStageIndex),
		FShopFormula::GetProtectionScrollCost(GlobalStageIndex),
		FShopFormula::GetResetCubeCost(GlobalStageIndex),
		FShopFormula::GetRankCubeCost(GlobalStageIndex),
		IdleGameInstance->GetGold(),
		LastShopPurchaseResult);

	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelWidth = FMath::Clamp(Canvas->SizeX * 0.22f, 300.0f * Scale, 380.0f * Scale);
	const float PanelHeight = (ViewModel.bHasLastResult ? 292.0f : 258.0f) * Scale;
	const float X = Canvas->SizeX - PanelWidth - 28.0f * Scale;
	const float Y = 92.0f * Scale;
	const float Padding = 14.0f * Scale;
	const float Border = 2.0f * Scale;
	const FLinearColor StateColor = ViewModel.bCanBuyGearRoll ? Theme::AccentGold : Theme::TextMuted.CopyWithNewOpacity(0.68f);

	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.91f), X, Y, PanelWidth, PanelHeight);
	DrawRect(StateColor, X, Y, PanelWidth, Border);
	DrawRect(StateColor, X, Y + PanelHeight - Border, PanelWidth, Border);
	DrawRect(StateColor, X, Y, Border, PanelHeight);
	DrawRect(StateColor, X + PanelWidth - Border, Y, Border, PanelHeight);

	DrawText(ViewModel.Title.ToString(), Theme::TextPrimary, X + Padding, Y + 12.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 0.92f * Scale);
	DrawText(ViewModel.GoldLabel.ToString(), Theme::AccentGold, X + PanelWidth - 110.0f * Scale, Y + 16.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.76f * Scale);

	const float ButtonWidth = 112.0f * Scale;
	const float ButtonHeight = 28.0f * Scale;
	const float ButtonX = X + PanelWidth - Padding - ButtonWidth;
	auto DrawShopRow = [&](float RowY, const FText& CostLabel, const FText& ButtonLabel, bool bCanBuy, FName HitBoxName, int32 Priority)
	{
		DrawText(CostLabel.ToString(), bCanBuy ? Theme::TextPrimary : Theme::AccentRed, X + Padding, RowY + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.72f * Scale);
		DrawRect(bCanBuy ? Theme::AccentGold : Theme::BgPrimary.CopyWithNewOpacity(0.94f), ButtonX, RowY, ButtonWidth, ButtonHeight);
		DrawText(ButtonLabel.ToString(), bCanBuy ? Theme::BgPrimary : Theme::TextMuted, ButtonX + 10.0f * Scale, RowY + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.66f * Scale);
		if (bCanBuy)
		{
			AddHitBox(FVector2D(ButtonX, RowY), FVector2D(ButtonWidth, ButtonHeight), HitBoxName, true, Priority);
		}
	};

	DrawShopRow(Y + 50.0f * Scale, ViewModel.CostLabel, ViewModel.ButtonLabel, ViewModel.bCanBuyGearRoll, ViewModel.GearRollHitBoxName, 84);
	DrawShopRow(Y + 86.0f * Scale, ViewModel.ProtectionScrollCostLabel, ViewModel.ProtectionScrollButtonLabel, ViewModel.bCanBuyProtectionScroll, ViewModel.ProtectionScrollHitBoxName, 85);
	DrawShopRow(Y + 122.0f * Scale, ViewModel.ResetCubeCostLabel, ViewModel.ResetCubeButtonLabel, ViewModel.bCanBuyResetCube, ViewModel.ResetCubeHitBoxName, 86);
	DrawShopRow(Y + 158.0f * Scale, ViewModel.RankCubeCostLabel, ViewModel.RankCubeButtonLabel, ViewModel.bCanBuyRankCube, ViewModel.RankCubeHitBoxName, 87);
	DrawText(ViewModel.StatusLabel.ToString(), ViewModel.bCanBuyGearRoll ? Theme::AccentBlue : Theme::AccentRed, X + Padding, Y + 198.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.72f * Scale);

	if (ViewModel.bHasLastResult)
	{
		const FLinearColor ResultColor = ViewModel.bLastResultError ? Theme::AccentRed : RarityToColor(ViewModel.LastResultRarity);
		DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.86f), X + Padding, Y + 224.0f * Scale, PanelWidth - Padding * 2.0f, 34.0f * Scale);
		DrawRect(ResultColor, X + Padding, Y + 224.0f * Scale, 4.0f * Scale, 34.0f * Scale);
		DrawText(ViewModel.LastResultLabel.ToString(), ResultColor, X + Padding + 12.0f * Scale, Y + 232.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.74f * Scale);
	}

	RefreshMouseInteraction();
}

void AIdleHUD::TryBuyGearRoll()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	BindPlayerInventory();
	if (!IdleGameInstance || !PlayerInventory)
	{
		return;
	}

	IdleGameInstance->TryBuyGearRoll(PlayerInventory);
	RefreshEquipmentSummary();
	RefreshMouseInteraction();
}

void AIdleHUD::TryBuyProtectionScroll()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (IdleGameInstance)
	{
		IdleGameInstance->TryBuyProtectionScroll();
	}
	RefreshMouseInteraction();
}

void AIdleHUD::TryBuyResetCube()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (IdleGameInstance)
	{
		IdleGameInstance->TryBuyResetCube();
	}
	RefreshMouseInteraction();
}

void AIdleHUD::TryBuyRankCube()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (IdleGameInstance)
	{
		IdleGameInstance->TryBuyRankCube();
	}
	RefreshMouseInteraction();
}

void AIdleHUD::DrawConsumablePanel()
{
	using namespace IdleProject::UI;

	if (!Canvas)
	{
		return;
	}
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance || !IdleGameInstance->GetBuffService())
	{
		return;
	}

	const FIdleHUDConsumablePanelViewModel ViewModel = BuildConsumablePanelViewModel(
		*IdleGameInstance->GetBuffService(),
		UIdleGameInstance::GetCurrentUnixSeconds());

	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelWidth = FMath::Clamp(Canvas->SizeX * 0.20f, 300.0f * Scale, 360.0f * Scale);
	const float RowHeight = 42.0f * Scale;
	const float RowGap = 5.0f * Scale;
	const float ActiveHeight = 34.0f * Scale;
	const float Padding = 12.0f * Scale;
	const float PanelHeight = 54.0f * Scale + ActiveHeight + Padding + ViewModel.Rows.Num() * RowHeight + FMath::Max(0, ViewModel.Rows.Num() - 1) * RowGap + Padding;
	const float X = Canvas->SizeX - PanelWidth - 28.0f * Scale;
	const float Y = 360.0f * Scale;
	const float Border = 2.0f * Scale;

	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.91f), X, Y, PanelWidth, PanelHeight);
	DrawRect(Theme::AccentBlue, X, Y, PanelWidth, Border);
	DrawRect(Theme::AccentBlue, X, Y + PanelHeight - Border, PanelWidth, Border);
	DrawRect(Theme::AccentBlue, X, Y, Border, PanelHeight);
	DrawRect(Theme::AccentBlue, X + PanelWidth - Border, Y, Border, PanelHeight);

	DrawText(ViewModel.Title.ToString(), Theme::TextPrimary, X + Padding, Y + 12.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 0.88f * Scale);
	DrawText(ViewModel.ActiveBuffTitle.ToString(), Theme::AccentGold, X + Padding, Y + 42.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.70f * Scale);

	const float ActiveY = Y + 66.0f * Scale;
	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.82f), X + Padding, ActiveY, PanelWidth - Padding * 2.0f, ActiveHeight);
	if (ViewModel.ActiveBuffRows.Num() == 0)
	{
		DrawText(ViewModel.EmptyActiveBuffLabel.ToString(), Theme::TextMuted, X + Padding + 8.0f * Scale, ActiveY + 8.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.68f * Scale);
	}
	else
	{
		const FIdleHUDConsumableRowViewModel& ActiveRow = ViewModel.ActiveBuffRows[0];
		DrawRect(Theme::AccentGold, X + Padding, ActiveY, 4.0f * Scale, ActiveHeight);
		DrawText(ActiveRow.NameLabel.ToString(), Theme::TextPrimary, X + Padding + 12.0f * Scale, ActiveY + 7.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.68f * Scale);
		DrawText(ActiveRow.RemainingLabel.ToString(), Theme::AccentGold, X + PanelWidth - Padding - 48.0f * Scale, ActiveY + 7.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.68f * Scale);
	}

	float RowY = ActiveY + ActiveHeight + Padding;
	for (const FIdleHUDConsumableRowViewModel& Row : ViewModel.Rows)
	{
		DrawConsumableRow(Row, X + Padding, RowY, PanelWidth - Padding * 2.0f, RowHeight);
		RowY += RowHeight + RowGap;
	}

	RefreshMouseInteraction();
}

void AIdleHUD::DrawConsumableRow(const FIdleHUDConsumableRowViewModel& Row, float X, float Y, float Width, float Height)
{
	using namespace IdleProject::UI;

	const float Scale = FMath::Clamp(Canvas ? Canvas->SizeY / 1080.0f : 1.0f, 1.0f, 2.0f);
	const float ButtonWidth = 54.0f * Scale;
	const float ButtonHeight = 24.0f * Scale;
	const float ButtonX = X + Width - ButtonWidth - 6.0f * Scale;
	const float ButtonY = Y + (Height - ButtonHeight) * 0.5f;
	const FLinearColor AccentColor = Row.bActive ? Theme::AccentGold : (Row.bCanUse ? Theme::AccentBlue : Theme::TextMuted);

	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.84f), X, Y, Width, Height);
	DrawRect(AccentColor, X, Y, 4.0f * Scale, Height);
	DrawText(Row.NameLabel.ToString(), Theme::TextPrimary, X + 12.0f * Scale, Y + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.66f * Scale);
	DrawText(Row.EffectLabel.ToString(), Theme::TextMuted, X + 12.0f * Scale, Y + 23.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.56f * Scale);
	DrawText(Row.CountLabel.ToString(), Theme::TextMuted, ButtonX - 62.0f * Scale, Y + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.58f * Scale);
	if (Row.bActive)
	{
		DrawText(Row.RemainingLabel.ToString(), Theme::AccentGold, ButtonX - 62.0f * Scale, Y + 23.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.56f * Scale);
	}

	DrawRect(Row.bCanUse ? Theme::AccentGold : Theme::BgPanel.CopyWithNewOpacity(0.94f), ButtonX, ButtonY, ButtonWidth, ButtonHeight);
	DrawText(Row.ActionLabel.ToString(), Row.bCanUse ? Theme::BgPrimary : Theme::TextMuted, ButtonX + 12.0f * Scale, ButtonY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.58f * Scale);
	if (Row.bCanUse)
	{
		AddHitBox(FVector2D(ButtonX, ButtonY), FVector2D(ButtonWidth, ButtonHeight), Row.UseHitBoxName, true, 89);
	}
}

void AIdleHUD::TryUseConsumableFromHitBox(FName BoxName)
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance)
	{
		return;
	}

	EConsumableType Type = EConsumableType::AttackTonic;
	EConsumableGrade Grade = EConsumableGrade::Standard;
	if (!ParseConsumableUseHitBoxName(BoxName, Type, Grade))
	{
		return;
	}

	if (IdleGameInstance->TryUseConsumable(Type, Grade))
	{
		RefreshMouseInteraction();
	}
}

void AIdleHUD::DrawLeaderboardPanel()
{
	using namespace IdleProject::UI;

	if (!Canvas)
	{
		return;
	}
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	const ULeaderboardService* LeaderboardService = IdleGameInstance ? IdleGameInstance->GetLeaderboardService() : nullptr;
	if (!IdleGameInstance || !LeaderboardService)
	{
		return;
	}

	const USeasonService* SeasonService = IdleGameInstance->GetSeasonService();
	const int32 SeasonId = SeasonService ? SeasonService->GetSeasonId() : 0;
	const bool bOffline = IdleGameInstance->GetCloudSyncState() == ECloudSyncState::Offline;
	const bool bShowLoading = bLeaderboardLoading;
	const FString WeekId = UQuestService::GetCurrentUtcWeekString();
	const FIdleHUDLeaderboardPanelViewModel ViewModel = BuildLeaderboardPanelViewModel(*LeaderboardService, SelectedLeaderboardKind, SeasonId, WeekId, bShowLoading, bOffline);
	bLeaderboardLoading = false;

	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelWidth = FMath::Clamp(Canvas->SizeX * 0.22f, 340.0f * Scale, 440.0f * Scale);
	const float RowHeight = 30.0f * Scale;
	const float RowGap = 4.0f * Scale;
	const float Padding = 12.0f * Scale;
	const int32 VisibleRows = FMath::Min(ViewModel.Rows.Num(), 8);
	const float PanelHeight = 118.0f * Scale + RowHeight + Padding + VisibleRows * RowHeight + FMath::Max(0, VisibleRows - 1) * RowGap + Padding;
	const float X = Canvas->SizeX - PanelWidth - 28.0f * Scale;
	const float Y = 688.0f * Scale;
	const float Border = 2.0f * Scale;

	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.91f), X, Y, PanelWidth, PanelHeight);
	DrawRect(Theme::AccentGold, X, Y, PanelWidth, Border);
	DrawRect(Theme::AccentGold, X, Y + PanelHeight - Border, PanelWidth, Border);
	DrawRect(Theme::AccentGold, X, Y, Border, PanelHeight);
	DrawRect(Theme::AccentGold, X + PanelWidth - Border, Y, Border, PanelHeight);

	DrawText(ViewModel.Title.ToString(), Theme::TextPrimary, X + Padding, Y + 10.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 0.88f * Scale);
	DrawText(ViewModel.SeasonLabel.ToString(), Theme::TextMuted, X + Padding, Y + 36.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.64f * Scale);

	const float ButtonHeight = 24.0f * Scale;

	// 새로고침 버튼: 상단 우측(타이틀 줄)으로 이동해 탭 3개 공간 확보
	const float RefreshWidth = 74.0f * Scale;
	const float RefreshX = X + PanelWidth - Padding - RefreshWidth;
	const float RefreshY = Y + 10.0f * Scale;
	DrawRect(Theme::AccentBlue, RefreshX, RefreshY, RefreshWidth, ButtonHeight);
	DrawText(ViewModel.RefreshLabel.ToString(), Theme::BgPrimary, RefreshX + 12.0f * Scale, RefreshY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.60f * Scale);
	AddHitBox(FVector2D(RefreshX, RefreshY), FVector2D(RefreshWidth, ButtonHeight), LeaderboardRefreshHitBoxName, true, 92);

	// 탭 3개(전투력/환생/주간 보스)를 한 줄에 균등 배치
	const float TabGap = 6.0f * Scale;
	const float TabRowWidth = PanelWidth - Padding * 2.0f;
	const float TabWidth = (TabRowWidth - TabGap * 2.0f) / 3.0f;
	const float TabY = Y + 58.0f * Scale;
	const float PowerTabX = X + Padding;
	const float RebirthTabX = PowerTabX + TabWidth + TabGap;
	const float WeeklyTabX = RebirthTabX + TabWidth + TabGap;
	const FLinearColor PowerColor = ViewModel.ActiveKind == ELeaderboardKind::Power ? Theme::AccentGold : Theme::BgPrimary;
	const FLinearColor RebirthColor = ViewModel.ActiveKind == ELeaderboardKind::Rebirth ? Theme::AccentGold : Theme::BgPrimary;
	const FLinearColor WeeklyColor = ViewModel.ActiveKind == ELeaderboardKind::WeeklyDamage ? Theme::AccentGold : Theme::BgPrimary;
	DrawRect(PowerColor, PowerTabX, TabY, TabWidth, ButtonHeight);
	DrawRect(RebirthColor, RebirthTabX, TabY, TabWidth, ButtonHeight);
	DrawRect(WeeklyColor, WeeklyTabX, TabY, TabWidth, ButtonHeight);
	DrawText(ViewModel.PowerTabLabel.ToString(), ViewModel.ActiveKind == ELeaderboardKind::Power ? Theme::BgPrimary : Theme::TextMuted, PowerTabX + 8.0f * Scale, TabY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.56f * Scale);
	DrawText(ViewModel.RebirthTabLabel.ToString(), ViewModel.ActiveKind == ELeaderboardKind::Rebirth ? Theme::BgPrimary : Theme::TextMuted, RebirthTabX + 8.0f * Scale, TabY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.56f * Scale);
	DrawText(ViewModel.WeeklyTabLabel.ToString(), ViewModel.ActiveKind == ELeaderboardKind::WeeklyDamage ? Theme::BgPrimary : Theme::TextMuted, WeeklyTabX + 8.0f * Scale, TabY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.56f * Scale);
	AddHitBox(FVector2D(PowerTabX, TabY), FVector2D(TabWidth, ButtonHeight), LeaderboardPowerTabHitBoxName, true, 90);
	AddHitBox(FVector2D(RebirthTabX, TabY), FVector2D(TabWidth, ButtonHeight), LeaderboardRebirthTabHitBoxName, true, 91);
	AddHitBox(FVector2D(WeeklyTabX, TabY), FVector2D(TabWidth, ButtonHeight), LeaderboardWeeklyTabHitBoxName, true, 93);

	// 주간 보스 탭일 때 현재 주 라벨 표시(시즌 라벨 옆)
	if (ViewModel.ActiveKind == ELeaderboardKind::WeeklyDamage)
	{
		DrawText(ViewModel.WeekLabel.ToString(), Theme::AccentGold, X + Padding + 140.0f * Scale, Y + 36.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);
	}

	const FText StateLabel = ViewModel.bLoading ? ViewModel.LoadingLabel : (ViewModel.bOffline ? ViewModel.OfflineLabel : FText::GetEmpty());
	if (!StateLabel.IsEmpty())
	{
		DrawText(StateLabel.ToString(), ViewModel.bOffline ? Theme::AccentRed : Theme::AccentBlue, X + Padding, Y + 88.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);
	}

	const float MyRowY = Y + 108.0f * Scale;
	DrawText(ViewModel.MyRankTitle.ToString(), Theme::AccentGold, X + Padding, MyRowY, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);
	DrawLeaderboardRow(ViewModel.MyEntry, X + Padding + 74.0f * Scale, MyRowY - 6.0f * Scale, PanelWidth - Padding * 2.0f - 74.0f * Scale, RowHeight);

	float RowY = MyRowY + RowHeight + Padding;
	if (VisibleRows == 0)
	{
		DrawText(ViewModel.EmptyLabel.ToString(), Theme::TextMuted, X + Padding, RowY + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.66f * Scale);
	}
	for (int32 Index = 0; Index < VisibleRows; ++Index)
	{
		DrawLeaderboardRow(ViewModel.Rows[Index], X + Padding, RowY, PanelWidth - Padding * 2.0f, RowHeight);
		RowY += RowHeight + RowGap;
	}

	RefreshMouseInteraction();
}

void AIdleHUD::DrawLeaderboardRow(const FIdleHUDLeaderboardRowViewModel& Row, float X, float Y, float Width, float Height)
{
	using namespace IdleProject::UI;

	const float Scale = Height / 30.0f;
	const FLinearColor AccentColor = Row.bSelf ? Theme::AccentGold : Theme::TextMuted.CopyWithNewOpacity(0.52f);
	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(Row.bSelf ? 0.96f : 0.84f), X, Y, Width, Height);
	DrawRect(AccentColor, X, Y, 4.0f * Scale, Height);
	DrawText(Row.RankLabel.ToString(), Row.bSelf ? Theme::AccentGold : Theme::TextPrimary, X + 10.0f * Scale, Y + 7.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);
	DrawText(Row.CharacterLabel.ToString(), Theme::TextMuted, X + 58.0f * Scale, Y + 7.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.58f * Scale);
	DrawText(Row.ScoreLabel.ToString(), Row.bSelf ? Theme::AccentGold : Theme::AccentBlue, X + Width - 104.0f * Scale, Y + 7.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.56f * Scale);
}

void AIdleHUD::SelectLeaderboardKind(ELeaderboardKind Kind)
{
	SelectedLeaderboardKind = Kind;
	RefreshSelectedLeaderboard();
}

void AIdleHUD::RefreshSelectedLeaderboard()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (IdleGameInstance)
	{
		bLeaderboardLoading = true;
		IdleGameInstance->RefreshLeaderboard(SelectedLeaderboardKind);
	}
	RefreshMouseInteraction();
}

void AIdleHUD::DrawRunePanel()
{
	using namespace IdleProject::UI;

	if (!Canvas)
	{
		return;
	}
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance)
	{
		return;
	}

	const URuneService* RuneService = IdleGameInstance->GetRuneService();
	if (!RuneService)
	{
		return;
	}

	const UStageService* StageService = IdleGameInstance->GetStageService();
	const int32 GlobalStageIndex = StageService ? StageService->GetGlobalStageIndex() : 0;
	const FIdleHUDRuneViewModel ViewModel = BuildRuneViewModel(
		*RuneService,
		IdleGameInstance->GetRuneEssence(),
		IdleGameInstance->GetGold(),
		GlobalStageIndex,
		SelectedRuneOwnedIndex,
		RuneTransferTargetOwnedIndex);

	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelWidth = FMath::Clamp(Canvas->SizeX * 0.26f, 380.0f * Scale, 500.0f * Scale);
	const float HeaderHeight = 62.0f * Scale;
	const float CraftHeight = 32.0f * Scale;
	const float SetHeaderHeight = 22.0f * Scale;
	const float SetRowHeight = 34.0f * Scale;
	const float SetGap = 4.0f * Scale;
	const float SlotHeight = 24.0f * Scale;
	const float RowHeight = 48.0f * Scale;
	const float RowGap = 6.0f * Scale;
	const float Padding = 14.0f * Scale;
	const int32 VisibleRows = FMath::Min(ViewModel.OwnedRows.Num(), 2);
	const float SetSectionHeight = SetHeaderHeight + ViewModel.SetRows.Num() * SetRowHeight + FMath::Max(0, ViewModel.SetRows.Num() - 1) * SetGap + 8.0f * Scale;
	const float SlotSectionHeight = (FRuneFormula::RuneSlotCount * SlotHeight) + 16.0f * Scale;
	const float ActionSectionHeight = 124.0f * Scale;
	const float PanelHeight = HeaderHeight + CraftHeight + SetSectionHeight + SlotSectionHeight + FMath::Max(1, VisibleRows) * RowHeight + FMath::Max(0, VisibleRows - 1) * RowGap + ActionSectionHeight + Padding;
	const float X = Canvas->SizeX - PanelWidth - 28.0f * Scale;
	const float Y = 282.0f * Scale;
	const float Border = 2.0f * Scale;
	const FLinearColor StateColor = ViewModel.bCanBuyRuneRoll ? Theme::RarityMythicStart : Theme::TextMuted.CopyWithNewOpacity(0.68f);

	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.91f), X, Y, PanelWidth, PanelHeight);
	DrawRect(StateColor, X, Y, PanelWidth, Border);
	DrawRect(Theme::RarityMythicEnd, X, Y + PanelHeight - Border, PanelWidth, Border);
	DrawRect(StateColor, X, Y, Border, PanelHeight);
	DrawRect(Theme::RarityMythicEnd, X + PanelWidth - Border, Y, Border, PanelHeight);

	DrawText(ViewModel.Title.ToString(), Theme::TextPrimary, X + Padding, Y + 9.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 0.86f * Scale);
	DrawText(ViewModel.EssenceLabel.ToString(), Theme::RarityMythicEnd, X + Padding, Y + 35.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.72f * Scale);
	DrawText(ViewModel.ShopCostLabel.ToString(), ViewModel.bCanBuyRuneRoll ? Theme::TextPrimary : Theme::AccentRed, X + 144.0f * Scale, Y + 35.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.68f * Scale);

	const float ShopButtonWidth = 92.0f * Scale;
	const float ShopButtonHeight = 25.0f * Scale;
	const float ShopButtonX = X + PanelWidth - Padding - ShopButtonWidth;
	const float ShopButtonY = Y + 32.0f * Scale;
	DrawRect(ViewModel.bCanBuyRuneRoll ? Theme::RarityMythicStart : Theme::BgPrimary.CopyWithNewOpacity(0.94f), ShopButtonX, ShopButtonY, ShopButtonWidth, ShopButtonHeight);
	DrawText(ViewModel.ShopButtonLabel.ToString(), ViewModel.bCanBuyRuneRoll ? Theme::BgPrimary : Theme::TextMuted, ShopButtonX + 10.0f * Scale, ShopButtonY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.66f * Scale);
	if (ViewModel.bCanBuyRuneRoll)
	{
		AddHitBox(FVector2D(ShopButtonX, ShopButtonY), FVector2D(ShopButtonWidth, ShopButtonHeight), ViewModel.ShopHitBoxName, true, 85);
	}

	const float CraftY = Y + HeaderHeight;
	const float CraftButtonWidth = 118.0f * Scale;
	const float CraftButtonHeight = 24.0f * Scale;
	const float CraftButtonX = X + PanelWidth - Padding - CraftButtonWidth;
	const float CraftButtonY = CraftY + 4.0f * Scale;
	DrawText(ViewModel.ClassCraftCostLabel.ToString(), ViewModel.bCanCraftClassRune ? Theme::TextPrimary : Theme::AccentRed, X + Padding, CraftY + 8.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.64f * Scale);
	DrawRect(ViewModel.bCanCraftClassRune ? Theme::AccentBlue : Theme::BgPrimary.CopyWithNewOpacity(0.94f), CraftButtonX, CraftButtonY, CraftButtonWidth, CraftButtonHeight);
	DrawText(ViewModel.ClassCraftButtonLabel.ToString(), ViewModel.bCanCraftClassRune ? Theme::BgPrimary : Theme::TextMuted, CraftButtonX + 8.0f * Scale, CraftButtonY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.58f * Scale);
	if (ViewModel.bCanCraftClassRune)
	{
		AddHitBox(FVector2D(CraftButtonX, CraftButtonY), FVector2D(CraftButtonWidth, CraftButtonHeight), ViewModel.ClassCraftHitBoxName, true, 86);
	}

	float RowY = Y + HeaderHeight + CraftHeight;
	DrawText(ViewModel.SetTitle.ToString(), Theme::TextPrimary, X + Padding, RowY + 3.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.68f * Scale);
	RowY += SetHeaderHeight;
	for (const FIdleHUDRuneSetRowViewModel& SetRow : ViewModel.SetRows)
	{
		DrawRuneSetRow(SetRow, X + Padding, RowY, PanelWidth - Padding * 2.0f, SetRowHeight);
		RowY += SetRowHeight + SetGap;
	}

	RowY += 4.0f * Scale;
	for (int32 SlotIndex = 0; SlotIndex < ViewModel.Slots.Num(); ++SlotIndex)
	{
		if (SlotIndex == FClassRuneFormula::ClassRuneSlotIndex)
		{
			RowY += 8.0f * Scale;
		}
		DrawRuneSlot(ViewModel.Slots[SlotIndex], X + Padding, RowY, PanelWidth - Padding * 2.0f, SlotHeight);
		RowY += SlotHeight;
	}

	RowY += 8.0f * Scale;
	if (ViewModel.OwnedRows.IsEmpty())
	{
		DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.90f), X + Padding, RowY, PanelWidth - Padding * 2.0f, RowHeight);
		DrawText(ViewModel.EmptyOwnedLabel.ToString(), Theme::TextMuted, X + Padding + 10.0f * Scale, RowY + 17.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.74f * Scale);
	}
	else
	{
		for (int32 Index = 0; Index < VisibleRows; ++Index)
		{
			DrawRuneOwnedRow(ViewModel.OwnedRows[Index], X + Padding, RowY, PanelWidth - Padding * 2.0f, RowHeight);
			RowY += RowHeight + RowGap;
		}
	}

	RowY += 6.0f * Scale;
	DrawRuneActionSection(ViewModel.Action, X + Padding, RowY, PanelWidth - Padding * 2.0f, ActionSectionHeight - 6.0f * Scale);

	RefreshMouseInteraction();
}

void AIdleHUD::DrawRuneCodexPanel()
{
	using namespace IdleProject::UI;

	if (!Canvas)
	{
		return;
	}
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	const URuneService* RuneService = IdleGameInstance ? IdleGameInstance->GetRuneService() : nullptr;
	if (!RuneService)
	{
		return;
	}

	const FIdleHUDRuneCodexViewModel ViewModel = BuildRuneCodexViewModel(*RuneService);
	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelWidth = 328.0f * Scale;
	const float Padding = 12.0f * Scale;
	const float HeaderHeight = 76.0f * Scale;
	const float CellSize = 23.0f * Scale;
	const float CellGap = 4.0f * Scale;
	const float RowLabelWidth = 72.0f * Scale;
	const float FooterHeight = 116.0f * Scale;
	const float PanelHeight = HeaderHeight + 6.0f * CellSize + 5.0f * CellGap + FooterHeight + Padding;
	const float RunePanelWidth = FMath::Clamp(Canvas->SizeX * 0.26f, 380.0f * Scale, 500.0f * Scale);
	const float RunePanelX = Canvas->SizeX - RunePanelWidth - 28.0f * Scale;
	const float X = FMath::Max(28.0f * Scale, RunePanelX - PanelWidth - 12.0f * Scale);
	const float Y = 282.0f * Scale;
	const float Border = 2.0f * Scale;

	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.90f), X, Y, PanelWidth, PanelHeight);
	DrawRect(Theme::AccentBlue, X, Y, PanelWidth, Border);
	DrawRect(Theme::RarityMythicStart, X, Y + PanelHeight - Border, PanelWidth, Border);
	DrawRect(Theme::AccentBlue, X, Y, Border, PanelHeight);
	DrawRect(Theme::RarityMythicStart, X + PanelWidth - Border, Y, Border, PanelHeight);

	DrawText(ViewModel.Title.ToString(), Theme::TextPrimary, X + Padding, Y + 10.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 0.82f * Scale);
	DrawText(ViewModel.ProgressLabel.ToString(), Theme::AccentGold, X + Padding, Y + 36.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.68f * Scale);
	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.92f), X + Padding, Y + 60.0f * Scale, PanelWidth - Padding * 2.0f, 5.0f * Scale);
	DrawRect(Theme::AccentGold, X + Padding, Y + 60.0f * Scale, (PanelWidth - Padding * 2.0f) * ViewModel.ProgressRatio, 5.0f * Scale);

	const float GridX = X + Padding + RowLabelWidth;
	float RowY = Y + HeaderHeight;
	for (const FIdleHUDRuneCodexRowViewModel& Row : ViewModel.Rows)
	{
		DrawText(Row.RarityLabel.ToString(), Row.bComplete ? Row.AccentColor : Theme::TextMuted, X + Padding, RowY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.56f * Scale);
		DrawText(Row.bComplete ? TEXT("*") : TEXT(""), Row.AccentColor, X + Padding + 56.0f * Scale, RowY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.58f * Scale);
		RowY += CellSize + CellGap;
	}

	for (const FIdleHUDRuneCodexCellViewModel& Cell : ViewModel.Cells)
	{
		const float CellX = GridX + Cell.ColumnIndex * (CellSize + CellGap);
		const float CellY = Y + HeaderHeight + Cell.RowIndex * (CellSize + CellGap);
		DrawRuneCodexCell(Cell, CellX, CellY, CellSize);
	}

	const float FooterY = Y + HeaderHeight + 6.0f * CellSize + 5.0f * CellGap + 10.0f * Scale;
	DrawText(ViewModel.CoreCategoryLabel.ToString(), ViewModel.bCoreCategoryComplete ? Theme::AccentGold : Theme::TextMuted, X + Padding, FooterY, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);
	DrawText(ViewModel.UtilCategoryLabel.ToString(), ViewModel.bUtilCategoryComplete ? Theme::AccentGold : Theme::TextMuted, X + 128.0f * Scale, FooterY, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);
	DrawText(ViewModel.CoreBonusLabel.ToString(), Theme::AccentBlue, X + Padding, FooterY + 20.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);
	DrawText(ViewModel.UtilCapLabel.ToString(), Theme::RarityMythicEnd, X + 128.0f * Scale, FooterY + 20.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);

	for (int32 Index = 0; Index < ViewModel.Rows.Num(); ++Index)
	{
		const FIdleHUDRuneCodexRowViewModel& Row = ViewModel.Rows[Index];
		DrawText(Row.BonusLabel.ToString(), Row.bComplete ? Row.AccentColor : Theme::TextMuted, X + Padding, FooterY + (40.0f + Index * 12.0f) * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.48f * Scale);
	}
}

void AIdleHUD::DrawRuneCodexCell(const FIdleHUDRuneCodexCellViewModel& Cell, float X, float Y, float Size)
{
	using namespace IdleProject::UI;

	const float Inner = FMath::Max(2.0f, Size - 4.0f);
	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.92f), X, Y, Size, Size);
	DrawRect(Cell.AccentColor.CopyWithNewOpacity(Cell.bUnlocked ? 0.95f : 0.34f), X + 2.0f, Y + 2.0f, Inner, Inner);
	if (Cell.bCoreType)
	{
		DrawRect(Theme::AccentBlue.CopyWithNewOpacity(Cell.bUnlocked ? 0.88f : 0.30f), X + 4.0f, Y + Size - 6.0f, Size - 8.0f, 2.0f);
	}
	else
	{
		DrawRect(Theme::AccentGold.CopyWithNewOpacity(Cell.bUnlocked ? 0.88f : 0.30f), X + 4.0f, Y + Size - 6.0f, Size - 8.0f, 2.0f);
	}
}

void AIdleHUD::DrawRuneSetRow(const FIdleHUDRuneSetRowViewModel& Row, float X, float Y, float Width, float Height)
{
	using namespace IdleProject::UI;

	const float Scale = Height / 34.0f;
	const FLinearColor StateColor = Row.bSixSetActive
		? Theme::RarityMythicEnd
		: (Row.bFourSetActive ? Theme::AccentGold : (Row.bTwoSetActive ? Theme::AccentBlue : Theme::TextMuted.CopyWithNewOpacity(0.50f)));

	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.88f), X, Y, Width, Height);
	DrawRect(StateColor, X, Y, 4.0f * Scale, Height);

	DrawText(Row.CountLabel.ToString(), Row.bActive ? Theme::TextPrimary : Theme::TextMuted, X + 10.0f * Scale, Y + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);
	DrawText(Row.TierLabel.ToString(), StateColor, X + 112.0f * Scale, Y + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.60f * Scale);
	DrawText(Row.NextTierLabel.ToString(), Theme::TextMuted, X + 216.0f * Scale, Y + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.56f * Scale);
	DrawText(Row.BonusLabel.ToString(), Row.bActive ? Theme::RarityMythicEnd : Theme::TextMuted, X + 10.0f * Scale, Y + 19.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.54f * Scale);
}

void AIdleHUD::DrawRuneSlot(const FIdleHUDRuneSlotViewModel& Slot, float X, float Y, float Width, float Height)
{
	using namespace IdleProject::UI;

	const float Scale = Height / 30.0f;
	const FLinearColor StateColor = Slot.bEquipped ? Slot.RarityColor : (Slot.bCanEquipSelected ? Theme::AccentBlue : Theme::TextMuted.CopyWithNewOpacity(0.50f));
	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.88f), X, Y, Width, Height - 3.0f * Scale);
	DrawRect(StateColor, X, Y, 4.0f * Scale, Height - 3.0f * Scale);

	DrawText(Slot.SlotLabel.ToString(), Theme::TextMuted, X + 10.0f * Scale, Y + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);
	DrawText(Slot.TypeLabel.ToString(), Slot.bEquipped ? Theme::TextPrimary : Theme::TextMuted, X + 54.0f * Scale, Y + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.64f * Scale);
	DrawText(Slot.RarityLabel.ToString(), Slot.bEquipped ? Slot.RarityColor : Theme::TextMuted, X + 146.0f * Scale, Y + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);
	DrawText(Slot.LevelLabel.ToString(), Theme::TextMuted, X + 216.0f * Scale, Y + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);
	DrawText(Slot.ValueLabel.ToString(), Slot.bEquipped ? Theme::RarityMythicEnd : Theme::TextMuted, X + 264.0f * Scale, Y + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);

	const bool bActionEnabled = Slot.bCanEquipSelected || Slot.bCanUnequip;
	const float ButtonWidth = 58.0f * Scale;
	const float ButtonHeight = 21.0f * Scale;
	const float ButtonX = X + Width - ButtonWidth - 7.0f * Scale;
	const float ButtonY = Y + 3.0f * Scale;
	DrawRect(bActionEnabled ? StateColor : Theme::BgPanel, ButtonX, ButtonY, ButtonWidth, ButtonHeight);
	DrawText(Slot.ActionLabel.ToString(), bActionEnabled ? Theme::BgPrimary : Theme::TextMuted, ButtonX + 8.0f * Scale, ButtonY + 4.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.60f * Scale);
	if (bActionEnabled)
	{
		AddHitBox(FVector2D(ButtonX, ButtonY), FVector2D(ButtonWidth, ButtonHeight), Slot.ActionHitBoxName, true, 87);
	}
}

void AIdleHUD::DrawRuneOwnedRow(const FIdleHUDRuneOwnedRowViewModel& Row, float X, float Y, float Width, float Height)
{
	using namespace IdleProject::UI;

	const float Scale = Height / 54.0f;
	const FLinearColor StateColor = Row.bSelected ? Theme::RarityMythicEnd : (Row.bEquipped ? Row.RarityColor : Theme::TextMuted.CopyWithNewOpacity(0.58f));
	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.90f), X, Y, Width, Height);
	DrawRect(StateColor, X, Y, 4.0f * Scale, Height);

	DrawText(Row.TypeLabel.ToString(), Row.bSelected ? Theme::RarityMythicEnd : Theme::TextPrimary, X + 10.0f * Scale, Y + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.70f * Scale);
	DrawText(Row.RarityLabel.ToString(), Row.RarityColor, X + 94.0f * Scale, Y + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.66f * Scale);
	DrawText(Row.LevelLabel.ToString(), Theme::TextMuted, X + 158.0f * Scale, Y + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.64f * Scale);
	DrawText(Row.ValueLabel.ToString(), Theme::RarityMythicEnd, X + 208.0f * Scale, Y + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.64f * Scale);
	DrawText(Row.EnhanceCostLabel.ToString(), Row.bCanEnhance ? Theme::TextMuted : Theme::AccentRed, X + 10.0f * Scale, Y + 25.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.58f * Scale);
	DrawText(Row.EnhancePreviewLabel.ToString(), Theme::TextMuted, X + 10.0f * Scale, Y + 39.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.56f * Scale);

	const float ButtonWidth = 58.0f * Scale;
	const float ButtonHeight = 20.0f * Scale;
	const float SelectX = X + Width - ButtonWidth * 3.0f - 20.0f * Scale;
	const float EnhanceX = X + Width - ButtonWidth * 2.0f - 14.0f * Scale;
	const float DisenchantX = X + Width - ButtonWidth - 8.0f * Scale;
	const float ButtonY = Y + 17.0f * Scale;

	DrawRect(Row.bSelected ? Theme::RarityMythicEnd : Theme::BgPanel, SelectX, ButtonY, ButtonWidth, ButtonHeight);
	DrawText(Row.SelectActionLabel.ToString(), Row.bSelected ? Theme::BgPrimary : Theme::TextMuted, SelectX + 8.0f * Scale, ButtonY + 4.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.56f * Scale);
	AddHitBox(FVector2D(SelectX, ButtonY), FVector2D(ButtonWidth, ButtonHeight), Row.SelectHitBoxName, true, 88);

	DrawRect(Row.bCanEnhance ? Theme::AccentGold : Theme::BgPanel, EnhanceX, ButtonY, ButtonWidth, ButtonHeight);
	DrawText(Row.EnhanceActionLabel.ToString(), Row.bCanEnhance ? Theme::BgPrimary : Theme::TextMuted, EnhanceX + 8.0f * Scale, ButtonY + 4.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.56f * Scale);
	if (Row.bCanEnhance)
	{
		AddHitBox(FVector2D(EnhanceX, ButtonY), FVector2D(ButtonWidth, ButtonHeight), Row.EnhanceHitBoxName, true, 89);
	}

	DrawRect(Row.bCanDisenchant ? Theme::AccentRed : Theme::BgPanel, DisenchantX, ButtonY, ButtonWidth, ButtonHeight);
	DrawText(Row.DisenchantActionLabel.ToString(), Row.bCanDisenchant ? Theme::BgPrimary : Theme::TextMuted, DisenchantX + 8.0f * Scale, ButtonY + 4.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.56f * Scale);
	if (Row.bCanDisenchant)
	{
		AddHitBox(FVector2D(DisenchantX, ButtonY), FVector2D(ButtonWidth, ButtonHeight), Row.DisenchantHitBoxName, true, 90);
	}
}

void AIdleHUD::DrawRuneActionSection(const FIdleHUDRuneActionViewModel& Action, float X, float Y, float Width, float Height)
{
	using namespace IdleProject::UI;

	const float Scale = FMath::Clamp(Height / 118.0f, 0.5f, 2.0f);
	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.92f), X, Y, Width, Height);
	DrawRect(Theme::RarityMythicEnd, X, Y, 4.0f * Scale, Height);

	DrawText(Action.TitleLabel.ToString(), Theme::TextPrimary, X + 10.0f * Scale, Y + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.66f * Scale);

	if (!Action.bHasSelection)
	{
		DrawText(Action.EmptyLabel.ToString(), Theme::TextMuted, X + 10.0f * Scale, Y + 26.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);
		return;
	}

	const float ButtonWidth = 70.0f * Scale;
	const float ButtonHeight = 19.0f * Scale;
	const float ButtonX = X + Width - ButtonWidth - 8.0f * Scale;

	// 세트 리롤
	const float RerollY = Y + 24.0f * Scale;
	DrawText(Action.CurrentSetLabel.ToString(), Theme::TextMuted, X + 10.0f * Scale, RerollY, GEngine ? GEngine->GetSmallFont() : nullptr, 0.58f * Scale);
	DrawText(Action.RerollCostLabel.ToString(), Action.bCanReroll ? Theme::TextMuted : Theme::AccentRed, X + 10.0f * Scale, RerollY + 12.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.54f * Scale);
	DrawRect(Action.bCanReroll ? Theme::AccentBlue : Theme::BgPanel, ButtonX, RerollY, ButtonWidth, ButtonHeight);
	DrawText(Action.RerollActionLabel.ToString(), Action.bCanReroll ? Theme::BgPrimary : Theme::TextMuted, ButtonX + 8.0f * Scale, RerollY + 3.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.54f * Scale);
	if (Action.bCanReroll)
	{
		AddHitBox(FVector2D(ButtonX, RerollY), FVector2D(ButtonWidth, ButtonHeight), Action.RerollHitBoxName, true, 91);
	}

	// 등급 상승 시도
	const float UpgradeY = RerollY + 26.0f * Scale;
	DrawText(Action.UpgradeInfoLabel.ToString(), Action.bIsMythic ? Theme::RarityMythicEnd : (Action.bCanUpgrade ? Theme::TextMuted : Theme::AccentRed), X + 10.0f * Scale, UpgradeY + 4.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.54f * Scale);
	DrawRect(Action.bCanUpgrade ? Theme::AccentGold : Theme::BgPanel, ButtonX, UpgradeY, ButtonWidth, ButtonHeight);
	DrawText(Action.UpgradeActionLabel.ToString(), Action.bCanUpgrade ? Theme::BgPrimary : Theme::TextMuted, ButtonX + 8.0f * Scale, UpgradeY + 3.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.54f * Scale);
	if (Action.bCanUpgrade)
	{
		AddHitBox(FVector2D(ButtonX, UpgradeY), FVector2D(ButtonWidth, ButtonHeight), Action.UpgradeHitBoxName, true, 92);
	}

	// 강화 전송
	const float TransferY = UpgradeY + 26.0f * Scale;
	DrawText(Action.TransferTargetLabel.ToString(), Theme::TextMuted, X + 10.0f * Scale, TransferY, GEngine ? GEngine->GetSmallFont() : nullptr, 0.54f * Scale);
	DrawText(Action.TransferCostLabel.ToString(), Action.bCanTransfer ? Theme::TextMuted : Theme::AccentRed, X + 10.0f * Scale, TransferY + 12.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.54f * Scale);

	const float CycleWidth = 56.0f * Scale;
	const float CycleX = ButtonX - CycleWidth - 6.0f * Scale;
	DrawRect(Action.bCanCycleTarget ? Theme::BgPanel : Theme::BgPanel.CopyWithNewOpacity(0.6f), CycleX, TransferY, CycleWidth, ButtonHeight);
	DrawText(Action.TransferCycleLabel.ToString(), Action.bCanCycleTarget ? Theme::TextPrimary : Theme::TextMuted, CycleX + 6.0f * Scale, TransferY + 3.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.52f * Scale);
	if (Action.bCanCycleTarget)
	{
		AddHitBox(FVector2D(CycleX, TransferY), FVector2D(CycleWidth, ButtonHeight), Action.TransferCycleHitBoxName, true, 93);
	}

	DrawRect(Action.bCanTransfer ? Theme::RarityMythicStart : Theme::BgPanel, ButtonX, TransferY, ButtonWidth, ButtonHeight);
	DrawText(Action.TransferActionLabel.ToString(), Action.bCanTransfer ? Theme::BgPrimary : Theme::TextMuted, ButtonX + 8.0f * Scale, TransferY + 3.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.54f * Scale);
	if (Action.bCanTransfer)
	{
		AddHitBox(FVector2D(ButtonX, TransferY), FVector2D(ButtonWidth, ButtonHeight), Action.TransferHitBoxName, true, 94);
	}

	// 피드백(성공/실패) — 일정 시간 후 사라짐.
	const UWorld* World = GetWorld();
	const float FeedbackElapsed = World ? World->GetTimeSeconds() - RuneActionFeedbackStartTime : 1000.0f;
	if (!RuneActionFeedbackLabel.IsEmpty() && FeedbackElapsed <= 3.0f)
	{
		DrawText(RuneActionFeedbackLabel.ToString(), bRuneActionFeedbackSuccess ? Theme::AccentGold : Theme::AccentRed, X + 10.0f * Scale, TransferY + 24.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.54f * Scale);
	}
}

void AIdleHUD::SelectRuneFromHitBox(FName BoxName)
{
	FString RawIndex = BoxName.ToString();
	RawIndex.RightChopInline(RuneSelectHitBoxPrefix.Len());
	SelectedRuneOwnedIndex = FCString::Atoi(*RawIndex);
	RefreshMouseInteraction();
}

void AIdleHUD::EquipSelectedRuneFromHitBox(FName BoxName)
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance || SelectedRuneOwnedIndex == INDEX_NONE)
	{
		return;
	}

	FString RawSlot = BoxName.ToString();
	RawSlot.RightChopInline(RuneEquipHitBoxPrefix.Len());
	IdleGameInstance->TryEquipRune(FCString::Atoi(*RawSlot), SelectedRuneOwnedIndex);
	RefreshMouseInteraction();
}

void AIdleHUD::UnequipRuneFromHitBox(FName BoxName)
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance)
	{
		return;
	}

	FString RawSlot = BoxName.ToString();
	RawSlot.RightChopInline(RuneUnequipHitBoxPrefix.Len());
	IdleGameInstance->UnequipRune(FCString::Atoi(*RawSlot));
	RefreshMouseInteraction();
}

void AIdleHUD::TryEnhanceRuneFromHitBox(FName BoxName)
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance)
	{
		return;
	}

	FString RawIndex = BoxName.ToString();
	RawIndex.RightChopInline(RuneEnhanceHitBoxPrefix.Len());
	IdleGameInstance->TryEnhanceRune(FCString::Atoi(*RawIndex));
	RefreshMouseInteraction();
}

void AIdleHUD::TryDisenchantRuneFromHitBox(FName BoxName)
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance)
	{
		return;
	}

	FString RawIndex = BoxName.ToString();
	RawIndex.RightChopInline(RuneDisenchantHitBoxPrefix.Len());
	const int32 OwnedIndex = FCString::Atoi(*RawIndex);
	if (IdleGameInstance->TryDisenchantRune(OwnedIndex) && SelectedRuneOwnedIndex == OwnedIndex)
	{
		SelectedRuneOwnedIndex = INDEX_NONE;
	}
	RefreshMouseInteraction();
}

void AIdleHUD::SetRuneActionFeedback(const TCHAR* Key, bool bSuccess)
{
	RuneActionFeedbackLabel = IdleProject::Localization::Text(TEXT("Rune"), Key);
	bRuneActionFeedbackSuccess = bSuccess;
	if (const UWorld* World = GetWorld())
	{
		RuneActionFeedbackStartTime = World->GetTimeSeconds();
	}
}

void AIdleHUD::TryRerollRuneSetAction()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance || SelectedRuneOwnedIndex == INDEX_NONE)
	{
		return;
	}

	const bool bSuccess = IdleGameInstance->TryRerollRuneSet(SelectedRuneOwnedIndex);
	SetRuneActionFeedback(bSuccess ? TEXT("RUNE_FEEDBACK_REROLL_DONE") : TEXT("RUNE_FEEDBACK_FAILED"), bSuccess);
	RefreshMouseInteraction();
}

void AIdleHUD::TryUpgradeRuneRarityAction()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance || SelectedRuneOwnedIndex == INDEX_NONE)
	{
		return;
	}

	bool bUpgradeSucceeded = false;
	const bool bAttempted = IdleGameInstance->TryUpgradeRuneRarity(SelectedRuneOwnedIndex, bUpgradeSucceeded);
	if (!bAttempted)
	{
		SetRuneActionFeedback(TEXT("RUNE_FEEDBACK_FAILED"), false);
	}
	else
	{
		SetRuneActionFeedback(bUpgradeSucceeded ? TEXT("RUNE_FEEDBACK_UPGRADE_SUCCESS") : TEXT("RUNE_FEEDBACK_UPGRADE_FAIL"), bUpgradeSucceeded);
	}
	RefreshMouseInteraction();
}

int32 AIdleHUD::ResolveRuneTransferTargetIndex() const
{
	if (!IdleGameInstance)
	{
		return INDEX_NONE;
	}
	const URuneService* RuneService = IdleGameInstance->GetRuneService();
	if (!RuneService || SelectedRuneOwnedIndex == INDEX_NONE)
	{
		return INDEX_NONE;
	}

	const TArray<FRuneInstance>& OwnedRunes = RuneService->GetOwnedRunes();
	if (!OwnedRunes.IsValidIndex(SelectedRuneOwnedIndex))
	{
		return INDEX_NONE;
	}
	if (OwnedRunes.IsValidIndex(RuneTransferTargetOwnedIndex) && RuneTransferTargetOwnedIndex != SelectedRuneOwnedIndex)
	{
		return RuneTransferTargetOwnedIndex;
	}
	for (int32 Candidate = 0; Candidate < OwnedRunes.Num(); ++Candidate)
	{
		if (Candidate != SelectedRuneOwnedIndex)
		{
			return Candidate;
		}
	}
	return INDEX_NONE;
}

void AIdleHUD::CycleRuneTransferTarget()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance || SelectedRuneOwnedIndex == INDEX_NONE)
	{
		return;
	}
	const URuneService* RuneService = IdleGameInstance->GetRuneService();
	if (!RuneService)
	{
		return;
	}

	const TArray<FRuneInstance>& OwnedRunes = RuneService->GetOwnedRunes();
	if (!OwnedRunes.IsValidIndex(SelectedRuneOwnedIndex))
	{
		return;
	}

	const int32 Current = ResolveRuneTransferTargetIndex();
	const int32 Count = OwnedRunes.Num();
	const int32 Start = OwnedRunes.IsValidIndex(Current) ? Current : SelectedRuneOwnedIndex;
	for (int32 Step = 1; Step <= Count; ++Step)
	{
		const int32 Candidate = (Start + Step) % Count;
		if (Candidate != SelectedRuneOwnedIndex)
		{
			RuneTransferTargetOwnedIndex = Candidate;
			break;
		}
	}
	RefreshMouseInteraction();
}

void AIdleHUD::TryTransferRuneEnhancementAction()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance || SelectedRuneOwnedIndex == INDEX_NONE)
	{
		return;
	}

	const int32 TargetIndex = ResolveRuneTransferTargetIndex();
	if (TargetIndex == INDEX_NONE)
	{
		SetRuneActionFeedback(TEXT("RUNE_FEEDBACK_FAILED"), false);
		RefreshMouseInteraction();
		return;
	}

	const bool bSuccess = IdleGameInstance->TransferRuneEnhancement(SelectedRuneOwnedIndex, TargetIndex);
	if (bSuccess)
	{
		// source(SelectedRuneOwnedIndex) 삭제 → 인덱스 재정렬. 선택/타깃 참조 초기화.
		SelectedRuneOwnedIndex = INDEX_NONE;
		RuneTransferTargetOwnedIndex = INDEX_NONE;
	}
	SetRuneActionFeedback(bSuccess ? TEXT("RUNE_FEEDBACK_TRANSFER_DONE") : TEXT("RUNE_FEEDBACK_FAILED"), bSuccess);
	RefreshMouseInteraction();
}

void AIdleHUD::TryBuyRuneRoll()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance)
	{
		return;
	}

	const int32 PreviousOwnedCount = IdleGameInstance->GetRuneService() ? IdleGameInstance->GetRuneService()->GetOwnedRunes().Num() : 0;
	if (IdleGameInstance->TryBuyRuneRoll())
	{
		SelectedRuneOwnedIndex = PreviousOwnedCount;
	}
	RefreshMouseInteraction();
}

void AIdleHUD::TryCraftClassRune()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance)
	{
		return;
	}

	const int32 PreviousOwnedCount = IdleGameInstance->GetRuneService() ? IdleGameInstance->GetRuneService()->GetOwnedRunes().Num() : 0;
	if (IdleGameInstance->TryCraftClassRune(EItemRarity::Common))
	{
		SelectedRuneOwnedIndex = PreviousOwnedCount;
	}
	RefreshMouseInteraction();
}

void AIdleHUD::DrawEnhancePanel()
{
	using namespace IdleProject::UI;

	if (!Canvas)
	{
		return;
	}
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	BindPlayerInventory();
	if (!IdleGameInstance || !PlayerInventory)
	{
		return;
	}

	const FIdleHUDEnhancePanelViewModel ViewModel = BuildEnhancePanelViewModel(
		*PlayerInventory,
		IdleGameInstance->GetGold(),
		IdleGameInstance->GetProtectionScrolls(),
		EnhanceFeedbackLabel,
		bEnhanceFeedbackSuccess);

	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelWidth = FMath::Clamp(Canvas->SizeX * 0.25f, 360.0f * Scale, 460.0f * Scale);
	const float HeaderHeight = 52.0f * Scale;
	const float RowHeight = 34.0f * Scale;
	const float RowGap = 6.0f * Scale;
	const float Padding = 14.0f * Scale;
	const float FeedbackHeight = ViewModel.FeedbackLabel.IsEmpty() ? 0.0f : 24.0f * Scale;
	const float PanelHeight = HeaderHeight + ViewModel.Rows.Num() * RowHeight + FMath::Max(0, ViewModel.Rows.Num() - 1) * RowGap + Padding + FeedbackHeight;
	const float X = Canvas->SizeX - PanelWidth - 28.0f * Scale;
	const float Y = 628.0f * Scale;
	const float Border = 2.0f * Scale;

	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.91f), X, Y, PanelWidth, PanelHeight);
	DrawRect(Theme::AccentGold, X, Y, PanelWidth, Border);
	DrawRect(Theme::AccentGold, X, Y + PanelHeight - Border, PanelWidth, Border);
	DrawRect(Theme::AccentGold, X, Y, Border, PanelHeight);
	DrawRect(Theme::AccentGold, X + PanelWidth - Border, Y, Border, PanelHeight);

	DrawText(ViewModel.Title.ToString(), Theme::TextPrimary, X + Padding, Y + 12.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 0.92f * Scale);
	DrawText(ViewModel.GoldLabel.ToString(), Theme::AccentGold, X + PanelWidth - 114.0f * Scale, Y + 16.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.78f * Scale);
	DrawText(ViewModel.ProtectionLabel.ToString(), Theme::AccentBlue, X + Padding, Y + 34.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.66f * Scale);

	float RowY = Y + HeaderHeight;
	for (const FIdleHUDEnhanceSlotViewModel& Row : ViewModel.Rows)
	{
		DrawEnhanceSlotRow(Row, X + Padding, RowY, PanelWidth - Padding * 2.0f, RowHeight);
		RowY += RowHeight + RowGap;
	}

	if (!ViewModel.FeedbackLabel.IsEmpty())
	{
		const FLinearColor FeedbackColor = ViewModel.bFeedbackSuccess ? Theme::AccentGold : Theme::AccentRed;
		DrawText(ViewModel.FeedbackLabel.ToString(), FeedbackColor, X + Padding, RowY + 4.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.82f * Scale);
	}

	RefreshMouseInteraction();
}

void AIdleHUD::DrawEnhanceSlotRow(const FIdleHUDEnhanceSlotViewModel& Row, float X, float Y, float Width, float Height)
{
	using namespace IdleProject::UI;

	const float Scale = Height / 40.0f;
	const FLinearColor StateColor = Row.bCanEnhance
		? Theme::AccentGold
		: (Row.bEquipped ? Theme::AccentBlue.CopyWithNewOpacity(0.72f) : Theme::TextMuted.CopyWithNewOpacity(0.46f));
	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.90f), X, Y, Width, Height);
	DrawRect(StateColor, X, Y, 4.0f * Scale, Height);

	DrawText(Row.SlotLabel.ToString(), Row.bCanEnhance ? Theme::AccentGold : Theme::TextPrimary, X + 10.0f * Scale, Y + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.72f * Scale);
	DrawText(Row.ItemName.ToString(), Row.bEquipped ? Theme::TextPrimary : Theme::TextMuted, X + 78.0f * Scale, Y + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.70f * Scale);
	DrawText(Row.LevelLabel.ToString(), Row.bMaxLevel ? Theme::AccentGold : Theme::TextMuted, X + 10.0f * Scale, Y + 22.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.64f * Scale);
	DrawText(Row.RarityLabel.ToString(), Row.bEquipped ? Row.RarityColor : Theme::TextMuted, X + 78.0f * Scale, Y + 22.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.64f * Scale);
	DrawText(Row.CostLabel.ToString(), Row.bGoldEnough || Row.bMaxLevel ? Theme::TextMuted : Theme::AccentRed, X + 132.0f * Scale, Y + 22.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.64f * Scale);
	DrawText(Row.SuccessRateLabel.ToString(), Row.bCanEnhance ? Theme::AccentBlue : Theme::TextMuted, X + 206.0f * Scale, Y + 22.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.60f * Scale);
	DrawText(Row.RiskLabel.ToString(), Row.bRiskLevel ? Theme::AccentRed : Theme::TextMuted, X + 276.0f * Scale, Y + 22.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.56f * Scale);
	DrawText(Row.FailStreakLabel.ToString(), Row.bRiskLevel ? Theme::AccentGold : Theme::TextMuted, X + 206.0f * Scale, Y + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.52f * Scale);

	const float ButtonWidth = 64.0f * Scale;
	const float ButtonHeight = 26.0f * Scale;
	const float ProtectButtonWidth = 50.0f * Scale;
	const float ButtonX = X + Width - ButtonWidth - 8.0f * Scale;
	const float ProtectButtonX = ButtonX - ProtectButtonWidth - 5.0f * Scale;
	const float ButtonY = Y + 7.0f * Scale;
	if (Row.bRiskLevel)
	{
		DrawRect(Row.bCanUseProtection ? Theme::AccentBlue : Theme::BgPanel, ProtectButtonX, ButtonY, ProtectButtonWidth, ButtonHeight);
		DrawText(Row.ProtectionButtonLabel.ToString(), Row.bCanUseProtection ? Theme::BgPrimary : Theme::TextMuted, ProtectButtonX + 7.0f * Scale, ButtonY + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.56f * Scale);
		if (Row.bCanUseProtection)
		{
			AddHitBox(FVector2D(ProtectButtonX, ButtonY), FVector2D(ProtectButtonWidth, ButtonHeight), MakeEnhanceProtectHitBoxName(Row.Slot), true, 87);
		}
	}
	DrawRect(Row.bCanEnhance ? Theme::AccentGold : Theme::BgPanel, ButtonX, ButtonY, ButtonWidth, ButtonHeight);
	DrawText(Row.bCanEnhance ? Row.ButtonLabel.ToString() : Row.StatusLabel.ToString(), Row.bCanEnhance ? Theme::BgPrimary : Theme::TextMuted, ButtonX + 8.0f * Scale, ButtonY + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.66f * Scale);
	if (Row.bCanEnhance)
	{
		AddHitBox(FVector2D(ButtonX, ButtonY), FVector2D(ButtonWidth, ButtonHeight), MakeEnhanceSlotHitBoxName(Row.Slot), true, 86);
	}
}

void AIdleHUD::TryEnhanceFromHitBox(FName BoxName)
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	BindPlayerInventory();
	if (!IdleGameInstance || !PlayerInventory)
	{
		return;
	}

	const bool bUseProtection = BoxName.ToString().StartsWith(EnhanceProtectHitBoxPrefix);
	FString RawSlot = BoxName.ToString();
	RawSlot.RightChopInline(bUseProtection ? EnhanceProtectHitBoxPrefix.Len() : EnhanceSlotHitBoxPrefix.Len());
	const int32 SlotValue = FCString::Atoi(*RawSlot);
	if (SlotValue < static_cast<int32>(EItemSlot::Weapon) || SlotValue > static_cast<int32>(EItemSlot::Accessory))
	{
		return;
	}

	IdleGameInstance->TryEnhanceEquipped(static_cast<EItemSlot>(SlotValue), bUseProtection, PlayerInventory);
	RefreshEquipmentSummary();
	RefreshMouseInteraction();
}

void AIdleHUD::DrawPotentialPanel()
{
	using namespace IdleProject::UI;

	if (!Canvas)
	{
		return;
	}
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	BindPlayerInventory();
	if (!IdleGameInstance || !PlayerInventory)
	{
		return;
	}

	const FIdleHUDPotentialPanelViewModel ViewModel = BuildPotentialPanelViewModel(
		*PlayerInventory,
		IdleGameInstance->GetResetCubes(),
		IdleGameInstance->GetRankCubes());
	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelWidth = FMath::Clamp(Canvas->SizeX * 0.25f, 360.0f * Scale, 460.0f * Scale);
	const float HeaderHeight = 52.0f * Scale;
	const float RowHeight = 30.0f * Scale;
	const float RowGap = 6.0f * Scale;
	const float Padding = 14.0f * Scale;
	const float PanelHeight = HeaderHeight + ViewModel.Rows.Num() * RowHeight + FMath::Max(0, ViewModel.Rows.Num() - 1) * RowGap + Padding;
	const float X = Canvas->SizeX - PanelWidth - 28.0f * Scale;
	const float Y = 238.0f * Scale;
	const float Border = 2.0f * Scale;

	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.91f), X, Y, PanelWidth, PanelHeight);
	DrawRect(Theme::AccentBlue, X, Y, PanelWidth, Border);
	DrawRect(Theme::AccentBlue, X, Y + PanelHeight - Border, PanelWidth, Border);
	DrawRect(Theme::AccentBlue, X, Y, Border, PanelHeight);
	DrawRect(Theme::AccentBlue, X + PanelWidth - Border, Y, Border, PanelHeight);

	DrawText(ViewModel.Title.ToString(), Theme::TextPrimary, X + Padding, Y + 12.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 0.86f * Scale);
	DrawText(ViewModel.ResetCubeLabel.ToString(), Theme::AccentBlue, X + Padding, Y + 34.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);
	DrawText(ViewModel.RankCubeLabel.ToString(), Theme::AccentGold, X + PanelWidth - 114.0f * Scale, Y + 34.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);

	float RowY = Y + HeaderHeight;
	for (const FIdleHUDPotentialSlotViewModel& Row : ViewModel.Rows)
	{
		DrawPotentialSlotRow(Row, X + Padding, RowY, PanelWidth - Padding * 2.0f, RowHeight);
		RowY += RowHeight + RowGap;
	}
}

void AIdleHUD::DrawPotentialSlotRow(const FIdleHUDPotentialSlotViewModel& Row, float X, float Y, float Width, float Height)
{
	using namespace IdleProject::UI;

	const float Scale = Height / 30.0f;
	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.90f), X, Y, Width, Height);
	DrawRect(Row.bHasPotential ? Row.GradeColor : Theme::TextMuted.CopyWithNewOpacity(0.46f), X, Y, 4.0f * Scale, Height);
	DrawText(Row.SlotLabel.ToString(), Row.bHasPotential ? Row.GradeColor : Theme::TextPrimary, X + 10.0f * Scale, Y + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.64f * Scale);
	DrawText(Row.ItemName.ToString(), Row.bEquipped ? Theme::TextPrimary : Theme::TextMuted, X + 72.0f * Scale, Y + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);
	DrawText(Row.GradeLabel.ToString(), Row.bHasPotential ? Row.GradeColor : Theme::TextMuted, X + 10.0f * Scale, Y + 22.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.56f * Scale);
	DrawText(Row.LineSummaryLabel.ToString(), Row.bHasPotential ? Theme::TextMuted : Theme::TextMuted.CopyWithNewOpacity(0.66f), X + 82.0f * Scale, Y + 22.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.50f * Scale);

	const float ButtonHeight = 22.0f * Scale;
	const float ButtonY = Y + 10.0f * Scale;
	const float ButtonWidth = 42.0f * Scale;
	const float LockWidth = 46.0f * Scale;
	const float LockX = X + Width - LockWidth - 6.0f * Scale;
	const float RankX = LockX - ButtonWidth - 5.0f * Scale;
	const float ResetX = RankX - ButtonWidth - 5.0f * Scale;
	DrawRect(Row.bCanResetPotential ? Theme::AccentBlue : Theme::BgPanel, ResetX, ButtonY, ButtonWidth, ButtonHeight);
	DrawText(Row.ResetActionLabel.ToString(), Row.bCanResetPotential ? Theme::BgPrimary : Theme::TextMuted, ResetX + 6.0f * Scale, ButtonY + 4.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.50f * Scale);
	if (Row.bCanResetPotential)
	{
		AddHitBox(FVector2D(ResetX, ButtonY), FVector2D(ButtonWidth, ButtonHeight), Row.ResetHitBoxName, true, 91);
	}

	DrawRect(Row.bCanRankPotential ? Theme::AccentGold : Theme::BgPanel, RankX, ButtonY, ButtonWidth, ButtonHeight);
	DrawText(Row.RankActionLabel.ToString(), Row.bCanRankPotential ? Theme::BgPrimary : Theme::TextMuted, RankX + 6.0f * Scale, ButtonY + 4.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.50f * Scale);
	if (Row.bCanRankPotential)
	{
		AddHitBox(FVector2D(RankX, ButtonY), FVector2D(ButtonWidth, ButtonHeight), Row.RankHitBoxName, true, 92);
	}

	DrawRect(Row.bEquipped ? (Row.bLocked ? Theme::AccentRed : Theme::BgPanel) : Theme::BgPanel, LockX, ButtonY, LockWidth, ButtonHeight);
	DrawText(Row.LockActionLabel.ToString(), Row.bEquipped ? Theme::TextPrimary : Theme::TextMuted, LockX + 6.0f * Scale, ButtonY + 4.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.50f * Scale);
	if (Row.bEquipped)
	{
		AddHitBox(FVector2D(LockX, ButtonY), FVector2D(LockWidth, ButtonHeight), Row.LockHitBoxName, true, 93);
	}
}

void AIdleHUD::TryRerollPotentialFromHitBox(FName BoxName, EPotentialCubeType CubeType)
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	BindPlayerInventory();
	if (!IdleGameInstance || !PlayerInventory)
	{
		return;
	}

	FString RawSlot = BoxName.ToString();
	RawSlot.RightChopInline(CubeType == EPotentialCubeType::Reset ? PotentialResetHitBoxPrefix.Len() : PotentialRankHitBoxPrefix.Len());
	const int32 SlotValue = FCString::Atoi(*RawSlot);
	if (SlotValue < static_cast<int32>(EItemSlot::Weapon) || SlotValue > static_cast<int32>(EItemSlot::Accessory))
	{
		return;
	}

	IdleGameInstance->TryRerollPotential(static_cast<EItemSlot>(SlotValue), CubeType, PlayerInventory);
	RefreshEquipmentSummary();
	RefreshMouseInteraction();
}

void AIdleHUD::ToggleItemLockFromHitBox(FName BoxName)
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	BindPlayerInventory();
	if (!IdleGameInstance || !PlayerInventory)
	{
		return;
	}

	FString RawSlot = BoxName.ToString();
	RawSlot.RightChopInline(PotentialLockHitBoxPrefix.Len());
	const int32 SlotValue = FCString::Atoi(*RawSlot);
	if (SlotValue < static_cast<int32>(EItemSlot::Weapon) || SlotValue > static_cast<int32>(EItemSlot::Accessory))
	{
		return;
	}

	const EItemSlot Slot = static_cast<EItemSlot>(SlotValue);
	const FItemInstance* Item = PlayerInventory->GetEquippedItem(Slot);
	if (Item)
	{
		IdleGameInstance->SetItemLocked(Slot, !Item->bLocked);
		RefreshEquipmentSummary();
		RefreshMouseInteraction();
	}
}

void AIdleHUD::DrawStatAllocationPanel()
{
	using namespace IdleProject::UI;

	if (!Canvas)
	{
		return;
	}
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	const AIdleCharacter* IdleCharacter = ResolvePlayerCharacter();
	if (!IdleGameInstance || !IdleCharacter)
	{
		return;
	}

	const FPrimaryStats BaseStats = FStatFormulas::DefaultPrimaryStats(IdleCharacter->GetClassId(), IdleGameInstance->GetCharacterLevel());
	const FIdleHUDStatPanelViewModel ViewModel = BuildStatPanelViewModel(
		BaseStats,
		IdleGameInstance->GetAllocatedPrimaryStats(),
		IdleGameInstance->GetAvailableStatPoints());

	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelWidth = 300.0f * Scale;
	const float HeaderHeight = 46.0f * Scale;
	const float RowHeight = 30.0f * Scale;
	const float RowGap = 5.0f * Scale;
	const float Padding = 12.0f * Scale;
	const float FooterHeight = 36.0f * Scale;
	const float PanelHeight = HeaderHeight + ViewModel.Rows.Num() * RowHeight + FMath::Max(0, ViewModel.Rows.Num() - 1) * RowGap + FooterHeight + Padding;
	const float X = Canvas->SizeX - 700.0f * Scale;
	const float Y = 92.0f * Scale;
	const float Border = 2.0f * Scale;

	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.91f), X, Y, PanelWidth, PanelHeight);
	DrawRect(Theme::AccentBlue, X, Y, PanelWidth, Border);
	DrawRect(Theme::AccentBlue, X, Y + PanelHeight - Border, PanelWidth, Border);
	DrawRect(Theme::AccentBlue, X, Y, Border, PanelHeight);
	DrawRect(Theme::AccentBlue, X + PanelWidth - Border, Y, Border, PanelHeight);

	DrawText(ViewModel.Title.ToString(), Theme::TextPrimary, X + Padding, Y + 10.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 0.88f * Scale);
	DrawText(ViewModel.AvailableLabel.ToString(), ViewModel.AvailablePoints > 0 ? Theme::AccentGold : Theme::TextMuted, X + PanelWidth - 118.0f * Scale, Y + 14.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.72f * Scale);

	float RowY = Y + HeaderHeight;
	for (const FIdleHUDStatRowViewModel& Row : ViewModel.Rows)
	{
		DrawStatAllocationRow(Row, X + Padding, RowY, PanelWidth - Padding * 2.0f, RowHeight);
		RowY += RowHeight + RowGap;
	}

	const float ButtonWidth = 82.0f * Scale;
	const float ButtonHeight = 26.0f * Scale;
	const float ButtonX = X + PanelWidth - Padding - ButtonWidth;
	const float ButtonY = Y + PanelHeight - Padding - ButtonHeight;
	DrawRect(ViewModel.bCanReset ? Theme::AccentRed.CopyWithNewOpacity(0.86f) : Theme::BgPrimary.CopyWithNewOpacity(0.94f), ButtonX, ButtonY, ButtonWidth, ButtonHeight);
	DrawText(ViewModel.ResetLabel.ToString(), ViewModel.bCanReset ? Theme::TextPrimary : Theme::TextMuted, ButtonX + 12.0f * Scale, ButtonY + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.68f * Scale);
	if (ViewModel.bCanReset)
	{
		AddHitBox(FVector2D(ButtonX, ButtonY), FVector2D(ButtonWidth, ButtonHeight), ViewModel.ResetHitBoxName, true, 88);
	}

	RefreshMouseInteraction();
}

void AIdleHUD::DrawStatAllocationRow(const FIdleHUDStatRowViewModel& Row, float X, float Y, float Width, float Height)
{
	using namespace IdleProject::UI;

	const float Scale = Height / 30.0f;
	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.90f), X, Y, Width, Height);
	DrawRect(Row.AllocatedValue > 0 ? Theme::AccentGold : Theme::TextMuted.CopyWithNewOpacity(0.42f), X, Y, 4.0f * Scale, Height);
	DrawText(Row.StatLabel.ToString(), Row.AllocatedValue > 0 ? Theme::AccentGold : Theme::TextPrimary, X + 10.0f * Scale, Y + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.74f * Scale);
	DrawText(Row.ValueLabel.ToString(), Theme::TextMuted, X + 58.0f * Scale, Y + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.68f * Scale);

	const float ButtonSize = 22.0f * Scale;
	const float ButtonX = X + Width - ButtonSize - 7.0f * Scale;
	const float ButtonY = Y + 4.0f * Scale;
	DrawRect(Row.bCanAllocate ? Theme::AccentGold : Theme::BgPanel, ButtonX, ButtonY, ButtonSize, ButtonSize);
	DrawText(TEXT("+"), Row.bCanAllocate ? Theme::BgPrimary : Theme::TextMuted, ButtonX + 6.0f * Scale, ButtonY + 2.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 0.72f * Scale);
	if (Row.bCanAllocate)
	{
		AddHitBox(FVector2D(ButtonX, ButtonY), FVector2D(ButtonSize, ButtonSize), Row.AllocationHitBoxName, true, 88);
	}
}

void AIdleHUD::AllocateStatFromHitBox(FName BoxName)
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance)
	{
		return;
	}

	FString RawStat = BoxName.ToString();
	RawStat.RightChopInline(StatAllocationHitBoxPrefix.Len());
	const int32 StatValue = FCString::Atoi(*RawStat);
	if (StatValue < static_cast<int32>(EPrimaryStat::Str) || StatValue > static_cast<int32>(EPrimaryStat::Luk))
	{
		return;
	}

	IdleGameInstance->AllocateStatPoint(static_cast<EPrimaryStat>(StatValue));
	RefreshMouseInteraction();
}

void AIdleHUD::ResetStatAllocation()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (IdleGameInstance)
	{
		IdleGameInstance->ResetStatPoints();
	}
	RefreshMouseInteraction();
}

void AIdleHUD::DrawStatInfoPanel()
{
	using namespace IdleProject::UI;

	if (!Canvas)
	{
		return;
	}
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	const AIdleCharacter* IdleCharacter = ResolvePlayerCharacter();
	if (!IdleGameInstance || !IdleCharacter)
	{
		return;
	}

	const FIdleHUDStatInfoViewModel ViewModel = BuildStatInfoViewModel(
		IdleCharacter->GetCurrentPrimaryStats(),
		IdleCharacter->GetCurrentDerivedStats(),
		IdleCharacter->GetCurrentLevel(),
		IdleCharacter->GetClassId(),
		IdleGameInstance->GetRebirthCount(),
		IdleCharacter->GetCombatPower());

	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float ToggleX = Canvas->SizeX - 392.0f * Scale;
	const float ToggleY = 92.0f * Scale;
	DrawStatInfoToggle(ViewModel, ToggleX, ToggleY, Scale);
	if (!bStatInfoVisible)
	{
		RefreshMouseInteraction();
		return;
	}

	const float PanelWidth = 364.0f * Scale;
	const float HeaderHeight = 84.0f * Scale;
	const float RowHeight = 25.0f * Scale;
	const float RowGap = 4.0f * Scale;
	const float Padding = 14.0f * Scale;
	const int32 RowCount = FMath::Max(ViewModel.PrimaryRows.Num(), ViewModel.DerivedRows.Num());
	const float PanelHeight = HeaderHeight + RowCount * RowHeight + FMath::Max(0, RowCount - 1) * RowGap + Padding;
	const float X = Canvas->SizeX - PanelWidth - 28.0f * Scale;
	const float Y = ToggleY + 38.0f * Scale;
	const float Border = 2.0f * Scale;
	const float ColumnGap = 12.0f * Scale;
	const float ColumnWidth = (PanelWidth - Padding * 2.0f - ColumnGap) * 0.5f;

	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.93f), X, Y, PanelWidth, PanelHeight);
	DrawRect(Theme::AccentGold, X, Y, PanelWidth, Border);
	DrawRect(Theme::AccentGold, X, Y + PanelHeight - Border, PanelWidth, Border);
	DrawRect(Theme::AccentGold, X, Y, Border, PanelHeight);
	DrawRect(Theme::AccentGold, X + PanelWidth - Border, Y, Border, PanelHeight);

	DrawText(ViewModel.Title.ToString(), Theme::TextPrimary, X + Padding, Y + 10.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 0.88f * Scale);
	DrawText(ViewModel.HeaderLabel.ToString(), Theme::TextMuted, X + Padding, Y + 38.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.72f * Scale);
	DrawText(ViewModel.CombatPowerLabel.ToString(), Theme::AccentGold, X + Padding, Y + 60.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.82f * Scale);

	float RowY = Y + HeaderHeight;
	for (int32 Index = 0; Index < RowCount; ++Index)
	{
		if (ViewModel.PrimaryRows.IsValidIndex(Index))
		{
			DrawStatInfoRow(ViewModel.PrimaryRows[Index], X + Padding, RowY, ColumnWidth, RowHeight, Theme::AccentBlue);
		}
		if (ViewModel.DerivedRows.IsValidIndex(Index))
		{
			DrawStatInfoRow(ViewModel.DerivedRows[Index], X + Padding + ColumnWidth + ColumnGap, RowY, ColumnWidth, RowHeight, Theme::AccentGold);
		}
		RowY += RowHeight + RowGap;
	}
	RefreshMouseInteraction();
}

void AIdleHUD::DrawStatInfoToggle(const FIdleHUDStatInfoViewModel& ViewModel, float X, float Y, float Scale)
{
	using namespace IdleProject::UI;

	const float Width = 126.0f * Scale;
	const float Height = 30.0f * Scale;
	DrawRect(bStatInfoVisible ? Theme::AccentGold : Theme::BgPanel.CopyWithNewOpacity(0.94f), X, Y, Width, Height);
	DrawText(ViewModel.ToggleLabel.ToString(), bStatInfoVisible ? Theme::BgPrimary : Theme::TextPrimary, X + 13.0f * Scale, Y + 7.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.72f * Scale);
	AddHitBox(FVector2D(X, Y), FVector2D(Width, Height), ViewModel.ToggleHitBoxName, true, 88);
}

void AIdleHUD::DrawStatInfoRow(const FIdleHUDStatInfoRowViewModel& Row, float X, float Y, float Width, float Height, const FLinearColor& AccentColor)
{
	using namespace IdleProject::UI;

	const float Scale = Height / 25.0f;
	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.88f), X, Y, Width, Height);
	DrawRect(AccentColor.CopyWithNewOpacity(0.85f), X, Y, 3.0f * Scale, Height);
	DrawText(Row.StatLabel.ToString(), Theme::TextMuted, X + 9.0f * Scale, Y + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.66f * Scale);
	DrawText(Row.ValueLabel.ToString(), Theme::TextPrimary, X + Width - 52.0f * Scale, Y + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.66f * Scale);
}

void AIdleHUD::ToggleStatInfoPanel()
{
	bStatInfoVisible = !bStatInfoVisible;
	RefreshMouseInteraction();
}

void AIdleHUD::DrawSkillHud(const USkillComponent& SkillComponent, float Now)
{
	using namespace IdleProject::UI;

	if (!Canvas)
	{
		return;
	}

	const TArray<FIdleHUDSkillSlotViewModel> Slots = IdleProject::UI::BuildSkillSlotViewModels(SkillComponent, Now);
	if (Slots.Num() <= 0)
	{
		return;
	}

	const float HudScale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float SlotWidth = 160.0f * HudScale;
	const float SlotHeight = 60.0f * HudScale;
	const float SlotGap = 10.0f * HudScale;
	const float TotalWidth = Slots.Num() * SlotWidth + (Slots.Num() - 1) * SlotGap;
	const float StartX = FMath::Max(24.0f * HudScale, (Canvas->SizeX - TotalWidth) * 0.5f);
	const float SlotY = FMath::Max(96.0f * HudScale, Canvas->SizeY - 116.0f * HudScale);
	const int32 AvailableSkillPoints = Slots[0].AvailableSkillPoints;

	for (int32 Index = 0; Index < Slots.Num(); ++Index)
	{
		DrawSkillSlot(Slots[Index], Index, StartX + Index * (SlotWidth + SlotGap), SlotY, SlotWidth, SlotHeight);
	}

	DrawUltimateGauge(
		IdleProject::UI::BuildUltimateViewModel(SkillComponent),
		StartX,
		SlotY - 34.0f * HudScale,
		TotalWidth,
		22.0f * HudScale);

	const FString PointsLabel = FormatLocalizedUI(TEXT("SKILL_POINTS_FORMAT"), [AvailableSkillPoints](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Points"), FText::AsNumber(AvailableSkillPoints));
	}).ToString();
	DrawText(PointsLabel, AvailableSkillPoints > 0 ? Theme::AccentGold : Theme::TextMuted, StartX + TotalWidth - 136.0f * HudScale, SlotY - 62.0f * HudScale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.88f * HudScale);
}

void AIdleHUD::DrawSkillSlot(const FIdleHUDSkillSlotViewModel& Slot, int32 SlotIndex, float X, float Y, float Width, float Height)
{
	using namespace IdleProject::UI;

	const float Scale = Height / 60.0f;
	const FLinearColor PanelColor = Theme::BgPanel.CopyWithNewOpacity(0.86f);
	const FLinearColor BorderColor = Slot.bReady ? Theme::AccentGold : Theme::TextMuted.CopyWithNewOpacity(0.70f);
	const FLinearColor FillColor = Slot.bReady ? Theme::AccentGold : Theme::AccentBlue;
	const FLinearColor CooldownOverlay = FLinearColor(0.0f, 0.0f, 0.0f, 0.34f);

	DrawRect(PanelColor, X, Y, Width, Height);
	DrawRect(BorderColor, X, Y, Width, 2.0f * Scale);
	DrawRect(BorderColor, X, Y + Height - 2.0f * Scale, Width, 2.0f * Scale);
	DrawRect(BorderColor, X, Y, 2.0f * Scale, Height);
	DrawRect(BorderColor, X + Width - 2.0f * Scale, Y, 2.0f * Scale, Height);

	const float ReadyRatio = 1.0f - Slot.CooldownRatio;
	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.92f), X + 8.0f * Scale, Y + Height - 12.0f * Scale, Width - 16.0f * Scale, 5.0f * Scale);
	DrawRect(FillColor, X + 8.0f * Scale, Y + Height - 12.0f * Scale, (Width - 16.0f * Scale) * ReadyRatio, 5.0f * Scale);

	if (!Slot.bReady)
	{
		DrawRect(CooldownOverlay, X + Width * ReadyRatio, Y, Width * Slot.CooldownRatio, Height);
	}

	const FString KeyLabel = FString::Printf(TEXT("%d"), SlotIndex + 1);
	DrawText(KeyLabel, Theme::TextMuted, X + 8.0f * Scale, Y + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.90f * Scale);
	DrawText(Slot.DisplayName.ToString(), Theme::TextPrimary, X + 28.0f * Scale, Y + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.90f * Scale);

	const FString RankLabel = FormatLocalizedUI(TEXT("SKILL_RANK_FORMAT"), [&Slot](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Rank"), FText::AsNumber(Slot.Rank));
		Args.Add(TEXT("MaxRank"), FText::AsNumber(Slot.MaxRank));
	}).ToString();
	DrawText(RankLabel, Slot.bCanRankUp ? Theme::AccentGold : Theme::TextMuted, X + 96.0f * Scale, Y + 24.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.76f * Scale);

	const FString StatusLabel = Slot.bReady
		? IdleProject::Localization::UI(TEXT("SKILL_READY")).ToString()
		: FormatLocalizedUI(TEXT("SKILL_COOLDOWN_FORMAT"), [&Slot](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Seconds"), FText::AsNumber(Slot.CooldownRemaining));
		}).ToString();
	DrawText(StatusLabel, Slot.bReady ? Theme::AccentGold : Theme::TextMuted, X + 10.0f * Scale, Y + 24.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.86f * Scale);

	const float ButtonSize = 24.0f * Scale;
	const float ButtonX = X + Width - ButtonSize - 8.0f * Scale;
	const float ButtonY = Y + 6.0f * Scale;
	DrawRect(Slot.bCanRankUp ? Theme::AccentGold : Theme::BgPrimary.CopyWithNewOpacity(0.94f), ButtonX, ButtonY, ButtonSize, ButtonSize);
	DrawText(TEXT("+"), Slot.bCanRankUp ? Theme::BgPrimary : Theme::TextMuted, ButtonX + 7.0f * Scale, ButtonY + 1.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 0.72f * Scale);
	if (Slot.bCanRankUp)
	{
		AddHitBox(FVector2D(ButtonX, ButtonY), FVector2D(ButtonSize, ButtonSize), MakeSkillRankHitBoxName(Slot.SkillId), true, 88);
	}
}

void AIdleHUD::DrawUltimateGauge(const FIdleHUDUltimateViewModel& Ultimate, float X, float Y, float Width, float Height)
{
	using namespace IdleProject::UI;

	const float Scale = Height / 22.0f;
	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.86f), X, Y, Width, Height);
	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.92f), X + 4.0f * Scale, Y + 4.0f * Scale, Width - 8.0f * Scale, Height - 8.0f * Scale);
	DrawRect(Theme::Auth, X + 4.0f * Scale, Y + 4.0f * Scale, (Width - 8.0f * Scale) * Ultimate.GaugeRatio, Height - 8.0f * Scale);

	const FString GaugeLabel = Ultimate.bReady
		? IdleProject::Localization::UI(TEXT("ULTIMATE_READY")).ToString()
		: FormatLocalizedUI(TEXT("ULTIMATE_CHARGE_FORMAT"), [&Ultimate](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Percent"), FText::AsNumber(FMath::RoundToInt(Ultimate.GaugePercent)));
		}).ToString();
	DrawText(GaugeLabel, Ultimate.bReady ? Theme::AccentGold : Theme::TextPrimary, X + 10.0f * Scale, Y + 2.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.86f * Scale);
}

void AIdleHUD::RankUpSkillFromHitBox(FName BoxName)
{
	USkillComponent* PlayerSkills = ResolvePlayerSkills();
	if (!PlayerSkills)
	{
		return;
	}

	FString SkillId = BoxName.ToString();
	SkillId.RightChopInline(SkillRankHitBoxPrefix.Len());
	if (!SkillId.IsEmpty())
	{
		const FName SkillName(*SkillId);
		if (PlayerSkills->RankUpSkill(SkillName))
		{
			if (!IdleGameInstance)
			{
				IdleGameInstance = GetGameInstance<UIdleGameInstance>();
			}
			if (IdleGameInstance)
			{
				const int32 NewRank = PlayerSkills->GetSkillRank(SkillName);
				IdleGameInstance->RecordAchievementMetric(EAchievementMetric::SkillRankUps, 1);
				IdleGameInstance->RecordAchievementMetric(EAchievementMetric::HighestSkillRank, NewRank);
			}
		}
	}
	RefreshMouseInteraction();
}

void AIdleHUD::PreviewOfflineRewardModal()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance)
	{
		return;
	}

	const int64 NowUnixSec = FDateTime::UtcNow().ToUnixTimestamp();
	OfflineRewardModal = IdleProject::UI::BuildOfflineRewardViewModel(IdleGameInstance->PreviewOfflineRewards(NowUnixSec));
	if (OfflineRewardModal.bVisible && PlayerOwner)
	{
		RefreshMouseInteraction();
	}
}

void AIdleHUD::ClaimOfflineRewardModal()
{
	if (!OfflineRewardModal.bVisible || !IdleGameInstance)
	{
		return;
	}

	IdleGameInstance->ClaimOfflineRewards();
	OfflineRewardModal.bVisible = false;
	RefreshMouseInteraction();
}

void AIdleHUD::DrawOfflineRewardModal()
{
	using namespace IdleProject::UI;

	if (!Canvas || !OfflineRewardModal.bVisible)
	{
		return;
	}

	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float ModalWidth = FMath::Clamp(Canvas->SizeX * 0.42f, 420.0f * Scale, 640.0f * Scale);
	const float ModalHeight = 284.0f * Scale;
	const float X = (Canvas->SizeX - ModalWidth) * 0.5f;
	const float Y = (Canvas->SizeY - ModalHeight) * 0.5f;
	const float Padding = 28.0f * Scale;
	const float Border = 2.0f * Scale;

	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.56f), 0.0f, 0.0f, Canvas->SizeX, Canvas->SizeY);
	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.96f), X, Y, ModalWidth, ModalHeight);
	DrawRect(Theme::AccentGold, X, Y, ModalWidth, Border);
	DrawRect(Theme::AccentGold, X, Y + ModalHeight - Border, ModalWidth, Border);
	DrawRect(Theme::AccentGold, X, Y, Border, ModalHeight);
	DrawRect(Theme::AccentGold, X + ModalWidth - Border, Y, Border, ModalHeight);

	DrawText(OfflineRewardModal.Title.ToString(), Theme::TextPrimary, X + Padding, Y + 22.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 1.05f * Scale);
	DrawText(OfflineRewardModal.ElapsedLabel.ToString(), Theme::TextMuted, X + Padding, Y + 72.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 1.0f * Scale);
	DrawText(OfflineRewardModal.GoldLabel.ToString(), Theme::AccentGold, X + Padding, Y + 120.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 1.0f * Scale);
	DrawText(OfflineRewardModal.ExpLabel.ToString(), Theme::AccentBlue, X + Padding, Y + 160.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 1.0f * Scale);

	const float ButtonWidth = 156.0f * Scale;
	const float ButtonHeight = 44.0f * Scale;
	const float ButtonX = X + ModalWidth - Padding - ButtonWidth;
	const float ButtonY = Y + ModalHeight - Padding - ButtonHeight;
	DrawRect(Theme::AccentGold, ButtonX, ButtonY, ButtonWidth, ButtonHeight);
	DrawText(OfflineRewardModal.ClaimLabel.ToString(), Theme::BgPrimary, ButtonX + 52.0f * Scale, ButtonY + 10.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 1.0f * Scale);
	AddHitBox(FVector2D(ButtonX, ButtonY), FVector2D(ButtonWidth, ButtonHeight), OfflineRewardClaimHitBoxName, true, 100);
}

void AIdleHUD::DrawQuestLog()
{
	using namespace IdleProject::UI;

	if (!Canvas || !bQuestLogVisible)
	{
		return;
	}
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance)
	{
		return;
	}

	const FIdleHUDQuestLogViewModel ViewModel = BuildQuestLogViewModel(IdleGameInstance->GetActiveQuestStates());
	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelWidth = FMath::Clamp(Canvas->SizeX * 0.36f, 420.0f * Scale, 620.0f * Scale);
	const float RowHeight = 58.0f * Scale;
	const float SectionHeight = 26.0f * Scale;
	const float HeaderHeight = 54.0f * Scale;
	const float Padding = 18.0f * Scale;
	const int32 VisibleSectionCount = ViewModel.Rows.IsEmpty() ? 0 : ViewModel.Sections.Num();
	const float ContentHeight = ViewModel.Rows.IsEmpty()
		? 52.0f * Scale
		: VisibleSectionCount * SectionHeight + ViewModel.Rows.Num() * (RowHeight + 8.0f * Scale);
	const float PanelHeight = FMath::Min(HeaderHeight + Padding + ContentHeight + Padding, Canvas->SizeY - 120.0f * Scale);
	const float X = Canvas->SizeX - PanelWidth - 28.0f * Scale;
	const float Y = 92.0f * Scale;
	const float Border = 2.0f * Scale;

	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.94f), X, Y, PanelWidth, PanelHeight);
	DrawRect(Theme::AccentBlue, X, Y, PanelWidth, Border);
	DrawRect(Theme::AccentBlue, X, Y + PanelHeight - Border, PanelWidth, Border);
	DrawRect(Theme::AccentBlue, X, Y, Border, PanelHeight);
	DrawRect(Theme::AccentBlue, X + PanelWidth - Border, Y, Border, PanelHeight);

	DrawText(ViewModel.Title.ToString(), Theme::TextPrimary, X + Padding, Y + 14.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 1.0f * Scale);
	DrawText(ViewModel.ShortcutLabel.ToString(), Theme::TextMuted, X + PanelWidth - 72.0f * Scale, Y + 18.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.82f * Scale);

	float RowY = Y + HeaderHeight;
	if (ViewModel.Rows.IsEmpty())
	{
		DrawText(ViewModel.EmptyLabel.ToString(), Theme::TextMuted, X + Padding, RowY + 20.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.95f * Scale);
		return;
	}

	for (const FIdleHUDQuestLogSectionViewModel& Section : ViewModel.Sections)
	{
		if (RowY + SectionHeight > Y + PanelHeight - Padding)
		{
			break;
		}

		const float RowX = X + Padding;
		const float InnerWidth = PanelWidth - Padding * 2.0f;
		const FLinearColor SectionColor = QuestTypeToAccentColor(Section.Type);
		DrawText(Section.TypeLabel.ToString(), SectionColor, RowX, RowY + 4.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.86f * Scale);
		DrawRect(SectionColor.CopyWithNewOpacity(0.70f), RowX + 58.0f * Scale, RowY + 14.0f * Scale, InnerWidth - 58.0f * Scale, 1.0f * Scale);
		RowY += SectionHeight;

		for (const FIdleHUDQuestLogRowViewModel& Row : Section.Rows)
		{
			if (RowY + RowHeight > Y + PanelHeight - Padding)
			{
				return;
			}

			DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.90f), RowX, RowY, InnerWidth, RowHeight);
			DrawRect(Row.bCanClaim ? Theme::AccentGold : SectionColor.CopyWithNewOpacity(0.64f), RowX, RowY, 4.0f * Scale, RowHeight);

			DrawText(Row.Title.ToString(), Theme::TextPrimary, RowX + 12.0f * Scale, RowY + 7.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.86f * Scale);
			DrawText(Row.ProgressLabel.ToString(), Theme::TextMuted, RowX + 12.0f * Scale, RowY + 27.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.76f * Scale);
			DrawText(Row.RewardLabel.ToString(), Theme::TextMuted, RowX + 132.0f * Scale, RowY + 27.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.76f * Scale);

			const float BarX = RowX + 12.0f * Scale;
			const float BarY = RowY + RowHeight - 11.0f * Scale;
			const float BarWidth = InnerWidth - 104.0f * Scale;
			DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.96f), BarX, BarY, BarWidth, 5.0f * Scale);
			DrawRect(Row.bCanClaim ? Theme::AccentGold : Theme::AccentBlue, BarX, BarY, BarWidth * Row.ProgressRatio, 5.0f * Scale);

			const float ButtonWidth = 72.0f * Scale;
			const float ButtonHeight = 26.0f * Scale;
			const float ButtonX = RowX + InnerWidth - ButtonWidth - 12.0f * Scale;
			const float ButtonY = RowY + 18.0f * Scale;
			DrawRect(Row.bCanClaim ? Theme::AccentGold : Theme::BgPanel, ButtonX, ButtonY, ButtonWidth, ButtonHeight);
			DrawText(Row.ActionLabel.ToString(), Row.bCanClaim ? Theme::BgPrimary : Theme::TextMuted, ButtonX + 19.0f * Scale, ButtonY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.80f * Scale);
			if (Row.bCanClaim)
			{
				AddHitBox(FVector2D(ButtonX, ButtonY), FVector2D(ButtonWidth, ButtonHeight), MakeQuestClaimHitBoxName(Row.QuestId), true, 90);
			}

			RowY += RowHeight + 8.0f * Scale;
		}
	}
}

void AIdleHUD::ClaimQuestFromHitBox(FName BoxName)
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance)
	{
		return;
	}

	FString QuestId = BoxName.ToString();
	QuestId.RightChopInline(QuestClaimHitBoxPrefix.Len());
	if (QuestId.IsEmpty())
	{
		return;
	}

	const FQuestClaimResult Claim = IdleGameInstance->ClaimQuest(QuestId);
	if (Claim.bSuccess)
	{
		UE_LOG(LogTemp, Display, TEXT("[QuestLog] ClaimQuest success questId=%s gold=%lld exp=%lld"), *QuestId, Claim.RewardGold, Claim.RewardExp);
	}
}

void AIdleHUD::DrawRebirthPanel()
{
	using namespace IdleProject::UI;

	if (!Canvas)
	{
		return;
	}
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance)
	{
		return;
	}

	const FIdleHUDRebirthViewModel ViewModel = BuildRebirthViewModel(
		IdleGameInstance->CanRebirth(),
		IdleGameInstance->HasDefeatedChapter1Boss(),
		IdleGameInstance->GetCharacterLevel(),
		IdleGameInstance->GetRebirthCount(),
		IdleGameInstance->GetRebirthBonusPoints(),
		IdleGameInstance->PreviewRebirthReward());

	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelWidth = FMath::Clamp(Canvas->SizeX * 0.24f, 320.0f * Scale, 440.0f * Scale);
	const float PanelHeight = 206.0f * Scale;
	const float X = Canvas->SizeX - PanelWidth - 28.0f * Scale;
	const float Y = 304.0f * Scale;
	const float Padding = 16.0f * Scale;
	const float Border = 2.0f * Scale;
	const FLinearColor StateColor = ViewModel.bCanRebirth ? Theme::AccentGold : Theme::TextMuted.CopyWithNewOpacity(0.72f);
	const FLinearColor BossColor = ViewModel.bBossDefeated ? Theme::AccentGold : Theme::AccentRed;
	const FLinearColor LevelColor = ViewModel.bLevelReady ? Theme::AccentBlue : Theme::TextMuted;

	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.91f), X, Y, PanelWidth, PanelHeight);
	DrawRect(StateColor, X, Y, PanelWidth, Border);
	DrawRect(StateColor, X, Y + PanelHeight - Border, PanelWidth, Border);
	DrawRect(StateColor, X, Y, Border, PanelHeight);
	DrawRect(StateColor, X + PanelWidth - Border, Y, Border, PanelHeight);

	DrawText(ViewModel.Title.ToString(), Theme::TextPrimary, X + Padding, Y + 12.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 0.95f * Scale);
	DrawText(ViewModel.StatusLabel.ToString(), StateColor, X + PanelWidth - 112.0f * Scale, Y + 17.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.80f * Scale);
	DrawText(ViewModel.BossLabel.ToString(), BossColor, X + Padding, Y + 52.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.88f * Scale);
	DrawText(ViewModel.LevelLabel.ToString(), LevelColor, X + 142.0f * Scale, Y + 52.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.88f * Scale);
	DrawText(ViewModel.CountLabel.ToString(), Theme::TextPrimary, X + Padding, Y + 86.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.90f * Scale);
	DrawText(ViewModel.BonusLabel.ToString(), Theme::AccentGold, X + Padding, Y + 116.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.90f * Scale);
	DrawText(ViewModel.NextBonusLabel.ToString(), Theme::TextMuted, X + Padding, Y + 144.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.84f * Scale);

	const float ButtonWidth = 112.0f * Scale;
	const float ButtonHeight = 34.0f * Scale;
	const float ButtonX = X + PanelWidth - Padding - ButtonWidth;
	const float ButtonY = Y + PanelHeight - Padding - ButtonHeight;
	DrawRect(ViewModel.bCanRebirth ? Theme::AccentGold : Theme::BgPrimary.CopyWithNewOpacity(0.94f), ButtonX, ButtonY, ButtonWidth, ButtonHeight);
	DrawText(ViewModel.ButtonLabel.ToString(), ViewModel.bCanRebirth ? Theme::BgPrimary : Theme::TextMuted, ButtonX + 18.0f * Scale, ButtonY + 8.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.84f * Scale);
	if (ViewModel.bCanRebirth)
	{
		AddHitBox(FVector2D(ButtonX, ButtonY), FVector2D(ButtonWidth, ButtonHeight), RebirthHitBoxName, true, 80);
	}
	RefreshMouseInteraction();
}

void AIdleHUD::TryRebirth()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance || !IdleGameInstance->Rebirth())
	{
		return;
	}

	if (AIdleCharacter* IdleCharacter = PlayerOwner ? Cast<AIdleCharacter>(PlayerOwner->GetPawn()) : nullptr)
	{
		IdleCharacter->RefreshDerivedStats();
	}
	RefreshMouseInteraction();
}

void AIdleHUD::DrawTranscendPanel()
{
	using namespace IdleProject::UI;

	if (!Canvas)
	{
		return;
	}
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance)
	{
		return;
	}

	const FIdleHUDTranscendViewModel ViewModel = BuildTranscendViewModel(
		IdleGameInstance->CanTranscend(),
		IdleGameInstance->GetRebirthCount(),
		FTranscendFormula::TranscendRebirthThreshold,
		IdleGameInstance->GetTranscendCount(),
		IdleGameInstance->GetTranscendStatMultiplier(),
		IdleGameInstance->PreviewTranscendMultiplier());

	const UWorld* World = GetWorld();
	const float FeedbackElapsed = World ? World->GetTimeSeconds() - TranscendFeedbackStartTime : 0.0f;
	const bool bShowFeedback = !TranscendFeedbackLabel.IsEmpty() && FeedbackElapsed <= TranscendFeedbackDurationSeconds;

	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelWidth = FMath::Clamp(Canvas->SizeX * 0.24f, 320.0f * Scale, 440.0f * Scale);
	const float FeedbackHeight = bShowFeedback ? 24.0f * Scale : 0.0f;
	const float PanelHeight = (178.0f * Scale) + FeedbackHeight;
	const float X = Canvas->SizeX - PanelWidth - 28.0f * Scale;
	const float Y = 522.0f * Scale;
	const float Padding = 16.0f * Scale;
	const float Border = 2.0f * Scale;
	const FLinearColor StateColor = ViewModel.bCanTranscend ? Theme::AccentGold : Theme::TextMuted.CopyWithNewOpacity(0.72f);
	const FLinearColor RequirementColor = ViewModel.bThresholdReady ? Theme::AccentBlue : Theme::TextMuted;

	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.91f), X, Y, PanelWidth, PanelHeight);
	DrawRect(StateColor, X, Y, PanelWidth, Border);
	DrawRect(StateColor, X, Y + PanelHeight - Border, PanelWidth, Border);
	DrawRect(StateColor, X, Y, Border, PanelHeight);
	DrawRect(StateColor, X + PanelWidth - Border, Y, Border, PanelHeight);

	DrawText(ViewModel.Title.ToString(), Theme::TextPrimary, X + Padding, Y + 12.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 0.95f * Scale);
	DrawText(ViewModel.StatusLabel.ToString(), StateColor, X + PanelWidth - 122.0f * Scale, Y + 17.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.80f * Scale);
	DrawText(ViewModel.RequirementLabel.ToString(), RequirementColor, X + Padding, Y + 52.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.88f * Scale);
	DrawText(ViewModel.CountLabel.ToString(), Theme::TextPrimary, X + 154.0f * Scale, Y + 52.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.88f * Scale);
	DrawText(ViewModel.CurrentMultiplierLabel.ToString(), Theme::AccentGold, X + Padding, Y + 84.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.90f * Scale);
	DrawText(ViewModel.PreviewMultiplierLabel.ToString(), Theme::TextMuted, X + Padding, Y + 112.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.84f * Scale);

	const float ButtonWidth = 112.0f * Scale;
	const float ButtonHeight = 34.0f * Scale;
	const float ButtonX = X + PanelWidth - Padding - ButtonWidth;
	const float ButtonY = Y + 128.0f * Scale;
	DrawRect(ViewModel.bCanTranscend ? Theme::AccentGold : Theme::BgPrimary.CopyWithNewOpacity(0.94f), ButtonX, ButtonY, ButtonWidth, ButtonHeight);
	DrawText(ViewModel.ButtonLabel.ToString(), ViewModel.bCanTranscend ? Theme::BgPrimary : Theme::TextMuted, ButtonX + 22.0f * Scale, ButtonY + 8.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.84f * Scale);
	if (ViewModel.bCanTranscend)
	{
		AddHitBox(FVector2D(ButtonX, ButtonY), FVector2D(ButtonWidth, ButtonHeight), TranscendHitBoxName, true, 81);
	}

	if (bShowFeedback)
	{
		DrawText(TranscendFeedbackLabel.ToString(), Theme::AccentGold, X + Padding, Y + 164.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.82f * Scale);
	}
	RefreshMouseInteraction();
}

void AIdleHUD::TryTranscend()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance || !IdleGameInstance->Transcend())
	{
		return;
	}

	if (AIdleCharacter* IdleCharacter = PlayerOwner ? Cast<AIdleCharacter>(PlayerOwner->GetPawn()) : nullptr)
	{
		IdleCharacter->RefreshDerivedStats();
	}
	RefreshMouseInteraction();
}

void AIdleHUD::DrawTowerPanel()
{
	using namespace IdleProject::UI;

	if (!Canvas)
	{
		return;
	}
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	BindTowerService();
	const UTowerService* TowerService = BoundTowerService.Get();
	const AIdleCharacter* IdleCharacter = ResolvePlayerCharacter();
	if (!TowerService || !IdleCharacter)
	{
		return;
	}

	const FIdleHUDTowerViewModel ViewModel = BuildTowerViewModel(
		TowerService->GetHighestFloor(),
		TowerService->GetNextFloorRequiredPower(),
		IdleCharacter->GetCombatPower(),
		TowerService->GetMilestoneMultiplier());

	const UWorld* World = GetWorld();
	const float FeedbackElapsed = World ? World->GetTimeSeconds() - TowerFeedbackStartTime : 0.0f;
	const bool bShowFeedback = !TowerFeedbackLabel.IsEmpty() && FeedbackElapsed <= TowerFeedbackDurationSeconds;

	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelWidth = FMath::Clamp(Canvas->SizeX * 0.22f, 340.0f * Scale, 440.0f * Scale);
	const float FeedbackHeight = bShowFeedback ? 24.0f * Scale : 0.0f;
	const float PanelHeight = 198.0f * Scale + FeedbackHeight;
	const float X = (Canvas->SizeX - PanelWidth) * 0.5f;
	const float Y = 214.0f * Scale;
	const float Padding = 14.0f * Scale;
	const float Border = 2.0f * Scale;
	const FLinearColor StateColor = ViewModel.bCanClimb ? Theme::AccentGold : Theme::TextMuted.CopyWithNewOpacity(0.72f);

	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.91f), X, Y, PanelWidth, PanelHeight);
	DrawRect(StateColor, X, Y, PanelWidth, Border);
	DrawRect(StateColor, X, Y + PanelHeight - Border, PanelWidth, Border);
	DrawRect(StateColor, X, Y, Border, PanelHeight);
	DrawRect(StateColor, X + PanelWidth - Border, Y, Border, PanelHeight);

	DrawText(ViewModel.Title.ToString(), Theme::TextPrimary, X + Padding, Y + 12.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 0.92f * Scale);
	DrawText(ViewModel.StatusLabel.ToString(), ViewModel.bCanClimb ? Theme::AccentGold : Theme::AccentRed, X + PanelWidth - 128.0f * Scale, Y + 17.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.76f * Scale);
	DrawText(ViewModel.HighestFloorLabel.ToString(), Theme::TextPrimary, X + Padding, Y + 50.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.86f * Scale);
	DrawText(ViewModel.NextRequiredPowerLabel.ToString(), ViewModel.bCanClimb ? Theme::AccentBlue : Theme::AccentRed, X + Padding, Y + 76.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.82f * Scale);
	DrawText(ViewModel.CombatPowerLabel.ToString(), Theme::AccentGold, X + Padding, Y + 102.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.82f * Scale);
	DrawText(ViewModel.MilestoneMultiplierLabel.ToString(), Theme::TextPrimary, X + Padding, Y + 128.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.80f * Scale);
	DrawText(ViewModel.NextMilestoneLabel.ToString(), Theme::TextMuted, X + Padding, Y + 152.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.76f * Scale);

	const float ButtonWidth = 94.0f * Scale;
	const float ButtonHeight = 32.0f * Scale;
	const float ButtonX = X + PanelWidth - Padding - ButtonWidth;
	const float ButtonY = Y + 142.0f * Scale;
	DrawRect(ViewModel.bCanClimb ? Theme::AccentGold : Theme::BgPrimary.CopyWithNewOpacity(0.94f), ButtonX, ButtonY, ButtonWidth, ButtonHeight);
	DrawText(ViewModel.ButtonLabel.ToString(), ViewModel.bCanClimb ? Theme::BgPrimary : Theme::TextMuted, ButtonX + 24.0f * Scale, ButtonY + 8.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.78f * Scale);
	if (ViewModel.bCanClimb)
	{
		AddHitBox(FVector2D(ButtonX, ButtonY), FVector2D(ButtonWidth, ButtonHeight), ViewModel.ClimbHitBoxName, true, 82);
	}

	if (bShowFeedback)
	{
		DrawText(TowerFeedbackLabel.ToString(), Theme::AccentGold, X + Padding, Y + 188.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.80f * Scale);
	}
	RefreshMouseInteraction();
}

void AIdleHUD::TryClimbTower()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance)
	{
		return;
	}

	const int64 Reward = IdleGameInstance->ClimbTower();
	if (Reward <= 0)
	{
		TowerFeedbackLabel = IdleProject::Localization::UI(TEXT("TOWER_CLIMB_BLOCKED"));
		if (const UWorld* World = GetWorld())
		{
			TowerFeedbackStartTime = World->GetTimeSeconds();
		}
	}
	RefreshMouseInteraction();
}

void AIdleHUD::DrawDungeonPanel()
{
	using namespace IdleProject::UI;

	if (!Canvas)
	{
		return;
	}
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	const UDungeonService* DungeonService = IdleGameInstance ? IdleGameInstance->GetDungeonService() : nullptr;
	const AIdleCharacter* IdleCharacter = ResolvePlayerCharacter();
	if (!DungeonService || !IdleCharacter)
	{
		return;
	}

	const FIdleHUDDungeonPanelViewModel ViewModel = BuildDungeonPanelViewModel(*DungeonService, IdleCharacter->GetCombatPower(), UQuestService::GetCurrentUtcDateString());

	const UWorld* World = GetWorld();
	const float FeedbackElapsed = World ? World->GetTimeSeconds() - DungeonFeedbackStartTime : 0.0f;
	const bool bShowFeedback = !DungeonFeedbackLabel.IsEmpty() && FeedbackElapsed <= DungeonFeedbackDurationSeconds;

	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelWidth = FMath::Clamp(Canvas->SizeX * 0.24f, 360.0f * Scale, 460.0f * Scale);
	const float RowHeight = 82.0f * Scale;
	const float RowGap = 8.0f * Scale;
	const float Padding = 14.0f * Scale;
	const float FeedbackHeight = bShowFeedback ? 26.0f * Scale : 0.0f;
	const float PanelHeight = 54.0f * Scale + ViewModel.Rows.Num() * RowHeight + FMath::Max(0, ViewModel.Rows.Num() - 1) * RowGap + Padding + FeedbackHeight;
	const float X = (Canvas->SizeX - PanelWidth) * 0.5f;
	const float Y = 428.0f * Scale;
	const float Border = 2.0f * Scale;

	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.91f), X, Y, PanelWidth, PanelHeight);
	DrawRect(Theme::AccentGold, X, Y, PanelWidth, Border);
	DrawRect(Theme::AccentGold, X, Y + PanelHeight - Border, PanelWidth, Border);
	DrawRect(Theme::AccentGold, X, Y, Border, PanelHeight);
	DrawRect(Theme::AccentGold, X + PanelWidth - Border, Y, Border, PanelHeight);

	DrawText(ViewModel.Title.ToString(), Theme::TextPrimary, X + Padding, Y + 12.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 0.92f * Scale);

	float RowY = Y + 48.0f * Scale;
	for (const FIdleHUDDungeonRowViewModel& Row : ViewModel.Rows)
	{
		DrawDungeonRow(Row, X + Padding, RowY, PanelWidth - Padding * 2.0f, RowHeight);
		RowY += RowHeight + RowGap;
	}

	if (bShowFeedback)
	{
		DrawText(DungeonFeedbackLabel.ToString(), Theme::AccentGold, X + Padding, RowY + 2.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.80f * Scale);
	}
	RefreshMouseInteraction();
}

void AIdleHUD::DrawDungeonRow(const FIdleHUDDungeonRowViewModel& Row, float X, float Y, float Width, float Height)
{
	using namespace IdleProject::UI;

	const float Scale = Height / 82.0f;
	const FLinearColor StateColor = Row.bCanEnter
		? Theme::AccentGold
		: (Row.bNeedsPower ? Theme::AccentRed : Theme::TextMuted.CopyWithNewOpacity(0.62f));
	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.90f), X, Y, Width, Height);
	DrawRect(StateColor, X, Y, 4.0f * Scale, Height);

	DrawText(Row.NameLabel.ToString(), Row.bCanEnter ? Theme::AccentGold : Theme::TextPrimary, X + 12.0f * Scale, Y + 7.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.84f * Scale);
	DrawText(Row.EntriesLabel.ToString(), Theme::TextMuted, X + 142.0f * Scale, Y + 8.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.74f * Scale);
	DrawText(Row.TierLabel.ToString(), Row.MaxAccessibleTier > 0 ? Theme::AccentGold : Theme::TextMuted, X + 12.0f * Scale, Y + 27.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.72f * Scale);
	DrawText(Row.NextTierLabel.ToString(), Theme::TextMuted, X + 142.0f * Scale, Y + 27.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.68f * Scale);
	DrawText(Row.RequiredPowerLabel.ToString(), Row.bNeedsPower ? Theme::AccentRed : Theme::AccentBlue, X + 12.0f * Scale, Y + 47.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.70f * Scale);
	DrawText(Row.RewardLabel.ToString(), Theme::TextMuted, X + 142.0f * Scale, Y + 47.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.70f * Scale);
	DrawText(Row.StatusLabel.ToString(), Row.bCanEnter ? Theme::AccentGold : StateColor, X + 12.0f * Scale, Y + 67.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.64f * Scale);

	const float ButtonWidth = 72.0f * Scale;
	const float ButtonHeight = 26.0f * Scale;
	const float ButtonX = X + Width - ButtonWidth - 8.0f * Scale;
	const float ButtonY = Y + 7.0f * Scale;
	DrawRect(Row.bCanEnter ? Theme::AccentGold : Theme::BgPanel, ButtonX, ButtonY, ButtonWidth, ButtonHeight);
	DrawText(Row.ActionLabel.ToString(), Row.bCanEnter ? Theme::BgPrimary : Theme::TextMuted, ButtonX + 16.0f * Scale, ButtonY + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.72f * Scale);
	if (Row.bCanEnter)
	{
		AddHitBox(FVector2D(ButtonX, ButtonY), FVector2D(ButtonWidth, ButtonHeight), Row.EnterHitBoxName, true, 85);
	}
}

void AIdleHUD::TryRunDungeonFromHitBox(FName BoxName)
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance)
	{
		return;
	}

	const FDungeonRunResult Result = IdleGameInstance->TryRunDungeon(DungeonTypeFromHitBoxName(BoxName), DungeonTierFromHitBoxName(BoxName));
	DungeonFeedbackLabel = Result.bSuccess ? BuildDungeonRewardLabel(Result) : IdleProject::Localization::UI(TEXT("DUNGEON_STATUS_NEED_CP"));
	if (const UWorld* World = GetWorld())
	{
		DungeonFeedbackStartTime = World->GetTimeSeconds();
	}
	RefreshMouseInteraction();
}

void AIdleHUD::DrawWeeklyBossPanel()
{
	using namespace IdleProject::UI;

	if (!Canvas)
	{
		return;
	}
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	const UWeeklyBossService* WeeklyBossService = IdleGameInstance ? IdleGameInstance->GetWeeklyBossService() : nullptr;
	if (!WeeklyBossService)
	{
		return;
	}

	const FIdleHUDWeeklyBossPanelViewModel ViewModel = BuildWeeklyBossPanelViewModel(*WeeklyBossService);
	const UWorld* World = GetWorld();
	const float FeedbackElapsed = World ? World->GetTimeSeconds() - WeeklyBossFeedbackStartTime : 0.0f;
	const bool bShowFeedback = !WeeklyBossFeedbackLabel.IsEmpty() && FeedbackElapsed <= WeeklyBossFeedbackDurationSeconds;

	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelWidth = FMath::Clamp(Canvas->SizeX * 0.24f, 360.0f * Scale, 460.0f * Scale);
	const float RowHeight = 44.0f * Scale;
	const float RowGap = 6.0f * Scale;
	const float Padding = 14.0f * Scale;
	const int32 VisibleRows = FMath::Min(ViewModel.Rows.Num(), 4);
	const float FeedbackHeight = bShowFeedback ? 24.0f * Scale : 0.0f;
	const float PanelHeight = 132.0f * Scale + VisibleRows * RowHeight + FMath::Max(0, VisibleRows - 1) * RowGap + Padding + FeedbackHeight;
	const float X = (Canvas->SizeX - PanelWidth) * 0.5f;
	const float Y = 650.0f * Scale;
	const float Border = 2.0f * Scale;

	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.91f), X, Y, PanelWidth, PanelHeight);
	DrawRect(Theme::AccentBlue, X, Y, PanelWidth, Border);
	DrawRect(Theme::AccentBlue, X, Y + PanelHeight - Border, PanelWidth, Border);
	DrawRect(Theme::AccentBlue, X, Y, Border, PanelHeight);
	DrawRect(Theme::AccentBlue, X + PanelWidth - Border, Y, Border, PanelHeight);

	DrawText(ViewModel.Title.ToString(), Theme::TextPrimary, X + Padding, Y + 10.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 0.88f * Scale);
	DrawText(ViewModel.WeekLabel.ToString(), Theme::TextMuted, X + 150.0f * Scale, Y + 13.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.64f * Scale);
	DrawText(ViewModel.DamageLabel.ToString(), Theme::AccentBlue, X + Padding, Y + 40.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.76f * Scale);
	DrawText(ViewModel.RemainingLabel.ToString(), ViewModel.bCanChallenge ? Theme::AccentGold : Theme::AccentRed, X + Padding, Y + 62.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.72f * Scale);
	DrawText(ViewModel.MilestoneSummaryLabel.ToString(), Theme::TextMuted, X + 152.0f * Scale, Y + 62.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.68f * Scale);

	const float BarX = X + Padding;
	const float BarY = Y + 88.0f * Scale;
	const float BarWidth = PanelWidth - Padding * 2.0f - 98.0f * Scale;
	const float BarHeight = 10.0f * Scale;
	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.92f), BarX, BarY, BarWidth, BarHeight);
	DrawRect(Theme::AccentBlue, BarX, BarY, BarWidth * ViewModel.ProgressRatio, BarHeight);
	DrawText(ViewModel.ResetLabel.ToString(), Theme::TextMuted, BarX, BarY + 14.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.58f * Scale);

	const float ButtonWidth = 88.0f * Scale;
	const float ButtonHeight = 30.0f * Scale;
	const float ButtonX = X + PanelWidth - Padding - ButtonWidth;
	const float ButtonY = Y + 82.0f * Scale;
	DrawRect(ViewModel.bCanChallenge ? Theme::AccentGold : Theme::BgPrimary, ButtonX, ButtonY, ButtonWidth, ButtonHeight);
	DrawText(ViewModel.ChallengeLabel.ToString(), ViewModel.bCanChallenge ? Theme::BgPrimary : Theme::TextMuted, ButtonX + 13.0f * Scale, ButtonY + 7.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.70f * Scale);
	if (ViewModel.bCanChallenge)
	{
		AddHitBox(FVector2D(ButtonX, ButtonY), FVector2D(ButtonWidth, ButtonHeight), ViewModel.ChallengeHitBoxName, true, 86);
	}

	float RowY = Y + 122.0f * Scale;
	for (int32 Index = 0; Index < VisibleRows; ++Index)
	{
		DrawWeeklyBossMilestoneRow(ViewModel.Rows[Index], X + Padding, RowY, PanelWidth - Padding * 2.0f, RowHeight);
		RowY += RowHeight + RowGap;
	}

	if (bShowFeedback)
	{
		DrawText(WeeklyBossFeedbackLabel.ToString(), Theme::AccentGold, X + Padding, RowY + 2.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.74f * Scale);
	}
	RefreshMouseInteraction();
}

void AIdleHUD::DrawWeeklyBossMilestoneRow(const FIdleHUDWeeklyBossMilestoneRowViewModel& Row, float X, float Y, float Width, float Height)
{
	using namespace IdleProject::UI;

	const float Scale = Height / 44.0f;
	const FLinearColor StateColor = Row.bCanClaim
		? Theme::AccentGold
		: (Row.bReached ? Theme::AccentBlue : Theme::TextMuted.CopyWithNewOpacity(0.62f));

	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.90f), X, Y, Width, Height);
	DrawRect(StateColor, X, Y, 4.0f * Scale, Height);
	DrawText(Row.MilestoneLabel.ToString(), Row.bCanClaim ? Theme::AccentGold : Theme::TextPrimary, X + 10.0f * Scale, Y + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.66f * Scale);
	DrawText(Row.ThresholdLabel.ToString(), Theme::TextMuted, X + 86.0f * Scale, Y + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.60f * Scale);
	DrawText(Row.RewardLabel.ToString(), Theme::TextMuted, X + 10.0f * Scale, Y + 24.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.58f * Scale);
	DrawText(Row.StatusLabel.ToString(), StateColor, X + 180.0f * Scale, Y + 24.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.58f * Scale);

	const float ButtonWidth = 58.0f * Scale;
	const float ButtonHeight = 24.0f * Scale;
	const float ButtonX = X + Width - ButtonWidth - 8.0f * Scale;
	const float ButtonY = Y + 10.0f * Scale;
	DrawRect(Row.bCanClaim ? Theme::AccentGold : Theme::BgPanel, ButtonX, ButtonY, ButtonWidth, ButtonHeight);
	DrawText(Row.ActionLabel.ToString(), Row.bCanClaim ? Theme::BgPrimary : Theme::TextMuted, ButtonX + 12.0f * Scale, ButtonY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);
	if (Row.bCanClaim)
	{
		AddHitBox(FVector2D(ButtonX, ButtonY), FVector2D(ButtonWidth, ButtonHeight), Row.ClaimHitBoxName, true, 86);
	}
}

void AIdleHUD::TryChallengeWeeklyBoss()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance)
	{
		return;
	}

	const FWeeklyBossChallengeResult Result = IdleGameInstance->TryChallengeWeeklyBoss();
	WeeklyBossFeedbackLabel = Result.bSuccess
		? FormatLocalizedUI(TEXT("WEEKLY_BOSS_CHALLENGE_RESULT_FORMAT"), [&Result](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Damage"), FText::FromString(FormatIntegerWithCommas(Result.DamageDealt)));
			Args.Add(TEXT("Reached"), FText::AsNumber(Result.ReachedMilestones));
		})
		: IdleProject::Localization::UI(TEXT("WEEKLY_BOSS_CHALLENGE_BLOCKED"));
	if (const UWorld* World = GetWorld())
	{
		WeeklyBossFeedbackStartTime = World->GetTimeSeconds();
	}
	RefreshMouseInteraction();
}

void AIdleHUD::ClaimWeeklyBossFromHitBox(FName BoxName)
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance)
	{
		return;
	}

	const int32 Milestone = WeeklyBossMilestoneFromHitBoxName(BoxName);
	const bool bClaimed = IdleGameInstance->ClaimWeeklyBossMilestone(Milestone);
	WeeklyBossFeedbackLabel = bClaimed
		? FormatLocalizedUI(TEXT("WEEKLY_BOSS_CLAIM_RESULT_FORMAT"), [Milestone](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Milestone"), FText::AsNumber(Milestone));
			Args.Add(TEXT("Gold"), FText::FromString(FormatIntegerWithCommas(FWeeklyBossFormula::MilestoneGoldReward(Milestone))));
			Args.Add(TEXT("Essence"), FText::FromString(FormatIntegerWithCommas(FWeeklyBossFormula::MilestoneEssenceReward(Milestone))));
		})
		: IdleProject::Localization::UI(TEXT("WEEKLY_BOSS_CLAIM_BLOCKED"));
	if (const UWorld* World = GetWorld())
	{
		WeeklyBossFeedbackStartTime = World->GetTimeSeconds();
	}
	RefreshMouseInteraction();
}

void AIdleHUD::DrawAttendancePanel()
{
	using namespace IdleProject::UI;

	if (!Canvas)
	{
		return;
	}
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	const UAttendanceService* AttendanceService = IdleGameInstance ? IdleGameInstance->GetAttendanceService() : nullptr;
	if (!AttendanceService)
	{
		return;
	}

	const FIdleHUDAttendancePanelViewModel ViewModel = BuildAttendancePanelViewModel(*AttendanceService, UQuestService::GetCurrentUtcDateString());
	const UWorld* World = GetWorld();
	const float FeedbackElapsed = World ? World->GetTimeSeconds() - AttendanceFeedbackStartTime : 0.0f;
	const bool bShowFeedback = !AttendanceFeedbackLabel.IsEmpty() && FeedbackElapsed <= AttendanceFeedbackDurationSeconds;

	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelWidth = FMath::Clamp(Canvas->SizeX * 0.24f, 360.0f * Scale, 460.0f * Scale);
	const float RowHeight = 44.0f * Scale;
	const float RowGap = 6.0f * Scale;
	const float Padding = 14.0f * Scale;
	const int32 VisibleRows = FMath::Min(ViewModel.Rows.Num(), 5);
	const float FeedbackHeight = bShowFeedback ? 24.0f * Scale : 0.0f;
	const float PanelHeight = 78.0f * Scale + VisibleRows * RowHeight + FMath::Max(0, VisibleRows - 1) * RowGap + Padding + FeedbackHeight;
	const float X = 28.0f * Scale;
	const float Y = 120.0f * Scale;
	const float Border = 2.0f * Scale;

	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.91f), X, Y, PanelWidth, PanelHeight);
	DrawRect(Theme::AccentGold, X, Y, PanelWidth, Border);
	DrawRect(Theme::AccentGold, X, Y + PanelHeight - Border, PanelWidth, Border);
	DrawRect(Theme::AccentGold, X, Y, Border, PanelHeight);
	DrawRect(Theme::AccentGold, X + PanelWidth - Border, Y, Border, PanelHeight);

	DrawText(ViewModel.Title.ToString(), Theme::TextPrimary, X + Padding, Y + 10.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 0.88f * Scale);
	DrawText(ViewModel.TotalLabel.ToString(), Theme::AccentGold, X + Padding, Y + 38.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.76f * Scale);
	DrawText(ViewModel.CheckInLabel.ToString(), ViewModel.bCheckedInToday ? Theme::AccentBlue : Theme::AccentGold, X + 180.0f * Scale, Y + 40.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.70f * Scale);
	DrawText(ViewModel.MilestoneSummaryLabel.ToString(), Theme::TextMuted, X + Padding, Y + 58.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.66f * Scale);

	float RowY = Y + 78.0f * Scale;
	for (int32 Index = 0; Index < VisibleRows; ++Index)
	{
		DrawAttendanceMilestoneRow(ViewModel.Rows[Index], X + Padding, RowY, PanelWidth - Padding * 2.0f, RowHeight);
		RowY += RowHeight + RowGap;
	}

	if (bShowFeedback)
	{
		DrawText(AttendanceFeedbackLabel.ToString(), Theme::AccentGold, X + Padding, RowY + 2.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.74f * Scale);
	}
	RefreshMouseInteraction();
}

void AIdleHUD::DrawAttendanceMilestoneRow(const FIdleHUDAttendanceMilestoneRowViewModel& Row, float X, float Y, float Width, float Height)
{
	using namespace IdleProject::UI;

	const float Scale = Height / 44.0f;
	const FLinearColor StateColor = Row.bCanClaim
		? Theme::AccentGold
		: (Row.bReached ? Theme::AccentBlue : Theme::TextMuted.CopyWithNewOpacity(0.62f));

	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.90f), X, Y, Width, Height);
	DrawRect(StateColor, X, Y, 4.0f * Scale, Height);
	DrawText(Row.MilestoneLabel.ToString(), Row.bCanClaim ? Theme::AccentGold : Theme::TextPrimary, X + 10.0f * Scale, Y + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.66f * Scale);
	DrawText(Row.ThresholdLabel.ToString(), Theme::TextMuted, X + 86.0f * Scale, Y + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.60f * Scale);
	DrawText(Row.RewardLabel.ToString(), Theme::TextMuted, X + 10.0f * Scale, Y + 24.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.58f * Scale);
	DrawText(Row.StatusLabel.ToString(), StateColor, X + 180.0f * Scale, Y + 24.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.58f * Scale);

	const float ButtonWidth = 58.0f * Scale;
	const float ButtonHeight = 24.0f * Scale;
	const float ButtonX = X + Width - ButtonWidth - 8.0f * Scale;
	const float ButtonY = Y + 10.0f * Scale;
	DrawRect(Row.bCanClaim ? Theme::AccentGold : Theme::BgPanel, ButtonX, ButtonY, ButtonWidth, ButtonHeight);
	DrawText(Row.ActionLabel.ToString(), Row.bCanClaim ? Theme::BgPrimary : Theme::TextMuted, ButtonX + 12.0f * Scale, ButtonY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);
	if (Row.bCanClaim)
	{
		AddHitBox(FVector2D(ButtonX, ButtonY), FVector2D(ButtonWidth, ButtonHeight), Row.ClaimHitBoxName, true, 86);
	}
}

void AIdleHUD::ClaimAttendanceFromHitBox(FName BoxName)
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance)
	{
		return;
	}

	const int32 Milestone = AttendanceMilestoneFromHitBoxName(BoxName);
	const UAttendanceService* AttendanceService = IdleGameInstance->GetAttendanceService();
	const FAttendanceMilestone Info = AttendanceService ? AttendanceService->GetMilestone(Milestone) : FAttendanceMilestone();
	const bool bClaimed = IdleGameInstance->ClaimAttendanceMilestone(Milestone);
	AttendanceFeedbackLabel = bClaimed
		? FormatLocalizedUI(TEXT("ATTENDANCE_CLAIM_RESULT_FORMAT"), [Milestone, &Info](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Milestone"), FText::AsNumber(Milestone));
			Args.Add(TEXT("Type"), AttendanceRewardTypeLabel(Info.RewardType));
			Args.Add(TEXT("Value"), FText::FromString(FormatIntegerWithCommas(Info.RewardValue)));
		})
		: IdleProject::Localization::UI(TEXT("ATTENDANCE_CLAIM_BLOCKED"));
	if (const UWorld* World = GetWorld())
	{
		AttendanceFeedbackStartTime = World->GetTimeSeconds();
	}
	RefreshMouseInteraction();
}

void AIdleHUD::DrawGuildPanel()
{
	using namespace IdleProject::UI;

	if (!Canvas)
	{
		return;
	}
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	const UGuildService* GuildService = IdleGameInstance ? IdleGameInstance->GetGuildService() : nullptr;
	if (!IdleGameInstance || !GuildService)
	{
		return;
	}

	const bool bOffline = IdleGameInstance->GetCloudSyncState() == ECloudSyncState::Offline;
	const FString PendingCreateName = GuildPresetCreateName(GuildCreateNamePresetIndex).ToString();
	const int64 PlayerGold = IdleGameInstance->GetGold();
	const int64 DonateAmount = GuildDonatePresetAmount(GuildDonatePresetIndex);
	// 내 길드 화면 진입 시 상점이 아직 현재 길드로 로드되지 않았으면 1회 fetch.
	if (GuildService->HasGuild() && !bOffline)
	{
		const FString CurrentGuildId = GuildService->GetCachedGuildId();
		if (!CurrentGuildId.IsEmpty() && GuildShopLoadedForId != CurrentGuildId && !bGuildShopLoading)
		{
			RefreshGuildShop();
		}
		// 주간 랭킹 탭 진입 시 1회 auto-fetch(로딩 가드, 캐시 재사용).
		if (bGuildRankingsView && !bGuildRankingsLoaded && !bGuildRankingsLoading)
		{
			RefreshGuildRankings();
		}
	}
	const FIdleHUDGuildPanelViewModel ViewModel = BuildGuildPanelViewModel(*GuildService, GuildBrowseList, PendingCreateName, bGuildBrowseLoading || bGuildActionPending, bOffline, PlayerGold, DonateAmount, GuildShopItems, bGuildRankingsView, bGuildRankingsLoading, GuildRankings, GuildMyRanking);

	const UWorld* World = GetWorld();
	const float FeedbackElapsed = World ? World->GetTimeSeconds() - GuildFeedbackStartTime : 0.0f;
	const bool bShowFeedback = !GuildFeedbackLabel.IsEmpty() && FeedbackElapsed <= GuildFeedbackDurationSeconds;

	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelWidth = FMath::Clamp(Canvas->SizeX * 0.24f, 360.0f * Scale, 460.0f * Scale);
	const float Padding = 14.0f * Scale;
	const float RowHeight = 34.0f * Scale;
	const float RowGap = 5.0f * Scale;
	const float ButtonHeight = 26.0f * Scale;
	const float Border = 2.0f * Scale;
	const float X = 28.0f * Scale;
	const float Y = 120.0f * Scale;

	// 화면 상태별 가시 행 수로 패널 높이 산출.
	float PanelHeight = 0.0f;
	if (!ViewModel.bHasGuild)
	{
		const int32 VisibleList = FMath::Min(ViewModel.ListRows.Num(), 6);
		PanelHeight = 84.0f * Scale
			+ FMath::Max(1, VisibleList) * (RowHeight + RowGap)
			+ 78.0f * Scale; // 생성 영역
	}
	else if (ViewModel.bRankingsView)
	{
		// 헤더(요약 2줄) + 탭 행 + 내 길드 순위 + 상위 랭킹 행.
		const int32 VisibleRanks = FMath::Min(ViewModel.RankingRows.Num(), 8);
		PanelHeight = 96.0f * Scale + ButtonHeight + RowGap; // 요약 + 탭 행
		PanelHeight += 48.0f * Scale;                        // 내 길드 순위 영역
		PanelHeight += 24.0f * Scale + FMath::Max(1, VisibleRanks) * (RowHeight + RowGap);
	}
	else
	{
		const int32 VisibleMembers = FMath::Min(ViewModel.MemberRows.Num(), 8);
		PanelHeight = 110.0f * Scale + VisibleMembers * (RowHeight + RowGap);
		// 탭 행 + G2 기여/버프 정보 4줄 + 출석/헌납 버튼 행 + 상점(제목+행).
		PanelHeight += ButtonHeight + RowGap + 132.0f * Scale + ButtonHeight + RowGap;
		const int32 VisibleShop = FMath::Min(ViewModel.ShopRows.Num(), 6);
		PanelHeight += 24.0f * Scale + FMath::Max(1, VisibleShop) * (RowHeight + RowGap);
		// 길드 보스 섹션(제목 + HP 바 + 정보 줄 + 도전/수령 버튼 행).
		PanelHeight += 96.0f * Scale + ButtonHeight + RowGap;
		if (ViewModel.bShowManage)
		{
			const int32 VisibleReq = FMath::Min(ViewModel.RequestRows.Num(), 4);
			PanelHeight += 64.0f * Scale + FMath::Max(1, VisibleReq) * (RowHeight + RowGap);
		}
	}
	PanelHeight += bShowFeedback ? 24.0f * Scale : 0.0f;
	PanelHeight += Padding;

	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.91f), X, Y, PanelWidth, PanelHeight);
	DrawRect(Theme::Auth, X, Y, PanelWidth, Border);
	DrawRect(Theme::Auth, X, Y + PanelHeight - Border, PanelWidth, Border);
	DrawRect(Theme::Auth, X, Y, Border, PanelHeight);
	DrawRect(Theme::Auth, X + PanelWidth - Border, Y, Border, PanelHeight);

	DrawText(ViewModel.Title.ToString(), Theme::TextPrimary, X + Padding, Y + 10.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 0.88f * Scale);

	// 새로고침(무소속 목록 갱신은 항상 가능 — 내 길드면 스냅샷 갱신 의미로 동일 버튼).
	const float RefreshWidth = 74.0f * Scale;
	const float RefreshX = X + PanelWidth - Padding - RefreshWidth;
	const float RefreshY = Y + 10.0f * Scale;
	DrawRect(Theme::AccentBlue, RefreshX, RefreshY, RefreshWidth, ButtonHeight);
	DrawText(ViewModel.RefreshLabel.ToString(), Theme::BgPrimary, RefreshX + 10.0f * Scale, RefreshY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.58f * Scale);
	AddHitBox(FVector2D(RefreshX, RefreshY), FVector2D(RefreshWidth, ButtonHeight), GuildRefreshListHitBoxName, true, 92);

	if (!ViewModel.StateLabel.IsEmpty())
	{
		DrawText(ViewModel.StateLabel.ToString(), ViewModel.bOffline ? Theme::AccentRed : Theme::AccentBlue, X + Padding, Y + 38.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.60f * Scale);
	}

	float CursorY = Y + 58.0f * Scale;

	if (!ViewModel.bHasGuild)
	{
		DrawText(ViewModel.NoneTitle.ToString(), Theme::AccentGold, X + Padding, CursorY, GEngine ? GEngine->GetSmallFont() : nullptr, 0.66f * Scale);
		CursorY += 24.0f * Scale;

		const int32 VisibleList = FMath::Min(ViewModel.ListRows.Num(), 6);
		if (VisibleList == 0)
		{
			DrawText(ViewModel.ListEmptyLabel.ToString(), Theme::TextMuted, X + Padding, CursorY + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);
			CursorY += RowHeight + RowGap;
		}
		for (int32 Index = 0; Index < VisibleList; ++Index)
		{
			DrawGuildListRow(ViewModel.ListRows[Index], X + Padding, CursorY, PanelWidth - Padding * 2.0f, RowHeight);
			CursorY += RowHeight + RowGap;
		}

		// 생성 영역.
		CursorY += 8.0f * Scale;
		DrawText(ViewModel.CreateTitle.ToString(), Theme::AccentGold, X + Padding, CursorY, GEngine ? GEngine->GetSmallFont() : nullptr, 0.64f * Scale);
		CursorY += 22.0f * Scale;
		DrawText(ViewModel.CreateNameLabel.ToString(), Theme::TextPrimary, X + Padding, CursorY + 4.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);

		const float CreateButtonWidth = 72.0f * Scale;
		const float CycleWidth = 84.0f * Scale;
		const float CreateX = X + PanelWidth - Padding - CreateButtonWidth;
		const float CycleX = CreateX - 8.0f * Scale - CycleWidth;
		const bool bCanCreate = !bGuildActionPending && !bOffline;
		DrawRect(Theme::BgPrimary, CycleX, CursorY, CycleWidth, ButtonHeight);
		DrawText(ViewModel.CreateNameCycleLabel.ToString(), Theme::TextMuted, CycleX + 8.0f * Scale, CursorY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.54f * Scale);
		AddHitBox(FVector2D(CycleX, CursorY), FVector2D(CycleWidth, ButtonHeight), GuildCreateNameCycleHitBoxName, true, 90);
		DrawRect(bCanCreate ? Theme::AccentGold : Theme::BgPrimary, CreateX, CursorY, CreateButtonWidth, ButtonHeight);
		DrawText(ViewModel.CreateLabel.ToString(), bCanCreate ? Theme::BgPrimary : Theme::TextMuted, CreateX + 12.0f * Scale, CursorY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.60f * Scale);
		if (bCanCreate)
		{
			AddHitBox(FVector2D(CreateX, CursorY), FVector2D(CreateButtonWidth, ButtonHeight), GuildCreateHitBoxName, true, 90);
		}
		CursorY += ButtonHeight + RowGap;
	}
	else
	{
		// 내 길드 요약.
		DrawText(ViewModel.GuildNameLabel.ToString(), Theme::AccentGold, X + Padding, CursorY, GEngine ? GEngine->GetMediumFont() : nullptr, 0.74f * Scale);
		DrawText(ViewModel.MyRankBadgeLabel.ToString(), Theme::Auth, X + PanelWidth - Padding - 96.0f * Scale, CursorY + 2.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.60f * Scale);
		CursorY += 24.0f * Scale;
		DrawText(ViewModel.SummaryLabel.ToString(), Theme::TextMuted, X + Padding, CursorY, GEngine ? GEngine->GetSmallFont() : nullptr, 0.60f * Scale);

		// 탈퇴 버튼.
		const float LeaveWidth = 64.0f * Scale;
		const float LeaveX = X + PanelWidth - Padding - LeaveWidth;
		const bool bCanLeave = !bGuildActionPending && !bOffline;
		DrawRect(bCanLeave ? Theme::AccentRed : Theme::BgPrimary, LeaveX, CursorY - 4.0f * Scale, LeaveWidth, ButtonHeight);
		DrawText(ViewModel.LeaveLabel.ToString(), bCanLeave ? Theme::TextPrimary : Theme::TextMuted, LeaveX + 14.0f * Scale, CursorY + 1.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.58f * Scale);
		if (bCanLeave)
		{
			AddHitBox(FVector2D(LeaveX, CursorY - 4.0f * Scale), FVector2D(LeaveWidth, ButtonHeight), GuildLeaveHitBoxName, true, 90);
		}
		CursorY += 26.0f * Scale;

		// ── 탭 토글(내 길드 / 주간 랭킹, PR-G3) ──
		const float TabGap = 6.0f * Scale;
		const float TabWidth = (PanelWidth - Padding * 2.0f - TabGap) * 0.5f;
		const float MyTabX = X + Padding;
		const float RankTabX = MyTabX + TabWidth + TabGap;
		const FLinearColor MyTabColor = !ViewModel.bRankingsView ? Theme::AccentGold : Theme::BgPrimary;
		const FLinearColor RankTabColor = ViewModel.bRankingsView ? Theme::AccentGold : Theme::BgPrimary;
		DrawRect(MyTabColor, MyTabX, CursorY, TabWidth, ButtonHeight);
		DrawRect(RankTabColor, RankTabX, CursorY, TabWidth, ButtonHeight);
		DrawText(ViewModel.MyTabLabel.ToString(), !ViewModel.bRankingsView ? Theme::BgPrimary : Theme::TextMuted, MyTabX + 10.0f * Scale, CursorY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.56f * Scale);
		DrawText(ViewModel.RankingsTabLabel.ToString(), ViewModel.bRankingsView ? Theme::BgPrimary : Theme::TextMuted, RankTabX + 10.0f * Scale, CursorY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.56f * Scale);
		AddHitBox(FVector2D(MyTabX, CursorY), FVector2D(TabWidth, ButtonHeight), GuildTabMyHitBoxName, true, 90);
		AddHitBox(FVector2D(RankTabX, CursorY), FVector2D(TabWidth, ButtonHeight), GuildTabRankingsHitBoxName, true, 90);
		CursorY += ButtonHeight + RowGap;

		if (ViewModel.bRankingsView)
		{
			// ── 주간 길드 랭킹 탭 ──
			DrawText(ViewModel.MyRankingTitle.ToString(), Theme::AccentGold, X + Padding, CursorY, GEngine ? GEngine->GetSmallFont() : nullptr, 0.60f * Scale);
			DrawText(ViewModel.MyRankingLabel.ToString(), Theme::TextPrimary, X + Padding + 96.0f * Scale, CursorY, GEngine ? GEngine->GetSmallFont() : nullptr, 0.58f * Scale);
			CursorY += 24.0f * Scale;

			DrawText(ViewModel.RankingsTitle.ToString(), Theme::AccentGold, X + Padding, CursorY, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);
			CursorY += 24.0f * Scale;

			const int32 VisibleRanks = FMath::Min(ViewModel.RankingRows.Num(), 8);
			if (VisibleRanks == 0)
			{
				const FText EmptyText = bGuildRankingsLoading ? ViewModel.RankingsLoadingLabel : ViewModel.RankingsEmptyLabel;
				DrawText(EmptyText.ToString(), Theme::TextMuted, X + Padding, CursorY + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.58f * Scale);
				CursorY += RowHeight + RowGap;
			}
			for (int32 Index = 0; Index < VisibleRanks; ++Index)
			{
				DrawGuildRankingRow(ViewModel.RankingRows[Index], X + Padding, CursorY, PanelWidth - Padding * 2.0f, RowHeight);
				CursorY += RowHeight + RowGap;
			}

			if (bShowFeedback)
			{
				DrawText(GuildFeedbackLabel.ToString(), bGuildFeedbackSuccess ? Theme::AccentGold : Theme::AccentRed, X + Padding, CursorY + 2.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);
			}
			RefreshMouseInteraction();
			return;
		}

		// ── 길드 레벨/EXP/버프/기여(PR-G2) ──
		DrawText(ViewModel.LevelLabel.ToString(), Theme::AccentBlue, X + Padding, CursorY, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);
		DrawText(ViewModel.ContributionLabel.ToString(), Theme::AccentGold, X + PanelWidth - Padding - 140.0f * Scale, CursorY, GEngine ? GEngine->GetSmallFont() : nullptr, 0.58f * Scale);
		CursorY += 18.0f * Scale;
		// EXP 진행 바.
		const float ExpBarWidth = PanelWidth - Padding * 2.0f;
		DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.9f), X + Padding, CursorY, ExpBarWidth, 6.0f * Scale);
		DrawRect(Theme::AccentBlue, X + Padding, CursorY, ExpBarWidth * FMath::Clamp(ViewModel.ExpProgressRatio, 0.0f, 1.0f), 6.0f * Scale);
		CursorY += 10.0f * Scale;
		DrawText(ViewModel.ExpLabel.ToString(), Theme::TextMuted, X + Padding, CursorY, GEngine ? GEngine->GetSmallFont() : nullptr, 0.52f * Scale);
		CursorY += 18.0f * Scale;
		DrawText(ViewModel.BuffLabel.ToString(), Theme::AccentGold, X + Padding, CursorY, GEngine ? GEngine->GetSmallFont() : nullptr, 0.56f * Scale);
		CursorY += 18.0f * Scale;
		DrawText(ViewModel.WeeklyContributionLabel.ToString(), Theme::TextMuted, X + Padding, CursorY, GEngine ? GEngine->GetSmallFont() : nullptr, 0.54f * Scale);
		CursorY += 22.0f * Scale;

		// ── 출석 / 헌납 액션 행 ──
		const bool bActionAllowed = !bGuildActionPending && !bOffline;
		const float AttendWidth = 86.0f * Scale;
		const bool bAttendEnabled = bActionAllowed && ViewModel.bCanAttend;
		DrawRect(bAttendEnabled ? Theme::AccentGold : Theme::BgPrimary, X + Padding, CursorY, AttendWidth, ButtonHeight);
		DrawText(ViewModel.AttendLabel.ToString(), bAttendEnabled ? Theme::BgPrimary : Theme::TextMuted, X + Padding + 8.0f * Scale, CursorY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.54f * Scale);
		if (bAttendEnabled)
		{
			AddHitBox(FVector2D(X + Padding, CursorY), FVector2D(AttendWidth, ButtonHeight), GuildAttendHitBoxName, true, 90);
		}

		const float DonateWidth = 104.0f * Scale;
		const float DonateCycleWidth = 56.0f * Scale;
		const float DonateCycleX = X + PanelWidth - Padding - DonateCycleWidth;
		const float DonateX = DonateCycleX - 6.0f * Scale - DonateWidth;
		const bool bDonateEnabled = bActionAllowed && ViewModel.bCanDonate;
		DrawRect(bDonateEnabled ? Theme::AccentBlue : Theme::BgPrimary, DonateX, CursorY, DonateWidth, ButtonHeight);
		DrawText(ViewModel.DonateLabel.ToString(), bDonateEnabled ? Theme::BgPrimary : Theme::TextMuted, DonateX + 6.0f * Scale, CursorY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.50f * Scale);
		if (bDonateEnabled)
		{
			AddHitBox(FVector2D(DonateX, CursorY), FVector2D(DonateWidth, ButtonHeight), GuildDonateHitBoxName, true, 90);
		}
		DrawRect(bActionAllowed ? Theme::BgPrimary : Theme::BgPanel, DonateCycleX, CursorY, DonateCycleWidth, ButtonHeight);
		DrawText(ViewModel.DonateCycleLabel.ToString(), Theme::TextMuted, DonateCycleX + 6.0f * Scale, CursorY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.48f * Scale);
		if (bActionAllowed)
		{
			AddHitBox(FVector2D(DonateCycleX, CursorY), FVector2D(DonateCycleWidth, ButtonHeight), GuildDonateCycleHitBoxName, true, 90);
		}
		CursorY += ButtonHeight + RowGap + 6.0f * Scale;

		// ── 길드 상점 ──
		DrawText(ViewModel.ShopTitle.ToString(), Theme::AccentGold, X + Padding, CursorY, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);
		CursorY += 22.0f * Scale;
		const int32 VisibleShop = FMath::Min(ViewModel.ShopRows.Num(), 6);
		if (VisibleShop == 0)
		{
			DrawText(ViewModel.ShopEmptyLabel.ToString(), Theme::TextMuted, X + Padding, CursorY + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.56f * Scale);
			CursorY += RowHeight + RowGap;
		}
		for (int32 Index = 0; Index < VisibleShop; ++Index)
		{
			DrawGuildShopRow(ViewModel.ShopRows[Index], X + Padding, CursorY, PanelWidth - Padding * 2.0f, RowHeight);
			CursorY += RowHeight + RowGap;
		}
		CursorY += 6.0f * Scale;

		// ── 길드 보스 섹션(공유 HP 풀, PR-G3) ──
		DrawText(ViewModel.BossTitle.ToString(), Theme::AccentRed, X + Padding, CursorY, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);
		DrawText(ViewModel.BossDefeatedLabel.ToString(), Theme::AccentGold, X + PanelWidth - Padding - 120.0f * Scale, CursorY, GEngine ? GEngine->GetSmallFont() : nullptr, 0.56f * Scale);
		CursorY += 20.0f * Scale;
		// 공유 HP 바(누적 데미지 / 보스 HP).
		const float BossBarWidth = PanelWidth - Padding * 2.0f;
		DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.9f), X + Padding, CursorY, BossBarWidth, 8.0f * Scale);
		DrawRect(Theme::AccentRed, X + Padding, CursorY, BossBarWidth * FMath::Clamp(ViewModel.BossHpRatio, 0.0f, 1.0f), 8.0f * Scale);
		CursorY += 12.0f * Scale;
		DrawText(ViewModel.BossHpLabel.ToString(), Theme::TextMuted, X + Padding, CursorY, GEngine ? GEngine->GetSmallFont() : nullptr, 0.52f * Scale);
		CursorY += 20.0f * Scale;
		// 도전 / 보상 수령 버튼 행.
		const bool bBossActionAllowed = !bGuildActionPending && !bOffline;
		const float BossChallengeWidth = 130.0f * Scale;
		const bool bChallengeEnabled = bBossActionAllowed && ViewModel.bCanChallengeBoss;
		DrawRect(bChallengeEnabled ? Theme::AccentRed : Theme::BgPrimary, X + Padding, CursorY, BossChallengeWidth, ButtonHeight);
		DrawText(ViewModel.BossChallengeLabel.ToString(), bChallengeEnabled ? Theme::TextPrimary : Theme::TextMuted, X + Padding + 8.0f * Scale, CursorY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.52f * Scale);
		if (bChallengeEnabled)
		{
			AddHitBox(FVector2D(X + Padding, CursorY), FVector2D(BossChallengeWidth, ButtonHeight), GuildBossChallengeHitBoxName, true, 90);
		}
		const float BossClaimWidth = 120.0f * Scale;
		const float BossClaimX = X + PanelWidth - Padding - BossClaimWidth;
		const bool bClaimEnabled = bBossActionAllowed && ViewModel.bCanClaimBoss;
		DrawRect(bClaimEnabled ? Theme::AccentGold : Theme::BgPrimary, BossClaimX, CursorY, BossClaimWidth, ButtonHeight);
		DrawText(ViewModel.BossClaimLabel.ToString(), bClaimEnabled ? Theme::BgPrimary : Theme::TextMuted, BossClaimX + 8.0f * Scale, CursorY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.52f * Scale);
		if (bClaimEnabled)
		{
			AddHitBox(FVector2D(BossClaimX, CursorY), FVector2D(BossClaimWidth, ButtonHeight), GuildBossClaimHitBoxName, true, 90);
		}
		CursorY += ButtonHeight + RowGap + 6.0f * Scale;

		DrawText(ViewModel.MemberListTitle.ToString(), Theme::AccentGold, X + Padding, CursorY, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);
		CursorY += 22.0f * Scale;
		const int32 VisibleMembers = FMath::Min(ViewModel.MemberRows.Num(), 8);
		for (int32 Index = 0; Index < VisibleMembers; ++Index)
		{
			DrawGuildMemberRow(ViewModel.MemberRows[Index], X + Padding, CursorY, PanelWidth - Padding * 2.0f, RowHeight);
			CursorY += RowHeight + RowGap;
		}

		// 길드장 관리.
		if (ViewModel.bShowManage)
		{
			CursorY += 8.0f * Scale;
			DrawText(ViewModel.ManageTitle.ToString(), Theme::Auth, X + Padding, CursorY, GEngine ? GEngine->GetSmallFont() : nullptr, 0.64f * Scale);

			const float ToggleWidth = 110.0f * Scale;
			const float ToggleX = X + PanelWidth - Padding - ToggleWidth;
			const bool bCanToggle = !bGuildActionPending && !bOffline;
			DrawRect(bCanToggle ? Theme::AccentBlue : Theme::BgPrimary, ToggleX, CursorY - 4.0f * Scale, ToggleWidth, ButtonHeight);
			DrawText(ViewModel.ToggleJoinModeLabel.ToString(), bCanToggle ? Theme::BgPrimary : Theme::TextMuted, ToggleX + 8.0f * Scale, CursorY + 1.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.52f * Scale);
			if (bCanToggle)
			{
				AddHitBox(FVector2D(ToggleX, CursorY - 4.0f * Scale), FVector2D(ToggleWidth, ButtonHeight), GuildToggleJoinModeHitBoxName, true, 90);
			}
			CursorY += 24.0f * Scale;

			DrawText(ViewModel.RequestsTitle.ToString(), Theme::TextMuted, X + Padding, CursorY, GEngine ? GEngine->GetSmallFont() : nullptr, 0.58f * Scale);
			CursorY += 20.0f * Scale;
			const int32 VisibleReq = FMath::Min(ViewModel.RequestRows.Num(), 4);
			if (VisibleReq == 0)
			{
				DrawText(ViewModel.RequestsEmptyLabel.ToString(), Theme::TextMuted.CopyWithNewOpacity(0.7f), X + Padding, CursorY + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.56f * Scale);
				CursorY += RowHeight + RowGap;
			}
			for (int32 Index = 0; Index < VisibleReq; ++Index)
			{
				DrawGuildRequestRow(ViewModel.RequestRows[Index], X + Padding, CursorY, PanelWidth - Padding * 2.0f, RowHeight);
				CursorY += RowHeight + RowGap;
			}
		}
	}

	if (bShowFeedback)
	{
		DrawText(GuildFeedbackLabel.ToString(), bGuildFeedbackSuccess ? Theme::AccentGold : Theme::AccentRed, X + Padding, CursorY + 2.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);
	}

	RefreshMouseInteraction();
}

void AIdleHUD::DrawGuildListRow(const FIdleHUDGuildListRowViewModel& Row, float X, float Y, float Width, float Height)
{
	using namespace IdleProject::UI;

	const float Scale = Height / 34.0f;
	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.88f), X, Y, Width, Height);
	DrawRect(Row.bApproval ? Theme::AccentBlue : Theme::AccentGold, X, Y, 4.0f * Scale, Height);
	DrawText(Row.NameLabel.ToString(), Theme::TextPrimary, X + 10.0f * Scale, Y + 4.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.60f * Scale);
	DrawText(Row.InfoLabel.ToString(), Theme::TextMuted, X + 10.0f * Scale, Y + 18.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.52f * Scale);

	const float ButtonWidth = 60.0f * Scale;
	const float ButtonHeight = 24.0f * Scale;
	const float ButtonX = X + Width - ButtonWidth - 6.0f * Scale;
	const float ButtonY = Y + (Height - ButtonHeight) * 0.5f;
	const bool bEnabled = !bGuildActionPending;
	DrawRect(bEnabled ? Theme::AccentGold : Theme::BgPanel, ButtonX, ButtonY, ButtonWidth, ButtonHeight);
	DrawText(Row.JoinLabel.ToString(), bEnabled ? Theme::BgPrimary : Theme::TextMuted, ButtonX + 12.0f * Scale, ButtonY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.56f * Scale);
	if (bEnabled)
	{
		AddHitBox(FVector2D(ButtonX, ButtonY), FVector2D(ButtonWidth, ButtonHeight), Row.JoinHitBoxName, true, 91);
	}
}

void AIdleHUD::DrawGuildMemberRow(const FIdleHUDGuildMemberRowViewModel& Row, float X, float Y, float Width, float Height)
{
	using namespace IdleProject::UI;

	const float Scale = Height / 34.0f;
	const FLinearColor RankColor = Row.Rank == EGuildRank::Master
		? Theme::AccentGold
		: (Row.Rank == EGuildRank::Vice ? Theme::Auth : (Row.Rank == EGuildRank::Officer ? Theme::AccentBlue : Theme::TextMuted));
	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.86f), X, Y, Width, Height);
	DrawRect(RankColor, X, Y, 4.0f * Scale, Height);
	DrawText(Row.NicknameLabel.ToString(), Theme::TextPrimary, X + 10.0f * Scale, Y + 9.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.58f * Scale);
	DrawText(Row.RankBadgeLabel.ToString(), RankColor, X + 130.0f * Scale, Y + 9.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.54f * Scale);

	// 길드장 관리 버튼(승급/강등).
	const float BtnWidth = 52.0f * Scale;
	const float BtnHeight = 22.0f * Scale;
	const float BtnY = Y + (Height - BtnHeight) * 0.5f;
	float BtnX = X + Width - BtnWidth - 6.0f * Scale;

	if (Row.bShowDemote)
	{
		const bool bEnabled = !bGuildActionPending;
		DrawRect(bEnabled ? Theme::AccentRed : Theme::BgPanel, BtnX, BtnY, BtnWidth, BtnHeight);
		DrawText(IdleProject::Localization::UI(TEXT("GUILD_DEMOTE")).ToString(), bEnabled ? Theme::TextPrimary : Theme::TextMuted, BtnX + 8.0f * Scale, BtnY + 4.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.50f * Scale);
		if (bEnabled)
		{
			AddHitBox(FVector2D(BtnX, BtnY), FVector2D(BtnWidth, BtnHeight), Row.DemoteHitBoxName, true, 91);
		}
		BtnX -= BtnWidth + 6.0f * Scale;
	}

	if (Row.bShowPromote)
	{
		const bool bEnabled = Row.bCanPromote && !bGuildActionPending;
		DrawRect(bEnabled ? Theme::AccentGold : Theme::BgPanel, BtnX, BtnY, BtnWidth, BtnHeight);
		DrawText(Row.PromoteLabel.ToString(), bEnabled ? Theme::BgPrimary : Theme::TextMuted, BtnX + 6.0f * Scale, BtnY + 4.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.46f * Scale);
		if (bEnabled)
		{
			AddHitBox(FVector2D(BtnX, BtnY), FVector2D(BtnWidth, BtnHeight), Row.PromoteHitBoxName, true, 91);
		}
	}
}

void AIdleHUD::DrawGuildRequestRow(const FIdleHUDGuildRequestRowViewModel& Row, float X, float Y, float Width, float Height)
{
	using namespace IdleProject::UI;

	const float Scale = Height / 34.0f;
	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.86f), X, Y, Width, Height);
	DrawRect(Theme::AccentBlue, X, Y, 4.0f * Scale, Height);
	DrawText(Row.CharacterLabel.ToString(), Theme::TextPrimary, X + 10.0f * Scale, Y + 9.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.56f * Scale);

	const float BtnWidth = 52.0f * Scale;
	const float BtnHeight = 22.0f * Scale;
	const float BtnY = Y + (Height - BtnHeight) * 0.5f;
	const float RejectX = X + Width - BtnWidth - 6.0f * Scale;
	const float ApproveX = RejectX - BtnWidth - 6.0f * Scale;
	const bool bEnabled = !bGuildActionPending;

	DrawRect(bEnabled ? Theme::AccentGold : Theme::BgPanel, ApproveX, BtnY, BtnWidth, BtnHeight);
	DrawText(IdleProject::Localization::UI(TEXT("GUILD_APPROVE")).ToString(), bEnabled ? Theme::BgPrimary : Theme::TextMuted, ApproveX + 8.0f * Scale, BtnY + 4.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.50f * Scale);
	DrawRect(bEnabled ? Theme::AccentRed : Theme::BgPanel, RejectX, BtnY, BtnWidth, BtnHeight);
	DrawText(IdleProject::Localization::UI(TEXT("GUILD_REJECT")).ToString(), bEnabled ? Theme::TextPrimary : Theme::TextMuted, RejectX + 8.0f * Scale, BtnY + 4.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.50f * Scale);
	if (bEnabled)
	{
		AddHitBox(FVector2D(ApproveX, BtnY), FVector2D(BtnWidth, BtnHeight), Row.ApproveHitBoxName, true, 91);
		AddHitBox(FVector2D(RejectX, BtnY), FVector2D(BtnWidth, BtnHeight), Row.RejectHitBoxName, true, 91);
	}
}

void AIdleHUD::DrawGuildShopRow(const FIdleHUDGuildShopRowViewModel& Row, float X, float Y, float Width, float Height)
{
	using namespace IdleProject::UI;

	const float Scale = Height / 34.0f;
	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.88f), X, Y, Width, Height);
	DrawRect(Theme::AccentGold, X, Y, 4.0f * Scale, Height);
	DrawText(Row.NameLabel.ToString(), Theme::TextPrimary, X + 10.0f * Scale, Y + 4.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.58f * Scale);
	DrawText(Row.PriceLabel.ToString(), Theme::TextMuted, X + 10.0f * Scale, Y + 18.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.52f * Scale);

	const float BtnWidth = 64.0f * Scale;
	const float BtnHeight = 24.0f * Scale;
	const float BtnX = X + Width - BtnWidth - 6.0f * Scale;
	const float BtnY = Y + (Height - BtnHeight) * 0.5f;
	const bool bEnabled = Row.bCanBuy && !bGuildActionPending;
	DrawRect(bEnabled ? Theme::AccentGold : Theme::BgPanel, BtnX, BtnY, BtnWidth, BtnHeight);
	DrawText(Row.BuyLabel.ToString(), bEnabled ? Theme::BgPrimary : Theme::TextMuted, BtnX + 8.0f * Scale, BtnY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.50f * Scale);
	if (bEnabled)
	{
		AddHitBox(FVector2D(BtnX, BtnY), FVector2D(BtnWidth, BtnHeight), Row.BuyHitBoxName, true, 91);
	}
}

void AIdleHUD::SetGuildFeedback(const TCHAR* Key, bool bSuccess)
{
	GuildFeedbackLabel = IdleProject::Localization::UI(Key);
	bGuildFeedbackSuccess = bSuccess;
	if (const UWorld* World = GetWorld())
	{
		GuildFeedbackStartTime = World->GetTimeSeconds();
	}
	RefreshMouseInteraction();
}

void AIdleHUD::RefreshGuildBrowseList()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance)
	{
		return;
	}

	// 내 길드 상태면 스냅샷만 갱신(목록 불필요), 무소속이면 목록 조회.
	const UGuildService* GuildService = IdleGameInstance->GetGuildService();
	if (GuildService && GuildService->HasGuild())
	{
		IdleGameInstance->RefreshGuildSnapshot();
		RefreshMouseInteraction();
		return;
	}

	bGuildBrowseLoading = true;
	TWeakObjectPtr<AIdleHUD> WeakThis(this);
	IdleGameInstance->GuildPanelRefreshList(FString(), [WeakThis](bool bSuccess, const TArray<FGuildSummary>& Summaries)
	{
		if (AIdleHUD* StrongThis = WeakThis.Get())
		{
			StrongThis->bGuildBrowseLoading = false;
			if (bSuccess)
			{
				StrongThis->GuildBrowseList = Summaries;
			}
			StrongThis->RefreshMouseInteraction();
		}
	});
	RefreshMouseInteraction();
}

void AIdleHUD::RefreshGuildShop()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	const UGuildService* GuildService = IdleGameInstance ? IdleGameInstance->GetGuildService() : nullptr;
	if (!IdleGameInstance || !GuildService || !GuildService->HasGuild() || bGuildShopLoading)
	{
		return;
	}

	const FString TargetGuildId = GuildService->GetCachedGuildId();
	bGuildShopLoading = true;
	TWeakObjectPtr<AIdleHUD> WeakThis(this);
	IdleGameInstance->GuildPanelFetchShop([WeakThis, TargetGuildId](bool bSuccess, const TArray<FGuildShopItemInfo>& Items)
	{
		if (AIdleHUD* StrongThis = WeakThis.Get())
		{
			StrongThis->bGuildShopLoading = false;
			if (bSuccess)
			{
				StrongThis->GuildShopItems = Items;
				StrongThis->GuildShopLoadedForId = TargetGuildId;
			}
			StrongThis->RefreshMouseInteraction();
		}
	});
	RefreshMouseInteraction();
}

void AIdleHUD::CycleGuildCreateName()
{
	GuildCreateNamePresetIndex = (GuildCreateNamePresetIndex + 1) % GuildCreateNamePresetCount;
	RefreshMouseInteraction();
}

void AIdleHUD::TryCreateGuild()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance || bGuildActionPending)
	{
		return;
	}

	const FString Name = GuildPresetCreateName(GuildCreateNamePresetIndex).ToString();
	if (Name.IsEmpty())
	{
		return;
	}

	bGuildActionPending = true;
	TWeakObjectPtr<AIdleHUD> WeakThis(this);
	IdleGameInstance->GuildPanelCreate(Name, [WeakThis](bool bSuccess)
	{
		if (AIdleHUD* StrongThis = WeakThis.Get())
		{
			StrongThis->bGuildActionPending = false;
			StrongThis->SetGuildFeedback(bSuccess ? TEXT("GUILD_FEEDBACK_CREATED") : TEXT("GUILD_FEEDBACK_FAILED"), bSuccess);
		}
	});
	RefreshMouseInteraction();
}

void AIdleHUD::JoinGuildFromHitBox(FName BoxName)
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance || bGuildActionPending)
	{
		return;
	}

	const FString GuildId = BoxName.ToString().RightChop(GuildJoinHitBoxPrefix.Len());
	if (GuildId.IsEmpty())
	{
		return;
	}

	// 가입 모드 판정(목록 캐시에서 조회 — 승인제면 신청 피드백).
	bool bApproval = false;
	for (const FGuildSummary& Summary : GuildBrowseList)
	{
		if (Summary.Id == GuildId)
		{
			bApproval = Summary.JoinMode == EGuildJoinMode::Approval;
			break;
		}
	}

	bGuildActionPending = true;
	TWeakObjectPtr<AIdleHUD> WeakThis(this);
	IdleGameInstance->GuildPanelJoin(GuildId, [WeakThis, bApproval](bool bSuccess)
	{
		if (AIdleHUD* StrongThis = WeakThis.Get())
		{
			StrongThis->bGuildActionPending = false;
			const TCHAR* Key = !bSuccess
				? TEXT("GUILD_FEEDBACK_FAILED")
				: (bApproval ? TEXT("GUILD_FEEDBACK_REQUESTED") : TEXT("GUILD_FEEDBACK_JOINED"));
			StrongThis->SetGuildFeedback(Key, bSuccess);
		}
	});
	RefreshMouseInteraction();
}

void AIdleHUD::TryLeaveGuild()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	const UGuildService* GuildService = IdleGameInstance ? IdleGameInstance->GetGuildService() : nullptr;
	if (!IdleGameInstance || !GuildService || bGuildActionPending)
	{
		return;
	}

	const FString GuildId = GuildService->GetCachedGuildId();
	if (GuildId.IsEmpty())
	{
		return;
	}

	bGuildActionPending = true;
	TWeakObjectPtr<AIdleHUD> WeakThis(this);
	IdleGameInstance->GuildPanelLeave(GuildId, [WeakThis](bool bSuccess)
	{
		if (AIdleHUD* StrongThis = WeakThis.Get())
		{
			StrongThis->bGuildActionPending = false;
			if (bSuccess)
			{
				// 탈퇴 시 상점 캐시 무효화(재가입 시 재조회).
				StrongThis->GuildShopItems.Reset();
				StrongThis->GuildShopLoadedForId.Reset();
			}
			StrongThis->SetGuildFeedback(bSuccess ? TEXT("GUILD_FEEDBACK_LEFT") : TEXT("GUILD_FEEDBACK_FAILED"), bSuccess);
		}
	});
	RefreshMouseInteraction();
}

void AIdleHUD::ToggleGuildJoinMode()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	const UGuildService* GuildService = IdleGameInstance ? IdleGameInstance->GetGuildService() : nullptr;
	if (!IdleGameInstance || !GuildService || bGuildActionPending)
	{
		return;
	}

	const FString GuildId = GuildService->GetCachedGuildId();
	if (GuildId.IsEmpty())
	{
		return;
	}

	const EGuildJoinMode Current = GuildService->GetGuildSummary().JoinMode;
	const EGuildJoinMode Next = Current == EGuildJoinMode::Open ? EGuildJoinMode::Approval : EGuildJoinMode::Open;

	bGuildActionPending = true;
	TWeakObjectPtr<AIdleHUD> WeakThis(this);
	IdleGameInstance->GuildPanelUpdateJoinMode(GuildId, Next, [WeakThis](bool bSuccess)
	{
		if (AIdleHUD* StrongThis = WeakThis.Get())
		{
			StrongThis->bGuildActionPending = false;
			StrongThis->SetGuildFeedback(bSuccess ? TEXT("GUILD_FEEDBACK_UPDATED") : TEXT("GUILD_FEEDBACK_FAILED"), bSuccess);
		}
	});
	RefreshMouseInteraction();
}

void AIdleHUD::ApproveGuildRequestFromHitBox(FName BoxName)
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	const UGuildService* GuildService = IdleGameInstance ? IdleGameInstance->GetGuildService() : nullptr;
	if (!IdleGameInstance || !GuildService || bGuildActionPending)
	{
		return;
	}

	const FString GuildId = GuildService->GetCachedGuildId();
	const FString TargetCharacterId = BoxName.ToString().RightChop(GuildApproveHitBoxPrefix.Len());
	if (GuildId.IsEmpty() || TargetCharacterId.IsEmpty())
	{
		return;
	}

	bGuildActionPending = true;
	TWeakObjectPtr<AIdleHUD> WeakThis(this);
	IdleGameInstance->GuildPanelApprove(GuildId, TargetCharacterId, [WeakThis](bool bSuccess)
	{
		if (AIdleHUD* StrongThis = WeakThis.Get())
		{
			StrongThis->bGuildActionPending = false;
			StrongThis->SetGuildFeedback(bSuccess ? TEXT("GUILD_FEEDBACK_APPROVED") : TEXT("GUILD_FEEDBACK_FAILED"), bSuccess);
		}
	});
	RefreshMouseInteraction();
}

void AIdleHUD::RejectGuildRequestFromHitBox(FName BoxName)
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	const UGuildService* GuildService = IdleGameInstance ? IdleGameInstance->GetGuildService() : nullptr;
	if (!IdleGameInstance || !GuildService || bGuildActionPending)
	{
		return;
	}

	const FString GuildId = GuildService->GetCachedGuildId();
	const FString TargetCharacterId = BoxName.ToString().RightChop(GuildRejectHitBoxPrefix.Len());
	if (GuildId.IsEmpty() || TargetCharacterId.IsEmpty())
	{
		return;
	}

	bGuildActionPending = true;
	TWeakObjectPtr<AIdleHUD> WeakThis(this);
	IdleGameInstance->GuildPanelReject(GuildId, TargetCharacterId, [WeakThis](bool bSuccess)
	{
		if (AIdleHUD* StrongThis = WeakThis.Get())
		{
			StrongThis->bGuildActionPending = false;
			StrongThis->SetGuildFeedback(bSuccess ? TEXT("GUILD_FEEDBACK_REJECTED") : TEXT("GUILD_FEEDBACK_FAILED"), bSuccess);
		}
	});
	RefreshMouseInteraction();
}

void AIdleHUD::SetGuildMemberRankFromHitBox(FName BoxName, bool bPromote)
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	const UGuildService* GuildService = IdleGameInstance ? IdleGameInstance->GetGuildService() : nullptr;
	if (!IdleGameInstance || !GuildService || bGuildActionPending)
	{
		return;
	}

	const FString GuildId = GuildService->GetCachedGuildId();
	const FString& Prefix = bPromote ? GuildPromoteHitBoxPrefix : GuildDemoteHitBoxPrefix;
	const FString TargetCharacterId = BoxName.ToString().RightChop(Prefix.Len());
	if (GuildId.IsEmpty() || TargetCharacterId.IsEmpty())
	{
		return;
	}

	// 대상의 현재 계급에서 다음 계급 산출(승급: Member→Officer→Vice / 강등: 모두→Member).
	EGuildRank CurrentRank = EGuildRank::Member;
	for (const FGuildMemberInfo& Member : GuildService->GetMembers())
	{
		if (Member.CharacterId == TargetCharacterId)
		{
			CurrentRank = Member.Rank;
			break;
		}
	}

	EGuildRank NewRank = EGuildRank::Member;
	if (bPromote)
	{
		NewRank = CurrentRank == EGuildRank::Member ? EGuildRank::Officer : EGuildRank::Vice;
	}
	else
	{
		NewRank = EGuildRank::Member;
	}

	bGuildActionPending = true;
	TWeakObjectPtr<AIdleHUD> WeakThis(this);
	IdleGameInstance->GuildPanelSetRank(GuildId, TargetCharacterId, NewRank, [WeakThis](bool bSuccess)
	{
		if (AIdleHUD* StrongThis = WeakThis.Get())
		{
			StrongThis->bGuildActionPending = false;
			StrongThis->SetGuildFeedback(bSuccess ? TEXT("GUILD_FEEDBACK_RANK_CHANGED") : TEXT("GUILD_FEEDBACK_FAILED"), bSuccess);
		}
	});
	RefreshMouseInteraction();
}

void AIdleHUD::TryGuildAttendance()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	const UGuildService* GuildService = IdleGameInstance ? IdleGameInstance->GetGuildService() : nullptr;
	if (!IdleGameInstance || !GuildService || bGuildActionPending)
	{
		return;
	}
	if (!GuildService->HasGuild() || !GuildService->GetSnapshot().bCanAttendToday)
	{
		return;
	}

	bGuildActionPending = true;
	TWeakObjectPtr<AIdleHUD> WeakThis(this);
	IdleGameInstance->GuildPanelAttendance([WeakThis](bool bSuccess)
	{
		if (AIdleHUD* StrongThis = WeakThis.Get())
		{
			StrongThis->bGuildActionPending = false;
			StrongThis->SetGuildFeedback(bSuccess ? TEXT("GUILD_FEEDBACK_ATTENDED") : TEXT("GUILD_FEEDBACK_FAILED"), bSuccess);
		}
	});
	RefreshMouseInteraction();
}

void AIdleHUD::CycleGuildDonateAmount()
{
	using namespace IdleProject::UI;
	GuildDonatePresetIndex = (GuildDonatePresetIndex + 1) % GuildDonatePresetCount;
	RefreshMouseInteraction();
}

void AIdleHUD::TryGuildDonate()
{
	using namespace IdleProject::UI;
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	const UGuildService* GuildService = IdleGameInstance ? IdleGameInstance->GetGuildService() : nullptr;
	if (!IdleGameInstance || !GuildService || bGuildActionPending)
	{
		return;
	}

	const int64 Amount = GuildDonatePresetAmount(GuildDonatePresetIndex);
	// 보유 골드/일일 상한 가드(UI 표시와 일치).
	if (!GuildService->HasGuild() || !GuildService->GetSnapshot().bCanDonateToday || Amount <= 0 || IdleGameInstance->GetGold() < Amount)
	{
		return;
	}

	bGuildActionPending = true;
	TWeakObjectPtr<AIdleHUD> WeakThis(this);
	IdleGameInstance->GuildPanelDonate(Amount, [WeakThis](bool bSuccess)
	{
		if (AIdleHUD* StrongThis = WeakThis.Get())
		{
			StrongThis->bGuildActionPending = false;
			StrongThis->SetGuildFeedback(bSuccess ? TEXT("GUILD_FEEDBACK_DONATED") : TEXT("GUILD_FEEDBACK_FAILED"), bSuccess);
		}
	});
	RefreshMouseInteraction();
}

void AIdleHUD::BuyGuildShopItemFromHitBox(FName BoxName)
{
	using namespace IdleProject::UI;
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	const UGuildService* GuildService = IdleGameInstance ? IdleGameInstance->GetGuildService() : nullptr;
	if (!IdleGameInstance || !GuildService || bGuildActionPending)
	{
		return;
	}

	const FString ItemId = BoxName.ToString().RightChop(GuildShopBuyHitBoxPrefix.Len());
	if (!GuildService->HasGuild() || ItemId.IsEmpty())
	{
		return;
	}

	bGuildActionPending = true;
	TWeakObjectPtr<AIdleHUD> WeakThis(this);
	IdleGameInstance->GuildPanelBuyShopItem(ItemId, [WeakThis](bool bSuccess)
	{
		if (AIdleHUD* StrongThis = WeakThis.Get())
		{
			StrongThis->bGuildActionPending = false;
			StrongThis->SetGuildFeedback(bSuccess ? TEXT("GUILD_FEEDBACK_PURCHASED") : TEXT("GUILD_FEEDBACK_FAILED"), bSuccess);
		}
	});
	RefreshMouseInteraction();
}

void AIdleHUD::DrawGuildRankingRow(const FIdleHUDGuildRankingRowViewModel& Row, float X, float Y, float Width, float Height)
{
	using namespace IdleProject::UI;

	const float Scale = Height / 34.0f;
	const FLinearColor AccentColor = Row.bSelf ? Theme::AccentGold : Theme::TextMuted.CopyWithNewOpacity(0.52f);
	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(Row.bSelf ? 0.96f : 0.86f), X, Y, Width, Height);
	DrawRect(AccentColor, X, Y, 4.0f * Scale, Height);
	DrawText(Row.RankLabel.ToString(), Row.bSelf ? Theme::AccentGold : Theme::TextPrimary, X + 10.0f * Scale, Y + 4.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.58f * Scale);
	DrawText(Row.NameLabel.ToString(), Row.bSelf ? Theme::AccentGold : Theme::TextPrimary, X + 48.0f * Scale, Y + 4.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.56f * Scale);
	DrawText(Row.InfoLabel.ToString(), Theme::TextMuted, X + 10.0f * Scale, Y + 18.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.50f * Scale);
	DrawText(Row.ContributionLabel.ToString(), Row.bSelf ? Theme::AccentGold : Theme::AccentBlue, X + Width - 132.0f * Scale, Y + 18.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.50f * Scale);
}

void AIdleHUD::SetGuildRankingsView(bool bRankings)
{
	if (bGuildRankingsView == bRankings)
	{
		return;
	}
	bGuildRankingsView = bRankings;
	// 랭킹 탭 최초 진입 시 1회 fetch(가드는 DrawGuildPanel 의 auto-fetch 가 처리).
	RefreshMouseInteraction();
}

void AIdleHUD::RefreshGuildRankings()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	const UGuildService* GuildService = IdleGameInstance ? IdleGameInstance->GetGuildService() : nullptr;
	if (!IdleGameInstance || !GuildService || !GuildService->HasGuild() || bGuildRankingsLoading)
	{
		return;
	}

	bGuildRankingsLoading = true;
	TWeakObjectPtr<AIdleHUD> WeakThis(this);
	IdleGameInstance->GuildPanelFetchRankings(10, [WeakThis](bool bSuccess, const TArray<FGuildRankingEntry>& Rankings, const FGuildRankingEntry& MyRanking)
	{
		if (AIdleHUD* StrongThis = WeakThis.Get())
		{
			StrongThis->bGuildRankingsLoading = false;
			if (bSuccess)
			{
				StrongThis->GuildRankings = Rankings;
				StrongThis->GuildMyRanking = MyRanking;
				StrongThis->bGuildRankingsLoaded = true;
			}
			StrongThis->RefreshMouseInteraction();
		}
	});
	RefreshMouseInteraction();
}

void AIdleHUD::TryChallengeGuildBoss()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	const UGuildService* GuildService = IdleGameInstance ? IdleGameInstance->GetGuildService() : nullptr;
	if (!IdleGameInstance || !GuildService || bGuildActionPending)
	{
		return;
	}
	// UI 표시(잔여 도전 > 0)와 일치하는 가드.
	if (!GuildService->HasGuild() || GuildService->GetBossChallengesRemaining() <= 0)
	{
		return;
	}

	bGuildActionPending = true;
	TWeakObjectPtr<AIdleHUD> WeakThis(this);
	IdleGameInstance->GuildPanelChallengeBoss([WeakThis](bool bSuccess)
	{
		if (AIdleHUD* StrongThis = WeakThis.Get())
		{
			StrongThis->bGuildActionPending = false;
			// 도전 후 기여/랭킹이 변동되므로 캐시 무효화(다음 탭 진입 시 재조회).
			StrongThis->bGuildRankingsLoaded = false;
			StrongThis->SetGuildFeedback(bSuccess ? TEXT("GUILD_FEEDBACK_BOSS_CHALLENGED") : TEXT("GUILD_FEEDBACK_FAILED"), bSuccess);
		}
	});
	RefreshMouseInteraction();
}

void AIdleHUD::TryClaimGuildBossReward()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	const UGuildService* GuildService = IdleGameInstance ? IdleGameInstance->GetGuildService() : nullptr;
	if (!IdleGameInstance || !GuildService || bGuildActionPending)
	{
		return;
	}
	// UI 표시(미수령 격파 > 0)와 일치하는 가드.
	if (!GuildService->HasGuild() || GuildService->GetBossUnclaimedDefeats() <= 0)
	{
		return;
	}

	bGuildActionPending = true;
	TWeakObjectPtr<AIdleHUD> WeakThis(this);
	IdleGameInstance->GuildPanelClaimBossReward([WeakThis](bool bSuccess)
	{
		if (AIdleHUD* StrongThis = WeakThis.Get())
		{
			StrongThis->bGuildActionPending = false;
			StrongThis->SetGuildFeedback(bSuccess ? TEXT("GUILD_FEEDBACK_BOSS_CLAIMED") : TEXT("GUILD_FEEDBACK_FAILED"), bSuccess);
		}
	});
	RefreshMouseInteraction();
}

void AIdleHUD::DrawAchievementPanel()
{
	using namespace IdleProject::UI;

	if (!Canvas)
	{
		return;
	}
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	BindAchievementService();
	const UAchievementService* AchievementService = BoundAchievementService.Get();
	if (!AchievementService)
	{
		return;
	}

	const FIdleHUDAchievementViewModel ViewModel = BuildAchievementViewModel(*AchievementService);
	const UWorld* World = GetWorld();
	const float FeedbackElapsed = World ? World->GetTimeSeconds() - AchievementFeedbackStartTime : 0.0f;
	const bool bShowFeedback = !AchievementFeedbackLabel.IsEmpty() && FeedbackElapsed <= AchievementFeedbackDurationSeconds;

	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelWidth = FMath::Clamp(Canvas->SizeX * 0.22f, 320.0f * Scale, 420.0f * Scale);
	const float RowHeight = 28.0f * Scale;
	const float Padding = 14.0f * Scale;
	const float FeedbackHeight = bShowFeedback ? 26.0f * Scale : 0.0f;
	const int32 VisibleRows = FMath::Min(ViewModel.Rows.Num(), 5);
	const float PanelHeight = 82.0f * Scale + VisibleRows * RowHeight + FeedbackHeight;
	const float X = Canvas->SizeX - PanelWidth - 28.0f * Scale;
	const float Y = 722.0f * Scale;
	const float Border = 2.0f * Scale;

	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.91f), X, Y, PanelWidth, PanelHeight);
	DrawRect(Theme::AccentBlue, X, Y, PanelWidth, Border);
	DrawRect(Theme::AccentBlue, X, Y + PanelHeight - Border, PanelWidth, Border);
	DrawRect(Theme::AccentBlue, X, Y, Border, PanelHeight);
	DrawRect(Theme::AccentBlue, X + PanelWidth - Border, Y, Border, PanelHeight);

	DrawText(ViewModel.Title.ToString(), Theme::TextPrimary, X + Padding, Y + 12.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 0.92f * Scale);
	DrawText(ViewModel.TotalPointsLabel.ToString(), Theme::AccentGold, X + Padding, Y + 44.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.80f * Scale);
	DrawText(ViewModel.StatMultiplierLabel.ToString(), Theme::AccentBlue, X + 128.0f * Scale, Y + 44.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.80f * Scale);

	float RowY = Y + 72.0f * Scale;
	for (int32 Index = 0; Index < VisibleRows; ++Index)
	{
		const FIdleHUDAchievementRowViewModel& Row = ViewModel.Rows[Index];
		const float BarWidth = PanelWidth - Padding * 2.0f;
		DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.90f), X + Padding, RowY, BarWidth, RowHeight - 4.0f * Scale);
		DrawRect(Theme::AccentBlue.CopyWithNewOpacity(0.62f), X + Padding, RowY + RowHeight - 9.0f * Scale, BarWidth, 4.0f * Scale);
		DrawRect(Theme::AccentGold, X + Padding, RowY + RowHeight - 9.0f * Scale, BarWidth * Row.ProgressRatio, 4.0f * Scale);
		DrawText(Row.CategoryLabel.ToString(), Theme::TextPrimary, X + Padding + 8.0f * Scale, RowY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.70f * Scale);
		DrawText(Row.TierLabel.ToString(), Theme::TextMuted, X + Padding + 92.0f * Scale, RowY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.68f * Scale);
		DrawText(Row.PointsLabel.ToString(), Theme::AccentGold, X + Padding + 154.0f * Scale, RowY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.68f * Scale);
		DrawText(Row.NextThresholdLabel.ToString(), Theme::TextMuted, X + PanelWidth - 108.0f * Scale, RowY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.66f * Scale);
		RowY += RowHeight;
	}

	if (bShowFeedback)
	{
		DrawText(AchievementFeedbackLabel.ToString(), Theme::AccentGold, X + Padding, RowY + 4.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.78f * Scale);
	}
}

void AIdleHUD::DrawMasteryPanel()
{
	using namespace IdleProject::UI;

	if (!Canvas)
	{
		return;
	}
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	const UMasteryService* MasteryService = IdleGameInstance ? IdleGameInstance->GetMasteryService() : nullptr;
	if (!MasteryService)
	{
		return;
	}

	const FIdleHUDMasteryPanelViewModel ViewModel = BuildMasteryPanelViewModel(*MasteryService);
	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelWidth = FMath::Clamp(Canvas->SizeX * 0.22f, 320.0f * Scale, 420.0f * Scale);
	const float RowHeight = 56.0f * Scale;
	const float Padding = 14.0f * Scale;
	const float PanelHeight = 72.0f * Scale + ViewModel.Rows.Num() * RowHeight + Padding;
	const float X = Canvas->SizeX - PanelWidth - 28.0f * Scale;
	const float Y = 454.0f * Scale;
	const float Border = 2.0f * Scale;

	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.91f), X, Y, PanelWidth, PanelHeight);
	DrawRect(Theme::AccentGold, X, Y, PanelWidth, Border);
	DrawRect(Theme::AccentGold, X, Y + PanelHeight - Border, PanelWidth, Border);
	DrawRect(Theme::AccentGold, X, Y, Border, PanelHeight);
	DrawRect(Theme::AccentGold, X + PanelWidth - Border, Y, Border, PanelHeight);

	DrawText(ViewModel.Title.ToString(), Theme::TextPrimary, X + Padding, Y + 12.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 0.92f * Scale);
	DrawText(ViewModel.WorldPowerLabel.ToString(), Theme::AccentGold, X + Padding, Y + 44.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.76f * Scale);

	float RowY = Y + 70.0f * Scale;
	for (const FIdleHUDMasteryTrackRowViewModel& Row : ViewModel.Rows)
	{
		const float RowX = X + Padding;
		const float RowWidth = PanelWidth - Padding * 2.0f;
		DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.90f), RowX, RowY, RowWidth, RowHeight - 5.0f * Scale);
		DrawRect(Theme::AccentBlue.CopyWithNewOpacity(0.52f), RowX, RowY + RowHeight - 10.0f * Scale, RowWidth, 4.0f * Scale);
		DrawRect(Theme::AccentGold, RowX, RowY + RowHeight - 10.0f * Scale, RowWidth * Row.ProgressRatio, 4.0f * Scale);

		DrawText(Row.TrackLabel.ToString(), Theme::TextPrimary, RowX + 8.0f * Scale, RowY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.70f * Scale);
		DrawText(Row.LevelLabel.ToString(), Theme::TextMuted, RowX + 78.0f * Scale, RowY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.66f * Scale);
		DrawText(Row.LocalBonusLabel.ToString(), Theme::AccentGold, RowX + 132.0f * Scale, RowY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.64f * Scale);
		DrawText(Row.LocalBonus2Label.ToString(), Theme::AccentBlue, RowX + 132.0f * Scale, RowY + 21.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.64f * Scale);
		DrawText(Row.XpLabel.ToString(), Theme::TextMuted, RowX + 8.0f * Scale, RowY + 23.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.58f * Scale);
		DrawText(Row.TooltipLabel.ToString(), Theme::TextMuted, RowX + 8.0f * Scale, RowY + 38.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.54f * Scale);
		RowY += RowHeight;
	}
}

void AIdleHUD::DrawPetPanel()
{
	using namespace IdleProject::UI;

	if (!Canvas)
	{
		return;
	}
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	const UPetService* PetService = IdleGameInstance ? IdleGameInstance->GetPetService() : nullptr;
	if (!PetService)
	{
		return;
	}

	const FIdleHUDPetPanelViewModel ViewModel = BuildPetPanelViewModel(
		PetService->GetPetDefinitions(),
		PetService->GetEquippedPetId(),
		IdleGameInstance->GetEquippedPetGoldBonusPercent(),
		IdleGameInstance->GetEquippedPetDropBonusPercent(),
		IdleGameInstance->GetGold(),
		[PetService](const FString& PetId)
		{
			return PetService->GetPetLevel(PetId);
		},
		[PetService](const FString& PetId)
		{
			return PetService->IsPetOwned(PetId);
		},
		[PetService](const FString& PetId)
		{
			return PetService->GetPetStar(PetId);
		});

	const UWorld* World = GetWorld();
	const float PetFeedbackElapsed = World ? World->GetTimeSeconds() - PetFeedbackStartTime : 0.0f;
	const bool bShowPetFeedback = !PetFeedbackLabel.IsEmpty() && PetFeedbackElapsed <= PetFeedbackDurationSeconds;

	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelWidth = 460.0f * Scale;
	const float HeaderHeight = 44.0f * Scale;
	const float RowHeight = 52.0f * Scale;
	const float RowGap = 4.0f * Scale;
	const float Padding = 14.0f * Scale;
	const float FeedbackHeight = bShowPetFeedback ? 26.0f * Scale : 0.0f;
	const float PanelHeight = HeaderHeight + 38.0f * Scale + ViewModel.Rows.Num() * RowHeight + FMath::Max(0, ViewModel.Rows.Num() - 1) * RowGap + Padding + FeedbackHeight;
	const float X = 28.0f * Scale;
	const float Y = 300.0f * Scale;
	const float Border = 2.0f * Scale;

	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.91f), X, Y, PanelWidth, PanelHeight);
	DrawRect(Theme::AccentGold, X, Y, PanelWidth, Border);
	DrawRect(Theme::AccentGold, X, Y + PanelHeight - Border, PanelWidth, Border);
	DrawRect(Theme::AccentGold, X, Y, Border, PanelHeight);
	DrawRect(Theme::AccentGold, X + PanelWidth - Border, Y, Border, PanelHeight);

	DrawText(ViewModel.Title.ToString(), Theme::TextPrimary, X + Padding, Y + 12.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 0.92f * Scale);
	DrawText(ViewModel.EquippedLabel.ToString(), Theme::TextMuted, X + 72.0f * Scale, Y + 16.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.78f * Scale);
	DrawText(ViewModel.GoldBonusLabel.ToString(), Theme::AccentGold, X + Padding, Y + 46.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.84f * Scale);
	DrawText(ViewModel.DropBonusLabel.ToString(), Theme::AccentBlue, X + 128.0f * Scale, Y + 46.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.84f * Scale);

	float RowY = Y + HeaderHeight + 32.0f * Scale;
	for (const FIdleHUDPetRowViewModel& Row : ViewModel.Rows)
	{
		DrawPetRow(Row, X + Padding, RowY, PanelWidth - Padding * 2.0f, RowHeight);
		RowY += RowHeight + RowGap;
	}

	if (bShowPetFeedback)
	{
		const FLinearColor FeedbackColor = bPetFeedbackSuccess ? Theme::AccentGold : Theme::Warn;
		DrawText(PetFeedbackLabel.ToString(), FeedbackColor, X + Padding, RowY + 2.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.82f * Scale);
	}
	RefreshMouseInteraction();
}

void AIdleHUD::DrawPetRow(const FIdleHUDPetRowViewModel& Row, float X, float Y, float Width, float Height)
{
	using namespace IdleProject::UI;

	const float Scale = Height / 52.0f;
	const FLinearColor StateColor = Row.bEquipped ? Theme::AccentGold : Theme::TextMuted.CopyWithNewOpacity(0.56f);
	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.90f), X, Y, Width, Height);
	DrawRect(StateColor, X, Y, 4.0f * Scale, Height);

	// 1행: 이름/별/레벨/보너스/먹이 비용/상태 + 장착/먹이 버튼
	DrawText(Row.Name.ToString(), Row.bEquipped ? Theme::AccentGold : Theme::TextPrimary, X + 10.0f * Scale, Y + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.68f * Scale);
	DrawText(Row.StarLabel.ToString(), Theme::AccentGold, X + 10.0f * Scale, Y + 18.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.58f * Scale);
	DrawText(Row.LevelLabel.ToString(), Theme::TextMuted, X + 76.0f * Scale, Y + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);
	DrawText(Row.BonusLabel.ToString(), Theme::TextMuted, X + 136.0f * Scale, Y + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);
	DrawText(Row.FeedCostLabel.ToString(), Theme::TextMuted, X + 214.0f * Scale, Y + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.56f * Scale);
	DrawText(Row.StatusLabel.ToString(), Row.bCanFeed ? Theme::AccentBlue : Theme::Warn, X + 214.0f * Scale, Y + 17.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.52f * Scale);

	const float ButtonWidth = 56.0f * Scale;
	const float ButtonHeight = 20.0f * Scale;
	const float ButtonGap = 4.0f * Scale;
	const float FeedButtonX = X + Width - ButtonWidth - 6.0f * Scale;
	const float ButtonX = FeedButtonX - ButtonWidth - ButtonGap;
	const float ButtonY = Y + 5.0f * Scale;
	DrawRect(Row.bCanEquip ? Theme::AccentGold : Theme::BgPanel, ButtonX, ButtonY, ButtonWidth, ButtonHeight);
	DrawText(Row.ActionLabel.ToString(), Row.bCanEquip ? Theme::BgPrimary : Theme::TextMuted, ButtonX + 6.0f * Scale, ButtonY + 4.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.52f * Scale);
	if (Row.bCanEquip)
	{
		AddHitBox(FVector2D(ButtonX, ButtonY), FVector2D(ButtonWidth, ButtonHeight), MakePetEquipHitBoxName(Row.PetId), true, 82);
	}

	DrawRect(Row.bCanFeed ? Theme::AccentBlue : Theme::BgPanel, FeedButtonX, ButtonY, ButtonWidth, ButtonHeight);
	DrawText(Row.FeedActionLabel.ToString(), Row.bCanFeed ? Theme::BgPrimary : Theme::TextMuted, FeedButtonX + 6.0f * Scale, ButtonY + 4.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.52f * Scale);
	if (Row.bCanFeed)
	{
		AddHitBox(FVector2D(FeedButtonX, ButtonY), FVector2D(ButtonWidth, ButtonHeight), MakePetFeedHitBoxName(Row.PetId), true, 83);
	}

	// 2행: 진화 비용/다음 별 효과 + 진화 버튼 (보유·골드 충분 시 활성)
	const float EvolveRowY = Y + 30.0f * Scale;
	DrawText(Row.EvolveCostLabel.ToString(), Theme::TextMuted, X + 10.0f * Scale, EvolveRowY + 2.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.56f * Scale);
	DrawText(Row.EvolveEffectLabel.ToString(), Theme::AccentGold, X + 136.0f * Scale, EvolveRowY + 2.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.56f * Scale);

	const float EvolveButtonX = X + Width - ButtonWidth - 6.0f * Scale;
	const float EvolveButtonY = EvolveRowY;
	DrawRect(Row.bCanEvolve ? Theme::AccentGold : Theme::BgPanel, EvolveButtonX, EvolveButtonY, ButtonWidth, ButtonHeight);
	DrawText(Row.EvolveActionLabel.ToString(), Row.bCanEvolve ? Theme::BgPrimary : Theme::TextMuted, EvolveButtonX + 6.0f * Scale, EvolveButtonY + 4.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.52f * Scale);
	if (Row.bCanEvolve)
	{
		AddHitBox(FVector2D(EvolveButtonX, EvolveButtonY), FVector2D(ButtonWidth, ButtonHeight), MakePetEvolveHitBoxName(Row.PetId), true, 84);
	}
}

void AIdleHUD::EquipPetFromHitBox(FName BoxName)
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance)
	{
		return;
	}

	FString PetId = BoxName.ToString();
	PetId.RightChopInline(PetEquipHitBoxPrefix.Len());
	if (!PetId.IsEmpty())
	{
		IdleGameInstance->EquipPet(PetId);
	}
	RefreshMouseInteraction();
}

void AIdleHUD::TryFeedPetFromHitBox(FName BoxName)
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance)
	{
		return;
	}

	FString PetId = BoxName.ToString();
	PetId.RightChopInline(PetFeedHitBoxPrefix.Len());
	if (!PetId.IsEmpty())
	{
		IdleGameInstance->TryFeedPet(PetId);
	}
	RefreshMouseInteraction();
}

void AIdleHUD::TryEvolvePetFromHitBox(FName BoxName)
{
	using namespace IdleProject::UI;

	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance)
	{
		return;
	}

	FString PetId = BoxName.ToString();
	PetId.RightChopInline(PetEvolveHitBoxPrefix.Len());
	if (!PetId.IsEmpty())
	{
		const bool bEvolved = IdleGameInstance->EvolvePet(PetId);
		if (bEvolved)
		{
			const UPetService* PetService = IdleGameInstance->GetPetService();
			const int32 NewStar = PetService ? PetService->GetPetStar(PetId) : 0;
			bPetFeedbackSuccess = true;
			PetFeedbackLabel = FormatLocalizedUI(TEXT("PET_EVOLVE_FEEDBACK_SUCCESS_FORMAT"), [NewStar](FFormatNamedArguments& Args)
			{
				Args.Add(TEXT("Star"), FText::AsNumber(NewStar));
			});
		}
		else
		{
			bPetFeedbackSuccess = false;
			PetFeedbackLabel = IdleProject::Localization::UI(TEXT("PET_EVOLVE_FEEDBACK_BLOCKED"));
		}
		const UWorld* World = GetWorld();
		PetFeedbackStartTime = World ? World->GetTimeSeconds() : 0.0f;
	}
	RefreshMouseInteraction();
}

void AIdleHUD::DrawTitlePanel()
{
	using namespace IdleProject::UI;

	if (!Canvas)
	{
		return;
	}
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	const UTitleService* TitleService = IdleGameInstance ? IdleGameInstance->GetTitleService() : nullptr;
	const UAchievementService* AchievementService = IdleGameInstance ? IdleGameInstance->GetAchievementService() : nullptr;
	if (!TitleService || !AchievementService)
	{
		return;
	}

	const FIdleHUDTitlePanelViewModel ViewModel = BuildTitlePanelViewModel(*TitleService, *AchievementService);

	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelWidth = 470.0f * Scale;
	const float HeaderHeight = 44.0f * Scale;
	const float RowHeight = 44.0f * Scale;
	const float RowGap = 4.0f * Scale;
	const float Padding = 14.0f * Scale;
	const float PanelHeight = HeaderHeight + ViewModel.Rows.Num() * RowHeight + FMath::Max(0, ViewModel.Rows.Num() - 1) * RowGap + Padding;
	const float X = Canvas->SizeX - PanelWidth - 28.0f * Scale;
	const float Y = 28.0f * Scale;
	const float Border = 2.0f * Scale;

	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.91f), X, Y, PanelWidth, PanelHeight);
	DrawRect(Theme::AccentGold, X, Y, PanelWidth, Border);
	DrawRect(Theme::AccentGold, X, Y + PanelHeight - Border, PanelWidth, Border);
	DrawRect(Theme::AccentGold, X, Y, Border, PanelHeight);
	DrawRect(Theme::AccentGold, X + PanelWidth - Border, Y, Border, PanelHeight);

	DrawText(ViewModel.Title.ToString(), Theme::TextPrimary, X + Padding, Y + 12.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 0.92f * Scale);
	DrawText(ViewModel.EquippedLabel.ToString(), Theme::AccentGold, X + 96.0f * Scale, Y + 16.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.78f * Scale);

	float RowY = Y + HeaderHeight;
	for (const FIdleHUDTitleRowViewModel& Row : ViewModel.Rows)
	{
		DrawTitleRow(Row, X + Padding, RowY, PanelWidth - Padding * 2.0f, RowHeight);
		RowY += RowHeight + RowGap;
	}

	RefreshMouseInteraction();
}

void AIdleHUD::DrawTitleRow(const FIdleHUDTitleRowViewModel& Row, float X, float Y, float Width, float Height)
{
	using namespace IdleProject::UI;

	const float Scale = Height / 44.0f;
	// 장착=골드, 해금=블루, 미해금=흐린 회색으로 상태 막대를 칠한다.
	const FLinearColor StateColor = Row.bEquipped
		? Theme::AccentGold
		: (Row.bUnlocked ? Theme::AccentBlue : Theme::TextMuted.CopyWithNewOpacity(0.45f));
	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.90f), X, Y, Width, Height);
	DrawRect(StateColor, X, Y, 4.0f * Scale, Height);

	// 1행: 이름 + 보너스. 해금 칭호는 이름을 강조색으로.
	DrawText(Row.Name.ToString(), Row.bEquipped ? Theme::AccentGold : (Row.bUnlocked ? Theme::TextPrimary : Theme::TextMuted), X + 10.0f * Scale, Y + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.66f * Scale);
	DrawText(Row.BonusLabel.ToString(), Theme::AccentGold, X + 150.0f * Scale, Y + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.60f * Scale);

	// 2행: 해금 칭호는 상태(해금), 미해금은 해금 조건 + 진행도 막대.
	if (Row.bUnlocked)
	{
		DrawText(Row.StatusLabel.ToString(), Theme::AccentBlue, X + 10.0f * Scale, Y + 23.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.56f * Scale);
	}
	else
	{
		DrawText(Row.ConditionLabel.ToString(), Theme::TextMuted, X + 10.0f * Scale, Y + 21.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.54f * Scale);
		const float BarY = Y + 36.0f * Scale;
		const float BarWidth = 190.0f * Scale;
		DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.80f), X + 10.0f * Scale, BarY, BarWidth, 4.0f * Scale);
		DrawRect(Theme::AccentBlue, X + 10.0f * Scale, BarY, BarWidth * Row.ProgressRatio, 4.0f * Scale);
		DrawText(Row.ProgressLabel.ToString(), Theme::TextMuted, X + 210.0f * Scale, Y + 30.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.50f * Scale);
	}

	// 장착/해제 버튼: 장착중=해제(블루), 해금=장착(골드), 미해금=비활성.
	const float ButtonWidth = 60.0f * Scale;
	const float ButtonHeight = 20.0f * Scale;
	const float ButtonX = X + Width - ButtonWidth - 6.0f * Scale;
	const float ButtonY = Y + 12.0f * Scale;
	if (Row.bEquipped)
	{
		DrawRect(Theme::AccentBlue, ButtonX, ButtonY, ButtonWidth, ButtonHeight);
		DrawText(Row.ActionLabel.ToString(), Theme::BgPrimary, ButtonX + 6.0f * Scale, ButtonY + 4.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.52f * Scale);
		AddHitBox(FVector2D(ButtonX, ButtonY), FVector2D(ButtonWidth, ButtonHeight), TitleUnequipHitBoxName, true, 85);
	}
	else
	{
		DrawRect(Row.bCanEquip ? Theme::AccentGold : Theme::BgPanel, ButtonX, ButtonY, ButtonWidth, ButtonHeight);
		DrawText(Row.ActionLabel.ToString(), Row.bCanEquip ? Theme::BgPrimary : Theme::TextMuted, ButtonX + 6.0f * Scale, ButtonY + 4.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.52f * Scale);
		if (Row.bCanEquip)
		{
			AddHitBox(FVector2D(ButtonX, ButtonY), FVector2D(ButtonWidth, ButtonHeight), MakeTitleEquipHitBoxName(Row.TitleId), true, 85);
		}
	}
}

void AIdleHUD::EquipTitleFromHitBox(FName BoxName)
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance)
	{
		return;
	}

	FString TitleId = BoxName.ToString();
	TitleId.RightChopInline(TitleEquipHitBoxPrefix.Len());
	if (!TitleId.IsEmpty())
	{
		IdleGameInstance->EquipTitle(TitleId);
	}
	RefreshMouseInteraction();
}

void AIdleHUD::UnequipTitleAction()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance)
	{
		return;
	}

	IdleGameInstance->UnequipTitle();
	RefreshMouseInteraction();
}

void AIdleHUD::DrawMissionPanel()
{
	using namespace IdleProject::UI;

	if (!Canvas)
	{
		return;
	}
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	const UMissionService* MissionService = IdleGameInstance ? IdleGameInstance->GetMissionService() : nullptr;
	if (!MissionService)
	{
		return;
	}

	const FIdleHUDMissionPanelViewModel ViewModel = BuildMissionPanelViewModel(*MissionService, SelectedMissionPeriod);

	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelWidth = 470.0f * Scale;
	const float HeaderHeight = 44.0f * Scale;
	const float TabHeight = 26.0f * Scale;
	const float RowHeight = 50.0f * Scale;
	const float RowGap = 4.0f * Scale;
	const float Padding = 14.0f * Scale;
	const int32 RowCount = FMath::Max(1, ViewModel.Rows.Num());
	const float PanelHeight = HeaderHeight + TabHeight + 8.0f * Scale + RowCount * RowHeight + FMath::Max(0, RowCount - 1) * RowGap + Padding;
	const float X = 28.0f * Scale;
	const float Y = 360.0f * Scale;
	const float Border = 2.0f * Scale;

	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.91f), X, Y, PanelWidth, PanelHeight);
	DrawRect(Theme::AccentBlue, X, Y, PanelWidth, Border);
	DrawRect(Theme::AccentBlue, X, Y + PanelHeight - Border, PanelWidth, Border);
	DrawRect(Theme::AccentBlue, X, Y, Border, PanelHeight);
	DrawRect(Theme::AccentBlue, X + PanelWidth - Border, Y, Border, PanelHeight);

	DrawText(ViewModel.Title.ToString(), Theme::TextPrimary, X + Padding, Y + 12.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 0.92f * Scale);

	// 일일/주간 탭 2개를 균등 배치(선택 탭=골드, 비선택=어둡게).
	const float TabGap = 6.0f * Scale;
	const float TabRowWidth = PanelWidth - Padding * 2.0f;
	const float TabWidth = (TabRowWidth - TabGap) / 2.0f;
	const float TabY = Y + HeaderHeight;
	const float DailyTabX = X + Padding;
	const float WeeklyTabX = DailyTabX + TabWidth + TabGap;
	const bool bDailyActive = ViewModel.ActivePeriod == EMissionPeriod::Daily;
	DrawRect(bDailyActive ? Theme::AccentGold : Theme::BgPrimary, DailyTabX, TabY, TabWidth, TabHeight);
	DrawRect(!bDailyActive ? Theme::AccentGold : Theme::BgPrimary, WeeklyTabX, TabY, TabWidth, TabHeight);
	DrawText(ViewModel.DailyTabLabel.ToString(), bDailyActive ? Theme::BgPrimary : Theme::TextMuted, DailyTabX + 8.0f * Scale, TabY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.60f * Scale);
	DrawText(ViewModel.WeeklyTabLabel.ToString(), !bDailyActive ? Theme::BgPrimary : Theme::TextMuted, WeeklyTabX + 8.0f * Scale, TabY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.60f * Scale);
	AddHitBox(FVector2D(DailyTabX, TabY), FVector2D(TabWidth, TabHeight), MissionDailyTabHitBoxName, true, 86);
	AddHitBox(FVector2D(WeeklyTabX, TabY), FVector2D(TabWidth, TabHeight), MissionWeeklyTabHitBoxName, true, 87);

	float RowY = TabY + TabHeight + 8.0f * Scale;
	if (ViewModel.Rows.Num() == 0)
	{
		DrawText(ViewModel.EmptyLabel.ToString(), Theme::TextMuted, X + Padding, RowY + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);
	}
	for (const FIdleHUDMissionRowViewModel& Row : ViewModel.Rows)
	{
		DrawMissionRow(Row, X + Padding, RowY, PanelWidth - Padding * 2.0f, RowHeight);
		RowY += RowHeight + RowGap;
	}

	RefreshMouseInteraction();
}

void AIdleHUD::DrawMissionRow(const FIdleHUDMissionRowViewModel& Row, float X, float Y, float Width, float Height)
{
	using namespace IdleProject::UI;

	const float Scale = Height / 50.0f;
	// 수령 가능=골드, 수령 완료=흐림, 진행중=블루로 상태 막대를 칠한다.
	const FLinearColor StateColor = Row.bClaimed
		? Theme::TextMuted.CopyWithNewOpacity(0.46f)
		: (Row.bCanClaim ? Theme::AccentGold : Theme::AccentBlue);
	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.90f), X, Y, Width, Height);
	DrawRect(StateColor, X, Y, 4.0f * Scale, Height);

	// 1행: 목표 설명 + 보상.
	DrawText(Row.ObjectiveLabel.ToString(), Row.bClaimed ? Theme::TextMuted : Theme::TextPrimary, X + 10.0f * Scale, Y + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);
	DrawText(Row.RewardLabel.ToString(), Theme::AccentGold, X + 10.0f * Scale, Y + 26.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.56f * Scale);

	// 진행 바(현재/목표, clamp) + 진행 라벨.
	const float BarY = Y + Height - 8.0f * Scale;
	const float BarWidth = 200.0f * Scale;
	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.80f), X + 10.0f * Scale, BarY, BarWidth, 4.0f * Scale);
	DrawRect(StateColor, X + 10.0f * Scale, BarY, BarWidth * Row.ProgressRatio, 4.0f * Scale);
	DrawText(Row.ProgressLabel.ToString(), Theme::TextMuted, X + 220.0f * Scale, Y + Height - 14.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.50f * Scale);

	// 수령 버튼: 수령 가능=활성(골드), 수령 완료=비활성(수령완료), 미완료=비활성(수령).
	const float ButtonWidth = 68.0f * Scale;
	const float ButtonHeight = 22.0f * Scale;
	const float ButtonX = X + Width - ButtonWidth - 7.0f * Scale;
	const float ButtonY = Y + 8.0f * Scale;
	DrawRect(Row.bCanClaim ? Theme::AccentGold : Theme::BgPanel, ButtonX, ButtonY, ButtonWidth, ButtonHeight);
	DrawText(Row.ActionLabel.ToString(), Row.bCanClaim ? Theme::BgPrimary : Theme::TextMuted, ButtonX + 8.0f * Scale, ButtonY + 4.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.56f * Scale);
	if (Row.bCanClaim)
	{
		AddHitBox(FVector2D(ButtonX, ButtonY), FVector2D(ButtonWidth, ButtonHeight), MakeMissionClaimHitBoxName(Row.MissionId), true, 88);
	}
}

void AIdleHUD::SelectMissionPeriod(EMissionPeriod Period)
{
	SelectedMissionPeriod = Period;
	RefreshMouseInteraction();
}

void AIdleHUD::ClaimMissionFromHitBox(FName BoxName)
{
	using namespace IdleProject::UI;

	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance)
	{
		return;
	}

	FString MissionId = BoxName.ToString();
	MissionId.RightChopInline(MissionClaimHitBoxPrefix.Len());
	if (MissionId.IsEmpty())
	{
		return;
	}

	if (IdleGameInstance->ClaimMission(MissionId))
	{
		UE_LOG(LogTemp, Display, TEXT("[Mission] ClaimMission success missionId=%s"), *MissionId);
	}
	RefreshMouseInteraction();
}

void AIdleHUD::DrawSeasonPassPanel()
{
	using namespace IdleProject::UI;

	if (!Canvas)
	{
		return;
	}
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	const USeasonService* SeasonService = IdleGameInstance ? IdleGameInstance->GetSeasonService() : nullptr;
	if (!SeasonService)
	{
		return;
	}

	const FIdleHUDSeasonPassViewModel ViewModel = BuildSeasonPassViewModel(
		SeasonService->GetSeasonTiers(),
		SeasonService->GetSeasonTokens(),
		SeasonService->GetReachedTier(),
		[SeasonService](int32 Tier)
		{
			return SeasonService->IsTierClaimed(Tier);
		});

	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelWidth = 460.0f * Scale;
	const float HeaderHeight = 54.0f * Scale;
	const float RowHeight = 23.0f * Scale;
	const float RowGap = 3.0f * Scale;
	const float Padding = 14.0f * Scale;
	const float PanelHeight = HeaderHeight + 24.0f * Scale + ViewModel.Rows.Num() * RowHeight + FMath::Max(0, ViewModel.Rows.Num() - 1) * RowGap + Padding;
	const float X = 28.0f * Scale;
	const float Y = 724.0f * Scale;
	const float Border = 2.0f * Scale;

	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.91f), X, Y, PanelWidth, PanelHeight);
	DrawRect(Theme::AccentBlue, X, Y, PanelWidth, Border);
	DrawRect(Theme::AccentBlue, X, Y + PanelHeight - Border, PanelWidth, Border);
	DrawRect(Theme::AccentBlue, X, Y, Border, PanelHeight);
	DrawRect(Theme::AccentBlue, X + PanelWidth - Border, Y, Border, PanelHeight);

	DrawText(ViewModel.Title.ToString(), Theme::TextPrimary, X + Padding, Y + 12.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 0.92f * Scale);
	DrawText(ViewModel.TokenLabel.ToString(), Theme::AccentGold, X + PanelWidth - 92.0f * Scale, Y + 16.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.82f * Scale);
	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.92f), X + Padding, Y + 42.0f * Scale, PanelWidth - Padding * 2.0f, 6.0f * Scale);
	DrawRect(Theme::AccentBlue, X + Padding, Y + 42.0f * Scale, (PanelWidth - Padding * 2.0f) * ViewModel.ProgressRatio, 6.0f * Scale);

	float RowY = Y + HeaderHeight + 18.0f * Scale;
	for (const FIdleHUDSeasonTierRowViewModel& Row : ViewModel.Rows)
	{
		DrawSeasonTierRow(Row, X + Padding, RowY, PanelWidth - Padding * 2.0f, RowHeight);
		RowY += RowHeight + RowGap;
	}
	RefreshMouseInteraction();
}

void AIdleHUD::DrawSeasonTierRow(const FIdleHUDSeasonTierRowViewModel& Row, float X, float Y, float Width, float Height)
{
	using namespace IdleProject::UI;

	const float Scale = Height / 29.0f;
	const FLinearColor StateColor = Row.bClaimed ? Theme::TextMuted.CopyWithNewOpacity(0.46f) : (Row.bCanClaim ? Theme::AccentGold : Theme::AccentBlue);
	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.90f), X, Y, Width, Height);
	DrawRect(StateColor, X, Y, 4.0f * Scale, Height);
	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.96f), X + 7.0f * Scale, Y + Height - 5.0f * Scale, Width - 14.0f * Scale, 3.0f * Scale);
	DrawRect(StateColor, X + 7.0f * Scale, Y + Height - 5.0f * Scale, (Width - 14.0f * Scale) * Row.ProgressRatio, 3.0f * Scale);

	DrawText(Row.TierLabel.ToString(), Row.bReached ? Theme::AccentGold : Theme::TextPrimary, X + 12.0f * Scale, Y + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.72f * Scale);
	DrawText(Row.RequirementLabel.ToString(), Theme::TextMuted, X + 72.0f * Scale, Y + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.72f * Scale);
	DrawText(Row.RewardLabel.ToString(), Theme::TextPrimary, X + 150.0f * Scale, Y + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.72f * Scale);

	const float ButtonWidth = 68.0f * Scale;
	const float ButtonHeight = 21.0f * Scale;
	const float ButtonX = X + Width - ButtonWidth - 7.0f * Scale;
	const float ButtonY = Y + 4.0f * Scale;
	DrawRect(Row.bCanClaim ? Theme::AccentGold : Theme::BgPanel, ButtonX, ButtonY, ButtonWidth, ButtonHeight);
	DrawText(Row.ActionLabel.ToString(), Row.bCanClaim ? Theme::BgPrimary : Theme::TextMuted, ButtonX + 8.0f * Scale, ButtonY + 4.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.68f * Scale);
	if (Row.bCanClaim)
	{
		AddHitBox(FVector2D(ButtonX, ButtonY), FVector2D(ButtonWidth, ButtonHeight), MakeSeasonClaimHitBoxName(Row.Tier), true, 84);
	}
}

void AIdleHUD::ClaimSeasonTierFromHitBox(FName BoxName)
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance)
	{
		return;
	}

	FString RawTier = BoxName.ToString();
	RawTier.RightChopInline(SeasonClaimHitBoxPrefix.Len());
	const int32 Tier = FCString::Atoi(*RawTier);
	if (Tier > 0)
	{
		IdleGameInstance->ClaimSeasonReward(Tier);
	}
	RefreshMouseInteraction();
}

void AIdleHUD::DrawFloatingDamageTexts(float Now)
{
	if (!Canvas || !PlayerOwner)
	{
		return;
	}

	const float HudScale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	for (int32 Index = FloatingDamageEntries.Num() - 1; Index >= 0; --Index)
	{
		const FIdleHUDFloatingDamageEntry& Entry = FloatingDamageEntries[Index];
		FVector2D ScreenPosition;
		if (!PlayerOwner->ProjectWorldLocationToScreen(Entry.WorldLocation, ScreenPosition))
		{
			FloatingDamageEntries.RemoveAtSwap(Index);
			continue;
		}

		const FIdleHUDFloatingDamageViewModel ViewModel = IdleProject::UI::BuildFloatingDamageViewModel(Entry, Now, ScreenPosition, HudScale);
		if (!ViewModel.bVisible)
		{
			FloatingDamageEntries.RemoveAtSwap(Index);
			continue;
		}

		UFont* Font = Entry.bWasCrit && GEngine ? GEngine->GetMediumFont() : (GEngine ? GEngine->GetSmallFont() : nullptr);
		const float ShadowOffset = 2.0f * HudScale;
		DrawText(
			ViewModel.Label,
			FLinearColor(0.0f, 0.0f, 0.0f, ViewModel.Color.A * 0.55f),
			ViewModel.ScreenPosition.X + ShadowOffset,
			ViewModel.ScreenPosition.Y + ShadowOffset,
			Font,
			ViewModel.TextScale);
		DrawText(
			ViewModel.Label,
			ViewModel.Color,
			ViewModel.ScreenPosition.X,
			ViewModel.ScreenPosition.Y,
			Font,
			ViewModel.TextScale);
	}
}

void AIdleHUD::DrawStatusIndicators(float Now)
{
	using namespace IdleProject::UI;

	if (!Canvas || !PlayerOwner)
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float HudScale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	for (TActorIterator<AActor> ActorIt(World); ActorIt; ++ActorIt)
	{
		AActor* Actor = *ActorIt;
		if (!Actor)
		{
			continue;
		}

		const UCombatComponent* Combat = Actor->FindComponentByClass<UCombatComponent>();
		if (!Combat)
		{
			continue;
		}

		const TArray<FIdleHUDStatusIndicatorViewModel> Indicators = BuildStatusIndicatorViewModels(Combat->GetActiveStatuses(), Now, HudScale);
		if (Indicators.IsEmpty())
		{
			continue;
		}

		FVector2D ScreenPosition;
		if (!PlayerOwner->ProjectWorldLocationToScreen(Actor->GetActorLocation() + FVector(0.0, 0.0, StatusIndicatorHeadOffsetZ), ScreenPosition))
		{
			continue;
		}

		const float Gap = 4.0f * HudScale;
		const float IndicatorSize = Indicators[0].Size;
		const float TotalWidth = Indicators.Num() * IndicatorSize + (Indicators.Num() - 1) * Gap;
		float X = ScreenPosition.X - TotalWidth * 0.5f;
		const float Y = ScreenPosition.Y - IndicatorSize;
		for (const FIdleHUDStatusIndicatorViewModel& Indicator : Indicators)
		{
			DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.74f), X - 1.0f * HudScale, Y - 1.0f * HudScale, IndicatorSize + 2.0f * HudScale, IndicatorSize + 2.0f * HudScale);
			DrawRect(Indicator.Color.CopyWithNewOpacity(0.92f), X, Y, IndicatorSize, IndicatorSize);
			DrawText(Indicator.Label, Theme::BgPrimary, X + 6.0f * HudScale, Y + 2.0f * HudScale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.78f * HudScale);
			X += IndicatorSize + Gap;
		}
	}
}

void AIdleHUD::DrawProgressSavedIndicator(float Now)
{
	using namespace IdleProject::UI::Theme;

	if (!Canvas || ProgressSavedFeedbackLabel.IsEmpty())
	{
		return;
	}

	const float Elapsed = Now - ProgressSavedFeedbackStartTime;
	if (Elapsed < 0.0f || Elapsed > ProgressSavedFeedbackDurationSeconds)
	{
		return;
	}

	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float Alpha = FMath::Clamp(1.0f - (Elapsed / ProgressSavedFeedbackDurationSeconds), 0.0f, 1.0f);
	const float Width = 132.0f * Scale;
	const float Height = 28.0f * Scale;
	const float X = Canvas->SizeX - Width - 24.0f * Scale;
	const float Y = 58.0f * Scale;

	DrawRect(BgPrimary.CopyWithNewOpacity(0.78f * Alpha), X, Y, Width, Height);
	DrawRect(AccentGold.CopyWithNewOpacity(0.92f * Alpha), X, Y, 3.0f * Scale, Height);
	DrawText(ProgressSavedFeedbackLabel.ToString(), AccentGold.CopyWithNewOpacity(Alpha), X + 14.0f * Scale, Y + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.78f * Scale);
}

void AIdleHUD::DrawCloudSyncIndicator(float Now)
{
	using namespace IdleProject::UI::Theme;

	if (!Canvas || !CloudSyncViewModel.bVisible || CloudSyncViewModel.Label.IsEmpty())
	{
		return;
	}

	const float Elapsed = Now - CloudSyncFeedbackStartTime;
	if (CloudSyncViewModel.bTransient && (Elapsed < 0.0f || Elapsed > CloudSyncFeedbackDurationSeconds))
	{
		return;
	}

	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float Alpha = CloudSyncViewModel.bTransient
		? FMath::Clamp(1.0f - (Elapsed / CloudSyncFeedbackDurationSeconds), 0.0f, 1.0f)
		: 1.0f;
	const float Width = 154.0f * Scale;
	const float Height = 28.0f * Scale;
	const float X = Canvas->SizeX - Width - 24.0f * Scale;
	const float Y = 92.0f * Scale;
	const FLinearColor AccentColor = CloudSyncViewModel.bError
		? AccentRed
		: (CloudSyncViewModel.State == ECloudSyncState::Syncing ? AccentBlue : AccentGold);

	DrawRect(BgPrimary.CopyWithNewOpacity(0.78f * Alpha), X, Y, Width, Height);
	DrawRect(AccentColor.CopyWithNewOpacity(0.92f * Alpha), X, Y, 3.0f * Scale, Height);
	DrawText(CloudSyncViewModel.Label.ToString(), AccentColor.CopyWithNewOpacity(Alpha), X + 14.0f * Scale, Y + 6.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.78f * Scale);
}

void AIdleHUD::RefreshMouseInteraction()
{
	if (!PlayerOwner)
	{
		return;
	}

	const bool bRebirthReady = IdleGameInstance && IdleGameInstance->CanRebirth();
	const bool bTranscendReady = IdleGameInstance && IdleGameInstance->CanTranscend();
	const bool bHasRunePanel = IdleGameInstance && IdleGameInstance->GetRuneService();
	const bool bHasDungeonPanel = IdleGameInstance && IdleGameInstance->GetDungeonService();
	const bool bHasConsumablePanel = IdleGameInstance && IdleGameInstance->GetBuffService();
	const bool bHasLeaderboardPanel = IdleGameInstance && IdleGameInstance->GetLeaderboardService();
	const bool bNeedsPointer = ResolvePlayerCharacter() || PlayerInventory || bHasRunePanel || bHasDungeonPanel || bHasConsumablePanel || bHasLeaderboardPanel || bQuestLogVisible || bStatInfoVisible || OfflineRewardModal.bVisible || bRebirthReady || bTranscendReady;
	PlayerOwner->bShowMouseCursor = bNeedsPointer;
	PlayerOwner->bEnableClickEvents = bNeedsPointer;
}
