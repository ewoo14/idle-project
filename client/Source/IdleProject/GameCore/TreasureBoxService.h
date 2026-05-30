#pragma once

#include "CoreMinimal.h"
#include "GameCore/TreasureBoxTypes.h"
#include "Math/RandomStream.h"
#include "UObject/Object.h"
#include "TreasureBoxService.generated.h"

/**
 * 보물 상자(Treasure Box / 일일 뽑기) 서비스. 하루 1회 무료 보물 상자에서 가중 랜덤 보상을 뽑는
 * RNG 리텐션 루프를 담당한다. 추첨(가중 구간 매핑)/수량은 클라 권위(FRandomStream, #71 패턴)이며,
 * 보상 풀(TREASURE_POOL)·누적 가중 구간 매핑(PickReward)은 서버 treasureBox.ts 를 1:1 미러링한다.
 * 1일 1회 판정(LastDrawDate)·누적 뽑기 횟수(TotalDraws)는 클라 로컬 세이브 권위.
 * 보상 지급은 GameInstance(DrawTreasureBox)가 단일 지점에서 담당한다(이중 지급 금지).
 * AttendanceService 패턴(서비스 구조·RestoreState·UTC date 1일 1회)을 모방한다.
 */
UCLASS(BlueprintType)
class IDLEPROJECT_API UTreasureBoxService : public UObject
{
	GENERATED_BODY()

public:
	// 보상 풀(서버 TREASURE_POOL 1:1 미러). 순서가 곧 누적 가중 구간 순서이므로 서버와 동일 순서를 엄수한다.
	static const TArray<FTreasurePoolEntry>& GetPool();

	// 풀 전체 가중치 합(= totalWeight). roll 범위는 0 ~ GetTotalWeight()-1. 서버 getTotalTreasureWeight() 와 1:1(=100).
	static int32 GetTotalWeight();

	// roll(0 ~ totalWeight-1)을 누적 가중 구간에 매핑하여 보상 종류 반환(결정적).
	// 음수 roll 은 0 으로, totalWeight-1 초과는 마지막 구간으로 클램프. 서버 pickTreasureReward(roll) 와 1:1.
	static ETreasureReward PickReward(int32 Roll);

	// 오늘(Date) 뽑기 가능 여부. Date != LastDrawDate 이고 Date 가 비어있지 않으면 true(1일 1회).
	UFUNCTION(BlueprintPure, Category = "Idle|TreasureBox")
	bool CanDrawToday(const FString& Date) const;

	// 오늘(Date) 보물 상자 1회 뽑기. CanDrawToday 아니면 빈 보상(Reward=None).
	// 아니면 Roll=Rng.RandRange(0, GetTotalWeight()-1) → PickReward → 수량 Rng.RandRange(min,max)
	// → LastDrawDate=Date·TotalDraws++ 후 결과 반환. RNG(roll·수량)는 클라 권위.
	FTreasureReward DrawTreasure(const FString& Date, FRandomStream& Rng);

	UFUNCTION(BlueprintPure, Category = "Idle|TreasureBox")
	int64 GetTotalDraws() const { return TotalDraws; }

	UFUNCTION(BlueprintPure, Category = "Idle|TreasureBox")
	const FString& GetLastDrawDate() const { return LastDrawDate; }

	// 세이브 복원. 음수 누적은 0 으로 클램프.
	void RestoreState(const FString& Date, int64 Total);

private:
	UPROPERTY()
	FString LastDrawDate;

	UPROPERTY()
	int64 TotalDraws = 0;
};
