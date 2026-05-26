/**
 * 장비 PowerScore - UE5 client FItemPowerScore::Compute 미러.
 * (Atk + Def + Hp/10 + CritRate*1000 + AtkSpeed*100 + MagicAtk) x (1 + EnhanceLevel x 0.1)
 */
export type ItemSlot = 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8;
// 1=Weapon, 2=Helmet, 3=Top, 4=Bottom, 5=Shoes, 6=Gloves, 7=Cloak, 8=Accessory

export type ItemRarity = 1 | 2 | 3 | 4 | 5;
// 1=Common, 2=Uncommon, 3=Rare, 4=Epic, 5=Legendary

export interface ItemInstance {
  itemId: string;
  slot: ItemSlot;
  rarity: ItemRarity;
  bonusAtk: number;
  bonusDef: number;
  bonusHp: number;
  bonusCritRate?: number;
  bonusAtkSpeed?: number;
  bonusMagicAtk?: number;
  enhanceLevel: number;
}

export function computeItemPowerScore(item: ItemInstance): number {
  const baseScore =
    item.bonusAtk +
    item.bonusDef +
    item.bonusHp / 10 +
    (item.bonusCritRate ?? 0) * 1000 +
    (item.bonusAtkSpeed ?? 0) * 100 +
    (item.bonusMagicAtk ?? 0);

  return Math.round(baseScore * (1 + item.enhanceLevel * 0.1));
}

export interface EquipmentBonus {
  bonusAtk: number;
  bonusDef: number;
  bonusHp: number;
  critRate: number;
  atkSpeed: number;
  magicAtk: number;
}

export function computeInventoryBonus(
  equippedItems: ItemInstance[],
): EquipmentBonus {
  return equippedItems.reduce(
    (acc, item) => {
      const enhanceMultiplier = 1 + item.enhanceLevel * 0.1;

      return {
        bonusAtk: acc.bonusAtk + item.bonusAtk * enhanceMultiplier,
        bonusDef: acc.bonusDef + item.bonusDef * enhanceMultiplier,
        bonusHp: acc.bonusHp + item.bonusHp * enhanceMultiplier,
        critRate: acc.critRate + (item.bonusCritRate ?? 0) * enhanceMultiplier,
        atkSpeed: acc.atkSpeed + (item.bonusAtkSpeed ?? 0) * enhanceMultiplier,
        magicAtk: acc.magicAtk + (item.bonusMagicAtk ?? 0) * enhanceMultiplier,
      };
    },
    {
      bonusAtk: 0,
      bonusDef: 0,
      bonusHp: 0,
      critRate: 0,
      atkSpeed: 0,
      magicAtk: 0,
    },
  );
}
