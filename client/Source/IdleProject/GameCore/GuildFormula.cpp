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
