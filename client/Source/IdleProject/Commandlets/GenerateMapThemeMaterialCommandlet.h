#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "GenerateMapThemeMaterialCommandlet.generated.h"

/**
 * 맵 테마용 파라미터 머티리얼(/Game/Maps/M_MapTheme)을 절차적으로 생성·저장하는
 * 에디터 전용 커맨드릿. "Color" 벡터 파라미터 → BaseColor(+약한 Emissive), Roughness 상수.
 * 실행: UnrealEditor-Cmd.exe "<IdleProject.uproject>" -run=GenerateMapThemeMaterial
 *       -unattended -nopause -nosplash
 */
UCLASS()
class UGenerateMapThemeMaterialCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	virtual int32 Main(const FString& Params) override;
};
