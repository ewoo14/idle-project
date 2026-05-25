import { mkdirSync, writeFileSync } from "node:fs";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";
import { cumulativeExp, expToNext, LEVEL_CAP } from "../../server/src/core/formulas/level.js";

const repoRoot = resolve(dirname(fileURLToPath(import.meta.url)), "../..");
const outputPath = resolve(repoRoot, "client/Content/Data/LevelCurveDB.csv");

const rows = ["Level,ExpToNext,CumulativeExp"];

for (let level = 1; level <= LEVEL_CAP; level += 1) {
  // Unreal DataTable imports integer columns, so level cap Infinity is represented as 0.
  const nextExp = level === LEVEL_CAP ? 0 : expToNext(level);
  rows.push(`${level},${nextExp},${cumulativeExp(level)}`);
}

mkdirSync(dirname(outputPath), { recursive: true });
writeFileSync(outputPath, `${rows.join("\n")}\n`, "utf8");

console.log(`Generated ${outputPath}`);
