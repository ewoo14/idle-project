import type { FastifyInstance } from "fastify";
import { pool } from "../../core/db.js";
import { rateLimitPolicies } from "../../plugins/rate-limit.js";
import { CharacterRepoPg } from "./character.repo.js";
import {
  createCharacterSchema,
  getCharacterSchema,
} from "./character.schema.js";
import { CharacterService } from "./character.service.js";

export async function characterRoutes(app: FastifyInstance) {
  const service = new CharacterService(new CharacterRepoPg(pool));

  app.post(
    "/",
    {
      preHandler: app.authenticate,
      schema: createCharacterSchema,
      config: { rateLimit: rateLimitPolicies.mutate },
    },
    async (request) => {
      const body = request.body as { classId: number };
      const character = await service.create(request.user.sub, body);
      return { ok: true, data: character };
    },
  );

  app.get(
    "/:id",
    {
      preHandler: app.authenticate,
      schema: getCharacterSchema,
      config: { rateLimit: rateLimitPolicies.read },
    },
    async (request) => {
      const params = request.params as { id: string };
      const character = await service.get(request.user.sub, params.id);
      return { ok: true, data: character };
    },
  );
}
