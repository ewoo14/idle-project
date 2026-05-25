export const LEVEL_CAP = 200;

export type EnhanceResourceCost = {
  gold: number;
  reinforcementStone: number;
  mysticStone: number;
};

/**
 * 경험치 곡선 초안.
 * docs/planning/05-balance-philosophy.md §2.1 의 근사값 검증 포인트를 따른다.
 */
export function expToNext(level: number): number {
  assertLevel(level);
  if (level >= LEVEL_CAP) {
    return Infinity;
  }
  return Math.round(25 * level ** 2 + 100 * level + (level * (level - 10)) / 9);
}

/**
 * 누적 경험치. 레벨 1에서 입력 레벨에 도달하기 직전까지 필요한 총량이다.
 */
export function cumulativeExp(level: number): number {
  assertLevel(level);
  let total = 0;
  for (
    let currentLevel = 1;
    currentLevel < Math.min(level, LEVEL_CAP);
    currentLevel += 1
  ) {
    total += expToNext(currentLevel);
  }
  return total;
}

/**
 * 강화 성공률. currentStage는 현재 강화 수치(+0~+15)를 의미한다.
 */
export function enhanceSuccessRate(currentStage: number): number {
  assertEnhanceStage(currentStage);
  if (currentStage <= 5) {
    return 1;
  }
  if (currentStage <= 10) {
    return 0.9;
  }
  return ({ 11: 0.7, 12: 0.6, 13: 0.5, 14: 0.4, 15: 0.3 } as const)[
    currentStage as 11 | 12 | 13 | 14 | 15
  ];
}

/**
 * 강화 시도당 자원 소비. docs/planning/05-balance-philosophy.md §2.3 표와 1:1로 맞춘다.
 */
export function enhanceResourceCost(currentStage: number): EnhanceResourceCost {
  assertEnhanceStage(currentStage);
  if (currentStage <= 5) {
    return {
      gold: Math.max(currentStage, 1) * 100,
      reinforcementStone: 0,
      mysticStone: 0,
    };
  }
  if (currentStage <= 10) {
    return {
      gold: currentStage * 500,
      reinforcementStone: 1,
      mysticStone: 0,
    };
  }
  const highStageCost: Record<11 | 12 | 13 | 14 | 15, EnhanceResourceCost> = {
    11: { gold: 5000, reinforcementStone: 3, mysticStone: 0 },
    12: { gold: 8000, reinforcementStone: 5, mysticStone: 0 },
    13: { gold: 12000, reinforcementStone: 8, mysticStone: 1 },
    14: { gold: 20000, reinforcementStone: 12, mysticStone: 2 },
    15: { gold: 35000, reinforcementStone: 18, mysticStone: 3 },
  };
  return highStageCost[currentStage as 11 | 12 | 13 | 14 | 15];
}

function assertLevel(level: number) {
  if (!Number.isInteger(level) || level < 1) {
    throw new Error("level must be >= 1");
  }
}

function assertEnhanceStage(currentStage: number) {
  if (
    !Number.isInteger(currentStage) ||
    currentStage < 0 ||
    currentStage > 15
  ) {
    throw new Error("currentStage must be 0~15");
  }
}
