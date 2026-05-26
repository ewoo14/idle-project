#include "UI/IdleHUDWidget.h"

#include "Internationalization/IdleLocalization.h"
#include "Styling/CoreStyle.h"
#include "UI/UIThemeTokens.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
FText FormatHp(float Current, float Max)
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("Current"), FText::AsNumber(FMath::RoundToInt(Current)));
	Args.Add(TEXT("Max"), FText::AsNumber(FMath::RoundToInt(Max)));
	return IdleProject::Localization::UI(TEXT("HUD_HP_FORMAT"), Args);
}

FText FormatExp(int64 Current, int64 Next)
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("Current"), FText::AsNumber(Current));
	Args.Add(TEXT("Next"), FText::AsNumber(Next));
	return IdleProject::Localization::UI(TEXT("HUD_EXP_FORMAT"), Args);
}

FText FormatGold(int64 Amount)
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("Amount"), FText::AsNumber(Amount));
	return IdleProject::Localization::UI(TEXT("HUD_GOLD_FORMAT"), Args);
}

FText FormatLevel(int32 Level)
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("Level"), FText::AsNumber(Level));
	return IdleProject::Localization::UI(TEXT("HUD_LEVEL_FORMAT"), Args);
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
			[
				SAssignNew(EquipmentText, STextBlock)
				.Font(LabelFont)
				.ColorAndOpacity(Theme::TextMuted)
				.Text(FText::Format(
					FText::FromString(TEXT("{0}\n{1}")),
					IdleProject::Localization::UI(TEXT("HUD_NO_WEAPON")),
					IdleProject::Localization::UI(TEXT("HUD_ARMOR_SUMMARY_FORMAT"), []()
					{
						FFormatNamedArguments Args;
						Args.Add(TEXT("Count"), FText::AsNumber(0));
						Args.Add(TEXT("Defense"), FText::AsNumber(0));
						Args.Add(TEXT("Hp"), FText::AsNumber(0));
						return Args;
					}())))
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

void SIdleHUDWidget::UpdateEquipment(const FText& WeaponName, const FText& ArmorSummary, FLinearColor WeaponRarityColor)
{
	if (EquipmentText)
	{
		EquipmentText->SetText(FText::Format(FText::FromString(TEXT("{0}\n{1}")), WeaponName, ArmorSummary));
		EquipmentText->SetColorAndOpacity(WeaponRarityColor);
	}
}
