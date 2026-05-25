import { pool } from "./core/db.js";
import { env } from "./core/env.js";
import { connectRedis, redis } from "./core/redis.js";
import { runMigration } from "./scripts/migrate.js";
import { buildServer } from "./server.js";

await runMigration("up");
await connectRedis();

const app = await buildServer({ redis });

const shutdown = async () => {
  await app.close();
  await redis.quit();
  await pool.end();
};

process.on("SIGTERM", () => void shutdown());
process.on("SIGINT", () => void shutdown());

await app.listen({ host: env.HOST, port: env.PORT });
