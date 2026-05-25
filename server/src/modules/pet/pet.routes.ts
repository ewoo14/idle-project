import type { FastifyInstance } from "fastify";
import { pool } from "../../core/db.js";
import { rateLimitPolicies } from "../../plugins/rate-limit.js";
import { PetRepoPg } from "./pet.repo.js";
import { petEquipSchema, petListSchema } from "./pet.schema.js";
import { PetService } from "./pet.service.js";

export async function petRoutes(app: FastifyInstance) {
  const service = new PetService(new PetRepoPg(pool));

  app.get(
    "/",
    {
      preHandler: app.authenticate,
      schema: petListSchema,
      config: { rateLimit: rateLimitPolicies.read },
    },
    async (request) => {
      const data = await service.list(request.user.sub);
      return { ok: true, data };
    },
  );

  app.post(
    "/:id/equip",
    {
      preHandler: app.authenticate,
      schema: petEquipSchema,
      config: { rateLimit: rateLimitPolicies.mutate },
    },
    async (request) => {
      const params = request.params as { id: string };
      const data = await service.equip(request.user.sub, params.id);
      return { ok: true, data };
    },
  );
}
