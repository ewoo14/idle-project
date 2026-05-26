import type { PrimaryStats } from "./stats.js";

export const STAT_POINTS_PER_LEVEL = 5;

export function getStatPointsForLevelUp(newLevel: number): number {
  assertIntegerLevel(newLevel);
  return newLevel > 1 ? STAT_POINTS_PER_LEVEL : 0;
}

export function getTotalStatPointsForLevel(level: number): number {
  assertIntegerLevel(level);
  return Math.max(0, level - 1) * STAT_POINTS_PER_LEVEL;
}

export function applyAllocatedStats(
  primary: PrimaryStats,
  allocated: PrimaryStats,
): PrimaryStats {
  return {
    str: primary.str + allocated.str,
    dex: primary.dex + allocated.dex,
    int: primary.int + allocated.int,
    wis: primary.wis + allocated.wis,
    con: primary.con + allocated.con,
    luk: primary.luk + allocated.luk,
  };
}

function assertIntegerLevel(level: number) {
  if (!Number.isInteger(level)) {
    throw new Error("level must be an integer");
  }
}
