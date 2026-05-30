#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "GenerateMapSkyAssetsCommandlet.generated.h"

/**
 * 절차 스카이 에셋 생성(에디터 전용): 그라데이션 TextureCube(/Game/Maps/TC_MapSky) +
 * Unlit 스카이 머티리얼(/Game/Maps/M_Sky, SkyCube 큐브 샘플 × SkyTint 벡터 파라미터).
 * 실행: UnrealEditor-Cmd.exe "<uproject>" -run=GenerateMapSkyAssets -unattended -nopause -nosplash
 */
UCLASS()
class UGenerateMapSkyAssetsCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	virtual int32 Main(const FString& Params) override;
};
