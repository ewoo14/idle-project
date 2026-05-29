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

// 펫 진화(별/Star) parity 상수 — 클라(PetLevelFormula)와 1:1.
// 진화 비용은 골드 기하 증가(무한 sink), 별 배수는 선형 무한.
export const PET_EVOLVE_BASE = 50000;
export const PET_EVOLVE_GROWTH = 1.8;
export const PET_STAR_STEP = 0.15;

// 다음 별로 진화하는 데 필요한 골드 비용. 기하 증가(무한 sink), 음수 별은 0성 처리.
export function getPetEvolveCost(star: number): number {
  return Math.floor(PET_EVOLVE_BASE * PET_EVOLVE_GROWTH ** Math.max(0, star));
}

// 장착 펫 보너스에 곱하는 별 배수. 선형 무한(star0=1.0), 음수 별은 0성 처리.
export function getPetStarMultiplier(star: number): number {
  return Math.fround(1 + PET_STAR_STEP * Math.max(0, star));
}
