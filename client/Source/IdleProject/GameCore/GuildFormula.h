#pragma once

#include "CoreMinimal.h"
#include "GameCore/GuildTypes.h"

/**
 * 길드 계급/정원 순수 함수.
 * 서버 `guild.service.ts` 의 상수·isRankUnlocked/getRankSlotCap 과 1:1 parity 필수.
 */
struct IDLEPROJECT_API FGuildFormula
{
	// ── 서버 parity 상수 ───────────────────────────────────────────────────────
	static constexpr int32 GUILD_CAPACITY = 30;   // v1 고정 정원
	static constexpr int32 VICE_UNLOCK_AT = 11;   // 부길드장 해금 멤버 수
	static constexpr int32 OFFICER_UNLOCK_AT = 21; // 간부 해금 멤버 수
	static constexpr int32 VICE_SLOT_CAP = 1;     // 부길드장 최대 인원
	static constexpr int32 OFFICER_SLOT_CAP = 3;  // 간부 최대 인원

	/**
	 * 계급 해금 여부 — 인원 기반(서버 isRankUnlocked parity).
	 * master/member 는 항상 true. vice 는 ≥11, officer 는 ≥21.
	 */
	static bool IsRankUnlocked(EGuildRank Rank, int32 MemberCount);

	/**
	 * 계급별 정원(슬롯 상한, 서버 getRankSlotCap parity).
	 * master/member 는 상한 없음(MAX_int32 반환).
	 */
	static int32 GetRankSlotCap(EGuildRank Rank);
};
