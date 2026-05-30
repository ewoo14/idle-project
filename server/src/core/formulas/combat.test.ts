import { describe, expect, it } from "vitest";
import {
  applyCrit,
  applyCurseAmplification,
  computeClassDamage,
  computeDamage,
  computeElementMultiplier,
  computeMagicDamage,
  computeResonanceMultiplier,
  rollCrit,
} from "./combat.js";
import type { DerivedStats } from "./stats.js";

const clientServerDamageAnchors = [
  { atk: 100, def: 20, clientResult: 88 },
  { atk: 10, def: 100, clientResult: 0.5 },
  { atk: 50, def: 0, clientResult: 50 },
  { atk: 0, def: 0, clientResult: 0 },
  { atk: 1000, def: 500, clientResult: 700 },
];

describe("combat formulas", () => {
  it.each(
    clientServerDamageAnchors,
  )("ATK $atk DEF $def 피해량을 클라이언트 기준값과 동일하게 계산한다", ({
    atk,
    def,
    clientResult,
  }) => {
    const serverResult = computeDamage(atk, def);

    expect(serverResult).toBe(clientResult);
    expect(serverResult - clientResult).toBe(0);
  });

  it.each(
    clientServerDamageAnchors,
  )("ATK $atk DEF $def cross-validation diff 0 anchor", ({
    atk,
    def,
    clientResult,
  }) => {
    expect(computeDamage(atk, def) - clientResult).toBe(0);
  });

  it("applies critical multiplier only when the roll crits", () => {
    expect(applyCrit(34, false, 1.8)).toBe(34);
    expect(applyCrit(34, true, 1.8)).toBe(61.2);
    expect(applyCrit(34, true, 0.5)).toBe(34);
  });

  it("rolls deterministic critical boundaries", () => {
    const rng = () => 0.5;

    expect(rollCrit(0, rng)).toBe(false);
    expect(rollCrit(1, rng)).toBe(true);
    expect(rollCrit(-0.1, () => 0)).toBe(false);
    expect(rollCrit(1.2, () => 0.999)).toBe(true);
  });

  it("computes magic damage with the same curve using MagicAtk vs MagicDef", () => {
    expect(computeMagicDamage(80, 20)).toBe(68);
  });

  it("증폭한다: 저주 활성 시 받는 피해를 Magnitude 비율만큼", () => {
    // 클라 CombatComponent::TakeDamageTyped 미러: damage x (1 + curseMagnitude)
    expect(applyCurseAmplification(100, 0.2)).toBe(120);
    expect(applyCurseAmplification(100, 0.15)).toBeCloseTo(115);
    expect(applyCurseAmplification(50, 0.5)).toBe(75);
  });

  it("원복한다: 저주가 없으면(Magnitude 0) 피해 불변", () => {
    expect(applyCurseAmplification(100, 0)).toBe(100);
    // 음수 방어: 0으로 클램프되어 증폭 없음
    expect(applyCurseAmplification(100, -0.5)).toBe(100);
  });

  it("computes element multipliers for weakness, resistance, and neutral hits", () => {
    expect(computeElementMultiplier("None", "Fire")).toBe(1);
    expect(computeElementMultiplier("Fire", "Fire")).toBe(1.5);
    expect(computeElementMultiplier("Ice", "Fire")).toBe(0.5);
    expect(computeElementMultiplier("Lightning", "Fire")).toBe(1);
    expect(computeElementMultiplier("Holy", "Dark")).toBe(1.5);
    expect(computeElementMultiplier("Dark", "Holy")).toBe(1.5);
    expect(computeElementMultiplier("Dark", "None")).toBe(1);
    expect(computeElementMultiplier("Dark", "Fire")).toBe(1);
  });

  it("computes resonance multipliers (weak/resist/backward-compat)", () => {
    // 약점 우선
    expect(computeResonanceMultiplier("Dark", "Dark", "Ice")).toBe(1.5);
    // 명시 저항
    expect(computeResonanceMultiplier("Ice", "Dark", "Ice")).toBe(0.5);
    // 저항 None → computeElementMultiplier와 동일(하위호환)
    expect(computeResonanceMultiplier("Fire", "Fire", "None")).toBe(1.5);
    expect(computeResonanceMultiplier("Ice", "Fire", "None")).toBe(0.5);
    expect(computeResonanceMultiplier("Lightning", "Fire", "None")).toBe(1);
    // 약점·저항 모두 불일치 → 폴백
    expect(computeResonanceMultiplier("Holy", "Dark", "Ice")).toBe(1.5); // Holy↔Dark 상극보정
  });

  it.each([
    {
      classId: 1,
      label: "Warrior",
      stats: { physAtk: 40, magicAtk: 80 },
      physDef: 10,
      magicDef: 80,
      expectedDamage: 34,
    },
    {
      classId: 2,
      label: "Mage",
      stats: { physAtk: 12, magicAtk: 40 },
      physDef: 20,
      magicDef: 10,
      expectedDamage: 34,
    },
    {
      classId: 3,
      label: "Archer",
      stats: { physAtk: 40, magicAtk: 12 },
      physDef: 10,
      magicDef: 80,
      expectedDamage: 34,
    },
    {
      classId: 4,
      label: "Thief",
      stats: { physAtk: 40, magicAtk: 12 },
      physDef: 10,
      magicDef: 80,
      expectedDamage: 34,
    },
    {
      classId: 5,
      label: "Cleric",
      stats: { physAtk: 12, magicAtk: 40 },
      physDef: 80,
      magicDef: 10,
      expectedDamage: 34,
    },
    {
      classId: 6,
      label: "Paladin",
      stats: { physAtk: 40, magicAtk: 12 },
      physDef: 10,
      magicDef: 80,
      expectedDamage: 34,
    },
    {
      classId: 7,
      label: "Berserker",
      stats: { physAtk: 40, magicAtk: 12 },
      physDef: 10,
      magicDef: 80,
      expectedDamage: 34,
    },
    {
      classId: 8,
      label: "Summoner",
      stats: { physAtk: 12, magicAtk: 40 },
      physDef: 80,
      magicDef: 10,
      expectedDamage: 34,
    },
  ])("$label class damage matches client CombatFormulas anchor", ({
    classId,
    stats,
    physDef,
    magicDef,
    expectedDamage,
  }) => {
    expect(
      computeClassDamage(stats as DerivedStats, classId, physDef, magicDef),
    ).toBe(expectedDamage);
  });
});
