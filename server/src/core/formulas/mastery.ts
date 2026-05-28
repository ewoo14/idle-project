export const MASTERY_TRACK_COUNT = 6;
export const MASTERY_XP_BASE = 100;
export const MASTERY_XP_GROWTH = 1.15;

export interface MasteryLevelInfo {
  level: number;
  xpIntoLevel: number;
  xpToNext: number;
}

export function xpToNext(level: number): number {
  const safe = Math.max(0, Math.floor(level));
  return Math.floor(MASTERY_XP_BASE * Math.pow(MASTERY_XP_GROWTH, safe));
}

export function levelFromTotalXp(totalXp: number): MasteryLevelInfo {
  let remaining = Math.max(0, Math.floor(totalXp));
  let level = 0;
  let need = xpToNext(0);
  while (remaining >= need) {
    remaining -= need;
    level += 1;
    need = xpToNext(level);
  }
  return { level, xpIntoLevel: remaining, xpToNext: need };
}

export function coreStatMultiplier(
  combatLv: number,
  equipLv: number,
  exploreLv: number,
): number {
  const sum =
    Math.max(0, combatLv) + Math.max(0, equipLv) + Math.max(0, exploreLv);
  return Math.fround(1 + 0.02 * Math.log(1 + sum));
}

export function critRateAdd(runeLv: number): number {
  return Math.fround(0.01 * Math.log(1 + Math.max(0, runeLv)));
}

export function dropRateAdd(abyssLv: number): number {
  return Math.fround(0.01 * Math.log(1 + Math.max(0, abyssLv)));
}

export function goldFindPct(beastLv: number): number {
  return Math.fround(0.02 * Math.log(1 + Math.max(0, beastLv)));
}

export function expBoostPct(beastLv: number): number {
  return Math.fround(0.02 * Math.log(1 + Math.max(0, beastLv)));
}

export function worldPower(trackLevels: number[]): number {
  return trackLevels.reduce((acc, lv) => acc + Math.max(0, Math.floor(lv)), 0);
}
