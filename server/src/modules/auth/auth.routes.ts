import bcrypt from "bcrypt";
import type { FastifyInstance } from "fastify";
import type { Redis } from "ioredis";
import { pool } from "../../core/db.js";
import { rateLimitPolicies } from "../../plugins/rate-limit.js";
import { AuthRepo } from "./auth.repo.js";
import {
  loginSchema,
  logoutSchema,
  refreshSchema,
  registerSchema,
} from "./auth.schema.js";
import {
  AuthService,
  type AuthUser,
  refreshTtlSeconds,
} from "./auth.service.js";

export async function authRoutes(app: FastifyInstance, opts: { redis: Redis }) {
  const service = new AuthService({
    users: new AuthRepo(pool),
    password: {
      hash: (password) => bcrypt.hash(password, 12),
      compare: (password, hash) => bcrypt.compare(password, hash),
    },
    tokens: {
      signAccess: async (user: AuthUser) =>
        app.jwt.sign(
          {
            sub: user.id,
            email: user.email,
            nickname: user.nickname,
            typ: "access",
          },
          { expiresIn: "15m" },
        ),
      signRefresh: async (user: AuthUser, jti: string) =>
        app.jwt.sign(
          {
            sub: user.id,
            email: user.email,
            nickname: user.nickname,
            typ: "refresh",
            jti,
          },
          { expiresIn: "30d" },
        ),
      verifyRefresh: async (token: string) => {
        const payload = await app.jwt.verify<{
          sub: string;
          email: string;
          nickname: string;
          jti: string;
          typ?: string;
        }>(token);
        return payload;
      },
    },
    tokenStore: {
      rememberRefresh: async (userId, jti, ttl) => {
        await opts.redis.set(`refresh:active:${jti}`, userId, "EX", ttl);
      },
      isRefreshActive: async (jti) =>
        (await opts.redis.exists(`refresh:active:${jti}`)) === 1,
      isRefreshRevoked: async (jti) =>
        (await opts.redis.exists(`refresh:revoked:${jti}`)) === 1,
      revokeRefresh: async (jti, ttl) => {
        await opts.redis.set(`refresh:revoked:${jti}`, "1", "EX", ttl);
        await opts.redis.del(`refresh:active:${jti}`);
      },
    },
  });

  app.post(
    "/register",
    { schema: registerSchema, config: { rateLimit: rateLimitPolicies.auth } },
    async (request) => {
      const result = await service.register(
        request.body as { email: string; password: string; nickname: string },
      );
      return { ok: true, data: result };
    },
  );

  app.post(
    "/login",
    { schema: loginSchema, config: { rateLimit: rateLimitPolicies.auth } },
    async (request) => {
      const result = await service.login(
        request.body as { email: string; password: string },
      );
      return { ok: true, data: result };
    },
  );

  app.post(
    "/refresh",
    { schema: refreshSchema, config: { rateLimit: rateLimitPolicies.auth } },
    async (request) => {
      const body = request.body as { refreshToken: string };
      const result = await service.refresh(body.refreshToken);
      return { ok: true, data: result };
    },
  );

  app.post(
    "/logout",
    { schema: logoutSchema, config: { rateLimit: rateLimitPolicies.auth } },
    async (request) => {
      const body = request.body as { refreshToken: string };
      await service.logout(body.refreshToken);
      return { ok: true, data: { loggedOut: true, refreshTtlSeconds } };
    },
  );
}
