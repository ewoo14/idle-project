import { describe, expect, it } from "vitest";
import {
  buildBalanceReport,
  evaluateBalance,
  type SimulationSample,
  simulateRebirthDistribution,
} from "../../tools/balance-sim/index.js";
import {
  computeKillExp,
  computeKillGold,
} from "../src/core/formulas/reward.js";

describe("balance simulator", () => {
  it("produces a deterministic 1000-run rebirth distribution", () => {
    const first = simulateRebirthDistribution({ runs: 1000, seed: 23 });
    const second = simulateRebirthDistribution({ runs: 1000, seed: 23 });

    expect(first.runs).toBe(1000);
    expect(first.samples).toHaveLength(1000);
    expect(first.summary).toEqual(second.summary);
    expect(first.summary.p10Hours).toBeGreaterThan(3);
    expect(first.summary.medianHours).toBeGreaterThan(5);
    expect(first.summary.p90Hours).toBeLessThan(20);
  });

  it("flags whether the median rebirth time is inside the 5-10h target", () => {
    const distribution = simulateRebirthDistribution({ runs: 1000, seed: 23 });
    const evaluation = evaluateBalance(distribution.summary);

    expect(evaluation.targetHours).toEqual({ min: 5, max: 10 });
    expect(evaluation.status).toBe("inside-target");
    expect(evaluation.recommendations).toEqual(
      expect.arrayContaining([
        expect.stringContaining("EXP curve"),
        expect.stringContaining("gold per hour"),
        expect.stringContaining("offline efficiency"),
      ]),
    );
  });

  it("builds markdown and JSON report artifacts with sensitivity notes", () => {
    const distribution = simulateRebirthDistribution({ runs: 1000, seed: 23 });
    const report = buildBalanceReport(distribution);

    expect(report.json.distribution.summary).toEqual(distribution.summary);
    expect(report.json.model.formulas).toContain(
      "server/src/core/formulas/enhance.ts",
    );
    expect(report.markdown).toContain("# Balance Simulator V1");
    expect(report.markdown).toContain("median");
    expect(report.markdown).toContain("Sensitivity");
    expect(report.markdown).toContain("EXP curve");
  });

  it("samples kill rewards through the shared stage reward formula", () => {
    const distribution = simulateRebirthDistribution({ runs: 1, seed: 23 });
    const sample = distribution.samples[0] as SimulationSample & {
      level50StageIndex?: number;
      normalKillExpAtLevel50?: number;
      normalKillGoldAtLevel50?: number;
      bossKillExpAtLevel50?: number;
      bossKillGoldAtLevel50?: number;
    };

    expect(sample.level50StageIndex).toBe(2);
    expect(sample.normalKillExpAtLevel50).toBe(
      computeKillExp(50 * 12, 2, false),
    );
    expect(sample.normalKillGoldAtLevel50).toBe(
      computeKillGold(50 * 8, 2, false),
    );
    expect(sample.bossKillExpAtLevel50).toBe(computeKillExp(50 * 12, 2, true));
    expect(sample.bossKillGoldAtLevel50).toBe(computeKillGold(50 * 8, 2, true));
  });

  it("documents reward scaling against monster HP scaling in the report", () => {
    const distribution = simulateRebirthDistribution({ runs: 1000, seed: 23 });
    const report = buildBalanceReport(distribution);

    expect(report.json.model.formulas).toContain(
      "server/src/core/formulas/reward.ts",
    );
    expect(report.markdown).toContain("## Reward Scaling");
    expect(report.markdown).toContain("1-5");
    expect(report.markdown).toContain("Boss bonus: 8x");
    expect(report.markdown).toContain(
      "| 1-1 | 0 | 1 | 1 | 12 | 10-15 | 96 | 80-120 |",
    );
  });

  it("reports enhancement spend pressure against sampled gold income", () => {
    const distribution = simulateRebirthDistribution({ runs: 1000, seed: 23 });
    const report = buildBalanceReport(distribution);

    expect(report.json.model.enhancementPressure.maxLevel).toBe(50);
    expect(report.json.model.enhancementPressure.goldCostFloorToMax).toBe(
      4292500,
    );
    expect(
      report.json.model.enhancementPressure.expectedGoldCostToMax,
    ).toBeCloseTo(22717602.46, 2);
    expect(
      report.json.model.enhancementPressure.rows.slice(0, 5).map((row) => ({
        currentLevel: row.currentLevel,
        cost: row.cost,
        successRate: Number(row.successRate.toFixed(3)),
      })),
    ).toEqual([
      { currentLevel: 0, cost: 100, successRate: 0.95 },
      { currentLevel: 1, cost: 400, successRate: 0.932 },
      { currentLevel: 2, cost: 900, successRate: 0.914 },
      { currentLevel: 3, cost: 1600, successRate: 0.896 },
      { currentLevel: 4, cost: 2500, successRate: 0.878 },
    ]);
    expect(report.json.model.enhancementPressure.rows.at(-1)).toEqual(
      expect.objectContaining({
        currentLevel: 49,
        nextLevel: 50,
        cost: 250000,
      }),
    );
    expect(
      report.json.model.enhancementPressure.expectedHoursAtMedianGoldPerHour,
    ).toBeGreaterThan(0);
    expect(report.markdown).toContain("## Enhancement Spend Pressure");
    expect(report.markdown).toContain("Expected +0 to +50 gold cost");
    expect(report.markdown).toContain(
      "V1 enhancement is a long-tail gold sink for infinite growth",
    );
  });

  it("reports rarity-scaled enhancement pressure and eight-slot high-rarity costs", () => {
    const distribution = simulateRebirthDistribution({ runs: 1000, seed: 23 });
    const report = buildBalanceReport(distribution);

    expect(
      report.json.model.enhancementPressure.rarityScenarios.map((row) => ({
        rarity: row.rarity,
        multiplier: row.multiplier,
        expectedGoldCostToMax: row.expectedGoldCostToMax,
        expectedHoursAtMedianGoldPerHour: row.expectedHoursAtMedianGoldPerHour,
      })),
    ).toEqual([
      {
        rarity: "Common",
        multiplier: 1,
        expectedGoldCostToMax: 22717602.46,
        expectedHoursAtMedianGoldPerHour: 34.7,
      },
      {
        rarity: "Rare",
        multiplier: 4,
        expectedGoldCostToMax: 90870409.85,
        expectedHoursAtMedianGoldPerHour: 138.799,
      },
      {
        rarity: "Epic",
        multiplier: 8,
        expectedGoldCostToMax: 181740819.7,
        expectedHoursAtMedianGoldPerHour: 277.599,
      },
      {
        rarity: "Legendary",
        multiplier: 16,
        expectedGoldCostToMax: 363481639.4,
        expectedHoursAtMedianGoldPerHour: 555.197,
      },
      {
        rarity: "Mythic",
        multiplier: 32,
        expectedGoldCostToMax: 726963278.8,
        expectedHoursAtMedianGoldPerHour: 1110.395,
      },
    ]);
    expect(
      report.json.model.enhancementPressure.legendaryEightSlotExpectedGoldCost,
    ).toBeCloseTo(2907853115.2, 2);
    expect(
      report.json.model.enhancementPressure
        .legendaryEightSlotExpectedHoursAtMedianGoldPerHour,
    ).toBe(4441.579);
    expect(
      report.json.model.enhancementPressure.mythicEightSlotExpectedGoldCost,
    ).toBeCloseTo(5815706230.4, 2);
    expect(
      report.json.model.enhancementPressure
        .mythicEightSlotExpectedHoursAtMedianGoldPerHour,
    ).toBe(8883.159);
    expect(report.markdown).toContain("## Rarity Enhancement Pressure");
    expect(report.markdown).toContain(
      "| Legendary | 16 | 68680000 | 363481639.4 | 555.197h |",
    );
    expect(report.markdown).toContain(
      "Eight Legendary slots: 2,907,853,115.20 expected gold",
    );
    expect(report.markdown).toContain(
      "4441.579h at sampled median Lv50 gold/hour",
    );
    expect(report.markdown).toContain(
      "| Mythic | 32 | 137360000 | 726963278.8 | 1110.395h |",
    );
    expect(report.markdown).toContain(
      "Eight Mythic slots: 5,815,706,230.40 expected gold",
    );
  });

  it("reports pet feed cost pressure and gold-bonus payback", () => {
    const distribution = simulateRebirthDistribution({ runs: 1000, seed: 23 });
    const report = buildBalanceReport(distribution);

    expect(report.json.model.petFeedPressure.maxLevel).toBe(10);
    expect(report.json.model.formulas).toContain(
      "server/src/core/formulas/petLevel.ts",
    );
    expect(report.json.model.petFeedPressure.totalFeedCostToMax).toBe(192500);
    expect(
      report.json.model.petFeedPressure.dogGoldBonusDeltaPercentAtMax,
    ).toBe(20);
    expect(
      report.json.model.petFeedPressure.paybackHoursAtMedianGoldPerHour,
    ).toBe(1.47);
    expect(report.json.model.petFeedPressure.rows.at(-1)).toEqual({
      currentLevel: 9,
      nextLevel: 10,
      feedCost: 50000,
      cumulativeFeedCost: 192500,
      dogGoldBonusPercentAfterFeed: 40,
      birdDropBonusPercentAfterFeed: 30,
    });
    expect(report.markdown).toContain("## Pet Feed Gold Pressure");
    expect(report.markdown).toContain("Total Lv0 to Lv10 feed cost: 192500");
    expect(report.markdown).toContain(
      "payback at median Lv50 gold/hour: 1.47h",
    );
  });
});
