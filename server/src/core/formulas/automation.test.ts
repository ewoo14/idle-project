import { describe, expect, it } from "vitest";
import {
  AUTOMATION_FEATURE,
  type DecideOnClearInput,
  type DecideOnDeathInput,
  decideOnClear,
  decideOnDeath,
  efficiencyUpgradeCost,
  evaluateSkillRule,
  isFeatureUnlocked,
  type SkillRuleContext,
} from "./automation.js";

describe("automation decideOnClear (progression on clear)", () => {
  const base: DecideOnClearInput = {
    mode: "Advance",
    clearedGlobalStage: 5,
    highestClearedGlobalStage: 5,
    farmLockStage: 3,
    autoBossChallenge: true,
    nextIsBoss: false,
  };

  it("advances to the next stage on a normal clear", () => {
    expect(decideOnClear(base)).toEqual({
      action: "advance",
      targetGlobalStage: 6,
    });
  });

  it("holds before a boss when auto boss challenge is disabled", () => {
    expect(
      decideOnClear({ ...base, nextIsBoss: true, autoBossChallenge: false }),
    ).toEqual({ action: "hold", targetGlobalStage: 5 });
  });

  it("advances into a boss when auto boss challenge is enabled", () => {
    expect(
      decideOnClear({ ...base, nextIsBoss: true, autoBossChallenge: true }),
    ).toEqual({ action: "advance", targetGlobalStage: 6 });
  });

  it("holds at the farm lock stage in FarmLock mode", () => {
    expect(decideOnClear({ ...base, mode: "FarmLock" })).toEqual({
      action: "hold",
      targetGlobalStage: 3,
    });
  });

  it("clamps the farm lock stage to highest+1", () => {
    expect(
      decideOnClear({ ...base, mode: "FarmLock", farmLockStage: 99 }),
    ).toEqual({ action: "hold", targetGlobalStage: 6 });
  });

  it("advances on clear in AutoRetreat mode (retreat only triggers on death)", () => {
    expect(decideOnClear({ ...base, mode: "AutoRetreat" })).toEqual({
      action: "advance",
      targetGlobalStage: 6,
    });
  });
});

describe("automation decideOnDeath (progression on death)", () => {
  const base: DecideOnDeathInput = {
    mode: "AutoRetreat",
    currentGlobalStage: 8,
    consecutiveDeaths: 0,
    deathThreshold: 3,
  };

  it("holds while consecutive deaths stay below the threshold", () => {
    expect(decideOnDeath({ ...base, consecutiveDeaths: 2 })).toEqual({
      action: "hold",
      targetGlobalStage: 8,
    });
  });

  it("retreats one stage once deaths reach the threshold", () => {
    expect(decideOnDeath({ ...base, consecutiveDeaths: 3 })).toEqual({
      action: "retreat",
      targetGlobalStage: 7,
    });
  });

  it("never retreats below stage 1 (floor)", () => {
    expect(
      decideOnDeath({ ...base, currentGlobalStage: 1, consecutiveDeaths: 5 }),
    ).toEqual({ action: "hold", targetGlobalStage: 1 });
  });

  it("ignores deaths entirely outside AutoRetreat mode", () => {
    expect(
      decideOnDeath({ ...base, mode: "Advance", consecutiveDeaths: 9 }),
    ).toEqual({ action: "hold", targetGlobalStage: 8 });
    expect(
      decideOnDeath({ ...base, mode: "FarmLock", consecutiveDeaths: 9 }),
    ).toEqual({ action: "hold", targetGlobalStage: 8 });
  });
});

