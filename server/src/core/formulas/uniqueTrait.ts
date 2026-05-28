export type UniqueTrait = 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8;

const toClientFloat = Math.fround;

export interface UniqueTraitFlatBonus {
  critDmg: number;
  critRate: number;
  atkSpeed: number;
}

export interface UniqueTraitCoreMultipliers {
  hp: number;
  physAtk: number;
  magicAtk: number;
  physDef: number;
  magicDef: number;
}

export interface UniqueTraitEffects {
  flat: UniqueTraitFlatBonus;
  multipliers: UniqueTraitCoreMultipliers;
}

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

export function accumulateTraitEffects(
  traits: number[],
  rarity: number,
): UniqueTraitEffects {
  const effects: UniqueTraitEffects = {
    flat: {
      critDmg: 0,
      critRate: 0,
      atkSpeed: 0,
    },
    multipliers: {
      hp: 0,
      physAtk: 0,
      magicAtk: 0,
      physDef: 0,
      magicDef: 0,
    },
  };

  for (const trait of traits) {
    const value = getTraitValue(trait, rarity);
    if (value <= 0) {
      continue;
    }

    switch (trait) {
      case 1:
        effects.multipliers.physAtk = combineMultiplier(
          effects.multipliers.physAtk,
          value,
        );
        effects.multipliers.magicAtk = combineMultiplier(
          effects.multipliers.magicAtk,
          value,
        );
        effects.multipliers.physDef = combineMultiplier(
          effects.multipliers.physDef,
          value,
        );
        effects.multipliers.magicDef = combineMultiplier(
          effects.multipliers.magicDef,
          value,
        );
        break;
      case 2:
        effects.flat.critDmg = toClientFloat(effects.flat.critDmg + value);
        break;
      case 3:
        effects.flat.critRate = toClientFloat(effects.flat.critRate + value);
        break;
      case 4:
        effects.multipliers.hp = combineMultiplier(
          effects.multipliers.hp,
          value,
        );
        break;
      case 5:
        effects.flat.atkSpeed = toClientFloat(effects.flat.atkSpeed + value);
        break;
      case 6:
        effects.multipliers.physAtk = combineMultiplier(
          effects.multipliers.physAtk,
          value,
        );
        break;
      case 7:
        effects.multipliers.magicAtk = combineMultiplier(
          effects.multipliers.magicAtk,
          value,
        );
        break;
      case 8:
        effects.multipliers.physDef = combineMultiplier(
          effects.multipliers.physDef,
          value,
        );
        effects.multipliers.magicDef = combineMultiplier(
          effects.multipliers.magicDef,
          value,
        );
        break;
    }
  }

  return effects;
}

export function applyUniqueTraitMultipliers<
  T extends UniqueTraitCoreMultipliers,
>(stats: T, multipliers: UniqueTraitCoreMultipliers): T {
  return {
    ...stats,
    hp: stats.hp * (1 + multipliers.hp),
    physAtk: stats.physAtk * (1 + multipliers.physAtk),
    magicAtk: stats.magicAtk * (1 + multipliers.magicAtk),
    physDef: stats.physDef * (1 + multipliers.physDef),
    magicDef: stats.magicDef * (1 + multipliers.magicDef),
  };
}

function combineMultiplier(current: number, next: number): number {
  return toClientFloat((1 + current) * (1 + next) - 1);
}
