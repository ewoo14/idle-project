import { describe, expect, it } from "vitest";
import { getRebirthPointsReward } from "./index.js";

describe("rebirth formulas", () => {
  it.each([
    [0, 100, 5],
    [4, 100, 13],
    [0, 150, 10],
    [4, 150, 18],
    [0, 109, 5],
    [0, 110, 6],
  ])("matches the client reward formula for rebirthCount=%i and levelAtRebirth=%i", (rebirthCount, levelAtRebirth, expected) => {
    expect(getRebirthPointsReward(rebirthCount, levelAtRebirth)).toBe(expected);
  });

  it("clamps negative inputs to the client formula floors", () => {
    expect(getRebirthPointsReward(-3, -20)).toBe(5);
  });
});
