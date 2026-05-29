import { describe, expect, it } from "vitest";
import {
  getChallengeDamage,
  getReachedMilestones,
  milestoneEssenceReward,
  milestoneGoldReward,
  milestoneThreshold,
  WEEKLY_CHALLENGE_LIMIT,
} from "./index.js";

describe("weekly boss formula", () => {
  it("uses the V1 weekly challenge limit", () => {
    expect(WEEKLY_CHALLENGE_LIMIT).toBe(7);
  });

  it("clamps challenge damage to non-negative truncated combat power", () => {
    expect(getChallengeDamage(1234.9)).toBe(1234);
    expect(getChallengeDamage(0.9)).toBe(0);
    expect(getChallengeDamage(-50)).toBe(0);
  });

  it("computes milestone thresholds with the client 1.5x floor curve", () => {
    expect(milestoneThreshold(1)).toBe(1000);
    expect(milestoneThreshold(2)).toBe(1500);
    expect(milestoneThreshold(3)).toBe(2250);
    expect(milestoneThreshold(4)).toBe(3375);
    expect(milestoneThreshold(5)).toBe(5062);
    expect(milestoneThreshold(0)).toBe(0);
  });

  it("returns the highest reached milestone at exact boundaries", () => {
    expect(getReachedMilestones(999)).toBe(0);
    expect(getReachedMilestones(1000)).toBe(1);
    expect(getReachedMilestones(1499)).toBe(1);
    expect(getReachedMilestones(1500)).toBe(2);
    expect(getReachedMilestones(5061)).toBe(4);
    expect(getReachedMilestones(5062)).toBe(5);
  });

  it("computes milestone rewards with gold growth and linear essence", () => {
    expect(milestoneGoldReward(1)).toBe(5000);
    expect(milestoneGoldReward(2)).toBe(7500);
    expect(milestoneGoldReward(3)).toBe(11250);
    expect(milestoneGoldReward(5)).toBe(25312);
    expect(milestoneGoldReward(0)).toBe(0);

    expect(milestoneEssenceReward(1)).toBe(3);
    expect(milestoneEssenceReward(5)).toBe(15);
    expect(milestoneEssenceReward(0)).toBe(0);
  });
});
