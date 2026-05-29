import { describe, expect, it } from "vitest";
import {
  applyRankCube,
  getMaxPotentialGrade,
  getPotentialLineCount,
  getPotentialRollRange,
  getPotentialStatRollRange,
  RANK_CUBE_TRANSCENDENT_CHANCE,
  RANK_CUBE_UPGRADE_CHANCE,
  rollPotentialLines,
} from "./potential.js";

describe("potential formulas", () => {
  it("gates potential grade by item rarity", () => {
    expect(getMaxPotentialGrade("Common")).toBe("None");
    expect(getMaxPotentialGrade("Rare")).toBe("Epic");
    expect(getMaxPotentialGrade("Epic")).toBe("Unique");
    expect(getMaxPotentialGrade("Unique")).toBe("Legendary");
    // 잠재 V2: 고레어도 아이템은 Transcendent 허용, 저레어도는 기존 상한 유지
    expect(getMaxPotentialGrade("Legendary")).toBe("Transcendent");
    expect(getMaxPotentialGrade("Transcendent")).toBe("Transcendent");
    expect(getMaxPotentialGrade("Mythic")).toBe("Transcendent");
  });

  it("does not allow Transcendent potential on low rarity items", () => {
    expect(getMaxPotentialGrade("Common")).not.toBe("Transcendent");
    expect(getMaxPotentialGrade("Rare")).not.toBe("Transcendent");
    expect(getMaxPotentialGrade("Epic")).not.toBe("Transcendent");
    expect(getMaxPotentialGrade("Unique")).not.toBe("Transcendent");
  });

  it("maps potential grade to deterministic line count", () => {
    expect(getPotentialLineCount("None")).toBe(0);
    expect(getPotentialLineCount("Rare")).toBe(1);
    expect(getPotentialLineCount("Epic")).toBe(2);
    expect(getPotentialLineCount("Unique")).toBe(3);
    expect(getPotentialLineCount("Legendary")).toBe(3);
    // 잠재 V2: Transcendent 4줄
    expect(getPotentialLineCount("Transcendent")).toBe(4);
  });

  it("returns client-float roll ranges by grade", () => {
    expect(getPotentialRollRange("Rare")).toEqual([0.01, 0.03]);
    expect(getPotentialRollRange("Epic")).toEqual([0.03, 0.06]);
    expect(getPotentialRollRange("Unique")).toEqual([0.06, 0.1]);
    expect(getPotentialRollRange("Legendary")).toEqual([0.1, 0.15]);
  });

  it("scales Transcendent roll range above Legendary monotonically", () => {
    const [legMin, legMax] = getPotentialRollRange("Legendary");
    const [transMin, transMax] = getPotentialRollRange("Transcendent");
    expect(transMin).toBeGreaterThan(legMin);
    expect(transMax).toBeGreaterThan(legMax);
    // Transcendent = Legendary × ~1.3
    expect(transMin).toBeCloseTo(legMin * 1.3, 5);
    expect(transMax).toBeCloseTo(legMax * 1.3, 5);
  });

  it("derives new option stat ranges from grade base with conservative scaling", () => {
    const [baseMin, baseMax] = getPotentialRollRange("Legendary");

    // 전투 8종은 등급 기본 범위 그대로
    expect(getPotentialStatRollRange("Legendary", "PhysAtkPercent")).toEqual([
      baseMin,
      baseMax,
    ]);

    const [allMin, allMax] = getPotentialStatRollRange(
      "Legendary",
      "AllStatPercent",
    );
    const [goldMin, goldMax] = getPotentialStatRollRange(
      "Legendary",
      "GoldFindPercent",
    );
    const [dropMin, dropMax] = getPotentialStatRollRange(
      "Legendary",
      "DropRatePercent",
    );

    // 양수 + min < max
    for (const [lo, hi] of [
      [allMin, allMax],
      [goldMin, goldMax],
      [dropMin, dropMax],
    ]) {
      expect(lo).toBeGreaterThan(0);
      expect(hi).toBeGreaterThan(lo);
    }

    // AllStat 은 전 스탯이라 보수적(전투 기본보다 작음), Gold/Drop 은 중(더 큼)
    expect(allMax).toBeLessThan(baseMax);
    expect(goldMax).toBeGreaterThan(baseMax);
    expect(dropMax).toBeGreaterThan(baseMax);
  });

  it("rolls Transcendent with four lines within stat-specific ranges", () => {
    const rng = () => 0.5;
    const lines = rollPotentialLines("Transcendent", rng);
    expect(lines).toHaveLength(4);
    for (const line of lines) {
      const [min, max] = getPotentialStatRollRange("Transcendent", line.stat);
      expect(line.value).toBeGreaterThanOrEqual(min);
      expect(line.value).toBeLessThanOrEqual(max);
    }
  });

  it("rank cube rerolls and upgrades by 8 percent within item cap", () => {
    expect(RANK_CUBE_UPGRADE_CHANCE).toBe(0.08);

    const upgradeRolls = [0.079, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
    const upgrade = applyRankCube(
      "Rare",
      "Legendary",
      () => upgradeRolls.shift() ?? 0,
    );
    expect(upgrade.grade).toBe("Epic");
    expect(upgrade.lines).toHaveLength(2);

    const cappedRolls = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
    const capped = applyRankCube(
      "Legendary",
      "Legendary",
      () => cappedRolls.shift() ?? 0,
    );
    expect(capped.grade).toBe("Legendary");
    expect(capped.lines).toHaveLength(3);
  });

  it("rank cube upgrades Legendary to Transcendent only at low chance", () => {
    expect(RANK_CUBE_TRANSCENDENT_CHANCE).toBeGreaterThan(0);
    expect(RANK_CUBE_TRANSCENDENT_CHANCE).toBeLessThanOrEqual(0.08);
    expect(RANK_CUBE_TRANSCENDENT_CHANCE).toBeLessThan(
      RANK_CUBE_UPGRADE_CHANCE,
    );

    // 확률 미만 → 상승, 4줄
    const upRolls = [RANK_CUBE_TRANSCENDENT_CHANCE - 0.001];
    const upgraded = applyRankCube(
      "Legendary",
      "Transcendent",
      () => upRolls.shift() ?? 0,
    );
    expect(upgraded.grade).toBe("Transcendent");
    expect(upgraded.lines).toHaveLength(4);

    // 기존 확률(0.08)보다 높은 롤은 Legendary→Transcendent 상승 실패 (낮은 확률 게이트)
    const failRolls = [0.06, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
    const failed = applyRankCube(
      "Legendary",
      "Transcendent",
      () => failRolls.shift() ?? 0,
    );
    expect(failed.grade).toBe("Legendary");
    expect(failed.lines).toHaveLength(3);
  });
});
