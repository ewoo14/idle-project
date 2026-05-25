import { Redis } from "ioredis";
import { env } from "./env.js";

export const redis = new Redis(env.REDIS_URL, {
  maxRetriesPerRequest: 3,
  lazyConnect: true,
});

export async function connectRedis() {
  if (redis.status === "wait") {
    await redis.connect();
  }
  return redis;
}

export async function closeRedis() {
  await redis.quit();
}
