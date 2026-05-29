#include "GameCore/GuildBossFormula.h"

int64 FGuildBossFormula::GetGuildBossHp(int32 DefeatedCount)
{
	// 서버 getGuildBossHp 와 1:1: floor(BASE * GROWTH^max(defeated,0)).
	// double 로 계산 후 floor → int64. 음수 격파수는 0 으로 클램프(서버 Math.max(d,0)).
	const int32 SafeDefeated = FMath::Max(DefeatedCount, 0);
	const double HpDouble = FMath::Floor(
		static_cast<double>(GUILD_BOSS_BASE_HP) * FMath::Pow(GUILD_BOSS_HP_GROWTH, static_cast<double>(SafeDefeated)));
	// double→int64 안전 변환: int64 범위를 넘으면 포화(현실적 격파수에선 도달 불가).
	if (!FMath::IsFinite(HpDouble) || HpDouble >= static_cast<double>(MAX_int64))
	{
		return MAX_int64;
	}
	return static_cast<int64>(HpDouble);
}

int64 FGuildBossFormula::GetChallengeDamage(int64 Cp)
{
	// 서버 getChallengeDamage parity: cp<=0 이면 0, 아니면 trunc(cp).
	// CP 는 이미 정수(int64)라 trunc 는 항등 — 음수만 0 으로 클램프.
	return FMath::Max<int64>(0, Cp);
}
