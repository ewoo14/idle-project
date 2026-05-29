import type { Redis } from "ioredis";
import type { Pool } from "pg";
import type { LeaderboardRow } from "./leaderboard.service.js";

export class LeaderboardRepoPg {
  constructor(private readonly pool: Pool) {}

  async upsertPower(characterId: string, seasonId: number, score: bigint) {
    await this.pool.query(
      `insert into leaderboard_power (character_id, season_id, power_score, updated_at)
       values ($1, $2, $3, now())
       on conflict (character_id)
       do update set season_id = excluded.season_id, power_score = excluded.power_score, updated_at = now()`,
      [characterId, seasonId, score.toString()],
    );
  }

  async upsertRebirth(
    characterId: string,
    seasonId: number,
    rebirthCount: number,
  ) {
    await this.pool.query(
      `insert into leaderboard_rebirth (character_id, season_id, rebirth_count, updated_at)
       values ($1, $2, $3, now())
       on conflict (character_id)
       do update set season_id = excluded.season_id, rebirth_count = excluded.rebirth_count, updated_at = now()`,
      [characterId, seasonId, rebirthCount],
    );
  }

  async listPower(seasonId: number, limit: number): Promise<LeaderboardRow[]> {
    const result = await this.pool.query(
      `select character_id as "characterId", power_score as score,
              row_number() over (order by power_score desc) as rank
       from leaderboard_power
       where season_id = $1
       order by power_score desc
       limit $2`,
      [seasonId, limit],
    );
    return result.rows.map((row) => ({
      characterId: row.characterId,
      score: BigInt(row.score),
      rank: Number(row.rank),
    }));
  }

  async listRebirth(
    seasonId: number,
    limit: number,
  ): Promise<LeaderboardRow[]> {
    const result = await this.pool.query(
      `select character_id as "characterId", rebirth_count as score,
              row_number() over (order by rebirth_count desc) as rank
       from leaderboard_rebirth
       where season_id = $1
       order by rebirth_count desc
       limit $2`,
      [seasonId, limit],
    );
    return result.rows.map((row) => ({
      characterId: row.characterId,
      score: BigInt(row.score),
      rank: Number(row.rank),
    }));
  }

  async getPowerRank(
    seasonId: number,
    characterId: string,
  ): Promise<Omit<LeaderboardRow, "characterId"> | null> {
    const result = await this.pool.query(
      `select rank, score
       from (
         select character_id,
                rank() over (order by power_score desc) as rank,
                power_score as score
         from leaderboard_power
         where season_id = $1
       ) ranked
       where character_id = $2`,
      [seasonId, characterId],
    );
    const row = result.rows[0];
    return row
      ? {
          rank: Number(row.rank),
          score: BigInt(row.score),
        }
      : null;
  }

  async getRebirthRank(
    seasonId: number,
    characterId: string,
  ): Promise<Omit<LeaderboardRow, "characterId"> | null> {
    const result = await this.pool.query(
      `select rank, score
       from (
         select character_id,
                rank() over (order by rebirth_count desc) as rank,
                rebirth_count as score
         from leaderboard_rebirth
         where season_id = $1
       ) ranked
       where character_id = $2`,
      [seasonId, characterId],
    );
    const row = result.rows[0];
    return row
      ? {
          rank: Number(row.rank),
          score: BigInt(row.score),
        }
      : null;
  }
}

export class LeaderboardCacheRedis {
  constructor(private readonly redis: Redis) {}

  async zadd(key: string, score: number, member: string) {
    return this.redis.zadd(key, score, member);
  }

  async zrevrangeWithScores(
    key: string,
    limit: number,
  ): Promise<LeaderboardRow[]> {
    const rows = await this.redis.zrevrange(key, 0, limit - 1, "WITHSCORES");
    const result: LeaderboardRow[] = [];
    for (let i = 0; i < rows.length; i += 2) {
      result.push({
        characterId: rows[i] ?? "",
        score: BigInt(rows[i + 1] ?? "0"),
        rank: result.length + 1,
      });
    }
    return result;
  }
}
