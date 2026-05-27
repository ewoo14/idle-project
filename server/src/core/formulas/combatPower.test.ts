import { describe, expect, it } from "vitest";
import { computeCombatPower } from "./combatPower.js";
import type { DerivedStats } from "./stats.js";

const baseDerivedStats: DerivedStats = {
  hp: 0,
  mp: 0,
  physAtk: 0,
  magicAtk: 0,
  physDef: 0,
  magicDef: 0,
  atkSpeed: 0,
  moveSpeed: 0,
  critRate: 0,
  critDmg: 0,
  dodge: 0,
  accuracy: 0,
};

describe("combat power formula", () => {
  it("matches the client weighted derived stats anchor", () => {
    expect(
      computeCombatPower({
        ...baseDerivedStats,
        hp: 1234,
        physAtk: 100,
        magicAtk: 50,
        physDef: 30,
        magicDef: 20,
        atkSpeed: 1.5,
        critRate: 0.25,
        critDmg: 1.8,
      }),
    ).toBe(978);
  });

  it("uses double precision accumulation for large parity anchors", () => {
    expect(
      computeCombatPower({
        ...baseDerivedStats,
        hp: 1234567,
        physAtk: 10000000,
        magicAtk: 3000000,
        physDef: 100000,
        magicDef: 50000,
        atkSpeed: 2.25,
        critRate: 0.333,
        critDmg: 2.75,
      }),
    ).toBe(13424348);
  });

  it("returns zero when all stats are zero", () => {
    expect(computeCombatPower(baseDerivedStats)).toBe(0);
  });

  it("clamps negative weighted totals to zero", () => {
    expect(
      computeCombatPower({
        ...baseDerivedStats,
        hp: -100,
        physAtk: -20,
        magicAtk: -10,
        physDef: -5,
        magicDef: -5,
        atkSpeed: -1,
        critRate: -0.2,
        critDmg: -1,
      }),
    ).toBe(0);
  });
});
