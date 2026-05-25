import type { FastifyInstance } from "fastify";
import { pool } from "../../core/db.js";
import { rateLimitPolicies } from "../../plugins/rate-limit.js";
import { QuestRepoPg } from "./quest.repo.js";
import {
  questClaimSchema,
  questListSchema,
  questProgressSchema,
} from "./quest.schema.js";
import { QuestService } from "./quest.service.js";

export async function questRoutes(app: FastifyInstance) {
  const service = new QuestService(new QuestRepoPg(pool));

  app.get(
    "/",
    {
      preHandler: app.authenticate,
      schema: questListSchema,
      config: { rateLimit: rateLimitPolicies.read },
    },
    async (request) => {
      const query = request.query as { characterId: string };
      const data = await service.list(request.user.sub, query.characterId);
      return { ok: true, data };
    },
  );

  app.post(
    "/:id/progress",
    {
      preHandler: app.authenticate,
      schema: questProgressSchema,
      config: { rateLimit: rateLimitPolicies.mutate },
    },
    async (request) => {
      const params = request.params as { id: string };
      const body = request.body as { characterId: string; amount: number };
      const data = await service.addProgress(
        request.user.sub,
        body.characterId,
        {
          questId: params.id,
          amount: body.amount,
        },
      );
      return { ok: true, data };
    },
  );

  app.post(
    "/:id/claim",
    {
      preHandler: app.authenticate,
      schema: questClaimSchema,
      config: { rateLimit: rateLimitPolicies.mutate },
    },
    async (request) => {
      const params = request.params as { id: string };
      const body = request.body as { characterId: string };
      const data = await service.claim(
        request.user.sub,
        body.characterId,
        params.id,
      );
      return { ok: true, data };
    },
  );
}
