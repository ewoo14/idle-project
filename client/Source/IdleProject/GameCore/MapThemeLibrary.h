#pragma once

#include "CoreMinimal.h"
#include "GameCore/MapThemeTypes.h"

/** 챕터별 맵 테마 제공(클라 전용 시각). 8 챕터 + 범위 클램프. */
class IDLEPROJECT_API FMapThemeLibrary
{
public:
	static constexpr int32 ThemeCount = 8;

	// 챕터(1~8) 테마. 범위 밖은 [1,8] 로 클램프.
	static FMapTheme GetTheme(int32 Chapter);
};
