#pragma once

#include "CoreMinimal.h"

/**
 * 공유 HP 풀 길드 보스 순수 함수.
 * 서버 `guild.service.ts` 의 보스 상수·getGuildBossHp/getChallengeDamage 와 1:1 parity 필수.
 * 보스 진행/격파는 서버 권위(클라는 표시·도전 요청만) — 본 공식은 UI 표시/검증용.
 */
struct IDLEPROJECT_API FGuildBossFormula
{
	// ── 서버 parity 상수 (guild.service.ts §보스) ───────────────────────────────
	static constexpr int64 GUILD_BOSS_BASE_HP = 1000000;        // 첫 격파(defeated=0) 보스 HP
	static constexpr double GUILD_BOSS_HP_GROWTH = 1.5;         // 격파마다 HP 성장률(무한 곡선)
	static constexpr int32 WEEKLY_GUILD_BOSS_CHALLENGE_LIMIT = 7; // 멤버당 주간 도전 횟수
	static constexpr int64 GUILD_BOSS_DMG_TO_CONTRIB = 10000;   // 보스 데미지 → 기여 환산 제수

	/**
	 * 격파 횟수 → 다음 보스의 공유 HP(무한 기하). HP = floor(BASE * GROWTH^max(defeated,0)).
	 * 서버 `getGuildBossHp(defeatedCount)` 와 동일 결과(floor·음수 클램프·int64)를 보장한다.
	 */
	static int64 GetGuildBossHp(int32 DefeatedCount);

	/**
	 * CP → 1회 도전 누적 데미지(#77 GetChallengeDamage 길드화). 음수/소수 방어.
	 * 서버 `getChallengeDamage(cp)` parity — trunc(max(0, cp)).
	 */
	static int64 GetChallengeDamage(int64 Cp);
};
