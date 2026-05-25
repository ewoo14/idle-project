import fp from "fastify-plugin";
import { registerErrorHandler } from "../core/errors.js";

export const errorHandlerPlugin = fp(async (app) => {
  registerErrorHandler(app);
});
