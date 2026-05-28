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
    expect(report.json.model.formulas).toContain(
      "server/src/core/formulas/drop.ts",
    );
    expect(report.json.model.formulas).toContain(
      "server/src/core/formulas/achievement.ts",
    );
    expect(report.markdown).toContain("# Balance Simulator V1");
    expect(report.markdown).toContain("median");
    expect(report.markdown).toContain("Sensitivity");
    expect(report.markdown).toContain("EXP curve");
  });

  it("reports seven-rarity item drop pressure through the shared drop formula", () => {
    const distribution = simulateRebirthDistribution({ runs: 1000, seed: 23 });
    const report = buildBalanceReport(distribution);

    expect(report.json.model.dropRarityPressure.rows).toHaveLength(7);
    expect(report.json.model.dropRarityPressure.totalChanceAtLevel100).toBe(1);
    expect(
      report.json.model.dropRarityPressure.rows.map((row) => row.rarity),
    ).toEqual([
      "Common",
      "Rare",
      "Epic",
      "Unique",
      "Legendary",
      "Transcendent",
      "Mythic",
    ]);
    expect(
      report.json.model.dropRarityPressure.rows.find(
        (row) => row.rarity === "Unique",
      )?.chanceAtLevel100Percent,
    ).toBeLessThan(
      report.json.model.dropRarityPressure.rows.find(
        (row) => row.rarity === "Epic",
      )?.chanceAtLevel100Percent ?? 0,
    );
    expect(
      report.json.model.dropRarityPressure.rows.find(
        (row) => row.rarity === "Transcendent",
      )?.chanceAtLevel100Percent,
    ).toBeLessThan(
      report.json.model.dropRarityPressure.rows.find(
        (row) => row.rarity === "Legendary",
      )?.chanceAtLevel100Percent ?? 0,
    );
    expect(report.markdown).toContain("## Item Drop Rarity Pressure");
    expect(report.markdown).toContain("Level 100 total probability: 100%");
    expect(report.markdown).toContain("| Unique | 2.75 | 0% | 2.5% | 2 |");
    expect(report.markdown).toContain(
      "| Transcendent | 3.85 | 0% | 0.7% | 2-3 |",
    );
  });

  it("reports achievement soft-cap pressure against transcend and tower multipliers", () => {
    const distribution = simulateRebirthDistribution({ runs: 1000, seed: 23 });
    const report = buildBalanceReport(distribution);

    expect(report.json.model.achievementPressure.softCapStartPoints).toBe(100);
    expect(report.json.model.achievementPressure.softCapBonusPoints).toBe(50);
    expect(report.json.model.achievementPressure.rows).toEqual([
      {
        totalPoints: 0,
        legacyMultiplier: 1,
        softCappedMultiplier: 1,
        compositeWithTranscendAndTower: 4.2,
      },
      {
        totalPoints: 3,
        legacyMultiplier: 1.03,
        softCappedMultiplier: 1.03,
        compositeWithTranscendAndTower: 4.326,
      },
      {
        totalPoints: 100,
        legacyMultiplier: 2,
        softCappedMultiplier: 2,
        compositeWithTranscendAndTower: 8.4,
      },
      {
        totalPoints: 125,
        legacyMultiplier: 2.25,
        softCappedMultiplier: 2.197,
        compositeWithTranscendAndTower: 9.226,
      },
      {
        totalPoints: 250,
        legacyMultiplier: 3.5,
        softCappedMultiplier: 2.475,
        compositeWithTranscendAndTower: 10.395,
      },
      {
        totalPoints: 500,
        legacyMultiplier: 6,
        softCappedMultiplier: 2.5,
        compositeWithTranscendAndTower: 10.499,
      },
    ]);
    expect(report.markdown).toContain("## Achievement Multiplier Pressure");
    expect(report.markdown).toContain(
      "100 points stays at x2 before the soft-cap slope decays",
    );
    expect(report.markdown).toContain("| 250 | 3.5 | 2.475 | 10.395 |");
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

    expect(sample.level50StageIndex).toBe(3);
    expect(sample.normalKillExpAtLevel50).toBe(
      computeKillExp(50 * 12, 3, false),
    );
    expect(sample.normalKillGoldAtLevel50).toBe(
      computeKillGold(50 * 8, 3, false),
    );
    expect(sample.bossKillExpAtLevel50).toBe(computeKillExp(50 * 12, 3, true));
    expect(sample.bossKillGoldAtLevel50).toBe(computeKillGold(50 * 8, 3, true));
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
      "| 1-1 | 1 | normal | Fire | 1 | 1 | 12 | 10-15 | 36 | 30-45 | 96 | 80-120 |",
    );
  });

  it("documents the PR #66 30-stage chapter expansion with elite and Dark pressure", () => {
    const distribution = simulateRebirthDistribution({ runs: 1000, seed: 23 });
    const report = buildBalanceReport(distribution);

    expect(report.json.model.rewardScaling).toHaveLength(30);
    expect(report.json.model.rewardScaling[0]).toEqual(
      expect.objectContaining({
        stage: "1-1",
        globalStageIndex: 1,
        encounterType: "normal",
        weakElement: "Fire",
      }),
    );
    expect(report.json.model.rewardScaling[4]).toEqual(
      expect.objectContaining({
        stage: "1-5",
        globalStageIndex: 5,
        encounterType: "elite",
        weakElement: "Dark",
        eliteExp: 58,
      }),
    );
    expect(report.json.model.rewardScaling[9]).toEqual(
      expect.objectContaining({
        stage: "1-10",
        globalStageIndex: 10,
        encounterType: "boss",
      }),
    );
    expect(report.json.model.rewardScaling[20]).toEqual(
      expect.objectContaining({
        stage: "3-1",
        globalStageIndex: 21,
        weakElement: "Dark",
      }),
    );
    expect(report.json.model.rewardScaling.at(-1)).toEqual(
      expect.objectContaining({
        stage: "3-10",
        globalStageIndex: 30,
        encounterType: "boss",
        weakElement: "Holy",
      }),
    );
    expect(report.markdown).toContain("30-stage Chapter 1-3 comparison");
    expect(report.markdown).toContain("Elite bonus: 3x");
    expect(report.json.model.darkElementPressure.darkWeakStageCount).toBe(9);
    expect(report.json.model.darkElementPressure.rows).toHaveLength(4);
    expect(report.json.model.darkElementPressure.rows).toContainEqual({
      skillElement: "Holy",
      targetWeakElement: "Dark",
      multiplier: 1.5,
      note: "Holy counter into Dark-heavy stages",
    });
    expect(report.markdown).toContain("Dark stage share: 9/30");
    expect(report.markdown).toContain("| 3-10 | 30 | boss | Holy |");
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
        multiplier: 2,
        expectedGoldCostToMax: 45435204.93,
        expectedHoursAtMedianGoldPerHour: 69.4,
      },
      {
        rarity: "Epic",
        multiplier: 4,
        expectedGoldCostToMax: 90870409.85,
        expectedHoursAtMedianGoldPerHour: 138.799,
      },
      {
        rarity: "Unique",
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
        rarity: "Transcendent",
        multiplier: 32,
        expectedGoldCostToMax: 726963278.8,
        expectedHoursAtMedianGoldPerHour: 1110.395,
      },
      {
        rarity: "Mythic",
        multiplier: 64,
        expectedGoldCostToMax: 1453926557.61,
        expectedHoursAtMedianGoldPerHour: 2220.79,
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
    ).toBeCloseTo(11631412460.88, 2);
    expect(
      report.json.model.enhancementPressure
        .mythicEightSlotExpectedHoursAtMedianGoldPerHour,
    ).toBe(17766.317);
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
      "| Mythic | 64 | 274720000 | 1453926557.61 | 2220.79h |",
    );
    expect(report.markdown).toContain(
      "Eight Mythic slots: 11,631,412,460.88 expected gold",
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

  it("reports rune growth pressure against CP, DPS, and economy caps", () => {
    const distribution = simulateRebirthDistribution({ runs: 1000, seed: 23 });
    const report = buildBalanceReport(distribution);

    expect(report.json.model.formulas).toContain(
      "server/src/core/formulas/rune.ts",
    );
    expect(report.json.model.runePressure.slotCount).toBe(6);
    expect(report.json.model.runePressure.coreRows).toContainEqual({
      rarity: "Mythic",
      enhanceLevel: 50,
      singleRuneBonusPercent: 168,
      sixSlotMultiplier: 11.08,
    });
    expect(report.json.model.runePressure.coreRows).toContainEqual({
      rarity: "Mythic",
      enhanceLevel: 100,
      singleRuneBonusPercent: 318,
      sixSlotMultiplier: 20.08,
    });
    expect(
      report.json.model.runePressure.combatRows.find(
        (row) => row.runeSet === "6x Mythic +50 PhysAtk",
      ),
    ).toEqual(
      expect.objectContaining({
        className: "Warrior",
        level: 100,
        cpMultiplier: expect.any(Number),
        dpsMultiplier: expect.any(Number),
      }),
    );
    expect(
      report.json.model.runePressure.combatRows.find(
        (row) => row.runeSet === "6x Mythic +50 PhysAtk",
      )?.dpsMultiplier,
    ).toBeGreaterThan(5);
    expect(report.json.model.runePressure.utilRows).toContainEqual({
      runeType: "GoldFind",
      rarity: "Mythic",
      enhanceLevel: 377,
      singleRuneValuePercent: 200,
      sixSlotUncappedTotalPercent: 1200,
      effectiveEconomicMultiplier: 3,
    });
    expect(report.json.model.runePressure.utilRows).toContainEqual({
      runeType: "OfflineEff",
      rarity: "Mythic",
      enhanceLevel: 127,
      singleRuneValuePercent: 50,
      sixSlotUncappedTotalPercent: 300,
      effectiveEconomicMultiplier: 1.5,
    });
    expect(report.markdown).toContain("## Rune Growth Pressure");
    expect(report.markdown).toContain("6x Mythic +50 PhysAtk");
    expect(report.markdown).toContain("GoldFind");
    expect(report.markdown).toContain(
      "Core rune growth is intentionally uncapped",
    );
  });

  it("reports class mastery rune pressure for all eight classes", () => {
    const distribution = simulateRebirthDistribution({ runs: 1000, seed: 23 });
    const report = buildBalanceReport(distribution);

    expect(report.json.model.formulas).toContain(
      "server/src/core/formulas/classRune.ts",
    );
    expect(report.json.model.runePressure.classMasteryRows).toHaveLength(8);
    expect(
      report.json.model.runePressure.classMasteryRows.map((row) => ({
        className: row.className,
        masteryStats: row.masteryStats,
      })),
    ).toEqual([
      { className: "Warrior", masteryStats: ["PhysAtk", "PhysDef"] },
      { className: "Mage", masteryStats: ["MagicAtk"] },
      { className: "Archer", masteryStats: ["PhysAtk"] },
      { className: "Thief", masteryStats: ["PhysAtk"] },
      { className: "Cleric", masteryStats: ["MagicAtk", "Hp"] },
      { className: "Paladin", masteryStats: ["PhysDef", "Hp"] },
      { className: "Berserker", masteryStats: ["PhysAtk"] },
      { className: "Summoner", masteryStats: ["MagicAtk"] },
    ]);
    expect(report.markdown).toContain("## Class Mastery Rune Pressure");
    expect(report.markdown).toContain("| Warrior | dps | PhysAtk, PhysDef |");
  });

  it("reports rune set tier pressure without injecting it into the sampled rebirth run", () => {
    const distribution = simulateRebirthDistribution({ runs: 1000, seed: 23 });
    const report = buildBalanceReport(distribution);

    expect(report.json.model.formulas).toContain(
      "server/src/core/formulas/runeSet.ts",
    );
    expect(report.json.model.runePressure.setRows).toHaveLength(12);
    expect(
      report.json.model.runePressure.setRows.map((row) => ({
        runeSet: row.runeSet,
        count: row.count,
        tierBonusPercent: row.tierBonusPercent,
        firstRebirthInjected: row.firstRebirthInjected,
      })),
    ).toEqual([
      {
        runeSet: "Offense",
        count: 2,
        tierBonusPercent: 5,
        firstRebirthInjected: false,
      },
      {
        runeSet: "Offense",
        count: 4,
        tierBonusPercent: 12,
        firstRebirthInjected: false,
      },
      {
        runeSet: "Offense",
        count: 6,
        tierBonusPercent: 25,
        firstRebirthInjected: false,
      },
      {
        runeSet: "Bastion",
        count: 2,
        tierBonusPercent: 5,
        firstRebirthInjected: false,
      },
      {
        runeSet: "Bastion",
        count: 4,
        tierBonusPercent: 12,
        firstRebirthInjected: false,
      },
      {
        runeSet: "Bastion",
        count: 6,
        tierBonusPercent: 25,
        firstRebirthInjected: false,
      },
      {
        runeSet: "Vitality",
        count: 2,
        tierBonusPercent: 5,
        firstRebirthInjected: false,
      },
      {
        runeSet: "Vitality",
        count: 4,
        tierBonusPercent: 12,
        firstRebirthInjected: false,
      },
      {
        runeSet: "Vitality",
        count: 6,
        tierBonusPercent: 25,
        firstRebirthInjected: false,
      },
      {
        runeSet: "Fortune",
        count: 2,
        tierBonusPercent: 5,
        firstRebirthInjected: false,
      },
      {
        runeSet: "Fortune",
        count: 4,
        tierBonusPercent: 12,
        firstRebirthInjected: false,
      },
      {
        runeSet: "Fortune",
        count: 6,
        tierBonusPercent: 25,
        firstRebirthInjected: false,
      },
    ]);
    expect(
      report.json.model.runePressure.setRows.find(
        (row) => row.runeSet === "Offense" && row.count === 6,
      ),
    ).toEqual(
      expect.objectContaining({
        bonusLanes: ["PhysAtk", "MagicAtk"],
        className: "Warrior",
        level: 100,
        cpMultiplier: expect.any(Number),
        dpsMultiplier: expect.any(Number),
      }),
    );
    expect(
      report.json.model.runePressure.setRows.find(
        (row) => row.runeSet === "Offense" && row.count === 6,
      )?.dpsMultiplier,
    ).toBeGreaterThan(1);
    expect(report.json.distribution.summary.medianHours).toBe(5.328);
    expect(report.markdown).toContain("## Rune Set Bonus Pressure");
    expect(report.markdown).toContain(
      "| Offense | 6 | 25% | PhysAtk, MagicAtk |",
    );
    expect(report.markdown).toContain(
      "not injected into the sampled first-rebirth run",
    );
  });

  it("keeps class mastery DPS rows inside the PR #60 damage-role band", () => {
    const distribution = simulateRebirthDistribution({ runs: 1000, seed: 23 });
    const report = buildBalanceReport(distribution);
    const dpsRows = report.json.model.runePressure.classMasteryRows.filter(
      (row) => row.role === "dps",
    );
    const median = dpsRows
      .map((row) => row.classRuneDps)
      .sort((left, right) => left - right)[Math.floor(dpsRows.length / 2)];

    for (const row of dpsRows) {
      expect(row.classRuneDps).toBeGreaterThanOrEqual(median * 0.85);
      expect(row.classRuneDps).toBeLessThanOrEqual(median * 1.15);
    }
  });

  it("shows two-stat class mastery rows as CP compensation rather than extra DPS pressure", () => {
    const distribution = simulateRebirthDistribution({ runs: 1000, seed: 23 });
    const report = buildBalanceReport(distribution);
    const rowsByName = new Map(
      report.json.model.runePressure.classMasteryRows.map((row) => [
        row.className,
        row,
      ]),
    );

    expect(rowsByName.get("Warrior")?.cpMultiplier).toBeGreaterThan(
      rowsByName.get("Berserker")?.cpMultiplier ?? 0,
    );
    expect(rowsByName.get("Cleric")?.cpMultiplier).toBeGreaterThan(
      rowsByName.get("Mage")?.cpMultiplier ?? 0,
    );
    expect(rowsByName.get("Paladin")?.dpsMultiplier).toBe(1);
    expect(rowsByName.get("Paladin")?.cpMultiplier).toBeGreaterThan(1);
  });

  it("reports rune codex collection pressure without injecting it into the sampled rebirth run", () => {
    const distribution = simulateRebirthDistribution({ runs: 1000, seed: 23 });
    const report = buildBalanceReport(distribution);

    expect(report.json.model.formulas).toContain(
      "server/src/core/formulas/runeCodex.ts",
    );
    expect(report.json.model.runeCodexPressure).toEqual({
      totalCells: 63,
      perCellCoreBonusPercent: 0.4,
      allCellsCoreBonusPercent: 25.2,
      allRowsCoreBonusPercent: 41,
      coreCategoryBonusPercent: 5,
      utilCategoryCapExtensionPercent: 10,
      fullCodexCoreStatAddPercent: 71.2,
      baseMedianRebirthHours: 5.328,
      projectedFullCodexMedianHours: 3.112,
      projectedMedianDeltaPercent: -41.6,
      injectedIntoSampledRun: false,
    });
    expect(report.markdown).toContain("## Rune Codex Collection Pressure");
    expect(report.markdown).toContain("Full codex core bonus: +71.2%");
    expect(report.markdown).toContain(
      "Not injected into the sampled first-rebirth run",
    );
  });
});
