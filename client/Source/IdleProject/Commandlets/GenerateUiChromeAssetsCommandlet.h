#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "GenerateUiChromeAssetsCommandlet.generated.h"

/**
 * 절차 UI 크롬 텍스처 생성(에디터 전용): 원신 풍 9-slice 패널/버튼 텍스처(/Game/UI/Chrome/T_*).
 * 둥근 사각형 + 골드 더블 프레임 패널 2종, 버튼 2종, 소프트 섀도우, 골드 스타를
 * UTexture2D(BGRA8) 로 절차 생성하여 LFS 로 커밋한다.
 * 실행: UnrealEditor-Cmd.exe "<uproject>" -run=GenerateUiChromeAssets -unattended -nopause -nosplash
 */
UCLASS()
class UGenerateUiChromeAssetsCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	virtual int32 Main(const FString& Params) override;
};
