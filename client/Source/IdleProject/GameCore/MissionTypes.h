#pragma once

#include "CoreMinimal.h"
#include "MissionTypes.generated.h"

// 미션 기간. 서버 mission.ts MissionPeriod("Daily"|"Weekly") 와 1:1.
UENUM(BlueprintType)
enum class EMissionPeriod : uint8
{
	Daily = 0,
	Weekly = 1
};

// 미션 진행 메트릭(미션 전용 누적형). 서버 mission.ts MissionMetric 문자열과 1:1.
// 업적 메트릭과 이름이 겹치는 항목은 의미도 동일(누적 카운터, 델타 누적).
UENUM(BlueprintType)
enum class EMissionMetric : uint8
{
	MonstersKilled = 0,
	BossesKilled = 1,
	StagesCleared = 2,
	DungeonRuns = 3,
	GearEnhanced = 4,
	GoldEarned = 5
};

// 미션 보상 종류. 서버 mission.ts MissionRewardType("gold"|"essence"|"consumable")와 1:1.
UENUM(BlueprintType)
enum class EMissionReward : uint8
{
	Gold = 0,
	Essence = 1,
	Consumable = 2
};

// 미션 정의. 서버 카탈로그(MISSION_CATALOG)를 1:1 이식한다.
// 진행이 Target 이상이면 완료, 수령 시 RewardType/RewardValue 만큼 재화 지급.
USTRUCT(BlueprintType)
struct IDLEPROJECT_API FMissionDefinition
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Mission")
	FString Id;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Mission")
	EMissionPeriod Period = EMissionPeriod::Daily;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Mission")
	EMissionMetric Metric = EMissionMetric::MonstersKilled;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Mission")
	int64 Target = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Mission")
	EMissionReward RewardType = EMissionReward::Gold;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Mission")
	int64 RewardValue = 0;
};

// UI 표시용 미션 진행 뷰(정의 + 현재 진행/완료/수령 상태).
USTRUCT(BlueprintType)
struct IDLEPROJECT_API FMissionProgressView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Mission")
	FString Id;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Mission")
	EMissionPeriod Period = EMissionPeriod::Daily;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Mission")
	EMissionMetric Metric = EMissionMetric::MonstersKilled;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Mission")
	int64 Target = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Mission")
	int64 Progress = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Mission")
	EMissionReward RewardType = EMissionReward::Gold;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Mission")
	int64 RewardValue = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Mission")
	bool bCompleted = false;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Mission")
	bool bClaimed = false;
};
