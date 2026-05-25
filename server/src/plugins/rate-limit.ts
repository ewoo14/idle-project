import rateLimit from "@fastify/rate-limit";
import fp from "fastify-plugin";
import type { Redis } from "ioredis";

export const rateLimitPlugin = fp<{ redis?: Redis }>(async (app, opts) => {
  await app.register(rateLimit, {
    redis: opts.redis,
    global: false,
    addHeaders: {
      "x-ratelimit-limit": true,
      "x-ratelimit-remaining": true,
      "x-ratelimit-reset": true,
      "retry-after": true,
    },
    keyGenerator(request) {
      const userId = request.user?.sub;
      return userId ? `${request.ip}:${userId}` : request.ip;
    },
  });
});

export const rateLimitPolicies = {
  auth: { max: 5, timeWindow: "1 minute" },
  save: { max: 30, timeWindow: "1 minute" },
  read: { max: 120, timeWindow: "1 minute" },
  mutate: { max: 20, timeWindow: "1 minute" },
} as const;
