import Fastify from "fastify";
import { describe, expect, it, vi } from "vitest";
import { pool } from "../../core/db.js";
import { errorHandlerPlugin } from "../../plugins/error-handler.js";
import { LeaderboardRepoPg } from "./leaderboard.repo.js";
import { leaderboardRoutes } from "./leaderboard.routes.js";
import { LeaderboardService } from "./leaderboard.service.js";

vi.mock("../../core/db.js", () => ({
  pool: { query: vi.fn() },
}));

describe("LeaderboardService", () => {
  it("updates power in Redis and PostgreSQL together", async () => {
    const repo = createRepo();
    const cache = createCache();
    const service = new LeaderboardService(repo, cache);

    await service.updatePower({ characterId: "c1", seasonId: 1, score: 123n });

    expect(repo.upsertPower).toHaveBeenCalledWith("c1", 1, 123n);
    expect(cache.zadd).toHaveBeenCalledWith("leaderboard:power:1", 123, "c1");
  });

  it("updates rebirth in Redis and PostgreSQL together", async () => {
    const repo = createRepo();
    const cache = createCache();
    const service = new LeaderboardService(repo, cache);

    await service.updateRebirth({
      characterId: "c1",
      seasonId: 1,
      rebirthCount: 3,
    });

    expect(repo.upsertRebirth).toHaveBeenCalledWith("c1", 1, 3);
    expect(cache.zadd).toHaveBeenCalledWith("leaderboard:rebirth:1", 3, "c1");
  });

  it("prefers Redis rows for power rankings", async () => {
    const service = new LeaderboardService(createRepo(), {
      ...createCache(),
      zrevrangeWithScores: vi
        .fn()
        .mockResolvedValue([{ characterId: "c1", score: 10n, rank: 1 }]),
    });

    const rows = await service.getPower(1, 100);

    expect(rows[0]?.rank).toBe(1);
  });

  it("falls back to PostgreSQL when Redis power rows are empty", async () => {
    const repo = createRepo();
    repo.listPower.mockResolvedValue([
      { characterId: "c2", score: 9n, rank: 1 },
    ]);
    const service = new LeaderboardService(repo, createCache());

    const rows = await service.getPower(1, 999);

    expect(repo.listPower).toHaveBeenCalledWith(1, 100);
    expect(rows[0]?.characterId).toBe("c2");
  });

  it("falls back to PostgreSQL when Redis rebirth rows are empty", async () => {
    const repo = createRepo();
    repo.listRebirth.mockResolvedValue([
      { characterId: "c3", score: 4n, rank: 1 },
    ]);
    const service = new LeaderboardService(repo, createCache());

    const rows = await service.getRebirth(1, 0);

    expect(repo.listRebirth).toHaveBeenCalledWith(1, 1);
    expect(rows[0]?.characterId).toBe("c3");
  });

  it("returns the current character power rank", async () => {
    const repo = createRepo();
    repo.getPowerRank.mockResolvedValue({ rank: 2, score: 900n });
    const service = new LeaderboardService(repo, createCache());

    const row = await service.getMyRank("power", 1, uuid(1));

    expect(repo.getPowerRank).toHaveBeenCalledWith(1, uuid(1));
    expect(row).toEqual({ rank: 2, score: 900n });
  });

  it("returns rank zero when the current character has no power row", async () => {
    const repo = createRepo();
    repo.getPowerRank.mockResolvedValue(null);
    const service = new LeaderboardService(repo, createCache());

    const row = await service.getMyRank("power", 1, uuid(1));

    expect(row).toEqual({ rank: 0, score: 0n });
  });

  it("returns the current character rebirth rank with tied scores sharing rank", async () => {
    const repo = createRepo();
    repo.getRebirthRank.mockResolvedValue({ rank: 1, score: 3n });
    const service = new LeaderboardService(repo, createCache());

    const row = await service.getMyRank("rebirth", 2, uuid(2));

    expect(repo.getRebirthRank).toHaveBeenCalledWith(2, uuid(2));
    expect(row).toEqual({ rank: 1, score: 3n });
  });
});

describe("LeaderboardRepoPg", () => {
  it("queries power rank inside the requested season only", async () => {
    const query = vi.fn().mockResolvedValue({
      rows: [{ rank: "2", score: "900" }],
    });
    const repo = new LeaderboardRepoPg({ query } as never);

    const row = await repo.getPowerRank(3, uuid(3));

    expect(row).toEqual({ rank: 2, score: 900n });
    expect(query).toHaveBeenCalledWith(
      expect.stringContaining("rank() over (order by power_score desc)"),
      [3, uuid(3)],
    );
    expect(query.mock.calls[0]?.[0]).toContain("where season_id = $1");
  });

  it("returns null when rebirth rank is missing", async () => {
    const query = vi.fn().mockResolvedValue({ rows: [] });
    const repo = new LeaderboardRepoPg({ query } as never);

    const row = await repo.getRebirthRank(4, uuid(4));

    expect(row).toBeNull();
    expect(query).toHaveBeenCalledWith(
      expect.stringContaining("rank() over (order by rebirth_count desc)"),
      [4, uuid(4)],
    );
  });
});

describe("leaderboardRoutes", () => {
  it("returns the current power rank", async () => {
    const app = Fastify({ logger: false });
    const redis = createCache();
    vi.mocked(pool.query).mockResolvedValue({
      rows: [{ rank: "1", score: "1200" }],
    } as never);
    await app.register(errorHandlerPlugin);
    await app.register(leaderboardRoutes, {
      prefix: "/v1/leaderboard",
      redis: redis as never,
    });
    await app.ready();

    const response = await app.inject({
      method: "GET",
      url: `/v1/leaderboard/power/me?season=1&characterId=${uuid(1)}`,
    });

    expect(response.statusCode).toBe(200);
    expect(response.json()).toEqual({
      ok: true,
      data: { rank: 1, score: "1200" },
    });
    expect(pool.query).toHaveBeenCalledWith(expect.any(String), [1, uuid(1)]);

    await app.close();
  });

  it("rejects invalid my-rank query values", async () => {
    const app = Fastify({ logger: false });
    await app.register(errorHandlerPlugin);
    await app.register(leaderboardRoutes, {
      prefix: "/v1/leaderboard",
      redis: createCache() as never,
    });
    await app.ready();

    const response = await app.inject({
      method: "GET",
      url: "/v1/leaderboard/rebirth/me?season=0&characterId=not-a-uuid",
    });

    expect(response.statusCode).toBe(400);

    await app.close();
  });
});

function createRepo() {
  return {
    upsertPower: vi.fn(),
    upsertRebirth: vi.fn(),
    listPower: vi.fn().mockResolvedValue([]),
    listRebirth: vi.fn().mockResolvedValue([]),
    getPowerRank: vi.fn().mockResolvedValue(null),
    getRebirthRank: vi.fn().mockResolvedValue(null),
  };
}

function createCache() {
  return {
    zadd: vi.fn(),
    zrevrangeWithScores: vi.fn().mockResolvedValue([]),
  };
}

function uuid(seed: number) {
  return `00000000-0000-4000-8000-${seed.toString().padStart(12, "0")}`;
}
