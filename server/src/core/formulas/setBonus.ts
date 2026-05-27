import type { ItemRarity as DropItemRarity, DropRng } from "./drop.js";
import type {
  EquipmentBonus,
  ItemSet as EquipmentItemSet,
  ItemInstance,
} from "./equipment.js";

export type ItemSet = EquipmentItemSet;

const SET_PIECE_THRESHOLDS = [2, 4] as const;

const ITEM_SET_NAMES = [
  "Warrior",
  "Guardian",
  "Arcane",
  "Assassin",
  "Hunter",
  "Holy",
  "Berserker",
] as const;
type CountedItemSet = (typeof ITEM_SET_NAMES)[number];

function zeroBonus(): Required<EquipmentBonus> {
  return {
    bonusAtk: 0,
    bonusDef: 0,
    bonusHp: 0,
    critRate: 0,
    atkSpeed: 0,
    magicAtk: 0,
    magicDef: 0,
    critDmg: 0,
  };
}

function addBonus(
  target: Required<EquipmentBonus>,
  source: Required<EquipmentBonus>,
) {
  target.bonusAtk += source.bonusAtk;
  target.bonusDef += source.bonusDef;
  target.bonusHp += source.bonusHp;
  target.critRate += source.critRate;
  target.atkSpeed += source.atkSpeed;
  target.magicAtk += source.magicAtk;
  target.magicDef += source.magicDef;
  target.critDmg += source.critDmg;
}

function normalizeItemSet(itemSet: ItemSet | undefined): CountedItemSet | null {
  switch (itemSet) {
    case 1:
    case "Warrior":
      return "Warrior";
    case 2:
    case "Guardian":
      return "Guardian";
    case 3:
    case "Arcane":
      return "Arcane";
    case 4:
    case "Assassin":
      return "Assassin";
    case 5:
    case "Hunter":
      return "Hunter";
    case 6:
    case "Holy":
      return "Holy";
    case 7:
    case "Berserker":
      return "Berserker";
    default:
      return null;
  }
}

function isEquippedSetPiece(item: ItemInstance): boolean {
  return item.slot !== 0 && item.rarity !== 0 && item.rarity !== "None";
}

export function getSetPieceThreshold(tierIndex: number): number {
  return tierIndex <= 0 ? SET_PIECE_THRESHOLDS[0] : SET_PIECE_THRESHOLDS[1];
}

export function getTwoPieceBonus(itemSet: ItemSet): Required<EquipmentBonus> {
  const bonus = zeroBonus();
  switch (normalizeItemSet(itemSet)) {
    case "Warrior":
      bonus.bonusAtk = 20;
      break;
    case "Guardian":
      bonus.bonusDef = 15;
      bonus.bonusHp = 100;
      break;
    case "Arcane":
      bonus.magicAtk = 20;
      break;
    case "Assassin":
      bonus.critRate = 0.03;
      break;
    case "Hunter":
      bonus.atkSpeed = 0.05;
      break;
    case "Holy":
      bonus.bonusHp = 120;
      break;
    case "Berserker":
      bonus.bonusAtk = 30;
      break;
    case null:
      break;
  }

  return bonus;
}

export function getFourPieceBonus(itemSet: ItemSet): Required<EquipmentBonus> {
  const bonus = zeroBonus();
  switch (normalizeItemSet(itemSet)) {
    case "Warrior":
      bonus.bonusAtk = 50;
      bonus.critRate = 0.05;
      break;
    case "Guardian":
      bonus.bonusDef = 35;
      bonus.bonusHp = 250;
      break;
    case "Arcane":
      bonus.magicAtk = 50;
      bonus.critDmg = 0.1;
      break;
    case "Assassin":
      bonus.critDmg = 0.15;
      break;
    case "Hunter":
      bonus.bonusAtk = 35;
      bonus.atkSpeed = 0.03;
      break;
    case "Holy":
      bonus.bonusDef = 20;
      bonus.magicDef = 30;
      break;
    case "Berserker":
      bonus.critRate = 0.04;
      bonus.critDmg = 0.1;
      break;
    case null:
      break;
  }

  return bonus;
}

export function computeSetBonus(
  equippedItems: ItemInstance[],
): Required<EquipmentBonus> {
  const pieceCounts = new Map<CountedItemSet, number>();

  for (const item of equippedItems) {
    const itemSet = normalizeItemSet(item.itemSet);
    if (itemSet === null || !isEquippedSetPiece(item)) {
      continue;
    }

    pieceCounts.set(itemSet, (pieceCounts.get(itemSet) ?? 0) + 1);
  }

  const totalBonus = zeroBonus();
  for (const [itemSet, pieceCount] of pieceCounts) {
    if (pieceCount >= getSetPieceThreshold(0)) {
      addBonus(totalBonus, getTwoPieceBonus(itemSet));
    }
    if (pieceCount >= getSetPieceThreshold(1)) {
      addBonus(totalBonus, getFourPieceBonus(itemSet));
    }
  }

  return totalBonus;
}

export function rollItemSet(
  rarity: DropItemRarity,
  rng: DropRng = Math.random,
): CountedItemSet | "None" {
  if (rarity === "None" || rarity === "Common") {
    return "None";
  }

  const roll = Math.min(
    Math.max(Math.floor(rng() * ITEM_SET_NAMES.length), 0),
    ITEM_SET_NAMES.length - 1,
  );
  return ITEM_SET_NAMES[roll];
}
