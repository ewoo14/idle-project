import { getCoreRuneMultiplier } from "./rune.js";

export const CLASS_RUNE_SLOT_INDEX = 6;

export interface ClassMasteryMultipliers {
  physAtk: number;
  magicAtk: number;
  physDef: number;
  magicDef: number;
  hp: number;
}

function zeroMultipliers(): ClassMasteryMultipliers {
  return {
    physAtk: 0,
    magicAtk: 0,
    physDef: 0,
    magicDef: 0,
    hp: 0,
  };
}

export function getClassMasteryMultipliers(
  classId: number,
  rarity: number,
  enhanceLevel: number,
): ClassMasteryMultipliers {
  const unit = getCoreRuneMultiplier(rarity, enhanceLevel);
  const result = zeroMultipliers();

  switch (classId) {
    case 1:
      result.physAtk = unit;
      result.physDef = unit;
      break;
    case 2:
      result.magicAtk = unit;
      break;
    case 3:
    case 4:
    case 7:
      result.physAtk = unit;
      break;
    case 5:
      result.magicAtk = unit;
      result.hp = unit;
      break;
    case 6:
      result.physDef = unit;
      result.hp = unit;
      break;
    case 8:
      result.magicAtk = unit;
      break;
    default:
      break;
  }

  return {
    physAtk: Math.fround(result.physAtk),
    magicAtk: Math.fround(result.magicAtk),
    physDef: Math.fround(result.physDef),
    magicDef: Math.fround(result.magicDef),
    hp: Math.fround(result.hp),
  };
}
