import { readFileSync } from "node:fs";
import { resolve } from "node:path";
import { describe, expect, it } from "vitest";
import {
  buildClassBalanceSnapshot,
  type ClassBalanceRow,
} from "../../tools/balance-sim/index.js";
import {
  berserkerSkillDefinitions,
  summonerSkillDefinitions,
} from "../src/core/data/skills.js";
import { defaultPrimaryStats } from "../src/core/formulas/stats.js";

const dpsClassIds = new Set([1, 2, 3, 4, 7, 8]);
const tunedClassIds = new Set([7, 8]);

function assertWithinBand(rows: ClassBalanceRow[], tolerance: number) {
  const dpsRows = rows.filter((row) => dpsClassIds.has(row.classId));
  const median = dpsRows.map((row) => row.effectiveDps).sort((a, b) => a - b)[
    Math.floor(dpsRows.length / 2)
  ];

  for (const row of dpsRows) {
    expect(row.dpsDeltaFromMedian).toBeGreaterThanOrEqual(-tolerance);
    expect(row.dpsDeltaFromMedian).toBeLessThanOrEqual(tolerance);
    expect(row.effectiveDps).toBeGreaterThan(0);
    expect(row.effectiveDps).toBeGreaterThanOrEqual(median * (1 - tolerance));
    expect(row.effectiveDps).toBeLessThanOrEqual(median * (1 + tolerance));
  }
}

describe("class balance snapshot", () => {
  it("evaluates all eight classes at the review anchor levels", () => {
    const snapshot = buildClassBalanceSnapshot([50, 100]);

    expect(snapshot.levels).toEqual([50, 100]);
    for (const level of snapshot.levels) {
      expect(snapshot.rowsByLevel[level]).toHaveLength(8);
      expect(snapshot.rowsByLevel[level].map((row) => row.classId)).toEqual([
        1, 2, 3, 4, 5, 6, 7, 8,
      ]);
    }
  });

  it("keeps DPS classes inside the plus/minus 15 percent band", () => {
    const snapshot = buildClassBalanceSnapshot([50, 100]);

    assertWithinBand(snapshot.rowsByLevel[50], 0.15);
    assertWithinBand(snapshot.rowsByLevel[100], 0.15);
  });

  it("keeps tank and healer below the DPS band while preserving their roles", () => {
    const snapshot = buildClassBalanceSnapshot([100]);
    const rows = snapshot.rowsByLevel[100];
    const dpsMedian = rows
      .filter((row) => dpsClassIds.has(row.classId))
      .map((row) => row.effectiveDps)
      .sort((a, b) => a - b)[3];
    const paladin = rows.find((row) => row.classId === 6);
    const cleric = rows.find((row) => row.classId === 5);

    expect(paladin?.role).toBe("tank");
    expect(cleric?.role).toBe("healer");
    expect(paladin?.effectiveDps).toBeLessThan(dpsMedian * 0.8);
    expect(cleric?.effectiveDps).toBeLessThan(dpsMedian * 0.8);
    expect(paladin?.hp).toBeGreaterThan(
      rows.find((row) => row.classId === 1)?.hp ?? 0,
    );
    expect(cleric?.magicDef).toBeGreaterThan(
      rows.find((row) => row.classId === 2)?.magicDef ?? 0,
    );
  });
});

describe("class balance formula anchors", () => {
  it("removes strict better growth from berserker and summoner", () => {
    expect(defaultPrimaryStats(7, 50)).toMatchObject({
      str: 112,
      con: 65.8,
      luk: 65.8,
    });
    expect(defaultPrimaryStats(8, 50)).toMatchObject({
      int: 108.6,
      wis: 99.2,
      con: 55,
    });
  });

  it("mirrors tuned berserker and summoner skill coefficients", () => {
    expect(
      berserkerSkillDefinitions.map(({ skillId, cooldown, damageCoeff }) => ({
        skillId,
        cooldown,
        damageCoeff,
      })),
    ).toContainEqual({
      skillId: "rage_cleave",
      cooldown: 3.5,
      damageCoeff: 2.35,
    });
    expect(
      summonerSkillDefinitions.map(({ skillId, cooldown, damageCoeff }) => ({
        skillId,
        cooldown,
        damageCoeff,
      })),
    ).toContainEqual({
      skillId: "spirit_bolt",
      cooldown: 3.2,
      damageCoeff: 1.9,
    });
  });

  it("keeps the client SkillDB tuned coefficients aligned with the server mirror", () => {
    const csvRows = readSkillDbRows();
    const serverSkills = [
      ...berserkerSkillDefinitions,
      ...summonerSkillDefinitions,
    ];

    for (const serverSkill of serverSkills) {
      const clientSkill = csvRows.get(serverSkill.skillId);

      expect(clientSkill).toBeDefined();
      expect(clientSkill?.classId).toBe(serverSkill.classId);
      expect(clientSkill?.cooldown).toBe(serverSkill.cooldown);
      expect(clientSkill?.damageCoeff).toBe(serverSkill.damageCoeff);
    }
  });
});

function readSkillDbRows() {
  const csvPath = resolve(process.cwd(), "../client/Content/Data/SkillDB.csv");
  const [, ...rows] = readFileSync(csvPath, "utf8").trim().split(/\r?\n/);
  return new Map(
    rows
      .map((row) => row.split(","))
      .filter((columns) => tunedClassIds.has(Number(columns[1])))
      .map((columns) => [
        columns[0],
        {
          classId: Number(columns[1]),
          cooldown: Number(columns[5]),
          damageCoeff: Number(columns[6]),
        },
      ]),
  );
}
