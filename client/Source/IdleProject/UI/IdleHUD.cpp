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
#include "GameCore/IdleGameInstance.h"
#include "GameCore/PetLevelFormula.h"
#include "GameCore/TowerMilestoneFormula.h"
#include "GameCore/TranscendFormula.h"
#include "Internationalization/IdleLocalization.h"
#include "ItemSystem/EnhanceFormula.h"
#include "ItemSystem/InventoryComponent.h"
#include "ItemSystem/SetBonusFormula.h"
#include "ItemSystem/ShopFormula.h"
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
const FString SeasonClaimHitBoxPrefix(TEXT("SeasonClaim_"));
const FString SkillRankHitBoxPrefix(TEXT("SkillRank_"));
const FString EnhanceSlotHitBoxPrefix(TEXT("EnhanceSlot_"));
const FString StatAllocationHitBoxPrefix(TEXT("StatAlloc_"));
const FName ShopGearRollHitBoxName(TEXT("ShopGearRoll"));
const FName StatResetHitBoxName(TEXT("StatReset"));
const FName StatInfoToggleHitBoxName(TEXT("StatInfoToggle"));
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
constexpr float ProgressSavedFeedbackDurationSeconds = 1.6f;

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

const TCHAR* RarityToLocalizationKey(EItemRarity Rarity)
{
	switch (Rarity)
	{
	case EItemRarity::Common:
		return TEXT("RARITY_COMMON");
	case EItemRarity::Uncommon:
		return TEXT("RARITY_UNCOMMON");
	case EItemRarity::Rare:
		return TEXT("RARITY_RARE");
	case EItemRarity::Epic:
		return TEXT("RARITY_EPIC");
	case EItemRarity::Legendary:
		return TEXT("RARITY_LEGENDARY");
	case EItemRarity::Mythic:
		return TEXT("RARITY_MYTHIC");
	case EItemRarity::None:
	default:
		return TEXT("RARITY_NONE");
	}
}

FText QuestTypeToLabel(EQuestType Type)
{
	return Type == EQuestType::Main
		? IdleProject::Localization::UI(TEXT("QUEST_TYPE_MAIN"))
		: IdleProject::Localization::UI(TEXT("QUEST_TYPE_DAILY"));
}

