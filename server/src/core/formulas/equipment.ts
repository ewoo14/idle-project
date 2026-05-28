import type { PotentialGrade, PotentialLine } from "./potential.js";
import { computeSetBonus } from "./setBonus.js";
import { accumulateTraitEffects, type UniqueTrait } from "./uniqueTrait.js";

/**
 * 장비 PowerScore - UE5 client FItemPowerScore::Compute 미러.
 * (Atk + Def + Hp/10 + CritRate*1000 + AtkSpeed*100 + MagicAtk) x (1 + EnhanceLevel x 0.1)
 */
export type ItemSlot = 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8;
// 0=None, 1=Weapon, 2=Helmet, 3=Top, 4=Bottom, 5=Shoes, 6=Gloves, 7=Cloak, 8=Accessory

export type ItemRarity = 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | "None";
// 0/"None"=None, 1=Common, 2=Rare, 3=Epic, 4=Unique, 5=Legendary, 6=Transcendent, 7=Mythic

export type ItemSet =
  | 0
  | 1
  | 2
  | 3
  | 4
  | 5
  | 6
  | 7
  | "None"
  | "Warrior"
  | "Guardian"
  | "Arcane"
  | "Assassin"
  | "Hunter"
  | "Holy"
  | "Berserker";

export interface ItemInstance {
  itemId: string;
  slot: ItemSlot;
  rarity: ItemRarity;
  itemSet?: ItemSet;
  bonusAtk: number;
  bonusDef: number;
  bonusHp: number;
  bonusCritRate?: number;
  bonusAtkSpeed?: number;
  bonusMagicAtk?: number;
  bonusPhysDef?: number;
  bonusMagicDef?: number;
  bonusAffixHp?: number;
  bonusCritDmg?: number;
  uniqueTrait1?: UniqueTrait;
  uniqueTrait2?: UniqueTrait;
  potentialGrade?: PotentialGrade;
  potentialLine1?: PotentialLine;
  potentialLine2?: PotentialLine;
  potentialLine3?: PotentialLine;
  enhanceLevel: number;
}

function getPotentialLines(item: ItemInstance): PotentialLine[] {
  return [item.potentialLine1, item.potentialLine2, item.potentialLine3].filter(
    (line): line is PotentialLine =>
      !!line && line.stat !== "None" && line.value > 0,
  );
}

function getPotentialPowerScoreBonus(item: ItemInstance): number {
  const enhanceMultiplier = 1 + item.enhanceLevel * 0.1;
  return getPotentialLines(item).reduce((sum, line) => {
    switch (line.stat) {
      case "PhysAtkPercent":
        return sum + item.bonusAtk * enhanceMultiplier * line.value;
      case "MagicAtkPercent":
        return sum + (item.bonusMagicAtk ?? 0) * enhanceMultiplier * line.value;
      case "HpPercent":
        return (
          sum +
          ((item.bonusHp + (item.bonusAffixHp ?? 0)) *
            enhanceMultiplier *
            line.value) /
            10
        );
      case "PhysDefPercent":
        return (
          sum +
          (item.bonusDef + (item.bonusPhysDef ?? 0)) *
            enhanceMultiplier *
            line.value
        );
      case "MagicDefPercent":
        return sum + (item.bonusMagicDef ?? 0) * enhanceMultiplier * line.value;
      case "CritRatePercent":
        return sum + line.value * 1000;
      case "AtkSpeedPercent":
        return sum + line.value * 100;
      case "CritDmgPercent":
        return sum + line.value * 100;
      case "None":
        return sum;
      default:
        return sum;
    }
  }, 0);
}

export function computeItemPowerScore(item: ItemInstance): number {
  const baseScore =
    item.bonusAtk +
    item.bonusDef +
    item.bonusHp / 10 +
    (item.bonusCritRate ?? 0) * 1000 +
    (item.bonusAtkSpeed ?? 0) * 100 +
    (item.bonusMagicAtk ?? 0) +
    (item.bonusPhysDef ?? 0) +
    (item.bonusMagicDef ?? 0) +
    (item.bonusAffixHp ?? 0) / 10 +
    (item.bonusCritDmg ?? 0) * 100;

  return Math.round(
    baseScore * (1 + item.enhanceLevel * 0.1) +
      getPotentialPowerScoreBonus(item),
  );
}

