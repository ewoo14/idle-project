#pragma once

#include "CoreMinimal.h"
#include "GameCore/AttendanceTypes.h"
#include "UObject/Object.h"
#include "AttendanceService.generated.h"

/**
 * 출석 보상(Attendance Rewards) 서비스. 하루 1회 출석 체크인으로 누적 출석일을 늘리고,
 * 무한 기하 임계 마일스톤(floor(BASE * GROWTH^(n-1)))에 도달하면 보상을 수령한다.
 * 체크인/누적/수령은 클라 로컬 세이브 권위이며, 마일스톤 임계/보상 매핑은 서버
 * attendance.ts 를 1:1 미러링한다(f32 곱셈 — 서버 Math.fround 와 동일, double 금지).
 * 보상 지급은 GameInstance(ClaimAttendanceMilestone)가 단일 지점에서 담당한다.
 * 주간 보스(WeeklyBossService) 마일스톤 패턴을 모방한다.
 */
UCLASS(BlueprintType)
class IDLEPROJECT_API UAttendanceService : public UObject
{
	GENERATED_BODY()

public:
	// 마일스톤 임계 기하 곡선 상수(서버 attendance.ts parity). f32.
	static constexpr float MilestoneBase = 2.0f;
	static constexpr float MilestoneGrowth = 1.6f;

	// 무한 루프 안전 상한(서버 ATTENDANCE_MILESTONE_LOOP_GUARD 와 동일).
	static constexpr int32 MilestoneLoopGuard = 10000;

	// 마일스톤 n(>=1)의 누적 출석일 임계. n<1 은 0. floor(BASE * fround(GROWTH^(n-1))).
	// 서버 getAttendanceMilestoneThreshold 와 1:1(f32 곱셈 후 floor).
	static int64 GetMilestoneThreshold(int32 N);

	// 마일스톤 n 의 보상(종류/수치). n<1 은 Gold 0. 3주기 순환(서버 getAttendanceMilestoneReward 1:1).
	static void GetMilestoneReward(int32 N, EAttendanceReward& OutType, int64& OutValue);

	// 오늘(Date) 출석 체크인. Date != LastAttendanceDate 면 누적++/날짜 갱신 후 true,
	// 이미 같은 날짜면 false(1일 1회). 빈 Date 는 무시(false).
	UFUNCTION(BlueprintCallable, Category = "Idle|Attendance")
	bool CheckIn(const FString& Date);

	// 누적 출석일로 도달한 최대 마일스톤 n. 없으면 0(무한 임계, 상한 가드).
	UFUNCTION(BlueprintPure, Category = "Idle|Attendance")
	int32 GetReachedMilestones() const;

	// 마일스톤 n(>=1)의 임계/보상 정보. n<1 은 빈 마일스톤(N=0).
	UFUNCTION(BlueprintPure, Category = "Idle|Attendance")
	FAttendanceMilestone GetMilestone(int32 N) const;

	// 마일스톤 n 수령 마킹(true). N>=1 && N<=GetReached && !Claimed 면 ClaimedMilestones 에 추가.
	// 보상 지급은 GameInstance 가 담당. 미달/중복/무효 N 은 false.
	UFUNCTION(BlueprintCallable, Category = "Idle|Attendance")
	bool ClaimMilestone(int32 N);

	UFUNCTION(BlueprintPure, Category = "Idle|Attendance")
	bool IsMilestoneClaimed(int32 N) const { return ClaimedMilestones.Contains(N); }

	UFUNCTION(BlueprintPure, Category = "Idle|Attendance")
	int64 GetTotalAttendance() const { return TotalAttendance; }

	UFUNCTION(BlueprintPure, Category = "Idle|Attendance")
	const FString& GetLastAttendanceDate() const { return LastAttendanceDate; }

	const TSet<int32>& GetClaimedMilestones() const { return ClaimedMilestones; }

	// 세이브 복원. 음수 누적은 0 으로 클램프하며, 수령 집합은 그대로 유지(복원 후 재계산은 호출 측 책임).
	void RestoreState(int64 Total, const FString& Date, const TSet<int32>& Claimed);

private:
	UPROPERTY()
	int64 TotalAttendance = 0;

	UPROPERTY()
	FString LastAttendanceDate;

	UPROPERTY()
	TSet<int32> ClaimedMilestones;
};
