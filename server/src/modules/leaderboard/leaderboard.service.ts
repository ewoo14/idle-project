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
  getPowerRank(
    seasonId: number,
    characterId: string,
  ): Promise<Omit<LeaderboardRow, "characterId"> | null>;
  getRebirthRank(
    seasonId: number,
    characterId: string,
  ): Promise<Omit<LeaderboardRow, "characterId"> | null>;
  upsertWeeklyDamage(
    weekId: string,
    characterId: string,
    damage: bigint,
  ): Promise<void>;
  listWeeklyDamage(weekId: string, limit: number): Promise<LeaderboardRow[]>;
  getWeeklyDamageRank(
    weekId: string,
    characterId: string,
  ): Promise<Omit<LeaderboardRow, "characterId"> | null>;
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

  async getMyRank(
    kind: "power" | "rebirth",
    seasonId: number,
    characterId: string,
  ) {
    const row =
      kind === "power"
        ? await this.repo.getPowerRank(seasonId, characterId)
        : await this.repo.getRebirthRank(seasonId, characterId);
    return row ?? { rank: 0, score: 0n };
  }

  async updateWeeklyDamage(input: {
    characterId: string;
    weekId: string;
    damage: bigint;
  }) {
    await this.repo.upsertWeeklyDamage(
      input.weekId,
      input.characterId,
      input.damage,
    );
  }

  async getWeekly(weekId: string, limit = 100) {
    const normalizedLimit = Math.min(Math.max(limit, 1), 100);
    return this.repo.listWeeklyDamage(weekId, normalizedLimit);
  }

  async getMyWeeklyRank(weekId: string, characterId: string) {
    const row = await this.repo.getWeeklyDamageRank(weekId, characterId);
    return row ?? { rank: 0, score: 0n };
  }
}

export const powerKey = (seasonId: number) => `leaderboard:power:${seasonId}`;
export const rebirthKey = (seasonId: number) =>
  `leaderboard:rebirth:${seasonId}`;
