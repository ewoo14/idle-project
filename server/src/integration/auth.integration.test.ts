import { generateKeyPairSync } from "node:crypto";
import { mkdtemp, writeFile } from "node:fs/promises";
import { tmpdir } from "node:os";
import { join } from "node:path";
import type { FastifyInstance } from "fastify";
import { GenericContainer } from "testcontainers";
import { afterAll, describe, expect, it } from "vitest";

describe.runIf(process.env.RUN_TESTCONTAINERS === "1")(
  "auth integration",
  () => {
    const containers: { stop(): Promise<unknown> }[] = [];
    let app: FastifyInstance;
    let pool: { end(): Promise<void> };

    afterAll(async () => {
      await app?.close();
      await pool?.end();
      await Promise.all(containers.map((container) => container.stop()));
    });

    it("registers, logs in, refreshes, and logs out with real Postgres and Redis", async () => {
      const postgres = await new GenericContainer("postgres:16-alpine")
        .withEnvironment({
          POSTGRES_DB: "idle",
          POSTGRES_USER: "idle",
          POSTGRES_PASSWORD: "idle",
        })
        .withExposedPorts(5432)
        .start();
      containers.push(postgres);

      const redis = await new GenericContainer("redis:7-alpine")
        .withExposedPorts(6379)
        .start();
      containers.push(redis);

      const keyDir = await mkdtemp(join(tmpdir(), "idle-auth-"));
      const { privateKey, publicKey } = generateKeyPairSync("rsa", {
        modulusLength: 2048,
        privateKeyEncoding: { type: "pkcs8", format: "pem" },
        publicKeyEncoding: { type: "spki", format: "pem" },
      });
      const privateKeyPath = join(keyDir, "jwt-private.pem");
      const publicKeyPath = join(keyDir, "jwt-public.pem");
      await writeFile(privateKeyPath, privateKey);
      await writeFile(publicKeyPath, publicKey);

      process.env.NODE_ENV = "test";
      process.env.LOG_LEVEL = "silent";
      process.env.DATABASE_URL = `postgres://idle:idle@${postgres.getHost()}:${postgres.getMappedPort(5432)}/idle`;
      process.env.REDIS_URL = `redis://${redis.getHost()}:${redis.getMappedPort(6379)}`;
      process.env.JWT_PRIVATE_KEY_PATH = privateKeyPath;
      process.env.JWT_PUBLIC_KEY_PATH = publicKeyPath;
      process.env.JWT_KEY_ID = "test";

      const [{ buildServer }, { runMigration }, dbModule, redisModule] =
        await Promise.all([
          import("../server.js"),
          import("../scripts/migrate.js"),
          import("../core/db.js"),
          import("../core/redis.js"),
        ]);

      pool = dbModule.pool;
      await runMigration("up");
      await redisModule.connectRedis();
      app = await buildServer({ redis: redisModule.redis });

      const email = `hero-${Date.now()}@example.com`;
      const register = await app.inject({
        method: "POST",
        url: "/v1/auth/register",
        payload: { email, password: "Password123!", nickname: "hero" },
      });
      expect(register.statusCode).toBe(200);

      const login = await app.inject({
        method: "POST",
        url: "/v1/auth/login",
        payload: { email, password: "Password123!" },
      });
      expect(login.statusCode).toBe(200);
      const loginBody = JSON.parse(login.body);

      const refresh = await app.inject({
        method: "POST",
        url: "/v1/auth/refresh",
        payload: { refreshToken: loginBody.data.refreshToken },
      });
      expect(refresh.statusCode).toBe(200);
      const refreshBody = JSON.parse(refresh.body);

      const logout = await app.inject({
        method: "POST",
        url: "/v1/auth/logout",
        payload: { refreshToken: refreshBody.data.refreshToken },
      });
      expect(logout.statusCode).toBe(200);
    });
  },
);