describe("automation feature unlock gating", () => {
  it("unlocks Progression from the very start", () => {
    expect(
      isFeatureUnlocked(AUTOMATION_FEATURE.Progression, {
        highestClearedChapter: 0,
        rebirthCount: 0,
      }),
    ).toBe(true);
  });

  it("gates SkillTactics behind chapter 3", () => {
    expect(
      isFeatureUnlocked(AUTOMATION_FEATURE.SkillTactics, {
        highestClearedChapter: 2,
        rebirthCount: 0,
      }),
    ).toBe(false);
    expect(
      isFeatureUnlocked(AUTOMATION_FEATURE.SkillTactics, {
        highestClearedChapter: 3,
        rebirthCount: 0,
      }),
    ).toBe(true);
  });

  it("gates AutoGear behind chapter 5", () => {
    expect(
      isFeatureUnlocked(AUTOMATION_FEATURE.AutoGear, {
        highestClearedChapter: 4,
        rebirthCount: 0,
      }),
    ).toBe(false);
    expect(
      isFeatureUnlocked(AUTOMATION_FEATURE.AutoGear, {
        highestClearedChapter: 5,
        rebirthCount: 0,
      }),
    ).toBe(true);
  });

  it("gates AutoConsumable behind the rebirth axis (chapter alone is insufficient)", () => {
    expect(
      isFeatureUnlocked(AUTOMATION_FEATURE.AutoConsumable, {
        highestClearedChapter: 9,
        rebirthCount: 0,
      }),
    ).toBe(false);
    expect(
      isFeatureUnlocked(AUTOMATION_FEATURE.AutoConsumable, {
        highestClearedChapter: 9,
        rebirthCount: 1,
      }),
    ).toBe(true);
  });
});

describe("automation efficiency upgrade cost curve", () => {
  it("returns the base cost at level 0", () => {
    expect(efficiencyUpgradeCost(1000, 1.5, 0)).toBe(1000);
  });

  it("grows geometrically and stays strictly monotonic (infinite, no cap)", () => {
    expect(efficiencyUpgradeCost(1000, 1.5, 2)).toBe(2250);
    expect(efficiencyUpgradeCost(1000, 1.5, 10)).toBeGreaterThan(
      efficiencyUpgradeCost(1000, 1.5, 9),
    );
  });

  it("guards negative levels back to the base cost", () => {
    expect(efficiencyUpgradeCost(1000, 1.5, -3)).toBe(1000);
  });

  it("rounds the cost to the nearest integer (Math.round, not floor/ceil)", () => {
    // 100 * 1.5^3 = 337.5 → Math.round → 338 (floor 라면 337)
    expect(efficiencyUpgradeCost(100, 1.5, 3)).toBe(338);
  });
});

describe("automation evaluateSkillRule (skill auto tactics)", () => {
  const ctx = (over: Partial<SkillRuleContext> = {}): SkillRuleContext => ({
    selfHpPct: 1,
    isBossElite: false,
    buffActive: false,
    ...over,
  });

  it("Always fires unconditionally", () => {
    expect(evaluateSkillRule("Always", 0, ctx())).toBe(true);
  });

  it("BossEliteOnly fires only against boss/elite targets", () => {
    expect(
      evaluateSkillRule("BossEliteOnly", 0, ctx({ isBossElite: true })),
    ).toBe(true);
    expect(
      evaluateSkillRule("BossEliteOnly", 0, ctx({ isBossElite: false })),
    ).toBe(false);
  });

  it("HpBelow fires when self HP is at or below the threshold", () => {
    expect(evaluateSkillRule("HpBelow", 0.3, ctx({ selfHpPct: 0.3 }))).toBe(
      true,
    );
    expect(evaluateSkillRule("HpBelow", 0.3, ctx({ selfHpPct: 0.29 }))).toBe(
      true,
    );
    expect(evaluateSkillRule("HpBelow", 0.3, ctx({ selfHpPct: 0.31 }))).toBe(
      false,
    );
  });

  it("HpBelow clamps both threshold and selfHpPct into [0,1]", () => {
    expect(evaluateSkillRule("HpBelow", 5, ctx({ selfHpPct: 1 }))).toBe(true);
    expect(evaluateSkillRule("HpBelow", -1, ctx({ selfHpPct: 0 }))).toBe(true);
  });

  it("MaintainBuff fires only while the buff is inactive", () => {
    expect(
      evaluateSkillRule("MaintainBuff", 0, ctx({ buffActive: false })),
    ).toBe(true);
    expect(
      evaluateSkillRule("MaintainBuff", 0, ctx({ buffActive: true })),
    ).toBe(false);
  });
});
