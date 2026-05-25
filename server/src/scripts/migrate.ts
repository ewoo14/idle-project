import { readdir, readFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";
import { pool } from "../core/db.js";

const here = dirname(fileURLToPath(import.meta.url));
const root = resolve(here, "../..");

export async function runMigration(direction: "up" | "down") {
  const migrationsDir = resolve(root, "migrations");
  const files = await readdir(migrationsDir);
  const migrationFiles =
    direction === "up"
      ? files
          .filter(
            (file) => /^\d+_.+\.sql$/.test(file) && !file.endsWith(".down.sql"),
          )
          .sort()
      : files
          .filter((file) => /^\d+_.+\.down\.sql$/.test(file))
          .sort()
          .reverse();

  for (const file of migrationFiles) {
    const sql = await readFile(resolve(migrationsDir, file), "utf8");
    await pool.query(sql);
  }
}

if (import.meta.url === `file://${process.argv[1]?.replace(/\\/g, "/")}`) {
  const direction = process.argv[2] === "down" ? "down" : "up";
  await runMigration(direction);
  await pool.end();
}
