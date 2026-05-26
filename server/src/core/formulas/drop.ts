import type { EquipmentBonus, ItemSlot } from "./equipment.js";

export type ItemRarity =
  | "None"
  | "Common"
  | "Uncommon"
  | "Rare"
  | "Epic"
  | "Legendary"
  | "Mythic";

export type DropRng = () => number;

const ARMOR_SLOTS = new Set<ItemSlot>([2, 3, 4, 5, 6, 7]);
const toClientFloat = Math.fround;
const AFFIX_KINDS = ["CritRate", "AtkSpeed", "MagicAtk"] as const;

type AffixKind = (typeof AFFIX_KINDS)[number];

export interface AffixBonus {
  bonusCritRate: number;
  bonusAtkSpeed: number;
  bonusMagicAtk: number;
}

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
    case "Mythic":
      return toClientFloat(4.5);
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
  const legendaryChance = toClientFloat(toClientFloat(0.015) * levelScale);
  const mythicChance = toClientFloat(toClientFloat(0.005) * levelScale);
  const commonChance = toClientFloat(
    Math.max(
      0,
      toClientFloat(
        toClientFloat(
          toClientFloat(toClientFloat(1 - noneChance) - uncommonChance) -
            rareChance,
        ) - epicChance,
      ) - legendaryChance - mythicChance,
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
  const legendaryThreshold = toClientFloat(epicThreshold + legendaryChance);
  if (roll < legendaryThreshold) {
    return "Legendary";
  }
  return "Mythic";
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
      critRate: 0,
      atkSpeed: 0,
      magicAtk: 0,
    };
  }

  if (slot === 8) {
    return {
      bonusAtk: toClientFloat(baseBonus * toClientFloat(0.5)),
      bonusDef: toClientFloat(baseBonus * toClientFloat(0.3)),
      bonusHp: toClientFloat(baseBonus * toClientFloat(2)),
      critRate: 0,
      atkSpeed: 0,
      magicAtk: 0,
    };
  }

  if (ARMOR_SLOTS.has(slot)) {
    return {
      bonusAtk: 0,
      bonusDef: toClientFloat(baseBonus * toClientFloat(0.7)),
      bonusHp: toClientFloat(baseBonus * toClientFloat(3)),
      critRate: 0,
      atkSpeed: 0,
      magicAtk: 0,
    };
  }

  return {
    bonusAtk: 0,
    bonusDef: 0,
    bonusHp: 0,
    critRate: 0,
    atkSpeed: 0,
    magicAtk: 0,
  };
}

export function getAffixCount(
  rarity: ItemRarity,
  rng: DropRng = Math.random,
): number {
  switch (rarity) {
    case "Uncommon":
    case "Rare":
      return 1;
    case "Epic":
      return 2;
    case "Legendary":
      return rng() < 0.5 ? 2 : 3;
    case "Mythic":
      return 3;
    case "None":
    case "Common":
      return 0;
  }
}

function randRangeInt(min: number, max: number, rng: DropRng): number {
  const roll = Math.floor(rng() * (max - min + 1)) + min;

  return Math.min(Math.max(roll, min), max);
}

function randRangeFloat(min: number, max: number, rng: DropRng): number {
  return toClientFloat(
    toClientFloat(min) + toClientFloat(toClientFloat(max - min) * rng()),
  );
}

function shuffleAffixKinds(rng: DropRng): AffixKind[] {
  const kinds = [...AFFIX_KINDS];
  for (let index = kinds.length - 1; index > 0; index -= 1) {
    const swapIndex = randRangeInt(0, index, rng);
    [kinds[index], kinds[swapIndex]] = [kinds[swapIndex], kinds[index]];
  }

  return kinds;
}

export function rollAffixes(
  rarity: ItemRarity,
  level: number,
  rng: DropRng = Math.random,
): AffixBonus {
  const affixes: AffixBonus = {
    bonusCritRate: 0,
    bonusAtkSpeed: 0,
    bonusMagicAtk: 0,
  };
  const affixCount = getAffixCount(rarity, rng);
  if (affixCount <= 0) {
    return affixes;
  }

  const safeLevel = Math.max(level, 1);
  const affixKinds = shuffleAffixKinds(rng);
  for (const affixKind of affixKinds.slice(0, affixCount)) {
    switch (affixKind) {
      case "CritRate":
        affixes.bonusCritRate =
          Math.round(randRangeFloat(0.01, 0.05, rng) * 1000) / 1000;
        break;
      case "AtkSpeed":
        affixes.bonusAtkSpeed =
          Math.round(randRangeFloat(0.05, 0.15, rng) * 1000) / 1000;
        break;
      case "MagicAtk":
        affixes.bonusMagicAtk = Math.round(
          toClientFloat(safeLevel) * randRangeFloat(0.5, 1.5, rng),
        );
        break;
    }
  }

  return affixes;
}