FText PetBonusTypeToLabel(EPetBonusType Type, float BonusPercent)
{
	const FString Percent = FString::Printf(TEXT("%.0f%%"), BonusPercent);
	switch (Type)
	{
	case EPetBonusType::Gold:
		return FormatLocalizedUI(TEXT("PET_BONUS_GOLD_FORMAT"), [&Percent](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Percent"), FText::FromString(Percent));
		});
	case EPetBonusType::Drop:
		return FormatLocalizedUI(TEXT("PET_BONUS_DROP_FORMAT"), [&Percent](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Percent"), FText::FromString(Percent));
		});
	case EPetBonusType::None:
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
	case ESkillElement::None:
	default:
		return IdleProject::Localization::UI(TEXT("ELEMENT_NONE"));
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

FName MakeStatAllocationHitBoxName(EPrimaryStat Stat)
{
	return FName(*(StatAllocationHitBoxPrefix + FString::FromInt(static_cast<int32>(Stat))));
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
		OutIndicator.Color = IdleProject::UI::Theme::RarityUncommon;
		return true;
	case ESkillStatusEffect::Burn:
		OutIndicator.Label = TEXT("B");
		OutIndicator.Color = IdleProject::UI::Theme::RarityLegendary;
		return true;
	case ESkillStatusEffect::Freeze:
		OutIndicator.Label = TEXT("F");
		OutIndicator.Color = IdleProject::UI::Theme::AccentBlue;
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
	case EItemRarity::Uncommon:
		return RarityUncommon;
	case EItemRarity::Rare:
		return RarityRare;
	case EItemRarity::Epic:
		return RarityEpic;
	case EItemRarity::Legendary:
		return RarityLegendary;
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
		EItemSet::Arcane
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
	ViewModel.ProgressRatio = SafeTarget > 0
		? FMath::Clamp(static_cast<float>(SafeCurrent) / static_cast<float>(SafeTarget), 0.0f, 1.0f)
		: 0.0f;
	ViewModel.bBossStage = StageInfo.bBossStage;
	ViewModel.BossBadgeLabel = StageInfo.bBossStage ? IdleProject::Localization::UI(TEXT("STAGE_BOSS_BADGE")) : FText::GetEmpty();
	ViewModel.BorderColor = StageInfo.bBossStage ? Theme::AccentGold : Theme::AccentBlue;
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
	FIdleHUDEnhancePanelViewModel ViewModel;
	ViewModel.Title = IdleProject::Localization::UI(TEXT("ENHANCE_PANEL_TITLE"));
	ViewModel.GoldLabel = FormatLocalizedUIWithInt64(TEXT("HUD_GOLD_FORMAT"), TEXT("Amount"), Gold);
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
		Row.LevelLabel = FormatLocalizedUI(TEXT("ENHANCE_LEVEL_FORMAT"), [&Row](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Level"), FText::AsNumber(Row.EnhanceLevel));
			Args.Add(TEXT("MaxLevel"), FText::AsNumber(FEnhanceFormula::MaxEnhanceLevel));
		});
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

FIdleHUDShopPanelViewModel IdleProject::UI::BuildShopPanelViewModel(int64 GearRollCost, int64 Gold, const FShopPurchaseResult& LastResult)
{
	FIdleHUDShopPanelViewModel ViewModel;
	ViewModel.Title = IdleProject::Localization::UI(TEXT("SHOP_PANEL_TITLE"));
	ViewModel.Gold = FMath::Max<int64>(0, Gold);
	ViewModel.GearRollCost = FMath::Max<int64>(0, GearRollCost);
	ViewModel.GearRollHitBoxName = ShopGearRollHitBoxName;
	ViewModel.GoldLabel = FormatLocalizedUIWithInt64(TEXT("HUD_GOLD_FORMAT"), TEXT("Amount"), ViewModel.Gold);
	ViewModel.CostLabel = FormatLocalizedUIWithInt64(TEXT("SHOP_GEAR_ROLL_COST_FORMAT"), TEXT("Cost"), ViewModel.GearRollCost);
	ViewModel.ButtonLabel = IdleProject::Localization::UI(TEXT("SHOP_GEAR_ROLL_BUTTON"));
	ViewModel.bCanBuyGearRoll = ViewModel.GearRollCost > 0 && ViewModel.Gold >= ViewModel.GearRollCost;
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

FText IdleProject::UI::BuildProgressSavedFeedbackLabel()
{
	return IdleProject::Localization::UI(TEXT("SAVE_PROGRESS_SAVED"));
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
	}

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
		{ EClassId::Cleric, TEXT("CLASS_CLERIC_NAME"), TEXT("CLASS_CLERIC_ROLE"), TEXT("WIS/INT") }
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

FIdleHUDPetPanelViewModel IdleProject::UI::BuildPetPanelViewModel(const TArray<FPetDefinition>& PetDefinitions, const FString& EquippedPetId, float GoldBonusPercent, float DropBonusPercent, int64 Gold, TFunctionRef<int32(const FString&)> GetPetLevel)
{
	FIdleHUDPetPanelViewModel ViewModel;
	ViewModel.Title = IdleProject::Localization::UI(TEXT("PET_PANEL_TITLE"));
	ViewModel.EquippedLabel = EquippedPetId.IsEmpty()
		? IdleProject::Localization::UI(TEXT("PET_EQUIPPED_NONE"))
		: FormatLocalizedUI(TEXT("PET_EQUIPPED_FORMAT"), [&EquippedPetId](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("PetId"), FText::FromString(EquippedPetId));
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
		const int64 FeedCost = FPetLevelFormula::GetFeedCost(Level);
		const bool bMaxLevel = Level >= FPetLevelFormula::MaxPetLevel;
		const bool bCanFeed = !bMaxLevel && FeedCost > 0 && Gold >= FeedCost;

		FIdleHUDPetRowViewModel Row;
		Row.PetId = Definition.PetId;
		Row.Name = Definition.Name;
		Row.BonusLabel = PetBonusTypeToLabel(Definition.BonusType, Definition.BonusPercent * FPetLevelFormula::GetBonusMultiplier(Level));
		Row.LevelLabel = BuildPetLevelLabel(Level);
		Row.FeedCostLabel = BuildPetFeedCostLabel(FeedCost);
		Row.bEquipped = Definition.PetId == EquippedPetId;
		Row.ActionLabel = IdleProject::Localization::UI(Row.bEquipped ? TEXT("ACTION_EQUIPPED") : TEXT("ACTION_EQUIP"));
		Row.FeedActionLabel = IdleProject::Localization::UI(TEXT("ACTION_FEED"));
		Row.bCanFeed = bCanFeed;
		Row.bFeedDisabled = !bCanFeed;
		Row.bMaxLevel = bMaxLevel;
		Row.StatusLabel = bMaxLevel
			? IdleProject::Localization::UI(TEXT("PET_FEED_STATUS_MAX"))
			: (bCanFeed
				? IdleProject::Localization::UI(TEXT("PET_FEED_STATUS_READY"))
				: IdleProject::Localization::UI(TEXT("PET_FEED_STATUS_NEED_GOLD")));
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
		ESkillStatusEffect::Freeze
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
}

void AIdleHUD::BeginPlay()
{
	Super::BeginPlay();
	BindPlayerCombat();
	BindPlayerInventory();
	BindStageService();
	BindTowerService();
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
	DrawStatAllocationPanel();
	DrawStatInfoPanel();
	DrawShopPanel();
	DrawEnhancePanel();
	DrawClassSelectionPanel();
	DrawPetPanel();
	DrawSeasonPassPanel();
	DrawQuestLog();
	DrawOfflineRewardModal();
	if (World)
	{
		DrawBossSpecialWarning(World->GetTimeSeconds());
		DrawFloatingDamageTexts(World->GetTimeSeconds());
		DrawStatusIndicators(World->GetTimeSeconds());
		DrawProgressSavedIndicator(World->GetTimeSeconds());
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
	if (BoxName == ShopGearRollHitBoxName)
	{
		TryBuyGearRoll();
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

void AIdleHUD::HandleProgressSaved()
{
	ProgressSavedFeedbackLabel = IdleProject::UI::BuildProgressSavedFeedbackLabel();
	if (const UWorld* World = GetWorld())
	{
		ProgressSavedFeedbackStartTime = World->GetTimeSeconds();
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

	for (EItemSlot Slot : ArmorSlots)
	{
		if (const FItemInstance* Item = PlayerInventory->GetEquippedItem(Slot))
		{
			EquippedItems.Add(*Item);
			++EquippedArmorCount;
			BonusDef += Item->BonusDef;
			BonusHp += Item->BonusHp;
		}
	}

	const FText ArmorLine = FormatLocalizedUI(TEXT("HUD_ARMOR_SUMMARY_FORMAT"), [EquippedArmorCount, BonusDef, BonusHp](FFormatNamedArguments& Args)
	{
		Args.Add(TEXT("Count"), FText::AsNumber(EquippedArmorCount));
		Args.Add(TEXT("Defense"), FText::AsNumber(FMath::RoundToInt(BonusDef)));
		Args.Add(TEXT("Hp"), FText::AsNumber(FMath::RoundToInt(BonusHp)));
	});

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
	if (ClassIdValue < static_cast<int32>(EClassId::Warrior) || ClassIdValue > static_cast<int32>(EClassId::Cleric))
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
	DrawText(ViewModel.WeaknessLabel.ToString(), ViewModel.WeaknessColor, X + PanelWidth - 118.0f * Scale, Y + 15.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.78f * Scale);
	if (ViewModel.bBossStage)
	{
		const float BadgeWidth = 58.0f * Scale;
		const float BadgeHeight = 22.0f * Scale;
		const float BadgeX = X + PanelWidth - Padding - BadgeWidth;
		const float BadgeY = Y + 42.0f * Scale;
		DrawRect(AccentGold, BadgeX, BadgeY, BadgeWidth, BadgeHeight);
		DrawText(ViewModel.BossBadgeLabel.ToString(), BgPrimary, BadgeX + 13.0f * Scale, BadgeY + 4.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.72f * Scale);
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
		IdleGameInstance->GetGold(),
		LastShopPurchaseResult);

	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelWidth = FMath::Clamp(Canvas->SizeX * 0.22f, 300.0f * Scale, 380.0f * Scale);
	const float PanelHeight = (ViewModel.bHasLastResult ? 178.0f : 144.0f) * Scale;
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
	DrawText(ViewModel.CostLabel.ToString(), ViewModel.bCanBuyGearRoll ? Theme::TextPrimary : Theme::AccentRed, X + Padding, Y + 54.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.82f * Scale);
	DrawText(ViewModel.StatusLabel.ToString(), ViewModel.bCanBuyGearRoll ? Theme::AccentBlue : Theme::AccentRed, X + Padding, Y + 82.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.76f * Scale);

	const float ButtonWidth = 112.0f * Scale;
	const float ButtonHeight = 32.0f * Scale;
	const float ButtonX = X + PanelWidth - Padding - ButtonWidth;
	const float ButtonY = Y + 68.0f * Scale;
	DrawRect(ViewModel.bCanBuyGearRoll ? Theme::AccentGold : Theme::BgPrimary.CopyWithNewOpacity(0.94f), ButtonX, ButtonY, ButtonWidth, ButtonHeight);
	DrawText(ViewModel.ButtonLabel.ToString(), ViewModel.bCanBuyGearRoll ? Theme::BgPrimary : Theme::TextMuted, ButtonX + 16.0f * Scale, ButtonY + 8.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.74f * Scale);
	if (ViewModel.bCanBuyGearRoll)
	{
		AddHitBox(FVector2D(ButtonX, ButtonY), FVector2D(ButtonWidth, ButtonHeight), ViewModel.GearRollHitBoxName, true, 84);
	}

	if (ViewModel.bHasLastResult)
	{
		const FLinearColor ResultColor = ViewModel.bLastResultError ? Theme::AccentRed : RarityToColor(ViewModel.LastResultRarity);
		DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.86f), X + Padding, Y + 118.0f * Scale, PanelWidth - Padding * 2.0f, 34.0f * Scale);
		DrawRect(ResultColor, X + Padding, Y + 118.0f * Scale, 4.0f * Scale, 34.0f * Scale);
		DrawText(ViewModel.LastResultLabel.ToString(), ResultColor, X + Padding + 12.0f * Scale, Y + 126.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.74f * Scale);
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
		EnhanceFeedbackLabel,
		bEnhanceFeedbackSuccess);

	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelWidth = FMath::Clamp(Canvas->SizeX * 0.25f, 360.0f * Scale, 460.0f * Scale);
	const float HeaderHeight = 52.0f * Scale;
	const float RowHeight = 40.0f * Scale;
	const float RowGap = 6.0f * Scale;
	const float Padding = 14.0f * Scale;
	const float FeedbackHeight = ViewModel.FeedbackLabel.IsEmpty() ? 0.0f : 24.0f * Scale;
	const float PanelHeight = HeaderHeight + ViewModel.Rows.Num() * RowHeight + FMath::Max(0, ViewModel.Rows.Num() - 1) * RowGap + Padding + FeedbackHeight;
	const float X = Canvas->SizeX - PanelWidth - 28.0f * Scale;
	const float Y = 526.0f * Scale;
	const float Border = 2.0f * Scale;

	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.91f), X, Y, PanelWidth, PanelHeight);
	DrawRect(Theme::AccentGold, X, Y, PanelWidth, Border);
	DrawRect(Theme::AccentGold, X, Y + PanelHeight - Border, PanelWidth, Border);
	DrawRect(Theme::AccentGold, X, Y, Border, PanelHeight);
	DrawRect(Theme::AccentGold, X + PanelWidth - Border, Y, Border, PanelHeight);

	DrawText(ViewModel.Title.ToString(), Theme::TextPrimary, X + Padding, Y + 12.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 0.92f * Scale);
	DrawText(ViewModel.GoldLabel.ToString(), Theme::AccentGold, X + PanelWidth - 114.0f * Scale, Y + 16.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.78f * Scale);

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
	DrawText(Row.SuccessRateLabel.ToString(), Row.bCanEnhance ? Theme::AccentBlue : Theme::TextMuted, X + 206.0f * Scale, Y + 22.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.64f * Scale);

	const float ButtonWidth = 72.0f * Scale;
	const float ButtonHeight = 26.0f * Scale;
	const float ButtonX = X + Width - ButtonWidth - 8.0f * Scale;
	const float ButtonY = Y + 7.0f * Scale;
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

	FString RawSlot = BoxName.ToString();
	RawSlot.RightChopInline(EnhanceSlotHitBoxPrefix.Len());
	const int32 SlotValue = FCString::Atoi(*RawSlot);
	if (SlotValue < static_cast<int32>(EItemSlot::Weapon) || SlotValue > static_cast<int32>(EItemSlot::Accessory))
	{
		return;
	}

	IdleGameInstance->TryEnhanceEquipped(static_cast<EItemSlot>(SlotValue), PlayerInventory);
	RefreshEquipmentSummary();
	RefreshMouseInteraction();
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
		PlayerSkills->RankUpSkill(FName(*SkillId));
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
	const float RowHeight = 74.0f * Scale;
	const float HeaderHeight = 54.0f * Scale;
	const float Padding = 18.0f * Scale;
	const float PanelHeight = HeaderHeight + Padding + FMath::Max(1, ViewModel.Rows.Num()) * (RowHeight + 10.0f * Scale) + Padding;
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

	for (const FIdleHUDQuestLogRowViewModel& Row : ViewModel.Rows)
	{
		const float RowX = X + Padding;
		const float InnerWidth = PanelWidth - Padding * 2.0f;
		DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.90f), RowX, RowY, InnerWidth, RowHeight);
		DrawRect(Row.bCanClaim ? Theme::AccentGold : Theme::TextMuted.CopyWithNewOpacity(0.42f), RowX, RowY, 4.0f * Scale, RowHeight);

		DrawText(Row.TypeLabel.ToString(), Row.bCanClaim ? Theme::AccentGold : Theme::AccentBlue, RowX + 12.0f * Scale, RowY + 8.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.82f * Scale);
		DrawText(Row.Title.ToString(), Theme::TextPrimary, RowX + 58.0f * Scale, RowY + 8.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.92f * Scale);
		DrawText(Row.ProgressLabel.ToString(), Theme::TextMuted, RowX + 12.0f * Scale, RowY + 33.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.82f * Scale);
		DrawText(Row.RewardLabel.ToString(), Theme::TextMuted, RowX + 132.0f * Scale, RowY + 33.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.82f * Scale);

		const float BarX = RowX + 12.0f * Scale;
		const float BarY = RowY + RowHeight - 12.0f * Scale;
		const float BarWidth = InnerWidth - 104.0f * Scale;
		DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.96f), BarX, BarY, BarWidth, 5.0f * Scale);
		DrawRect(Row.bCanClaim ? Theme::AccentGold : Theme::AccentBlue, BarX, BarY, BarWidth * Row.ProgressRatio, 5.0f * Scale);

		const float ButtonWidth = 72.0f * Scale;
		const float ButtonHeight = 30.0f * Scale;
		const float ButtonX = RowX + InnerWidth - ButtonWidth - 12.0f * Scale;
		const float ButtonY = RowY + 22.0f * Scale;
		DrawRect(Row.bCanClaim ? Theme::AccentGold : Theme::BgPanel, ButtonX, ButtonY, ButtonWidth, ButtonHeight);
		DrawText(Row.ActionLabel.ToString(), Row.bCanClaim ? Theme::BgPrimary : Theme::TextMuted, ButtonX + 19.0f * Scale, ButtonY + 7.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.86f * Scale);
		if (Row.bCanClaim)
		{
			AddHitBox(FVector2D(ButtonX, ButtonY), FVector2D(ButtonWidth, ButtonHeight), MakeQuestClaimHitBoxName(Row.QuestId), true, 90);
		}

		RowY += RowHeight + 10.0f * Scale;
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
		});

	const UWorld* World = GetWorld();
	const float PetFeedbackElapsed = World ? World->GetTimeSeconds() - PetFeedbackStartTime : 0.0f;
	const bool bShowPetFeedback = !PetFeedbackLabel.IsEmpty() && PetFeedbackElapsed <= PetFeedbackDurationSeconds;

	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelWidth = 322.0f * Scale;
	const float HeaderHeight = 44.0f * Scale;
	const float RowHeight = 64.0f * Scale;
	const float RowGap = 8.0f * Scale;
	const float Padding = 14.0f * Scale;
	const float FeedbackHeight = bShowPetFeedback ? 26.0f * Scale : 0.0f;
	const float PanelHeight = HeaderHeight + 48.0f * Scale + ViewModel.Rows.Num() * RowHeight + FMath::Max(0, ViewModel.Rows.Num() - 1) * RowGap + Padding + FeedbackHeight;
	const float X = 28.0f * Scale;
	const float Y = 360.0f * Scale;
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

	float RowY = Y + HeaderHeight + 42.0f * Scale;
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

	const float Scale = Height / 64.0f;
	const FLinearColor StateColor = Row.bEquipped ? Theme::AccentGold : Theme::TextMuted.CopyWithNewOpacity(0.56f);
	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.90f), X, Y, Width, Height);
	DrawRect(StateColor, X, Y, 4.0f * Scale, Height);

	DrawText(Row.Name.ToString(), Row.bEquipped ? Theme::AccentGold : Theme::TextPrimary, X + 12.0f * Scale, Y + 7.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.86f * Scale);
	DrawText(Row.LevelLabel.ToString(), Theme::TextMuted, X + 92.0f * Scale, Y + 8.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.76f * Scale);
	DrawText(Row.BonusLabel.ToString(), Theme::TextMuted, X + 12.0f * Scale, Y + 29.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.74f * Scale);
	DrawText(Row.FeedCostLabel.ToString(), Theme::TextMuted, X + 110.0f * Scale, Y + 29.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.74f * Scale);
	DrawText(Row.StatusLabel.ToString(), Row.bCanFeed ? Theme::AccentBlue : Theme::Warn, X + 12.0f * Scale, Y + 47.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.66f * Scale);

	const float ButtonWidth = 78.0f * Scale;
	const float ButtonHeight = 24.0f * Scale;
	const float ButtonX = X + Width - ButtonWidth - 8.0f * Scale;
	const float ButtonY = Y + 7.0f * Scale;
	DrawRect(Row.bEquipped ? Theme::BgPanel : Theme::AccentGold, ButtonX, ButtonY, ButtonWidth, ButtonHeight);
	DrawText(Row.ActionLabel.ToString(), Row.bEquipped ? Theme::TextMuted : Theme::BgPrimary, ButtonX + 11.0f * Scale, ButtonY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.72f * Scale);
	if (!Row.bEquipped)
	{
		AddHitBox(FVector2D(ButtonX, ButtonY), FVector2D(ButtonWidth, ButtonHeight), MakePetEquipHitBoxName(Row.PetId), true, 82);
	}

	const float FeedButtonY = Y + 34.0f * Scale;
	DrawRect(Row.bCanFeed ? Theme::AccentBlue : Theme::BgPanel, ButtonX, FeedButtonY, ButtonWidth, ButtonHeight);
	DrawText(Row.FeedActionLabel.ToString(), Row.bCanFeed ? Theme::BgPrimary : Theme::TextMuted, ButtonX + 11.0f * Scale, FeedButtonY + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.72f * Scale);
	if (Row.bCanFeed)
	{
		AddHitBox(FVector2D(ButtonX, FeedButtonY), FVector2D(ButtonWidth, ButtonHeight), MakePetFeedHitBoxName(Row.PetId), true, 83);
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
	const float PanelWidth = 420.0f * Scale;
	const float HeaderHeight = 54.0f * Scale;
	const float RowHeight = 29.0f * Scale;
	const float RowGap = 5.0f * Scale;
	const float Padding = 14.0f * Scale;
	const float PanelHeight = HeaderHeight + 30.0f * Scale + ViewModel.Rows.Num() * RowHeight + FMath::Max(0, ViewModel.Rows.Num() - 1) * RowGap + Padding;
	const float X = 28.0f * Scale;
	const float Y = 574.0f * Scale;
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

	float RowY = Y + HeaderHeight + 22.0f * Scale;
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

void AIdleHUD::RefreshMouseInteraction()
{
	if (!PlayerOwner)
	{
		return;
	}

	const bool bRebirthReady = IdleGameInstance && IdleGameInstance->CanRebirth();
	const bool bTranscendReady = IdleGameInstance && IdleGameInstance->CanTranscend();
	const bool bNeedsPointer = ResolvePlayerCharacter() || PlayerInventory || bQuestLogVisible || bStatInfoVisible || OfflineRewardModal.bVisible || bRebirthReady || bTranscendReady;
	PlayerOwner->bShowMouseCursor = bNeedsPointer;
	PlayerOwner->bEnableClickEvents = bNeedsPointer;
}
