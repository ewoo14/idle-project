import { readFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";
import { pool } from "../core/db.js";

const here = dirname(fileURLToPath(import.meta.url));
const root = resolve(here, "../..");

export async function runMigration(direction: "up" | "down") {
  const file = direction === "up" ? "0001_init.sql" : "0001_init.down.sql";
  const sql = await readFile(resolve(root, "migrations", file), "utf8");
  await pool.query(sql);
}

if (import.meta.url === `file://${process.argv[1]?.replace(/\\/g, "/")}`) {
  const direction = process.argv[2] === "down" ? "down" : "up";
  await runMigration(direction);
  await pool.end();
}
