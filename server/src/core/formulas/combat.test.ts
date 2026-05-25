import { describe, expect, it } from "vitest";
import { computeDamage } from "./combat.js";

describe("combat formulas", () => {
  it.each([
    [100, 20, 88],
    [10, 100, 0.5],
    [50, 0, 50],
    [0, 0, 0],
    [1000, 500, 700],
  ])("ATK %i DEF %i 피해량을 계산한다", (atk, def, expected) => {
    expect(computeDamage(atk, def)).toBe(expected);
  });
});
