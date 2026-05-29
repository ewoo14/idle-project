#pragma once

#include "CoreMinimal.h"
#include "GameCore/MissionTypes.h"
#include "UObject/Object.h"
#include "MissionService.generated.h"

/**
 * 일일/주간 미션(Daily/Weekly Mission) 서비스. 처치/스테이지/던전/강화/골드 등 메트릭을
 * 누적하여 목표 달성 시 보상을 수령한다. 진행/수령/리셋은 클라 로컬 세이브 권위.
 * 카탈로그는 서버 mission.ts(MISSION_CATALOG) 10종(일일 6 + 주간 4)을 1:1 이식한다
 * (id/period/metric/target/rewardType/rewardValue 동일). 보상 지급은 GameInstance 가 담당.
 */
UCLASS(BlueprintType)
class IDLEPROJECT_API UMissionService : public UObject
{
	GENERATED_BODY()

public:
	// 서버 MISSION_CATALOG 10종을 1:1 로 빌드한다(중복 호출 시 재구성하지 않음).
	UFUNCTION(BlueprintCallable, Category = "Idle|Mission")
	void InitializeDefaultMissions();

	// 해당 metric 을 사용하는 모든 미션의 진행을 Delta 만큼 누적한다(누적형 델타). Delta<=0 은 무시.
	UFUNCTION(BlueprintCallable, Category = "Idle|Mission")
	void RecordProgress(EMissionMetric Metric, int64 Delta);

	// 진행 >= 목표면 완료. 정의에 없는 id 는 false.
	UFUNCTION(BlueprintPure, Category = "Idle|Mission")
	bool IsComplete(const FString& Id) const;

	UFUNCTION(BlueprintPure, Category = "Idle|Mission")
	bool IsClaimed(const FString& Id) const { return Claimed.Contains(Id); }

	// 완료 && 미수령이면 수령 마킹(true). 보상 지급은 GameInstance 가 담당. 미달/중복/무효 id 는 false.
	UFUNCTION(BlueprintCallable, Category = "Idle|Mission")
	bool ClaimMission(const FString& Id);

	// 기간 경계 처리. Date 가 DailyResetDate 와 다르면 일일 미션 진행/수령 제거 후 마커 갱신,
	// Week 가 WeeklyResetWeek 와 다르면 주간 미션 동일 처리. 빈 문자열은 현재 UTC 값을 사용.
	UFUNCTION(BlueprintCallable, Category = "Idle|Mission")
	void EnsurePeriodFresh(const FString& Date, const FString& Week);

	UFUNCTION(BlueprintPure, Category = "Idle|Mission")
	const TArray<FMissionDefinition>& GetDefinitions() const { return Definitions; }

	// 정의에 없거나 미진행이면 0.
	UFUNCTION(BlueprintPure, Category = "Idle|Mission")
	int64 GetProgress(const FString& Id) const { return Progress.FindRef(Id); }

	// 정의 + 현재 진행/완료/수령 상태를 카탈로그 순서로 반환(UI 용).
	UFUNCTION(BlueprintPure, Category = "Idle|Mission")
	TArray<FMissionProgressView> GetProgressViews() const;

	bool GetDefinition(const FString& Id, FMissionDefinition& OutDefinition) const;

	const TMap<FString, int64>& GetProgressMap() const { return Progress; }
	const TSet<FString>& GetClaimedSet() const { return Claimed; }
	const FString& GetDailyResetDate() const { return DailyResetDate; }
	const FString& GetWeeklyResetWeek() const { return WeeklyResetWeek; }

	// 세이브 복원. 정의에 없는 id 는 무시하며, 수령은 진행/완료와 무관하게 그대로 유지(복원 후 재계산은 호출 측 책임).
	void RestoreState(const TMap<FString, int64>& InProgress, const TSet<FString>& InClaimed, const FString& InDate, const FString& InWeek);

private:
	TArray<FMissionDefinition> Definitions;
	TMap<FString, FMissionDefinition> DefinitionById;
	TMap<FString, int64> Progress;
	TSet<FString> Claimed;
	FString DailyResetDate;
	FString WeeklyResetWeek;

	void BuildDefaultDefinitions();
	// 지정 기간의 모든 미션 진행/수령을 제거한다.
	void ResetPeriod(EMissionPeriod Period);
};
