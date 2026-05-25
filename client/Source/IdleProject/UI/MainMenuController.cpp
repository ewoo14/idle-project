#include "UI/MainMenuController.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UMainMenuController::StartGame()
{
	// PR #4에서는 PM이 후속으로 만든 Game 맵 또는 기본 테스트 맵으로 전환합니다.
	UGameplayStatics::OpenLevel(this, FName(TEXT("Game")));
}

void UMainMenuController::OpenSettings()
{
	UE_LOG(LogTemp, Display, TEXT("MainMenu settings requested."));
}

void UMainMenuController::Quit()
{
	APlayerController* PlayerController = GetOwningPlayer();
	UKismetSystemLibrary::QuitGame(this, PlayerController, EQuitPreference::Quit, false);
}
