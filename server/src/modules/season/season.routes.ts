import type { FastifyInstance } from "fastify";
import { pool } from "../../core/db.js";
import { rateLimitPolicies } from "../../plugins/rate-limit.js";
import { SeasonRepoPg } from "./season.repo.js";
import {
  seasonAddTokensSchema,
  seasonClaimSchema,
  seasonProgressSchema,
} from "./season.schema.js";
import { SeasonService } from "./season.service.js";

export async function seasonRoutes(app: FastifyInstance) {
  const service = new SeasonService(new SeasonRepoPg(pool));

  app.get(
    "/",
    {
      preHandler: app.authenticate,
      schema: seasonProgressSchema,
      config: { rateLimit: rateLimitPolicies.read },
    },
    async (request) => {
      const data = await service.progress(request.user.sub);
      return { ok: true, data };
    },
  );

  app.post(
    "/tokens",
    {
      preHandler: app.authenticate,
      schema: seasonAddTokensSchema,
      config: { rateLimit: rateLimitPolicies.mutate },
    },
    async (request) => {
      const body = request.body as { characterId: string; amount: number };
      const data = await service.addTokens(
        request.user.sub,
        body.characterId,
        body.amount,
      );
      return { ok: true, data };
    },
  );

  app.post(
    "/tiers/:tier/claim",
    {
      preHandler: app.authenticate,
      schema: seasonClaimSchema,
      config: { rateLimit: rateLimitPolicies.mutate },
    },
    async (request) => {
      const params = request.params as { tier: number };
      const body = request.body as { characterId: string };
      const data = await service.claim(
        request.user.sub,
        body.characterId,
        params.tier,
      );
      return { ok: true, data };
    },
  );
}
