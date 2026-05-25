import { describe, expect, it, vi } from "vitest";
import { LeaderboardService } from "./leaderboard.service.js";

describe("LeaderboardService", () => {
  it("점수 업데이트 시 Redis와 PostgreSQL을 함께 갱신한다", async () => {
    const repo = {
      upsertPower: vi.fn(),
      upsertRebirth: vi.fn(),
      listPower: vi.fn(),
      listRebirth: vi.fn(),
    };
    const cache = {
      zadd: vi.fn(),
      zrevrangeWithScores: vi.fn(),
    };
    const service = new LeaderboardService(repo, cache);

    await service.updatePower({ characterId: "c1", seasonId: 1, score: 123n });

    expect(repo.upsertPower).toHaveBeenCalledWith("c1", 1, 123n);
    expect(cache.zadd).toHaveBeenCalledWith("leaderboard:power:1", 123, "c1");
  });

  it("조회는 Redis 결과가 있으면 Redis를 우선한다", async () => {
    const service = new LeaderboardService(
      {
        upsertPower: vi.fn(),
        upsertRebirth: vi.fn(),
        listPower: vi.fn(),
        listRebirth: vi.fn(),
      },
      {
        zadd: vi.fn(),
        zrevrangeWithScores: vi
          .fn()
          .mockResolvedValue([{ characterId: "c1", score: 10n, rank: 1 }]),
      },
    );

    const rows = await service.getPower(1, 100);

    expect(rows[0]?.rank).toBe(1);
  });
});
