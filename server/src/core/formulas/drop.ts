import type { EquipmentBonus, ItemSlot } from "./equipment.js";

export type ItemRarity =
  | "None"
  | "Common"
  | "Uncommon"
  | "Rare"
  | "Epic"
  | "Legendary";

export type DropRng = () => number;

const ARMOR_SLOTS = new Set<ItemSlot>([2, 3, 4, 5, 6, 7]);
const toClientFloat = Math.fround;

export function getRarityStatMultiplier(rarity: ItemRarity): number {
  switch (rarity) {
    case "Common":
      return toClientFloat(1);
    case "Uncommon":
      return toClientFloat(1.3);
    case "Rare":
      return toClientFloat(1.7);
    case "Epic":
      return toClientFloat(2.3);
    case "Legendary":
      return toClientFloat(3.2);
    case "None":
      return toClientFloat(0);
  }
}

export function rollRarityForLevel(
  level: number,
  rng: DropRng = Math.random,
): ItemRarity {
  const safeLevel = Math.max(level, 1);
  const levelScale = toClientFloat(
    Math.min(Math.max(toClientFloat(toClientFloat(safeLevel - 1) / 99), 0), 1),
  );

  const noneChance = toClientFloat(0.02);
  const uncommonChance = toClientFloat(0.2);
  const rareChance = toClientFloat(
    toClientFloat(0.08) + toClientFloat(toClientFloat(0.12) * levelScale),
  );
  const epicChance = toClientFloat(toClientFloat(0.06) * levelScale);
  const legendaryChance = toClientFloat(toClientFloat(0.02) * levelScale);
  const commonChance = toClientFloat(
    Math.max(
      0,
      toClientFloat(
        toClientFloat(
          toClientFloat(toClientFloat(1 - noneChance) - uncommonChance) -
            rareChance,
        ) - epicChance,
      ) - legendaryChance,
    ),
  );

  const roll = rng();
  if (roll < noneChance) {
    return "None";
  }
  const commonThreshold = toClientFloat(noneChance + commonChance);
  if (roll < commonThreshold) {
    return "Common";
  }
  const uncommonThreshold = toClientFloat(commonThreshold + uncommonChance);
  if (roll < uncommonThreshold) {
    return "Uncommon";
  }
  const rareThreshold = toClientFloat(uncommonThreshold + rareChance);
  if (roll < rareThreshold) {
    return "Rare";
  }
  const epicThreshold = toClientFloat(rareThreshold + epicChance);
  if (roll < epicThreshold) {
    return "Epic";
  }
  return "Legendary";
}

export function computeItemBonus(
  slot: ItemSlot,
  level: number,
  rarity: ItemRarity,
  variance: number,
): EquipmentBonus {
  const safeLevel = Math.max(level, 1);
  const safeVariance = toClientFloat(Math.max(variance, 0));
  const baseBonus = toClientFloat(
    toClientFloat(toClientFloat(safeLevel) * safeVariance) *
      getRarityStatMultiplier(rarity),
  );

  if (slot === 1) {
    return {
      bonusAtk: baseBonus,
      bonusDef: 0,
      bonusHp: 0,
    };
  }

  if (slot === 8) {
    return {
      bonusAtk: toClientFloat(baseBonus * toClientFloat(0.5)),
      bonusDef: toClientFloat(baseBonus * toClientFloat(0.3)),
      bonusHp: toClientFloat(baseBonus * toClientFloat(2)),
    };
  }

  if (ARMOR_SLOTS.has(slot)) {
    return {
      bonusAtk: 0,
      bonusDef: toClientFloat(baseBonus * toClientFloat(0.7)),
      bonusHp: toClientFloat(baseBonus * toClientFloat(3)),
    };
  }

  return {
    bonusAtk: 0,
    bonusDef: 0,
    bonusHp: 0,
  };
}
