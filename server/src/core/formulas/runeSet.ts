export const RUNE_SET_NONE = 0;
export const RUNE_SET_OFFENSE = 1;
export const RUNE_SET_BASTION = 2;
export const RUNE_SET_VITALITY = 3;
export const RUNE_SET_FORTUNE = 4;

export const RUNE_SET_TIER1_COUNT = 2;
export const RUNE_SET_TIER2_COUNT = 4;
export const RUNE_SET_TIER3_COUNT = 6;
export const RUNE_SET_TIER1_BONUS = Math.fround(0.05);
export const RUNE_SET_TIER2_BONUS = Math.fround(0.12);
export const RUNE_SET_TIER3_BONUS = Math.fround(0.25);

export interface RuneSetCoreBonus {
  physAtk: number;
  magicAtk: number;
  physDef: number;
  magicDef: number;
  hp: number;
}

export interface RuneSetUtilBonus {
  critDamage: number;
  goldFind: number;
  expBoost: number;
  offlineEff: number;
}

export interface RuneSetBonus {
  core: RuneSetCoreBonus;
  util: RuneSetUtilBonus;
}

function zeroCoreBonus(): RuneSetCoreBonus {
  return {
    physAtk: 0,
    magicAtk: 0,
    physDef: 0,
    magicDef: 0,
    hp: 0,
  };
}

function zeroUtilBonus(): RuneSetUtilBonus {
  return {
    critDamage: 0,
    goldFind: 0,
    expBoost: 0,
    offlineEff: 0,
  };
}

function froundBonus(bonus: RuneSetBonus): RuneSetBonus {
  return {
    core: {
      physAtk: Math.fround(bonus.core.physAtk),
      magicAtk: Math.fround(bonus.core.magicAtk),
      physDef: Math.fround(bonus.core.physDef),
      magicDef: Math.fround(bonus.core.magicDef),
      hp: Math.fround(bonus.core.hp),
    },
    util: {
      critDamage: Math.fround(bonus.util.critDamage),
      goldFind: Math.fround(bonus.util.goldFind),
      expBoost: Math.fround(bonus.util.expBoost),
      offlineEff: Math.fround(bonus.util.offlineEff),
    },
  };
}

export function getSetTierBonus(equippedCount: number): number {
  const count = Math.max(0, Math.trunc(equippedCount));
  if (count >= RUNE_SET_TIER3_COUNT) {
    return RUNE_SET_TIER3_BONUS;
  }
  if (count >= RUNE_SET_TIER2_COUNT) {
    return RUNE_SET_TIER2_BONUS;
  }
  if (count >= RUNE_SET_TIER1_COUNT) {
    return RUNE_SET_TIER1_BONUS;
  }
  return 0;
}

export function computeRuneSetBonus(
  setCounts: Record<number, number>,
): RuneSetBonus {
  const bonus: RuneSetBonus = {
    core: zeroCoreBonus(),
    util: zeroUtilBonus(),
  };

  for (const [setKey, count] of Object.entries(setCounts)) {
    const runeSet = Number(setKey);
    const tierBonus = getSetTierBonus(count);
    if (tierBonus <= 0) {
      continue;
    }

    switch (runeSet) {
      case RUNE_SET_OFFENSE:
        bonus.core.physAtk += tierBonus;
        bonus.core.magicAtk += tierBonus;
        break;
      case RUNE_SET_BASTION:
        bonus.core.physDef += tierBonus;
        bonus.core.magicDef += tierBonus;
        break;
      case RUNE_SET_VITALITY:
        bonus.core.hp += tierBonus;
        bonus.util.offlineEff += tierBonus;
        break;
      case RUNE_SET_FORTUNE:
        bonus.util.goldFind += tierBonus;
        bonus.util.expBoost += tierBonus;
        bonus.util.critDamage += tierBonus;
        break;
      default:
        break;
    }
  }

  return froundBonus(bonus);
}
