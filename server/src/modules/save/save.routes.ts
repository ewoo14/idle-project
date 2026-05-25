import type { FastifyInstance } from "fastify";
import type { Redis } from "ioredis";
import { pool } from "../../core/db.js";
import { logger } from "../../core/logger.js";
import { rateLimitPolicies } from "../../plugins/rate-limit.js";
import {
  LeaderboardCacheRedis,
  LeaderboardRepoPg,
} from "../leaderboard/leaderboard.repo.js";
import { LeaderboardService } from "../leaderboard/leaderboard.service.js";
import { SaveRepoPg } from "./save.repo.js";
import { getSaveSchema, historySchema, putSaveSchema } from "./save.schema.js";
import { type SavePayload, SaveService } from "./save.service.js";

export async function saveRoutes(app: FastifyInstance, opts: { redis: Redis }) {
  const leaderboard = new LeaderboardService(
    new LeaderboardRepoPg(pool),
    new LeaderboardCacheRedis(opts.redis),
  );
  const service = new SaveService(new SaveRepoPg(pool), leaderboard, logger);

  app.get(
    "/",
    {
      preHandler: app.authenticate,
      schema: getSaveSchema,
      config: { rateLimit: rateLimitPolicies.read },
    },
    async (request) => {
      const query = request.query as { characterId: string };
      const data = await service.current(request.user.sub, query.characterId);
      return { ok: true, data: data[0] ?? null };
    },
  );

  app.put(
    "/",
    {
      preHandler: app.authenticate,
      schema: putSaveSchema,
      config: { rateLimit: rateLimitPolicies.save },
    },
    async (request) => {
      const body = request.body as {
        characterId: string;
        version: number;
        payload: SavePayload;
      };
      const data = await service.upload(request.user.sub, body);
      return { ok: true, data };
    },
  );

  app.get(
    "/history",
    {
      preHandler: app.authenticate,
      schema: historySchema,
      config: { rateLimit: rateLimitPolicies.read },
    },
    async (request) => {
      const query = request.query as { characterId: string; limit?: number };
      const data = await service.history(
        request.user.sub,
        query.characterId,
        query.limit ?? 10,
      );
      return { ok: true, data };
    },
  );
}
