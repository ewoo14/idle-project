export const MAX_ENHANCE_LEVEL = 50;
export const ENHANCE_SAFE_MAX_LEVEL = 9;
export const ENHANCE_PITY_THRESHOLD = 12;

export type EnhanceItemRarity =
  | "None"
  | "Common"
  | "Rare"
  | "Epic"
  | "Unique"
  | "Legendary"
  | "Transcendent"
  | "Mythic";

const BASE_ENHANCE_COST = 100;
const MAX_ENHANCE_SUCCESS_RATE = 0.95;
const MIN_ENHANCE_SUCCESS_RATE = 0.05;
const ENHANCE_SUCCESS_RATE_DECAY_PER_LEVEL = 0.018;

export function getRarityCostMultiplier(rarity: EnhanceItemRarity): number {
  switch (rarity) {
    case "Common":
      return 1;
    case "Rare":
      return 2;
    case "Epic":
      return 4;
    case "Unique":
      return 8;
    case "Legendary":
      return 16;
    case "Transcendent":
      return 32;
    case "Mythic":
      return 64;
    case "None":
      return 0;
  }
}

export function getEnhanceCost(
  currentLevel: number,
  rarity: EnhanceItemRarity = "Common",
): number {
  if (currentLevel >= MAX_ENHANCE_LEVEL) {
    return 0;
  }

  const clampedLevel = Math.max(0, currentLevel);
  return (
    BASE_ENHANCE_COST *
    (clampedLevel + 1) ** 2 *
    getRarityCostMultiplier(rarity)
  );
}

export function getEnhanceSuccessRate(currentLevel: number): number {
  if (currentLevel >= MAX_ENHANCE_LEVEL) {
    return 0;
  }

  const clampedLevel = Math.max(
    0,
    Math.min(currentLevel, MAX_ENHANCE_LEVEL - 1),
  );
  return Math.fround(
    Math.max(
      MIN_ENHANCE_SUCCESS_RATE,
      Math.min(
        MAX_ENHANCE_SUCCESS_RATE,
        MAX_ENHANCE_SUCCESS_RATE -
          clampedLevel * ENHANCE_SUCCESS_RATE_DECAY_PER_LEVEL,
      ),
    ),
  );
}

export function rollEnhanceSuccess(
  rate: number,
  rng: () => number = Math.random,
): boolean {
  const clampedRate = Math.max(0, Math.min(rate, 1));
  return rng() < clampedRate;
}

export interface EnhanceAttemptInput {
  currentLevel: number;
  failStreak: number;
  useProtection: boolean;
  hasProtection: boolean;
  roll: number;
}

export interface EnhanceAttemptOutcome {
  attempted: boolean;
  success: boolean;
  consumedProtection: boolean;
  newLevel: number;
  newFailStreak: number;
  pityTriggered: boolean;
}

export function isRiskLevel(currentLevel: number): boolean {
  return (
    currentLevel > ENHANCE_SAFE_MAX_LEVEL && currentLevel < MAX_ENHANCE_LEVEL
  );
}

export function resolveEnhanceAttempt(
  input: EnhanceAttemptInput,
): EnhanceAttemptOutcome {
  const currentLevel = Math.max(
    0,
    Math.min(Math.trunc(input.currentLevel), MAX_ENHANCE_LEVEL),
  );
  const failStreak = Math.max(0, Math.trunc(input.failStreak));
  if (currentLevel >= MAX_ENHANCE_LEVEL) {
    return {
      attempted: false,
      success: false,
      consumedProtection: false,
      newLevel: currentLevel,
      newFailStreak: failStreak,
      pityTriggered: false,
    };
  }

  const risk = isRiskLevel(currentLevel);
  const pityTriggered = risk && failStreak >= ENHANCE_PITY_THRESHOLD;
  const success =
    pityTriggered || input.roll < getEnhanceSuccessRate(currentLevel);
  if (success) {
    return {
      attempted: true,
      success: true,
      consumedProtection: false,
      newLevel: Math.min(currentLevel + 1, MAX_ENHANCE_LEVEL),
      newFailStreak: 0,
      pityTriggered,
    };
  }

  const consumedProtection = risk && input.useProtection && input.hasProtection;
  return {
    attempted: true,
    success: false,
    consumedProtection,
    newLevel:
      risk && !consumedProtection
        ? Math.max(0, currentLevel - 1)
        : currentLevel,
    newFailStreak: failStreak + 1,
    pityTriggered: false,
  };
}
