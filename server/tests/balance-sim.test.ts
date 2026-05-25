import { describe, expect, it } from "vitest";
import {
  buildBalanceReport,
  evaluateBalance,
  simulateRebirthDistribution,
} from "../../tools/balance-sim/index.js";

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
    expect(report.markdown).toContain("# Balance Simulator V1");
    expect(report.markdown).toContain("median");
    expect(report.markdown).toContain("Sensitivity");
    expect(report.markdown).toContain("EXP curve");
  });
});
