#include "GameCore/AttendanceService.h"

namespace
{
// 보상 기본값(서버 attendance.ts parity). f32.
constexpr float AttendanceBaseGoldReward = 10000.0f;
constexpr float AttendanceGoldGrowth = 1.5f;
constexpr float AttendanceBaseEssenceReward = 5.0f;
constexpr float AttendanceBaseConsumableReward = 1.0f;

// f32 곱셈 결과를 floor 하여 int64 로 클램프. 서버 Math.floor(Math.fround(...)) 와 1:1.
// 모든 중간값을 float 로 유지하여 IEEE-754 단정밀도(Math.fround) 의미를 보존한다(double 금지).
int64 AttendanceFloorToInt64(float Value)
{
	if (!FMath::IsFinite(Value) || Value >= static_cast<float>(MAX_int64))
	{
		return MAX_int64;
	}

	return FMath::Max<int64>(0, FMath::FloorToInt64(static_cast<double>(Value)));
}

// fround(growth^(exp)) — float powf. 각 대입이 f32 로 라운딩되어 서버 Math.fround 와 동일.
float AttendanceFroundPow(float Base, int32 Exp)
{
	const float Result = FMath::Pow(Base, static_cast<float>(Exp));
	return Result;
}
}

int64 UAttendanceService::GetMilestoneThreshold(int32 N)
{
	if (N < 1)
	{
		return 0;
	}

	// 서버: floor(fround(BASE * fround(GROWTH^(n-1)))). float 곱셈으로 f32 parity.
	const float Growth = AttendanceFroundPow(MilestoneGrowth, N - 1);
	const float Threshold = MilestoneBase * Growth;
	return AttendanceFloorToInt64(Threshold);
}

void UAttendanceService::GetMilestoneReward(int32 N, EAttendanceReward& OutType, int64& OutValue)
{
	if (N < 1)
	{
		OutType = EAttendanceReward::Gold;
		OutValue = 0;
		return;
	}

	// 3주기 순환: 1=gold, 2=essence, 0(3,6,..)=consumable (서버 attendance.ts 와 동일).
	const int32 Cycle = N % 3;
	if (Cycle == 1)
	{
		const float Growth = AttendanceFroundPow(AttendanceGoldGrowth, N - 1);
		const float Reward = AttendanceBaseGoldReward * Growth;
		OutType = EAttendanceReward::Gold;
		OutValue = AttendanceFloorToInt64(Reward);
		return;
	}
	if (Cycle == 2)
	{
		const float Reward = AttendanceBaseEssenceReward * static_cast<float>(N);
		OutType = EAttendanceReward::Essence;
		OutValue = AttendanceFloorToInt64(Reward);
		return;
	}

	const float Reward = AttendanceBaseConsumableReward * static_cast<float>(N);
	OutType = EAttendanceReward::Consumable;
	OutValue = AttendanceFloorToInt64(Reward);
}

bool UAttendanceService::CheckIn(const FString& Date)
{
	if (Date.IsEmpty() || Date == LastAttendanceDate)
	{
		return false;
	}

	LastAttendanceDate = Date;
	TotalAttendance = FMath::Max<int64>(0, TotalAttendance + 1);
	return true;
}

int32 UAttendanceService::GetReachedMilestones() const
{
	const int64 Total = FMath::Max<int64>(0, TotalAttendance);
	int32 Reached = 0;
	for (int32 Next = 1; Next <= MilestoneLoopGuard; ++Next)
	{
		const int64 Threshold = GetMilestoneThreshold(Next);
		if (Threshold <= 0 || Total < Threshold)
		{
			break;
		}
		Reached = Next;
	}
	return Reached;
}

FAttendanceMilestone UAttendanceService::GetMilestone(int32 N) const
{
	FAttendanceMilestone Milestone;
	if (N < 1)
	{
		return Milestone;
	}

	Milestone.N = N;
	Milestone.Threshold = GetMilestoneThreshold(N);
	GetMilestoneReward(N, Milestone.RewardType, Milestone.RewardValue);
	return Milestone;
}

bool UAttendanceService::ClaimMilestone(int32 N)
{
	if (N < 1 || N > GetReachedMilestones() || ClaimedMilestones.Contains(N))
	{
		return false;
	}

	ClaimedMilestones.Add(N);
	return true;
}

void UAttendanceService::RestoreState(int64 Total, const FString& Date, const TSet<int32>& Claimed)
{
	TotalAttendance = FMath::Max<int64>(0, Total);
	LastAttendanceDate = Date;
	ClaimedMilestones = Claimed;
}
