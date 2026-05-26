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

    expect(report.json.model.enhancementPressure.maxLevel).toBe(5);
    expect(report.json.model.enhancementPressure.goldCostFloorToMax).toBe(5500);
    expect(
      report.json.model.enhancementPressure.expectedGoldCostToMax,
    ).toBeCloseTo(11020.66, 2);
    expect(
      report.json.model.enhancementPressure.rows.map((row) => ({
        currentLevel: row.currentLevel,
        cost: row.cost,
        successRate: row.successRate,
      })),
    ).toEqual([
      { currentLevel: 0, cost: 100, successRate: 0.95 },
      { currentLevel: 1, cost: 400, successRate: 0.85 },
      { currentLevel: 2, cost: 900, successRate: 0.7 },
      { currentLevel: 3, cost: 1600, successRate: 0.55 },
      { currentLevel: 4, cost: 2500, successRate: 0.4 },
    ]);
    expect(
      report.json.model.enhancementPressure.expectedHoursAtMedianGoldPerHour,
    ).toBeGreaterThan(0);
    expect(report.markdown).toContain("## Enhancement Spend Pressure");
    expect(report.markdown).toContain("Expected +0 to +5 gold cost");
    expect(report.markdown).toContain("11020.66");
    expect(report.markdown).toContain(
      "V1 enhancement is a light early sink, not a Lv50 progression blocker",
    );
  });
});
