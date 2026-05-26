import { describe, expect, it } from "vitest";
import {
  canTranscend,
  getTranscendStatMultiplier,
  TRANSCEND_REBIRTH_THRESHOLD,
} from "./index.js";

describe("transcend formulas", () => {
  it("mirrors the client rebirth threshold", () => {
    expect(TRANSCEND_REBIRTH_THRESHOLD).toBe(5);
  });

  it.each([
    [0, 1],
    [1, 1.25],
    [4, 2],
    [10, 3.5],
  ])("matches FTranscendFormula stat multiplier for count=%i", (count, expected) => {
    expect(getTranscendStatMultiplier(count)).toBe(expected);
  });

  it("clamps negative transcend counts like the client formula", () => {
    expect(getTranscendStatMultiplier(-1)).toBe(1);
  });

  it.each([
    [4, false],
    [5, true],
    [6, true],
  ])("matches FTranscendFormula eligibility for rebirthCount=%i", (rebirthCount, expected) => {
    expect(canTranscend(rebirthCount)).toBe(expected);
  });
});
