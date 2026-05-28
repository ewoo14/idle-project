import { getBonusMultiplier } from "./petLevel.js";

export const PET_BONUS_TYPE = {
  None: 0,
  Gold: 1,
  Drop: 2,
  Exp: 3,
  PhysAtk: 4,
  MagicAtk: 5,
  Hp: 6,
  Def: 7,
  AllStat: 8,
} as const;

export type PetBonusType = (typeof PET_BONUS_TYPE)[keyof typeof PET_BONUS_TYPE];

export type PetDefinition = {
  id: string;
  name: string;
  bonusType: PetBonusType;
  bonusPercent: number;
};

const PET_CATALOG: PetDefinition[] = [
  { id: "dog", name: "Dog", bonusType: PET_BONUS_TYPE.Gold, bonusPercent: 20 },
  {
    id: "bird",
    name: "Bird",
    bonusType: PET_BONUS_TYPE.Drop,
    bonusPercent: 15,
  },
  { id: "cat", name: "Cat", bonusType: PET_BONUS_TYPE.Exp, bonusPercent: 15 },
  {
    id: "wolf",
    name: "Wolf",
    bonusType: PET_BONUS_TYPE.PhysAtk,
    bonusPercent: 10,
  },
  {
    id: "owl",
    name: "Owl",
    bonusType: PET_BONUS_TYPE.MagicAtk,
    bonusPercent: 10,
  },
  { id: "bear", name: "Bear", bonusType: PET_BONUS_TYPE.Hp, bonusPercent: 12 },
  {
    id: "turtle",
    name: "Turtle",
    bonusType: PET_BONUS_TYPE.Def,
    bonusPercent: 12,
  },
  { id: "fox", name: "Fox", bonusType: PET_BONUS_TYPE.Gold, bonusPercent: 30 },
  {
    id: "rabbit",
    name: "Rabbit",
    bonusType: PET_BONUS_TYPE.Drop,
    bonusPercent: 25,
  },
  {
    id: "dragon",
    name: "Dragon",
    bonusType: PET_BONUS_TYPE.AllStat,
    bonusPercent: 8,
  },
];

export function getPetCatalog(): PetDefinition[] {
  return PET_CATALOG.map((pet) => ({ ...pet }));
}

export function getEffectiveBonusPercent(
  basePercent: number,
  level: number,
): number {
  return Math.fround(Math.fround(basePercent) * getBonusMultiplier(level));
}
