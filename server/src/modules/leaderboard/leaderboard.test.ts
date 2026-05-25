import { describe, expect, it, vi } from "vitest";
import { LeaderboardService } from "./leaderboard.service.js";

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
});

function createRepo() {
  return {
    upsertPower: vi.fn(),
    upsertRebirth: vi.fn(),
    listPower: vi.fn().mockResolvedValue([]),
    listRebirth: vi.fn().mockResolvedValue([]),
  };
}

function createCache() {
  return {
    zadd: vi.fn(),
    zrevrangeWithScores: vi.fn().mockResolvedValue([]),
  };
}
