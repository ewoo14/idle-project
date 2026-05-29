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

	// ── 길드 레벨/버프 공식 상수 (서버 guild.service.ts parity, 스펙 §4) ──────────
	static constexpr int64 GUILD_LEVEL_BASE = 10000;     // 레벨 1→2 임계 기준값
	static constexpr double GUILD_LEVEL_GROWTH = 1.6;    // 레벨당 임계 성장률
	static constexpr float GUILD_BUFF_PER_LEVEL = 0.004f; // 레벨당 버프 계수(공격/골드 +0.4%)
	static constexpr int64 AUTO_WEEKLY_CAP = 2000;       // 전투/던전 자동 기여 주간 상한

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

	/**
	 * 누적 EXP → 길드 레벨(무한 기하). 레벨 L 도달 누적 임계는
	 * Σ_{i=1}^{L-1} floor(BASE * GROWTH^(i-1)). 레벨은 1 부터 시작.
	 * 서버 `getGuildLevel(exp)` 와 동일 결과(누적 임계·floor·int64)를 보장한다.
	 */
	static int32 GetGuildLevel(int64 Exp);

	/** 길드 레벨 → 영구 버프(공격 +0.4%*L, 골드 +0.4%*L). 서버 `getGuildBuff` parity. */
	static FGuildBuff GetGuildBuff(int32 Level);
};
