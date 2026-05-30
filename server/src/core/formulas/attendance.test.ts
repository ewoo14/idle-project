import { describe, expect, it } from "vitest";
import {
  ATTENDANCE_MILESTONE_BASE,
  ATTENDANCE_MILESTONE_GROWTH,
  getAttendanceMilestoneReward,
  getAttendanceMilestoneThreshold,
  getReachedAttendanceMilestones,
} from "./index.js";

describe("attendance formula", () => {
  it("exposes the milestone curve constants for client parity", () => {
    expect(ATTENDANCE_MILESTONE_BASE).toBe(2);
    expect(ATTENDANCE_MILESTONE_GROWTH).toBeCloseTo(1.6, 5);
  });

  it("computes milestone thresholds with the client 1.6x floor curve", () => {
    // n=1 = BASE, 무한 기하: 2,3,5,8,13,20,33,53,...
    expect(getAttendanceMilestoneThreshold(1)).toBe(ATTENDANCE_MILESTONE_BASE);
    expect(getAttendanceMilestoneThreshold(1)).toBe(2);
    expect(getAttendanceMilestoneThreshold(2)).toBe(3);
    expect(getAttendanceMilestoneThreshold(3)).toBe(5);
    expect(getAttendanceMilestoneThreshold(4)).toBe(8);
    expect(getAttendanceMilestoneThreshold(5)).toBe(13);
    expect(getAttendanceMilestoneThreshold(6)).toBe(20);
    expect(getAttendanceMilestoneThreshold(7)).toBe(33);
    expect(getAttendanceMilestoneThreshold(8)).toBe(53);
  });

  it("treats non-positive milestone indices as zero threshold", () => {
    expect(getAttendanceMilestoneThreshold(0)).toBe(0);
    expect(getAttendanceMilestoneThreshold(-3)).toBe(0);
  });

  it("is monotonically increasing and positive for n>=1", () => {
    let previous = 0;
    for (let n = 1; n <= 30; n += 1) {
      const threshold = getAttendanceMilestoneThreshold(n);
      expect(threshold).toBeGreaterThan(0);
      expect(threshold).toBeGreaterThan(previous);
      previous = threshold;
    }
  });

  it("returns the highest reached milestone at exact boundaries", () => {
    expect(getReachedAttendanceMilestones(0)).toBe(0);
    // threshold(1)=2 → total 1(threshold-1) 미달, 2 도달.
    expect(getReachedAttendanceMilestones(1)).toBe(0);
    expect(getReachedAttendanceMilestones(2)).toBe(1);
    // threshold(2)=3 → total 2 = 마일스톤1, 3 = 마일스톤2.
    expect(getReachedAttendanceMilestones(3)).toBe(2);
    // threshold(3)=5, threshold(4)=8.
    expect(getReachedAttendanceMilestones(4)).toBe(2);
    expect(getReachedAttendanceMilestones(5)).toBe(3);
    expect(getReachedAttendanceMilestones(7)).toBe(3);
    expect(getReachedAttendanceMilestones(8)).toBe(4);
  });

  it("resolves multiple milestones for large totals (무한)", () => {
    // threshold(7)=33 ≤ 34 < threshold(8)=53.
    expect(getReachedAttendanceMilestones(34)).toBe(7);
    expect(getReachedAttendanceMilestones(1000)).toBe(14);
  });

  it("clamps negative/fractional totals before resolving milestones", () => {
    expect(getReachedAttendanceMilestones(-100)).toBe(0);
    expect(getReachedAttendanceMilestones(2.9)).toBe(1);
  });

  it("maps milestone rewards with a 3-cycle gold/essence/consumable rotation", () => {
    // gold 기하(BASE 10000 × 1.5^(n-1)), essence/consumable 비례.
    expect(getAttendanceMilestoneReward(1)).toEqual({
      type: "gold",
      value: 10000,
    });
    expect(getAttendanceMilestoneReward(2)).toEqual({
      type: "essence",
      value: 10,
    });
    expect(getAttendanceMilestoneReward(3)).toEqual({
      type: "consumable",
      value: 3,
    });
    expect(getAttendanceMilestoneReward(4)).toEqual({
      type: "gold",
      value: 33750,
    });
    expect(getAttendanceMilestoneReward(5)).toEqual({
      type: "essence",
      value: 25,
    });
    expect(getAttendanceMilestoneReward(6)).toEqual({
      type: "consumable",
      value: 6,
    });
    expect(getAttendanceMilestoneReward(7)).toEqual({
      type: "gold",
      value: 113906,
    });
  });

  it("returns an empty gold reward for non-positive milestone indices", () => {
    expect(getAttendanceMilestoneReward(0)).toEqual({ type: "gold", value: 0 });
    expect(getAttendanceMilestoneReward(-5)).toEqual({
      type: "gold",
      value: 0,
    });
  });
});
