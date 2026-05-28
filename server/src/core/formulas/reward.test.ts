import { describe, expect, it } from "vitest";
import {
  BOSS_REWARD_BONUS,
  computeKillExp,
  computeKillGold,
  ELITE_REWARD_BONUS,
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
    expect(computeKillGold(10, 1, false)).toBe(10);
    expect(computeKillGold(15, 2, false)).toBe(17);
    expect(computeKillGold(10, 1, true)).toBe(80);
  });

  it("keeps current monster kill reward baselines at stage 1-1", () => {
    expect(computeKillExp(12, 1, false)).toBe(12);
    expect(computeKillGold(10, 1, false)).toBe(10);
    expect(computeKillGold(15, 1, false)).toBe(15);
    expect(getMonsterLevelForStage(1)).toBe(1);
  });

  it("applies the client boss reward bonus after stage scaling", () => {
    expect(BOSS_REWARD_BONUS).toBe(8);
    expect(computeKillExp(12, 1, true)).toBe(96);
    expect(computeKillGold(7, 10, true)).toBe(
      Math.round(7 * computeRewardMultiplier(10) * BOSS_REWARD_BONUS),
    );
  });

  it("applies the client elite reward bonus below boss rewards", () => {
    expect(ELITE_REWARD_BONUS).toBe(3);
    expect(computeKillGold(10, 5, false, true)).toBe(
      Math.round(10 * computeRewardMultiplier(5) * ELITE_REWARD_BONUS),
    );
    expect(computeKillGold(10, 5, true, true)).toBe(
      Math.round(10 * computeRewardMultiplier(5) * BOSS_REWARD_BONUS),
    );
  });

  it("clamps negative stages and negative computed rewards", () => {
    expect(computeKillExp(12, -3, false)).toBe(12);
    expect(computeKillGold(-7, 4, false)).toBe(0);
  });

  it("mirrors the client monster level by global stage index", () => {
    expect(getMonsterLevelForStage(1)).toBe(1);
    expect(getMonsterLevelForStage(10)).toBe(10);
    expect(getMonsterLevelForStage(-2)).toBe(1);
  });
});
