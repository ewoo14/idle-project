#include "GameCore/GuildFormula.h"

bool FGuildFormula::IsRankUnlocked(EGuildRank Rank, int32 MemberCount)
{
	switch (Rank)
	{
	case EGuildRank::Master:
	case EGuildRank::Member:
		return true;
	case EGuildRank::Vice:
		return MemberCount >= VICE_UNLOCK_AT;
	case EGuildRank::Officer:
		return MemberCount >= OFFICER_UNLOCK_AT;
	default:
		return false;
	}
}

int32 FGuildFormula::GetRankSlotCap(EGuildRank Rank)
{
	switch (Rank)
	{
	case EGuildRank::Vice:
		return VICE_SLOT_CAP;
	case EGuildRank::Officer:
		return OFFICER_SLOT_CAP;
	default:
		// master/member 는 슬롯 상한 없음(서버 Number.POSITIVE_INFINITY 대응).
		return MAX_int32;
	}
}

int32 FGuildFormula::GetGuildLevel(int64 Exp)
{
	if (Exp <= 0)
	{
		return 1;
	}

	int32 Level = 1;
	int64 Cumulative = 0;
	// 서버 getGuildLevel 과 1:1: step 은 BASE*GROWTH^(level-1) 을 double 로 계산 후 floor.
	// 누적 임계가 기하급수라 현실적 EXP 에서 수십 레벨 내 수렴(무한 루프 방지 가드 포함).
	while (Level < MAX_int32)
	{
		const double StepDouble = FMath::Floor(static_cast<double>(GUILD_LEVEL_BASE) * FMath::Pow(GUILD_LEVEL_GROWTH, static_cast<double>(Level - 1)));
		// double→int64 안전 변환: 임계가 int64 범위를 넘으면 이후 EXP 로는 도달 불가 → 현재 레벨 확정.
		if (StepDouble >= static_cast<double>(MAX_int64))
		{
			return Level;
		}
		const int64 Step = static_cast<int64>(StepDouble);
		if (Cumulative > MAX_int64 - Step || Exp < Cumulative + Step)
		{
			return Level;
		}
		Cumulative += Step;
		++Level;
	}
	return Level;
}

FGuildBuff FGuildFormula::GetGuildBuff(int32 Level)
{
	const float Pct = GUILD_BUFF_PER_LEVEL * static_cast<float>(Level);
	FGuildBuff Buff;
	Buff.AttackPct = Pct;
	Buff.GoldPct = Pct;
	return Buff;
}
