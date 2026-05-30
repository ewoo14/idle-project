#pragma once

#include "CoreMinimal.h"
#include "AttendanceTypes.generated.h"

// 출석 마일스톤 보상 종류. 서버 attendance.ts AttendanceRewardType("gold"|"essence"|"consumable")와 1:1.
UENUM(BlueprintType)
enum class EAttendanceReward : uint8
{
	Gold,
	Essence,
	Consumable
};

// 출석 마일스톤 1건(N>=1). 누적 출석일 임계 도달 시 RewardType/RewardValue 보상을 수령한다.
// 임계/보상 값은 AttendanceService(서버 attendance.ts) 미러 공식으로 산출한다.
USTRUCT(BlueprintType)
struct IDLEPROJECT_API FAttendanceMilestone
{
	GENERATED_BODY()

	// 마일스톤 번호(1-based). N<1 은 무효(빈 마일스톤).
	UPROPERTY(BlueprintReadOnly, Category = "Idle|Attendance")
	int32 N = 0;

	// 이 마일스톤을 해금하는 누적 출석일 임계. floor(BASE * GROWTH^(N-1)) (f32 parity).
	UPROPERTY(BlueprintReadOnly, Category = "Idle|Attendance")
	int64 Threshold = 0;

	// 보상 종류(3주기 순환: 1=Gold, 2=Essence, 0=Consumable).
	UPROPERTY(BlueprintReadOnly, Category = "Idle|Attendance")
	EAttendanceReward RewardType = EAttendanceReward::Gold;

	// 보상 수치(종류별 공식).
	UPROPERTY(BlueprintReadOnly, Category = "Idle|Attendance")
	int64 RewardValue = 0;
};
