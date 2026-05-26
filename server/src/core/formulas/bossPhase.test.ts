import { describe, expect, it } from "vitest";
import {
  getBossPhase,
  getPhaseAtkMultiplier,
  getPhaseAtkSpeedMultiplier,
  getSpecialAttackDamageMultiplier,
  SPECIAL_ATTACK_INTERVAL_SECONDS,
} from "./bossPhase.js";

describe("boss phase formulas", () => {
  it.each([
    { hpRatio: 1.0, expectedPhase: 1 },
    { hpRatio: 0.7, expectedPhase: 1 },
    { hpRatio: 0.66, expectedPhase: 2 },
    { hpRatio: 0.5, expectedPhase: 2 },
    { hpRatio: 0.33, expectedPhase: 3 },
    { hpRatio: 0.2, expectedPhase: 3 },
    { hpRatio: 0.0, expectedPhase: 3 },
  ])("matches FBossPhaseFormula phase boundary for HP ratio $hpRatio", ({
    hpRatio,
    expectedPhase,
  }) => {
    expect(getBossPhase(hpRatio)).toBe(expectedPhase);
  });

  it("clamps HP ratio before resolving the phase", () => {
    expect(getBossPhase(2)).toBe(1);
    expect(getBossPhase(-1)).toBe(3);
  });

  it.each([
    { phase: 1, expectedMultiplier: 1.0 },
    { phase: 2, expectedMultiplier: 1.25 },
    { phase: 3, expectedMultiplier: 1.6 },
    { phase: 0, expectedMultiplier: 1.0 },
    { phase: 4, expectedMultiplier: 1.0 },
  ])("matches FBossPhaseFormula attack multiplier for phase $phase", ({
    phase,
    expectedMultiplier,
  }) => {
    expect(getPhaseAtkMultiplier(phase)).toBe(expectedMultiplier);
  });

  it.each([
    { phase: 1, expectedMultiplier: 1.0 },
    { phase: 2, expectedMultiplier: 1.15 },
    { phase: 3, expectedMultiplier: 1.3 },
    { phase: 0, expectedMultiplier: 1.0 },
    { phase: 4, expectedMultiplier: 1.0 },
  ])("matches FBossPhaseFormula attack speed multiplier for phase $phase", ({
    phase,
    expectedMultiplier,
  }) => {
    expect(getPhaseAtkSpeedMultiplier(phase)).toBe(expectedMultiplier);
  });

  it("matches FBossPhaseFormula special attack constants", () => {
    expect(SPECIAL_ATTACK_INTERVAL_SECONDS).toBe(6);
    expect(getSpecialAttackDamageMultiplier()).toBe(2.5);
  });
});
