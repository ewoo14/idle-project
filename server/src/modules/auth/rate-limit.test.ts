import Fastify from "fastify";
import { afterEach, describe, expect, it } from "vitest";
import {
  rateLimitPlugin,
  rateLimitPolicies,
} from "../../plugins/rate-limit.js";

describe("auth rate limit", () => {
  const app = Fastify({ logger: false });

  afterEach(async () => {
    await app.close();
  });

  it("returns 429 on the sixth auth login request", async () => {
    await app.register(rateLimitPlugin);
    app.post(
      "/v1/auth/login",
      { config: { rateLimit: rateLimitPolicies.auth } },
      async () => ({ ok: true, data: {} }),
    );
    await app.ready();

    for (let i = 0; i < 5; i += 1) {
      const response = await app.inject({
        method: "POST",
        url: "/v1/auth/login",
      });
      expect(response.statusCode).toBe(200);
    }

    const response = await app.inject({
      method: "POST",
      url: "/v1/auth/login",
    });

    expect(response.statusCode).toBe(429);
  });
});
