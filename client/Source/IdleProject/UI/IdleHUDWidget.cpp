#include "UI/IdleHUDWidget.h"

#include "Styling/CoreStyle.h"
#include "UI/UIThemeTokens.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
FText FormatHp(float Current, float Max)
{
	return FText::FromString(FString::Printf(TEXT("HP %.0f / %.0f"), Current, Max));
}

FText FormatExp(int64 Current, int64 Next)
{
	return FText::FromString(FString::Printf(TEXT("EXP %lld / %lld"), Current, Next));
}

FText FormatGold(int64 Amount)
{
	return FText::FromString(FString::Printf(TEXT("골드 %lld"), Amount));
}

FText FormatLevel(int32 Level)
{
	return FText::FromString(FString::Printf(TEXT("Lv. %d"), Level));
}

FString RarityToString(EItemRarity Rarity)
{
	switch (Rarity)
	{
	case EItemRarity::Uncommon:
		return TEXT("Uncommon");
	case EItemRarity::Rare:
		return TEXT("Rare");
	case EItemRarity::Common:
		return TEXT("Common");
	case EItemRarity::None:
	default:
		return TEXT("None");
	}
}

FText FormatEquipmentLine(const TCHAR* Label, const FItemInstance* Item)
{
	if (!Item)
	{
		return FText::FromString(FString::Printf(TEXT("%s: -"), Label));
	}

	return FText::FromString(FString::Printf(
		TEXT("%s: %s (%s, ATK+%.0f DEF+%.0f HP+%.0f)"),
		Label,
		*Item->DisplayName.ToString(),
		*RarityToString(Item->Rarity),
		Item->BonusAtk,
		Item->BonusDef,
		Item->BonusHp));
}
}

void SIdleHUDWidget::Construct(const FArguments& InArgs)
{
	using namespace IdleProject::UI;

	const FSlateFontInfo LabelFont = FCoreStyle::GetDefaultFontStyle("Bold", 18);

	ChildSlot
	[
		SNew(SOverlay)

		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(FMargin(24.0f, 24.0f, 0.0f, 0.0f))
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.0f, 0.0f, 0.0f, 6.0f))
			[
				SAssignNew(HpText, STextBlock)
				.Font(LabelFont)
				.ColorAndOpacity(Theme::AccentRed)
				.Text(FormatHp(0.0f, 0.0f))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.0f, 0.0f, 0.0f, 6.0f))
			[
				SAssignNew(ExpText, STextBlock)
				.Font(LabelFont)
				.ColorAndOpacity(Theme::AccentBlue)
				.Text(FormatExp(0, 0))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(LevelText, STextBlock)
				.Font(LabelFont)
				.ColorAndOpacity(Theme::TextPrimary)
				.Text(FormatLevel(1))
			]
		]

		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.Padding(FMargin(0.0f, 24.0f, 24.0f, 0.0f))
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(GoldText, STextBlock)
				.Font(LabelFont)
				.ColorAndOpacity(Theme::AccentGold)
				.Text(FormatGold(0))
			]
		]

		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(FMargin(24.0f, 0.0f, 0.0f, 24.0f))
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.0f, 0.0f, 0.0f, 6.0f))
			[
				SAssignNew(WeaponText, STextBlock)
				.Font(LabelFont)
				.ColorAndOpacity(Theme::TextPrimary)
				.Text(FormatEquipmentLine(TEXT("무기"), nullptr))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(ArmorText, STextBlock)
				.Font(LabelFont)
				.ColorAndOpacity(Theme::TextMuted)
				.Text(FormatEquipmentLine(TEXT("방어구"), nullptr))
			]
		]
	];
}

void SIdleHUDWidget::UpdateHp(float Current, float Max)
{
	if (HpText)
	{
		HpText->SetText(FormatHp(Current, Max));
	}
}

void SIdleHUDWidget::UpdateExp(int64 Current, int64 Next)
{
	if (ExpText)
	{
		ExpText->SetText(FormatExp(Current, Next));
	}
}

void SIdleHUDWidget::UpdateGold(int64 Amount)
{
	if (GoldText)
	{
		GoldText->SetText(FormatGold(Amount));
	}
}

void SIdleHUDWidget::UpdateLevel(int32 Level)
{
	if (LevelText)
	{
		LevelText->SetText(FormatLevel(Level));
	}
}

void SIdleHUDWidget::UpdateEquipmentSummary(const FItemInstance* Weapon, const FItemInstance* Armor)
{
	if (WeaponText)
	{
		WeaponText->SetText(FormatEquipmentLine(TEXT("무기"), Weapon));
	}
	if (ArmorText)
	{
		ArmorText->SetText(FormatEquipmentLine(TEXT("방어구"), Armor));
	}
}
