#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/UIThemeTokens.h"
#include "MainMenuController.generated.h"

/**
 * W_MainMenu Blueprint가 상속할 메인 메뉴 C++ 베이스입니다.
 * 버튼 바인딩은 BP에서 이 함수를 호출하도록 연결합니다.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class IDLEPROJECT_API UMainMenuController : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Theme")
	FLinearColor BackgroundColor = IdleProject::UI::Theme::BgPrimary;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Theme")
	FLinearColor PanelColor = IdleProject::UI::Theme::BgPanel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Theme")
	FLinearColor PrimaryTextColor = IdleProject::UI::Theme::TextPrimary;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Theme")
	FLinearColor MutedTextColor = IdleProject::UI::Theme::TextMuted;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Theme")
	FLinearColor PrimaryActionColor = IdleProject::UI::Theme::AccentGold;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Theme")
	FLinearColor ErrorColor = IdleProject::UI::Theme::ErrorCritical;

	UFUNCTION(BlueprintCallable, Category = "Idle|MainMenu")
	void StartGame();

	UFUNCTION(BlueprintCallable, Category = "Idle|MainMenu")
	void OpenSettings();

	UFUNCTION(BlueprintCallable, Category = "Idle|MainMenu")
	void Quit();
};
