import { describe, expect, it } from "vitest";
import {
  defaultPrimaryStats,
  deriveStats,
  type PrimaryStats,
} from "./stats.js";

describe("stats formulas", () => {
  it.each([
    [1, { str: 12, dex: 6, int: 3, wis: 3, con: 10, luk: 4 }],
    [10, { str: 29.1, dex: 14.1, int: 8.9, wis: 8.9, con: 24.4, luk: 10.8 }],
    [50, { str: 105.1, dex: 50.1, int: 34.9, wis: 34.9, con: 88.4, luk: 40.8 }],
    [
      100,
      { str: 200.1, dex: 95.1, int: 67.4, wis: 67.4, con: 168.4, luk: 78.3 },
    ],
  ])("전사 레벨 %i 기본 1차 능력치를 계산한다", (level, expected) => {
    expect(defaultPrimaryStats(1, level)).toEqual(expected);
  });

  it.each([
    [1, { hp: 120, mp: 30, physAtk: 24, magicAtk: 8 }],
    [10, { hp: 444, mp: 107, physAtk: 58, magicAtk: 22 }],
    [50, { hp: 1884, mp: 449, physAtk: 210, magicAtk: 87 }],
    [100, { hp: 3684, mp: 877, physAtk: 400, magicAtk: 169 }],
  ])("전사 레벨 %i 기본 2차 능력치를 계산한다", (level, expected) => {
    const primary = defaultPrimaryStats(1, level);

    expect(deriveStats(primary, level)).toMatchObject(expected);
  });

  it.each([
    [0, { hp: 120, physAtk: 24 }],
    [5, { hp: 170, physAtk: 34 }],
    [10, { hp: 220, physAtk: 44 }],
  ])("applies rebirth bonus point anchor %i with +10 HP and +2 PhysAtk per point", (rebirthBonusPoints, expected) => {
    const primary = defaultPrimaryStats(1, 1);

    expect(deriveStats(primary, 1, {}, rebirthBonusPoints)).toMatchObject(
      expected,
    );
  });

  it("마법사 기본 능력치는 INT와 WIS 중심으로 성장한다", () => {
    const primary = defaultPrimaryStats(2, 10);

    expect(primary.int).toBeGreaterThan(primary.str);
    expect(primary.wis).toBeGreaterThan(primary.dex);
    expect(deriveStats(primary, 10).magicAtk).toBeGreaterThan(
      deriveStats(primary, 10).physAtk,
    );
  });

  it("궁수 기본 능력치는 DEX와 LUK 중심으로 성장한다", () => {
    const primary = defaultPrimaryStats(3, 10);

    expect(primary.dex).toBeGreaterThan(primary.str);
    expect(primary.luk).toBeGreaterThan(primary.con);
    expect(deriveStats(primary, 10).critRate).toBeGreaterThan(0.02);
  });

  it("장비 보너스를 2차 능력치에 더한다", () => {
    const primary = defaultPrimaryStats(1, 1);

    expect(
      deriveStats(primary, 1, { hp: 50, physAtk: 10, critRate: 0.1 }),
    ).toMatchObject({
      hp: 170,
      physAtk: 34,
      critRate: 0.108,
    });
  });

  it("확률형 2차 능력치는 상한 1.0을 넘지 않는다", () => {
    const primary: PrimaryStats = {
      str: 1,
      dex: 1000,
      int: 1,
      wis: 1,
      con: 1,
      luk: 1000,
    };

    expect(
      deriveStats(primary, 1, { critRate: 1, dodge: 1, accuracy: 1 }),
    ).toMatchObject({
      critRate: 1,
      dodge: 1,
      accuracy: 1,
    });
  });

  it("같은 입력은 항상 같은 결과를 반환한다", () => {
    const primary = defaultPrimaryStats(5, 42);

    expect(deriveStats(primary, 42, { magicAtk: 7 })).toEqual(
      deriveStats(primary, 42, { magicAtk: 7 }),
    );
  });
  it.each([
    [6, 1, { str: 11, dex: 5, int: 4, wis: 6, con: 13, luk: 3 }],
    [
      6,
      10,
      { str: 26.3, dex: 12.7, int: 10.3, wis: 15.9, con: 32.4, luk: 9.3 },
    ],
    [7, 1, { str: 14, dex: 6, int: 2, wis: 2, con: 7, luk: 7 }],
    [7, 10, { str: 32, dex: 15.5, int: 7.4, wis: 7.4, con: 17.8, luk: 17.8 }],
    [8, 1, { str: 3, dex: 5, int: 13, wis: 11, con: 5, luk: 4 }],
    [
      8,
      10,
      { str: 8.9, dex: 12.2, int: 30.6, wis: 27.2, con: 14.2, luk: 11.2 },
    ],
  ])("mirrors class expansion growth for classId %i at level %i", (classId, level, expected) => {
    expect(
      defaultPrimaryStats(
        classId as Parameters<typeof defaultPrimaryStats>[0],
        level,
      ),
    ).toEqual(expected);
  });

  it("applies affix equipment bonuses and keeps final clamps", () => {
    const primary = defaultPrimaryStats(1, 1);

    expect(
      deriveStats(primary, 1, {
        critRate: 0.02,
        atkSpeed: 0.1,
        magicAtk: 5,
      }),
    ).toMatchObject({
      critRate: 0.028,
      atkSpeed: 1.1,
      magicAtk: 13,
    });

    expect(
      deriveStats(primary, 1, {
        critRate: 3,
        atkSpeed: 9,
      }),
    ).toMatchObject({
      critRate: 1,
      atkSpeed: 3,
    });
  });
});