export interface EquipmentBonus {
  bonusAtk: number;
  bonusDef: number;
  bonusHp: number;
  critRate: number;
  atkSpeed: number;
  magicAtk: number;
  magicDef?: number;
  critDmg?: number;
}

export function computeInventoryBonus(
  equippedItems: ItemInstance[],
): EquipmentBonus {
  const itemBonus = equippedItems.reduce(
    (acc, item) => {
      const enhanceMultiplier = 1 + item.enhanceLevel * 0.1;
      const traitEffects = accumulateTraitEffects(
        [item.uniqueTrait1 ?? 0, item.uniqueTrait2 ?? 0],
        Number(item.rarity),
      );
      const potential = getPotentialLines(item);
      const potentialAtkMultiplier = potential
        .filter((line) => line.stat === "PhysAtkPercent")
        .reduce((sum, line) => sum + line.value, 0);
      const potentialMagicAtkMultiplier = potential
        .filter((line) => line.stat === "MagicAtkPercent")
        .reduce((sum, line) => sum + line.value, 0);
      const potentialHpMultiplier = potential
        .filter((line) => line.stat === "HpPercent")
        .reduce((sum, line) => sum + line.value, 0);
      const potentialPhysDefMultiplier = potential
        .filter((line) => line.stat === "PhysDefPercent")
        .reduce((sum, line) => sum + line.value, 0);
      const potentialMagicDefMultiplier = potential
        .filter((line) => line.stat === "MagicDefPercent")
        .reduce((sum, line) => sum + line.value, 0);
      const potentialCritRate = potential
        .filter((line) => line.stat === "CritRatePercent")
        .reduce((sum, line) => sum + line.value, 0);
      const potentialAtkSpeed = potential
        .filter((line) => line.stat === "AtkSpeedPercent")
        .reduce((sum, line) => sum + line.value, 0);
      const potentialCritDmg = potential
        .filter((line) => line.stat === "CritDmgPercent")
        .reduce((sum, line) => sum + line.value, 0);

      return {
        bonusAtk:
          acc.bonusAtk +
          item.bonusAtk * enhanceMultiplier * (1 + potentialAtkMultiplier),
        bonusDef:
          acc.bonusDef +
          (item.bonusDef + (item.bonusPhysDef ?? 0)) *
            enhanceMultiplier *
            (1 + potentialPhysDefMultiplier),
        bonusHp:
          acc.bonusHp +
          (item.bonusHp + (item.bonusAffixHp ?? 0)) *
            enhanceMultiplier *
            (1 + potentialHpMultiplier),
        critRate:
          acc.critRate +
          (item.bonusCritRate ?? 0) * enhanceMultiplier +
          traitEffects.flat.critRate +
          potentialCritRate,
        atkSpeed:
          acc.atkSpeed +
          (item.bonusAtkSpeed ?? 0) * enhanceMultiplier +
          traitEffects.flat.atkSpeed +
          potentialAtkSpeed,
        magicAtk:
          acc.magicAtk +
          (item.bonusMagicAtk ?? 0) *
            enhanceMultiplier *
            (1 + potentialMagicAtkMultiplier),
        magicDef:
          (acc.magicDef ?? 0) +
          (item.bonusMagicDef ?? 0) *
            enhanceMultiplier *
            (1 + potentialMagicDefMultiplier),
        critDmg:
          (acc.critDmg ?? 0) +
          (item.bonusCritDmg ?? 0) * enhanceMultiplier +
          traitEffects.flat.critDmg +
          potentialCritDmg,
      };
    },
    {
      bonusAtk: 0,
      bonusDef: 0,
      bonusHp: 0,
      critRate: 0,
      atkSpeed: 0,
      magicAtk: 0,
      magicDef: 0,
      critDmg: 0,
    },
  );
  const setBonus = computeSetBonus(equippedItems);

  return {
    bonusAtk: itemBonus.bonusAtk + setBonus.bonusAtk,
    bonusDef: itemBonus.bonusDef + setBonus.bonusDef,
    bonusHp: itemBonus.bonusHp + setBonus.bonusHp,
    critRate: itemBonus.critRate + setBonus.critRate,
    atkSpeed: itemBonus.atkSpeed + setBonus.atkSpeed,
    magicAtk: itemBonus.magicAtk + setBonus.magicAtk,
    magicDef: (itemBonus.magicDef ?? 0) + setBonus.magicDef,
    critDmg: (itemBonus.critDmg ?? 0) + setBonus.critDmg,
  };
}
