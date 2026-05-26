export const SPECIAL_ATTACK_INTERVAL_SECONDS = 6;

export function getBossPhase(hpRatio: number): number {
  const clampedHpRatio = clamp(hpRatio, 0, 1);

  if (clampedHpRatio > 0.66) {
    return 1;
  }

  if (clampedHpRatio > 0.33) {
    return 2;
  }

  return 3;
}

export function getPhaseAtkMultiplier(phase: number): number {
  switch (phase) {
    case 1:
      return 1.0;
    case 2:
      return 1.25;
    case 3:
      return 1.6;
    default:
      return 1.0;
  }
}

export function getPhaseAtkSpeedMultiplier(phase: number): number {
  switch (phase) {
    case 1:
      return 1.0;
    case 2:
      return 1.15;
    case 3:
      return 1.3;
    default:
      return 1.0;
  }
}

export function getSpecialAttackDamageMultiplier(): number {
  return 2.5;
}

function clamp(value: number, min: number, max: number): number {
  return Math.min(Math.max(value, min), max);
}
