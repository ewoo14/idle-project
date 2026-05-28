import { describe, expect, it } from "vitest";
import {
  applyRankCube,
  getMaxPotentialGrade,
  getPotentialLineCount,
  getPotentialRollRange,
  RANK_CUBE_UPGRADE_CHANCE,
  rollPotentialLines,
} from "./potential.js";

describe("potential formulas", () => {
  it("gates potential grade by item rarity", () => {
    expect(getMaxPotentialGrade("Common")).toBe("None");
    expect(getMaxPotentialGrade("Rare")).toBe("Epic");
    expect(getMaxPotentialGrade("Epic")).toBe("Unique");
    expect(getMaxPotentialGrade("Unique")).toBe("Legendary");
    expect(getMaxPotentialGrade("Legendary")).toBe("Legendary");
    expect(getMaxPotentialGrade("Mythic")).toBe("Legendary");
  });

  it("maps potential grade to deterministic line count", () => {
    expect(getPotentialLineCount("None")).toBe(0);
    expect(getPotentialLineCount("Rare")).toBe(1);
    expect(getPotentialLineCount("Epic")).toBe(2);
    expect(getPotentialLineCount("Unique")).toBe(3);
    expect(getPotentialLineCount("Legendary")).toBe(3);
  });

  it("returns client-float roll ranges by grade", () => {
    expect(getPotentialRollRange("Rare")).toEqual([0.01, 0.03]);
    expect(getPotentialRollRange("Epic")).toEqual([0.03, 0.06]);
    expect(getPotentialRollRange("Unique")).toEqual([0.06, 0.1]);
    expect(getPotentialRollRange("Legendary")).toEqual([0.1, 0.15]);
  });

  it("rolls potential lines within grade range using provided RNG", () => {
    const rolls = [0, 0.2, 0.5, 0.8, 0.999, 0.1];
    const rng = () => rolls.shift() ?? 0;

    expect(rollPotentialLines("Unique", rng)).toEqual([
      { stat: "AtkSpeedPercent", value: 0.06 },
      { stat: "HpPercent", value: 0.06 },
      { stat: "CritDmgPercent", value: 0.06 },
    ]);
  });

  it("rank cube rerolls and upgrades by 8 percent within item cap", () => {
    expect(RANK_CUBE_UPGRADE_CHANCE).toBe(0.08);

    const upgradeRolls = [0.079, 0, 0, 0, 0, 0, 0, 0];
    const upgrade = applyRankCube(
      "Rare",
      "Legendary",
      () => upgradeRolls.shift() ?? 0,
    );
    expect(upgrade.grade).toBe("Epic");
    expect(upgrade.lines).toHaveLength(2);

    const cappedRolls = [0, 0, 0, 0, 0, 0, 0, 0];
    const capped = applyRankCube(
      "Legendary",
      "Legendary",
      () => cappedRolls.shift() ?? 0,
    );
    expect(capped.grade).toBe("Legendary");
    expect(capped.lines).toHaveLength(3);
  });
});
