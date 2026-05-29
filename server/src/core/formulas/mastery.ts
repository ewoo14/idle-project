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
  return Math.floor(MASTERY_XP_BASE * MASTERY_XP_GROWTH ** safe);
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

export function localBonus(track: number, level: number): number {
  const safeLevel = Math.max(0, Math.floor(level));
  if (safeLevel <= 0) {
    return 0;
  }

  const raw = Math.fround(0.01 * Math.log(1 + safeLevel));
  return track === 1 ? Math.min(0.5, raw) : raw;
}

// V2 로컬 보너스(트랙당 2종째). 1종과 동일 ln 곡선·계수(0.01).
// Equipment(track 1, 큐브 가격 절감)·Beast(track 4, 펫 먹이 비용 절감)는 0.5 상한 클램프.
export function localBonus2(track: number, level: number): number {
  const safeLevel = Math.max(0, Math.floor(level));
  if (safeLevel <= 0) {
    return 0;
  }

  const raw = Math.fround(0.01 * Math.log(1 + safeLevel));
  return track === 1 || track === 4 ? Math.min(0.5, raw) : raw;
}

// 심연(track 2) 2종: 던전 일일 입장 추가 정수 보너스. floor(level/50), 상한 +3.
export function abyssBonusEntries(level: number): number {
  const safeLevel = Math.max(0, Math.floor(level));
  return Math.min(3, Math.floor(safeLevel / 50));
}

export function worldPower(trackLevels: number[]): number {
  return trackLevels.reduce((acc, lv) => acc + Math.max(0, Math.floor(lv)), 0);
}
