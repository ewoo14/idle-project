import { describe, expect, it } from "vitest";
import {
  getEffectiveCooldown,
  getEffectiveDamageCoeff,
  MAX_SKILL_RANK,
} from "./skillRank.js";

describe("skill rank formulas", () => {
  it("matches the UE5 max skill rank", () => {
    expect(MAX_SKILL_RANK).toBe(50);
  });

  it.each([
    [0, 2.5],
    [5, 3.75],
    [50, 15],
  ])("computes effective damage coefficient at rank %i", (rank, expected) => {
    expect(getEffectiveDamageCoeff(2.5, rank)).toBe(expected);
  });

  it.each([
    [0, 4],
    [5, 3],
    [20, Math.fround(0.1)],
    [50, Math.fround(0.1)],
  ])("computes effective cooldown at rank %i", (rank, expected) => {
    expect(getEffectiveCooldown(4, rank)).toBe(expected);
  });

  it("keeps zero cooldown skills at zero", () => {
    expect(getEffectiveCooldown(0, 50)).toBe(0);
  });
});
