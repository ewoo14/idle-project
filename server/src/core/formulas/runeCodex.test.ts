import { describe, expect, it } from "vitest";
import {
  computeRuneCodexBonus,
  getRowCompletionBonus,
  RUNE_CODEX_CORE_CELLS,
  RUNE_CODEX_TOTAL_CELLS,
  RUNE_CODEX_UTIL_CELLS,
} from "./runeCodex.js";

describe("rune codex formulas", () => {
  it("matches the UE codex grid dimensions", () => {
    expect(RUNE_CODEX_TOTAL_CELLS).toBe(54);
    expect(RUNE_CODEX_CORE_CELLS).toBe(30);
    expect(RUNE_CODEX_UTIL_CELLS).toBe(24);
  });

  it.each([
    [1, 0.01],
    [2, 0.02],
    [3, 0.03],
    [4, 0.05],
    [5, 0.08],
    [6, 0.12],
    [0, 0],
  ])("computes row completion bonus for rarity %i", (rarity, expected) => {
    expect(getRowCompletionBonus(rarity)).toBe(Math.fround(expected));
  });

  it("returns zero bonus for an empty codex", () => {
    expect(
      computeRuneCodexBonus({
        unlockedCells: 0,
        rowComplete: [false, false, false, false, false, false],
        coreCategoryComplete: false,
        utilCategoryComplete: false,
      }),
    ).toEqual({ coreStatAdd: 0, utilCapExtension: 0 });
  });

  it("computes full codex core and util cap bonuses with float parity", () => {
    const bonus = computeRuneCodexBonus({
      unlockedCells: 54,
      rowComplete: [true, true, true, true, true, true],
      coreCategoryComplete: true,
      utilCategoryComplete: true,
    });

    expect(bonus.coreStatAdd).toBe(
      Math.fround(54 * 0.004 + 0.01 + 0.02 + 0.03 + 0.05 + 0.08 + 0.12 + 0.05),
    );
    expect(bonus.utilCapExtension).toBe(Math.fround(0.1));
  });
});
