import { readFileSync } from "node:fs";
import { resolve } from "node:path";
import { describe, expect, it } from "vitest";
import { defaultOwnedPetIds, petDefinitions } from "./pets.js";
import { currentSeasonId, seasonTiers } from "./season.js";

describe("pet and season V1 contracts", () => {
  it("keeps V1 pet definitions and docs aligned", () => {
    expect(petDefinitions).toEqual([
      {
        petId: "dog",
        name: "강아지",
        bonusType: "gold",
        bonusPercent: 20,
      },
      {
        petId: "bird",
        name: "새",
        bonusType: "drop",
        bonusPercent: 15,
      },
    ]);
    expect(defaultOwnedPetIds).toEqual(["dog", "bird"]);

    const schemaDoc = readFileSync(
      resolve(process.cwd(), "../docs/api/schema.md"),
      "utf8",
    );
    expect(schemaDoc).toContain("| `dog` | `gold +20%` |");
    expect(schemaDoc).toContain("| `bird` | `drop +15%` |");
  });

  it("keeps the season pass free-track tier ladder fixed for V1", () => {
    expect(currentSeasonId).toBe(1);
    expect(seasonTiers).toEqual([
      { tier: 1, requiredTokens: 10, rewardType: "gold", rewardAmount: 500 },
      { tier: 2, requiredTokens: 25, rewardType: "gold", rewardAmount: 1_000 },
      { tier: 3, requiredTokens: 45, rewardType: "exp", rewardAmount: 300 },
      { tier: 4, requiredTokens: 70, rewardType: "gold", rewardAmount: 1_800 },
      { tier: 5, requiredTokens: 100, rewardType: "exp", rewardAmount: 650 },
      { tier: 6, requiredTokens: 135, rewardType: "gold", rewardAmount: 3_000 },
      { tier: 7, requiredTokens: 175, rewardType: "exp", rewardAmount: 1_100 },
      { tier: 8, requiredTokens: 220, rewardType: "gold", rewardAmount: 4_800 },
      { tier: 9, requiredTokens: 270, rewardType: "exp", rewardAmount: 1_750 },
      {
        tier: 10,
        requiredTokens: 325,
        rewardType: "gold",
        rewardAmount: 7_500,
      },
    ]);
  });
});
