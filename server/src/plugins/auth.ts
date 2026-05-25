import { readFileSync } from "node:fs";
import jwt from "@fastify/jwt";
import fp from "fastify-plugin";
import { env } from "../core/env.js";
import { AuthError } from "../core/errors.js";

declare module "fastify" {
  interface FastifyInstance {
    authenticate: (
      request: FastifyRequest,
      reply: FastifyReply,
    ) => Promise<void>;
  }
}

declare module "@fastify/jwt" {
  interface FastifyJWT {
    payload: {
      sub: string;
      email: string;
      nickname: string;
      jti?: string;
      typ?: string;
    };
    user: {
      sub: string;
      email: string;
      nickname: string;
      jti?: string;
      typ?: string;
    };
  }
}

import type { FastifyReply, FastifyRequest } from "fastify";

export const authPlugin = fp(async (app) => {
  const privateKey = readFileSync(env.JWT_PRIVATE_KEY_PATH, "utf8");
  const publicKey = readFileSync(env.JWT_PUBLIC_KEY_PATH, "utf8");

  await app.register(jwt, {
    secret: { private: privateKey, public: publicKey },
    sign: { algorithm: "RS256", kid: env.JWT_KEY_ID },
  });

  app.decorate("authenticate", async (request: FastifyRequest) => {
    try {
      await request.jwtVerify();
    } catch {
      throw new AuthError("access 토큰이 없거나 올바르지 않습니다.", {
        code: "AUTH_ACCESS_TOKEN_INVALID",
      });
    }
  });
});
