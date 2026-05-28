import type { ItemRarity } from "./drop.js";

export type PotentialGrade = "None" | "Rare" | "Epic" | "Unique" | "Legendary";
export type PotentialStat =
  | "None"
  | "PhysAtkPercent"
  | "MagicAtkPercent"
  | "HpPercent"
  | "PhysDefPercent"
  | "MagicDefPercent"
  | "CritRatePercent"
  | "AtkSpeedPercent"
  | "CritDmgPercent";

export interface PotentialLine {
  stat: PotentialStat;
  value: number;
}

export const RANK_CUBE_UPGRADE_CHANCE = 0.08;

const POTENTIAL_STATS: Exclude<PotentialStat, "None">[] = [
  "PhysAtkPercent",
  "MagicAtkPercent",
  "HpPercent",
  "PhysDefPercent",
  "MagicDefPercent",
  "CritRatePercent",
  "AtkSpeedPercent",
  "CritDmgPercent",
];

const GRADE_ORDER: PotentialGrade[] = [
  "None",
  "Rare",
  "Epic",
  "Unique",
  "Legendary",
];

function gradeIndex(grade: PotentialGrade): number {
  return GRADE_ORDER.indexOf(grade);
}

export function getMaxPotentialGrade(rarity: ItemRarity): PotentialGrade {
  switch (rarity) {
    case "Rare":
      return "Epic";
    case "Epic":
      return "Unique";
    case "Unique":
    case "Legendary":
    case "Transcendent":
    case "Mythic":
      return "Legendary";
    case "None":
    case "Common":
      return "None";
  }
}

export function getPotentialLineCount(grade: PotentialGrade): number {
  switch (grade) {
    case "Rare":
      return 1;
    case "Epic":
      return 2;
    case "Unique":
    case "Legendary":
      return 3;
    case "None":
      return 0;
  }
}

export function getPotentialRollRange(grade: PotentialGrade): [number, number] {
  switch (grade) {
    case "Rare":
      return [0.01, 0.03];
    case "Epic":
      return [0.03, 0.06];
    case "Unique":
      return [0.06, 0.1];
    case "Legendary":
      return [0.1, 0.15];
    case "None":
      return [0, 0];
  }
}

export function rollPotentialLines(
  grade: PotentialGrade,
  rng: () => number = Math.random,
): PotentialLine[] {
  const lineCount = getPotentialLineCount(grade);
  if (lineCount <= 0) {
    return [];
  }

  const [min, max] = getPotentialRollRange(grade);
  const stats = [...POTENTIAL_STATS];
  for (let index = stats.length - 1; index > 0; index -= 1) {
    const swapIndex = Math.min(Math.floor(rng() * (index + 1)), index);
    [stats[index], stats[swapIndex]] = [stats[swapIndex], stats[index]];
  }

  return stats.slice(0, lineCount).map((stat) => ({
    stat,
    value: Math.round((min + (max - min) * rng()) * 1000) / 1000,
  }));
}

export function applyRankCube(
  currentGrade: PotentialGrade,
  maxGrade: PotentialGrade,
  rng: () => number = Math.random,
): { grade: PotentialGrade; lines: PotentialLine[] } {
  let grade = currentGrade;
  if (
    gradeIndex(currentGrade) > 0 &&
    gradeIndex(currentGrade) < gradeIndex(maxGrade) &&
    rng() < RANK_CUBE_UPGRADE_CHANCE
  ) {
    grade = GRADE_ORDER[gradeIndex(currentGrade) + 1];
  } else {
    rng();
  }

  return {
    grade,
    lines: rollPotentialLines(grade, rng),
  };
}
