import type { FastifyInstance } from "fastify";
import type { Redis } from "ioredis";
import { pool } from "../../core/db.js";
import { rateLimitPolicies } from "../../plugins/rate-limit.js";
import {
  LeaderboardCacheRedis,
  LeaderboardRepoPg,
} from "./leaderboard.repo.js";
import { leaderboardQuerySchema } from "./leaderboard.schema.js";
import { LeaderboardService } from "./leaderboard.service.js";

export async function leaderboardRoutes(
  app: FastifyInstance,
  opts: { redis: Redis },
) {
  const service = new LeaderboardService(
    new LeaderboardRepoPg(pool),
    new LeaderboardCacheRedis(opts.redis),
  );

  app.get(
    "/power",
    {
      schema: leaderboardQuerySchema,
      config: { rateLimit: rateLimitPolicies.read },
    },
    async (request) => {
      const query = request.query as { season: number; limit?: number };
      const data = await service.getPower(query.season, query.limit ?? 100);
      return { ok: true, data };
    },
  );

  app.get(
    "/rebirth",
    {
      schema: leaderboardQuerySchema,
      config: { rateLimit: rateLimitPolicies.read },
    },
    async (request) => {
      const query = request.query as { season: number; limit?: number };
      const data = await service.getRebirth(query.season, query.limit ?? 100);
      return { ok: true, data };
    },
  );
}
