import { describe, expect, it } from "vitest";
import {
  BOSS_REWARD_BONUS,
  computeKillExp,
  computeKillGold,
  getMonsterLevelForStage,
} from "./reward.js";
import { computeRewardMultiplier } from "./stage.js";

describe("reward formulas", () => {
  it("keeps stage 0 kill rewards equal to base rewards", () => {
    expect(computeKillExp(12, 0, false)).toBe(12);
    expect(computeKillGold(7, 0, false)).toBe(7);
  });

  it("scales kill rewards with the shared reward multiplier", () => {
    expect(computeKillExp(12, 4, false)).toBe(
      Math.round(12 * computeRewardMultiplier(4)),
    );
    expect(computeKillGold(7, 4, false)).toBe(
      Math.round(7 * computeRewardMultiplier(4)),
    );
  });

  it("matches the client float rounding at reward half boundaries", () => {
    expect(computeKillGold(10, 1, false)).toBe(11);
    expect(computeKillGold(15, 2, false)).toBe(19);
    expect(computeKillGold(10, 1, true)).toBe(92);
  });

  it("keeps current monster kill reward baselines at stage 1-1", () => {
    expect(computeKillExp(12, 0, false)).toBe(12);
    expect(computeKillGold(10, 0, false)).toBe(10);
    expect(computeKillGold(15, 0, false)).toBe(15);
    expect(getMonsterLevelForStage(0)).toBe(1);
  });

  it("applies the client boss reward bonus after stage scaling", () => {
    expect(BOSS_REWARD_BONUS).toBe(8);
    expect(computeKillExp(12, 0, true)).toBe(96);
    expect(computeKillGold(7, 4, true)).toBe(
      Math.round(7 * computeRewardMultiplier(4) * BOSS_REWARD_BONUS),
    );
  });

  it("clamps negative stages and negative computed rewards", () => {
    expect(computeKillExp(12, -3, false)).toBe(12);
    expect(computeKillGold(-7, 4, false)).toBe(0);
  });

  it("mirrors the client monster level by global stage index", () => {
    expect(getMonsterLevelForStage(0)).toBe(1);
    expect(getMonsterLevelForStage(4)).toBe(5);
    expect(getMonsterLevelForStage(-2)).toBe(1);
  });
});
