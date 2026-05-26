import { describe, expect, it } from "vitest";
import {
  clampSkillRank,
  getEffectiveCooldown,
  getEffectiveDamageCoeff,
  MAX_SKILL_RANK,
} from "./skillRank.js";

describe("skill rank formulas", () => {
  it("matches the UE5 max skill rank", () => {
    expect(MAX_SKILL_RANK).toBe(50);
  });

  it.each([
    [-1, 0],
    [0, 0],
    [5.9, 5],
    [20, 20],
    [50, 50],
    [999, 50],
  ])("clamps rank %f to the UE5 integer rank range", (rank, expected) => {
    expect(clampSkillRank(rank)).toBe(expected);
  });

  it.each([
    [0, 2.5],
    [5, 3.75],
    [20, 7.5],
    [50, 15],
  ])("computes effective damage coefficient at rank %i", (rank, expected) => {
    expect(getEffectiveDamageCoeff(2.5, rank)).toBe(expected);
  });

  it.each([
    [5, 3.6000001430511475],
    [20, 7.200000286102295],
    [50, 14.40000057220459],
  ])("matches UE5 float parity for mage base coefficient at rank %i", (rank, expected) => {
    expect(getEffectiveDamageCoeff(2.4, rank)).toBe(expected);
  });

  it.each([
    [0, 4],
    [5, 3],
    [19, 0.20000004768371582],
    [20, Math.fround(0.1)],
    [50, Math.fround(0.1)],
  ])("computes effective cooldown at rank %i", (rank, expected) => {
    expect(getEffectiveCooldown(4, rank)).toBe(expected);
  });

  it("keeps zero cooldown skills at zero", () => {
    expect(getEffectiveCooldown(0, 50)).toBe(0);
  });
});
