import type { ItemRarity } from "./drop.js";

export type PotentialGrade =
  | "None"
  | "Rare"
  | "Epic"
  | "Unique"
  | "Legendary"
  | "Transcendent";
export type PotentialStat =
  | "None"
  | "PhysAtkPercent"
  | "MagicAtkPercent"
  | "HpPercent"
  | "PhysDefPercent"
  | "MagicDefPercent"
  | "CritRatePercent"
  | "AtkSpeedPercent"
  | "CritDmgPercent"
  // 잠재 V2 신규 옵션 (클라 EPotentialStat 9/10/11 미러)
  | "AllStatPercent"
  | "GoldFindPercent"
  | "DropRatePercent";

export interface PotentialLine {
  stat: PotentialStat;
  value: number;
}

export const RANK_CUBE_UPGRADE_CHANCE = 0.08;
// 잠재 V2: Legendary→Transcendent 상승은 무한 chase 위해 낮은 확률 (스펙 §3)
export const RANK_CUBE_TRANSCENDENT_CHANCE = 0.05;

const POTENTIAL_STATS: Exclude<PotentialStat, "None">[] = [
  "PhysAtkPercent",
  "MagicAtkPercent",
  "HpPercent",
  "PhysDefPercent",
  "MagicDefPercent",
  "CritRatePercent",
  "AtkSpeedPercent",
  "CritDmgPercent",
  // 잠재 V2 신규 옵션 — 추첨 풀에 포함
  "AllStatPercent",
  "GoldFindPercent",
  "DropRatePercent",
];

const GRADE_ORDER: PotentialGrade[] = [
  "None",
  "Rare",
  "Epic",
  "Unique",
  "Legendary",
  "Transcendent",
];

// 잠재 V2: 등급 상승 시 적용 확률 (인덱스 = 상승 후 등급). Transcendent 도달만 별도 낮은 확률.
function upgradeChanceTo(nextGrade: PotentialGrade): number {
  return nextGrade === "Transcendent"
    ? RANK_CUBE_TRANSCENDENT_CHANCE
    : RANK_CUBE_UPGRADE_CHANCE;
}

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
      return "Legendary";
    // 잠재 V2: 고레어도 아이템은 Transcendent 잠재 허용 (무한 chase)
    case "Legendary":
    case "Transcendent":
    case "Mythic":
      return "Transcendent";
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
    // 잠재 V2: Transcendent 4줄
    case "Transcendent":
      return 4;
    case "None":
      return 0;
  }
}

// 잠재 V2 신규 옵션 값 배수 (전투 8종 기준 1.0). AllStat 은 전 스탯이라 보수적 소, Gold/Drop 은 중.
const NEW_OPTION_VALUE_SCALE: Partial<Record<PotentialStat, number>> = {
  AllStatPercent: 0.4,
  GoldFindPercent: 1.5,
  DropRatePercent: 1.5,
};

function optionValueScale(stat: PotentialStat): number {
  return NEW_OPTION_VALUE_SCALE[stat] ?? 1;
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
    // 잠재 V2: Transcendent = Legendary × 1.3 상향
    case "Transcendent":
      return [0.13, 0.195];
    case "None":
      return [0, 0];
  }
}

// 잠재 V2: 스탯별 값 범위 (전투 8종 = 등급 기본, 신규 옵션 3종은 배수 적용)
export function getPotentialStatRollRange(
  grade: PotentialGrade,
  stat: PotentialStat,
): [number, number] {
  const [min, max] = getPotentialRollRange(grade);
  const scale = optionValueScale(stat);
  return [
    Math.round(min * scale * 1000) / 1000,
    Math.round(max * scale * 1000) / 1000,
  ];
}

export function rollPotentialLines(
  grade: PotentialGrade,
  rng: () => number = Math.random,
): PotentialLine[] {
  const lineCount = getPotentialLineCount(grade);
  if (lineCount <= 0) {
    return [];
  }

  const stats = [...POTENTIAL_STATS];
  for (let index = stats.length - 1; index > 0; index -= 1) {
    const swapIndex = Math.min(Math.floor(rng() * (index + 1)), index);
    [stats[index], stats[swapIndex]] = [stats[swapIndex], stats[index]];
  }

  return stats.slice(0, lineCount).map((stat) => {
    // 잠재 V2: 신규 옵션은 스탯별 배수로 값 범위 조정
    const [min, max] = getPotentialStatRollRange(grade, stat);
    return {
      stat,
      value: Math.round((min + (max - min) * rng()) * 1000) / 1000,
    };
  });
}

export function applyRankCube(
  currentGrade: PotentialGrade,
  maxGrade: PotentialGrade,
  rng: () => number = Math.random,
): { grade: PotentialGrade; lines: PotentialLine[] } {
  let grade = currentGrade;
  const nextGrade = GRADE_ORDER[gradeIndex(currentGrade) + 1] as
    | PotentialGrade
    | undefined;
  if (
    gradeIndex(currentGrade) > 0 &&
    gradeIndex(currentGrade) < gradeIndex(maxGrade) &&
    nextGrade !== undefined &&
    // 잠재 V2: Legendary→Transcendent 만 낮은 확률, 나머지는 기존 확률
    rng() < upgradeChanceTo(nextGrade)
  ) {
    grade = nextGrade;
  } else {
    rng();
  }

  return {
    grade,
    lines: rollPotentialLines(grade, rng),
  };
}
