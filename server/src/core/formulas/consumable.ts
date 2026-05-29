export const CONSUMABLE_TYPE = {
  AttackTonic: 0,
  GuardTonic: 1,
  AllStatElixir: 2,
  FortuneScroll: 3,
  GoldFeast: 4,
  WisdomBooster: 5,
} as const;

export type ConsumableType =
  (typeof CONSUMABLE_TYPE)[keyof typeof CONSUMABLE_TYPE];

const BUFF_DURATION_SEC = 1800;

export function getConsumableBuffPercent(type: number): number {
  switch (type) {
    case CONSUMABLE_TYPE.AttackTonic:
    case CONSUMABLE_TYPE.GuardTonic:
    case CONSUMABLE_TYPE.FortuneScroll:
      return Math.fround(0.3);
    case CONSUMABLE_TYPE.AllStatElixir:
      return Math.fround(0.2);
    case CONSUMABLE_TYPE.GoldFeast:
    case CONSUMABLE_TYPE.WisdomBooster:
      return Math.fround(0.5);
    default:
      return 0;
  }
}

export function getConsumableBuffDurationSec(type: number): number {
  return isConsumableType(type) ? BUFF_DURATION_SEC : 0;
}

export function isConsumableType(type: number): type is ConsumableType {
  return (
    type === CONSUMABLE_TYPE.AttackTonic ||
    type === CONSUMABLE_TYPE.GuardTonic ||
    type === CONSUMABLE_TYPE.AllStatElixir ||
    type === CONSUMABLE_TYPE.FortuneScroll ||
    type === CONSUMABLE_TYPE.GoldFeast ||
    type === CONSUMABLE_TYPE.WisdomBooster
  );
}
