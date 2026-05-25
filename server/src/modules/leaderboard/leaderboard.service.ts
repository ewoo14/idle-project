export type LeaderboardRow = {
  characterId: string;
  score: bigint;
  rank: number;
};

export type LeaderboardRepo = {
  upsertPower(
    characterId: string,
    seasonId: number,
    score: bigint,
  ): Promise<void>;
  upsertRebirth(
    characterId: string,
    seasonId: number,
    rebirthCount: number,
  ): Promise<void>;
  listPower(seasonId: number, limit: number): Promise<LeaderboardRow[]>;
  listRebirth(seasonId: number, limit: number): Promise<LeaderboardRow[]>;
};

export type LeaderboardCache = {
  zadd(key: string, score: number, member: string): Promise<unknown>;
  zrevrangeWithScores(key: string, limit: number): Promise<LeaderboardRow[]>;
};

export class LeaderboardService {
  constructor(
    private readonly repo: LeaderboardRepo,
    private readonly cache: LeaderboardCache,
  ) {}

  async updatePower(input: {
    characterId: string;
    seasonId: number;
    score: bigint;
  }) {
    await Promise.all([
      this.repo.upsertPower(input.characterId, input.seasonId, input.score),
      this.cache.zadd(
        powerKey(input.seasonId),
        Number(input.score),
        input.characterId,
      ),
    ]);
  }

  async updateRebirth(input: {
    characterId: string;
    seasonId: number;
    rebirthCount: number;
  }) {
    await Promise.all([
      this.repo.upsertRebirth(
        input.characterId,
        input.seasonId,
        input.rebirthCount,
      ),
      this.cache.zadd(
        rebirthKey(input.seasonId),
        input.rebirthCount,
        input.characterId,
      ),
    ]);
  }

  async getPower(seasonId: number, limit = 100) {
    const normalizedLimit = Math.min(Math.max(limit, 1), 100);
    const cached = await this.cache.zrevrangeWithScores(
      powerKey(seasonId),
      normalizedLimit,
    );
    return cached.length > 0
      ? cached
      : this.repo.listPower(seasonId, normalizedLimit);
  }

  async getRebirth(seasonId: number, limit = 100) {
    const normalizedLimit = Math.min(Math.max(limit, 1), 100);
    const cached = await this.cache.zrevrangeWithScores(
      rebirthKey(seasonId),
      normalizedLimit,
    );
    return cached.length > 0
      ? cached
      : this.repo.listRebirth(seasonId, normalizedLimit);
  }
}

export const powerKey = (seasonId: number) => `leaderboard:power:${seasonId}`;
export const rebirthKey = (seasonId: number) =>
  `leaderboard:rebirth:${seasonId}`;
