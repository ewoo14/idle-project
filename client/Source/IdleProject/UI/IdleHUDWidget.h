#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class STextBlock;

class IDLEPROJECT_API SIdleHUDWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SIdleHUDWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void UpdateHp(float Current, float Max);
	void UpdateExp(int64 Current, int64 Next);
	void UpdateGold(int64 Amount);
	void UpdateLevel(int32 Level);

private:
	TSharedPtr<STextBlock> HpText;
	TSharedPtr<STextBlock> ExpText;
	TSharedPtr<STextBlock> GoldText;
	TSharedPtr<STextBlock> LevelText;
};
