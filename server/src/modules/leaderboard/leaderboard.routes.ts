import type { FastifyInstance } from "fastify";
import type { Redis } from "ioredis";
import { pool } from "../../core/db.js";
import { rateLimitPolicies } from "../../plugins/rate-limit.js";
import {
  LeaderboardCacheRedis,
  LeaderboardRepoPg,
} from "./leaderboard.repo.js";
import {
  leaderboardQuerySchema,
  myRankQuerySchema,
  weeklyMyRankQuerySchema,
  weeklyQuerySchema,
} from "./leaderboard.schema.js";
import {
  type LeaderboardRow,
  LeaderboardService,
} from "./leaderboard.service.js";

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
      return { ok: true, data: data.map(serializeLeaderboardRow) };
    },
  );

  app.get(
    "/power/me",
    {
      schema: myRankQuerySchema,
      config: { rateLimit: rateLimitPolicies.read },
    },
    async (request) => {
      const query = request.query as { season: number; characterId: string };
      const data = await service.getMyRank(
        "power",
        query.season,
        query.characterId,
      );
      return { ok: true, data: serializeMyRank(data) };
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
      return { ok: true, data: data.map(serializeLeaderboardRow) };
    },
  );

  app.get(
    "/rebirth/me",
    {
      schema: myRankQuerySchema,
      config: { rateLimit: rateLimitPolicies.read },
    },
    async (request) => {
      const query = request.query as { season: number; characterId: string };
      const data = await service.getMyRank(
        "rebirth",
        query.season,
        query.characterId,
      );
      return { ok: true, data: serializeMyRank(data) };
    },
  );

  app.get(
    "/weekly",
    {
      schema: weeklyQuerySchema,
      config: { rateLimit: rateLimitPolicies.read },
    },
    async (request) => {
      const query = request.query as { week: string; limit?: number };
      const data = await service.getWeekly(query.week, query.limit ?? 100);
      return { ok: true, data: data.map(serializeLeaderboardRow) };
    },
  );

  app.get(
    "/weekly/me",
    {
      schema: weeklyMyRankQuerySchema,
      config: { rateLimit: rateLimitPolicies.read },
    },
    async (request) => {
      const query = request.query as { week: string; characterId: string };
      const data = await service.getMyWeeklyRank(query.week, query.characterId);
      return { ok: true, data: serializeMyRank(data) };
    },
  );
}

function serializeLeaderboardRow(row: LeaderboardRow) {
  return {
    characterId: row.characterId,
    score: row.score.toString(),
    rank: row.rank,
  };
}

function serializeMyRank(row: Omit<LeaderboardRow, "characterId">) {
  return {
    rank: row.rank,
    score: row.score.toString(),
  };
}
