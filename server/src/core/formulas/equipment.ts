/**
 * 장비 PowerScore - UE5 client FItemPowerScore::Compute 미러.
 * (Atk + Def + Hp/10) x (1 + EnhanceLevel x 0.1)
 */
export type ItemSlot = 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8;
// 1=Weapon, 2=Helmet, 3=Top, 4=Bottom, 5=Shoes, 6=Gloves, 7=Cloak, 8=Accessory

export type ItemRarity = 1 | 2 | 3;
// 1=Common, 2=Uncommon, 3=Rare (PR #9 scope)

export interface ItemInstance {
  itemId: string;
  slot: ItemSlot;
  rarity: ItemRarity;
  bonusAtk: number;
  bonusDef: number;
  bonusHp: number;
  enhanceLevel: number;
}

export function computeItemPowerScore(item: ItemInstance): number {
  return (
    (item.bonusAtk + item.bonusDef + item.bonusHp / 10) *
    (1 + item.enhanceLevel * 0.1)
  );
}

export interface EquipmentBonus {
  bonusAtk: number;
  bonusDef: number;
  bonusHp: number;
}

export function computeInventoryBonus(
  equippedItems: ItemInstance[],
): EquipmentBonus {
  return equippedItems.reduce(
    (acc, item) => ({
      bonusAtk: acc.bonusAtk + item.bonusAtk * (1 + item.enhanceLevel * 0.1),
      bonusDef: acc.bonusDef + item.bonusDef * (1 + item.enhanceLevel * 0.1),
      bonusHp: acc.bonusHp + item.bonusHp * (1 + item.enhanceLevel * 0.1),
    }),
    { bonusAtk: 0, bonusDef: 0, bonusHp: 0 },
  );
}
