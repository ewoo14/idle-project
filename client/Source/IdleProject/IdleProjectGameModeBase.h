#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "IdleProjectGameModeBase.generated.h"

/**
 * IdleProject 기본 게임 모드입니다.
 * PR #4에서는 전사 캐릭터 1종과 기본 Pawn 연결만 담당합니다.
 */
UCLASS()
class IDLEPROJECT_API AIdleProjectGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AIdleProjectGameModeBase();
};
