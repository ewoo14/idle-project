const BASE_REQUIRED_POWER = 100.0;
const REQUIRED_POWER_GROWTH = 1.15;
const BASE_REWARD_PER_FLOOR = 50.0;
const INT64_MAX = 9223372036854776000;

function roundAndClampToInt64(value: number): number {
  if (!Number.isFinite(value) || value >= INT64_MAX) {
    return INT64_MAX;
  }

  return Math.max(0, Math.round(value));
}

function clampFloor(floor: number): number {
  return Math.max(1, Math.trunc(floor));
}

export function getFloorRequiredPower(floor: number): number {
  const clampedFloor = clampFloor(floor);
  const requiredPower =
    // biome-ignore lint/style/useExponentiationOperator: Keep Math.pow parity with the UE FTowerFormula implementation.
    BASE_REQUIRED_POWER * Math.pow(REQUIRED_POWER_GROWTH, clampedFloor - 1);

  return roundAndClampToInt64(requiredPower);
}

export function canClearFloor(combatPower: number, floor: number): boolean {
  return combatPower >= getFloorRequiredPower(floor);
}

export function getFloorReward(floor: number): number {
  const clampedFloor = clampFloor(floor);

  return roundAndClampToInt64(BASE_REWARD_PER_FLOOR * clampedFloor);
}
