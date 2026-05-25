import cors from "@fastify/cors";
import helmet from "@fastify/helmet";
import Fastify from "fastify";
import type { Redis } from "ioredis";
import { logger } from "./core/logger.js";
import { authRoutes } from "./modules/auth/auth.routes.js";
import { characterRoutes } from "./modules/character/character.routes.js";
import { leaderboardRoutes } from "./modules/leaderboard/leaderboard.routes.js";
import { saveRoutes } from "./modules/save/save.routes.js";
import { authPlugin } from "./plugins/auth.js";
import { errorHandlerPlugin } from "./plugins/error-handler.js";
import { rateLimitPlugin } from "./plugins/rate-limit.js";
import { requestIdPlugin } from "./plugins/request-id.js";

export async function buildServer(opts: { redis: Redis }) {
  const app = Fastify({ logger, disableRequestLogging: false });

  await app.register(requestIdPlugin);
  await app.register(errorHandlerPlugin);
  await app.register(helmet);
  await app.register(cors, { origin: true });
  await app.register(rateLimitPlugin, { redis: opts.redis });
  await app.register(authPlugin);

  app.get("/healthz", async () => ({ ok: true, data: { status: "healthy" } }));

  await app.register(authRoutes, { prefix: "/v1/auth", redis: opts.redis });
  await app.register(characterRoutes, { prefix: "/v1/characters" });
  await app.register(saveRoutes, { prefix: "/v1/save" });
  await app.register(leaderboardRoutes, {
    prefix: "/v1/leaderboard",
    redis: opts.redis,
  });

  return app;
}
