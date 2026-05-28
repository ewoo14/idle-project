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
  enhanceLevel: number;
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

  return Math.round(baseScore * (1 + item.enhanceLevel * 0.1));
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

      return {
        bonusAtk: acc.bonusAtk + item.bonusAtk * enhanceMultiplier,
        bonusDef:
          acc.bonusDef +
          (item.bonusDef + (item.bonusPhysDef ?? 0)) * enhanceMultiplier,
        bonusHp:
          acc.bonusHp +
          (item.bonusHp + (item.bonusAffixHp ?? 0)) * enhanceMultiplier,
        critRate:
          acc.critRate +
          (item.bonusCritRate ?? 0) * enhanceMultiplier +
          traitEffects.flat.critRate,
        atkSpeed:
          acc.atkSpeed +
          (item.bonusAtkSpeed ?? 0) * enhanceMultiplier +
          traitEffects.flat.atkSpeed,
        magicAtk: acc.magicAtk + (item.bonusMagicAtk ?? 0) * enhanceMultiplier,
        magicDef:
          (acc.magicDef ?? 0) + (item.bonusMagicDef ?? 0) * enhanceMultiplier,
        critDmg:
          (acc.critDmg ?? 0) +
          (item.bonusCritDmg ?? 0) * enhanceMultiplier +
          traitEffects.flat.critDmg,
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
