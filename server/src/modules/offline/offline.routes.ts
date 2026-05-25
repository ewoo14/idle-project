import type { FastifyInstance } from "fastify";
import { pool } from "../../core/db.js";
import { rateLimitPolicies } from "../../plugins/rate-limit.js";
import { OfflineRepoPg } from "./offline.repo.js";
import { offlineClaimSchema, offlinePreviewSchema } from "./offline.schema.js";
import { OfflineService } from "./offline.service.js";

export async function offlineRoutes(app: FastifyInstance) {
  const service = new OfflineService(new OfflineRepoPg(pool));

  app.get(
    "/preview",
    {
      preHandler: app.authenticate,
      schema: offlinePreviewSchema,
      config: { rateLimit: rateLimitPolicies.read },
    },
    async (request) => {
      const query = request.query as { characterId: string };
      const data = await service.preview(request.user.sub, query.characterId);
      return { ok: true, data };
    },
  );

  app.post(
    "/claim",
    {
      preHandler: app.authenticate,
      schema: offlineClaimSchema,
      config: { rateLimit: rateLimitPolicies.mutate },
    },
    async (request) => {
      const body = request.body as { characterId: string };
      const data = await service.claim(request.user.sub, body.characterId);
      return { ok: true, data };
    },
  );
}
