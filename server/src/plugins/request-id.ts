import fp from "fastify-plugin";
import { nanoid } from "nanoid";

export const requestIdPlugin = fp(async (app) => {
  app.addHook("onRequest", async (request) => {
    const incoming = request.headers["x-request-id"];
    request.id = Array.isArray(incoming) ? incoming[0] : incoming || nanoid(16);
  });
  app.addHook("onSend", async (request, reply) => {
    reply.header("x-request-id", request.id);
  });
});
