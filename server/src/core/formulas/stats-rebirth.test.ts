import { describe, expect, it } from "vitest";
import { defaultPrimaryStats, deriveStats } from "./stats.js";

describe("rebirth stat formula", () => {
  it("adds permanent rebirth bonus points to core combat stats", () => {
    const primary = defaultPrimaryStats(1, 1);
    const base = deriveStats(primary, 1);
    const rebirthed = deriveStats(primary, 1, {}, 5);

    expect(rebirthed.physAtk).toBe(base.physAtk + 10);
    expect(rebirthed.hp).toBe(base.hp + 50);
    expect(rebirthed.atkSpeed).toBe(base.atkSpeed);
  });
});
