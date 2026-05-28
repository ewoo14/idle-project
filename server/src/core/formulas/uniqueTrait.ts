export type UniqueTrait = 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8;

const toClientFloat = Math.fround;

function getUniqueBaseValue(trait: number): number {
  switch (trait) {
    case 1:
      return toClientFloat(0.08);
    case 2:
      return toClientFloat(0.15);
    case 3:
      return toClientFloat(0.05);
    case 4:
      return toClientFloat(0.1);
    case 5:
      return toClientFloat(0.08);
    case 6:
    case 7:
      return toClientFloat(0.12);
    case 8:
      return toClientFloat(0.1);
    default:
      return toClientFloat(0);
  }
}

export function rarityGrantsUnique(rarity: number): boolean {
  return rarity === 4 || rarity === 6;
}

export function rarityGrantsTwoTraits(rarity: number): boolean {
  return rarity === 6;
}

export function getTraitValue(trait: number, rarity: number): number {
  const baseValue = getUniqueBaseValue(trait);
  if (baseValue <= 0) {
    return toClientFloat(0);
  }
  if (rarity === 4) {
    return toClientFloat(baseValue);
  }
  if (rarity === 6) {
    return toClientFloat(baseValue * 1.5);
  }
  return toClientFloat(0);
}
