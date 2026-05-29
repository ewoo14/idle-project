#pragma once

#include "CoreMinimal.h"

struct IDLEPROJECT_API FPetLevelFormula
{
	static constexpr int32 MaxPetLevel = 10;

	// 펫 진화(별/Star) parity 상수 — 서버 petBonus.ts 와 1:1.
	static constexpr int64 PetEvolveBase = 50000;
	static constexpr double PetEvolveGrowth = 1.8;
	static constexpr float PetStarStep = 0.15f;

	static int64 GetFeedCost(int32 CurrentLevel);
	static float GetBonusMultiplier(int32 Level);

	// 다음 별로 진화하는 데 필요한 골드 비용. 기하 증가(무한 sink), 음수 별은 0성 처리.
	// 서버 getPetEvolveCost(star) = floor(BASE * GROWTH^max(0,star)) 와 1:1.
	static int64 GetPetEvolveCost(int32 Star);

	// 장착 펫 보너스에 곱하는 별 배수. 선형 무한(star0=1.0), 음수 별은 0성 처리.
	// 서버 getPetStarMultiplier(star) = fround(1 + STEP * max(0,star)) 와 1:1(float 연산 순서 동일).
	static float GetPetStarMultiplier(int32 Star);
};
