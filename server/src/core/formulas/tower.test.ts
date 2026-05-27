import { describe, expect, it } from "vitest";
import {
  canClearFloor,
  getFloorRequiredPower,
  getFloorReward,
} from "./tower.js";

describe("tower formula", () => {
  it.each([
    [1, 100],
    [2, 115],
    [10, 352],
    [50, 94231],
  ])("matches the client required combat power anchor for floor %i", (floor, expectedRequiredPower) => {
    expect(getFloorRequiredPower(floor)).toBe(expectedRequiredPower);
  });

  it("clamps floors below one before calculating required combat power", () => {
    expect(getFloorRequiredPower(0)).toBe(100);
    expect(getFloorRequiredPower(-10)).toBe(100);
  });

  it("rounds required combat power with double precision parity anchors", () => {
    const floor = 10;
    const requiredPower = getFloorRequiredPower(floor);

    expect(requiredPower).toBe(352);
    expect(canClearFloor(requiredPower - 1, floor)).toBe(false);
    expect(canClearFloor(requiredPower, floor)).toBe(true);
  });

  it("clamps non-finite and oversized required combat power to int64 max", () => {
    expect(getFloorRequiredPower(500)).toBe(9223372036854776000);
  });

  it.each([
    [1, 50],
    [2, 100],
    [10, 500],
    [50, 2500],
  ])("matches the client reward anchor for floor %i", (floor, expectedReward) => {
    expect(getFloorReward(floor)).toBe(expectedReward);
  });

  it("clamps floors below one before calculating rewards", () => {
    expect(getFloorReward(0)).toBe(50);
    expect(getFloorReward(-10)).toBe(50);
  });
});
