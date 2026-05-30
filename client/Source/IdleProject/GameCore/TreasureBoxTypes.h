#pragma once

#include "CoreMinimal.h"
#include "TreasureBoxTypes.generated.h"

// 보물 상자(Treasure Box / 일일 뽑기) 보상 종류.
// 서버 treasureBox.ts TreasureReward("gold"|"essence"|"consumable"|"protectionScroll"|"resetCube"|"rankCube")와 1:1.
// 모두 실존 재화(신규 재화 없음). None 은 뽑기 불가(이미 오늘 뽑음) 시 빈 보상 표식.
UENUM(BlueprintType)
enum class ETreasureReward : uint8
{
	None,
	Gold,
	Essence,
	Consumable,
	ProtectionScroll,
	ResetCube,
	RankCube
};

// 보상 풀 엔트리. Reward(보상 종류)/Weight(가중치)/MinAmount~MaxAmount(수량 범위).
// 서버 treasureBox.ts TREASURE_POOL 과 1:1 미러(순서=누적 가중 구간 순서 엄수).
USTRUCT(BlueprintType)
struct IDLEPROJECT_API FTreasurePoolEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|TreasureBox")
	ETreasureReward Reward = ETreasureReward::None;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|TreasureBox")
	int32 Weight = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|TreasureBox")
	int64 MinAmount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|TreasureBox")
	int64 MaxAmount = 0;
};

// 1회 뽑기 결과(보상 종류/수량). Reward==None 이면 빈 보상(뽑기 불가).
USTRUCT(BlueprintType)
struct IDLEPROJECT_API FTreasureReward
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|TreasureBox")
	ETreasureReward Reward = ETreasureReward::None;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|TreasureBox")
	int64 Amount = 0;
};
