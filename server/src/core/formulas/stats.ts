/**
 * PR #2 임시 스탯 공식 출처 메모:
 * - sharedGrowth = 1 + (level - 1) * 0.5 는 PR #2 자동 분배 초안 기준이다.
 *   GDD §3.3 자유 분배 스탯은 후속 단계에서 반영한다.
 * - critDmg base 1.5 는 1.5x, 즉 +50% critical damage 표준값이다.
 * - accuracy base 0.75 는 PR #2 임시 초안 기준이며 M2 시뮬레이터로 보정 예정이다.
 */
export type ClassId = 1 | 2 | 3 | 4 | 5;

export interface PrimaryStats {
  str: number;
  dex: number;
  int: number;
  wis: number;
  con: number;
  luk: number;
}

export interface DerivedStats {
  hp: number;
  mp: number;
  physAtk: number;
  magicAtk: number;
  physDef: number;
  magicDef: number;
  atkSpeed: number;
  moveSpeed: number;
  critRate: number;
  critDmg: number;
  dodge: number;
  accuracy: number;
}

type ClassGrowth = {
  initialBonus: PrimaryStats;
  levelBonus: PrimaryStats;
};

const CLASS_GROWTH: Record<ClassId, ClassGrowth> = {
  1: {
    initialBonus: { str: 11, dex: 5, int: 2, wis: 2, con: 9, luk: 3 },
    levelBonus: {
      str: 1.4,
      dex: 0.4,
      int: 0.15,
      wis: 0.15,
      con: 1.1,
      luk: 0.25,
    },
  },
  2: {
    initialBonus: { str: 2, dex: 3, int: 11, wis: 9, con: 4, luk: 3 },
    levelBonus: {
      str: 0.15,
      dex: 0.25,
      int: 1.4,
      wis: 1.1,
      con: 0.35,
      luk: 0.25,
    },
  },
  3: {
    initialBonus: { str: 4, dex: 11, int: 3, wis: 3, con: 5, luk: 9 },
    levelBonus: { str: 0.35, dex: 1.4, int: 0.2, wis: 0.2, con: 0.4, luk: 1.1 },
  },
  4: {
    initialBonus: { str: 3, dex: 10, int: 3, wis: 3, con: 4, luk: 11 },
    levelBonus: {
      str: 0.25,
      dex: 1.25,
      int: 0.2,
      wis: 0.2,
      con: 0.35,
      luk: 1.3,
    },
  },
  5: {
    initialBonus: { str: 2, dex: 3, int: 7, wis: 11, con: 5, luk: 4 },
    levelBonus: {
      str: 0.15,
      dex: 0.2,
      int: 1.0,
      wis: 1.4,
      con: 0.45,
      luk: 0.3,
    },
  },
};

export function defaultPrimaryStats(
  classId: ClassId,
  level: number,
): PrimaryStats {
  assertPositiveLevel(level);
  const profile = CLASS_GROWTH[classId];
  const sharedGrowth = 1 + (level - 1) * 0.5;
  const levelIndex = level - 1;

  return {
    str: roundStat(
      sharedGrowth +
        profile.initialBonus.str +
        levelIndex * profile.levelBonus.str,
    ),
    dex: roundStat(
      sharedGrowth +
        profile.initialBonus.dex +
        levelIndex * profile.levelBonus.dex,
    ),
    int: roundStat(
      sharedGrowth +
        profile.initialBonus.int +
        levelIndex * profile.levelBonus.int,
    ),
    wis: roundStat(
      sharedGrowth +
        profile.initialBonus.wis +
        levelIndex * profile.levelBonus.wis,
    ),
    con: roundStat(
      sharedGrowth +
        profile.initialBonus.con +
        levelIndex * profile.levelBonus.con,
    ),
    luk: roundStat(
      sharedGrowth +
        profile.initialBonus.luk +
        levelIndex * profile.levelBonus.luk,
    ),
  };
}

export function deriveStats(
  primary: PrimaryStats,
  level: number,
  equipmentBonus: Partial<DerivedStats> = {},
): DerivedStats {
  assertPositiveLevel(level);

  return {
    hp: roundWhole(primary.con * 10 + level * 20 + bonus(equipmentBonus.hp)),
    mp: roundWhole(
      primary.wis * 5 +
        primary.int * 2 +
        level * 4 +
        5 +
        bonus(equipmentBonus.mp),
    ),
    physAtk: roundWhole(primary.str * 2 + bonus(equipmentBonus.physAtk)),
    magicAtk: roundWhole(
      primary.int * 2 + primary.wis * 0.5 + bonus(equipmentBonus.magicAtk),
    ),
    physDef: roundWhole(
      primary.con * 1.5 + primary.dex * 0.25 + bonus(equipmentBonus.physDef),
    ),
    magicDef: roundWhole(
      primary.wis * 1.5 + primary.int * 0.25 + bonus(equipmentBonus.magicDef),
    ),
    atkSpeed: roundStat(
      clamp(primary.dex * 0.004 + 1 + bonus(equipmentBonus.atkSpeed), 0.5, 3),
    ),
    moveSpeed: roundStat(
      clamp(primary.dex * 0.002 + 1 + bonus(equipmentBonus.moveSpeed), 0.5, 3),
    ),
    critRate: roundRate(
      clamp(primary.luk * 0.002 + bonus(equipmentBonus.critRate), 0, 1),
    ),
    critDmg: roundStat(
      clamp(1.5 + primary.luk * 0.001 + bonus(equipmentBonus.critDmg), 1, 3),
    ),
    dodge: roundRate(
      clamp(
        primary.dex * 0.0015 +
          primary.luk * 0.001 +
          bonus(equipmentBonus.dodge),
        0,
        1,
      ),
    ),
    accuracy: roundRate(
      clamp(0.75 + primary.dex * 0.002 + bonus(equipmentBonus.accuracy), 0, 1),
    ),
  };
}

function assertPositiveLevel(level: number) {
  if (!Number.isInteger(level) || level < 1) {
    throw new Error("level must be >= 1");
  }
}

function bonus(value: number | undefined) {
  return value ?? 0;
}

function clamp(value: number, min: number, max: number) {
  return Math.min(Math.max(value, min), max);
}

function roundStat(value: number) {
  return Math.round(value * 10) / 10;
}

function roundRate(value: number) {
  return Math.round(value * 1000) / 1000;
}

function roundWhole(value: number) {
  return Math.round(value);
}
