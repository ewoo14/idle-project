export const OFFLINE_CAP_SECONDS = 12 * 3_600;
export const OFFLINE_EFFICIENCY = 0.75;
export const OFFLINE_TIME_BONUS_PER_HOUR = 0.005;
export const OFFLINE_REBIRTH_BONUS = 0.05;

export type OfflineRewardInput = {
  level: number;
  lastSeenUnixSec: number;
  nowUnixSec: number;
  rebirthCount?: number;
};

export type OfflineRewardResult = {
  cappedSeconds: number;
  gold: number;
  exp: number;
  timeBonusMultiplier: number;
};

export function baseGoldPerSec(level: number): number {
  assertLevel(level);
  return level * 4;
}

export function baseExpPerSec(level: number): number {
  assertLevel(level);
  return level * 4;
}

export function computeOfflineRewards(
  input: OfflineRewardInput,
): OfflineRewardResult {
  assertLevel(input.level);
  assertUnixSeconds(input.lastSeenUnixSec, "lastSeenUnixSec");
  assertUnixSeconds(input.nowUnixSec, "nowUnixSec");

  const elapsedSeconds = Math.max(0, input.nowUnixSec - input.lastSeenUnixSec);
  const cappedSeconds = Math.min(elapsedSeconds, OFFLINE_CAP_SECONDS);
  if (cappedSeconds === 0) {
    return {
      cappedSeconds,
      gold: 0,
      exp: 0,
      timeBonusMultiplier: 1,
    };
  }

  const timeBonusMultiplier = computeTimeBonusMultiplier({
    cappedSeconds,
    rebirthCount: input.rebirthCount ?? 0,
  });

  return {
    cappedSeconds,
    gold: Math.round(
      baseGoldPerSec(input.level) *
        cappedSeconds *
        OFFLINE_EFFICIENCY *
        timeBonusMultiplier,
    ),
    exp: Math.round(
      baseExpPerSec(input.level) *
        cappedSeconds *
        OFFLINE_EFFICIENCY *
        timeBonusMultiplier,
    ),
    timeBonusMultiplier,
  };
}

export function computeTimeBonusMultiplier(input: {
  cappedSeconds: number;
  rebirthCount: number;
}): number {
  if (!Number.isInteger(input.rebirthCount) || input.rebirthCount < 0) {
    throw new Error("rebirthCount must be >= 0");
  }
  const cappedHours = Math.min(input.cappedSeconds / 3_600, 12);
  return (
    1 +
    cappedHours * OFFLINE_TIME_BONUS_PER_HOUR +
    input.rebirthCount * OFFLINE_REBIRTH_BONUS
  );
}

function assertLevel(level: number) {
  if (!Number.isInteger(level) || level < 1) {
    throw new Error("level must be >= 1");
  }
}

function assertUnixSeconds(value: number, name: string) {
  if (!Number.isFinite(value) || value < 0) {
    throw new Error(`${name} must be a non-negative unix timestamp`);
  }
}
