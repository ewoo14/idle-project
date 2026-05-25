import type { Pool } from "pg";
import { currentSeasonId } from "../../core/data/season.js";
import type { SeasonStateRecord } from "./season.service.js";

export class SeasonRepoPg {
  constructor(private readonly pool: Pool) {}

  async findCharacter(userId: string, characterId: string) {
    const result = await this.pool.query(
      `select id, user_id as "userId"
       from characters
       where user_id = $1 and id = $2`,
      [userId, characterId],
    );
    return result.rows[0] ?? null;
  }

  async getOrCreateState(userId: string): Promise<SeasonStateRecord> {
    const result = await this.pool.query(
      `insert into season_state (user_id, season_id, tokens, claimed_tiers, updated_at)
       values ($1, $2, 0, '[]'::jsonb, now())
       on conflict (user_id, season_id) do update
       set user_id = excluded.user_id
       returning user_id as "userId",
                 season_id as "seasonId",
                 tokens,
                 claimed_tiers as "claimedTiers",
                 updated_at as "updatedAt"`,
      [userId, currentSeasonId],
    );
    return result.rows[0];
  }

  async addTokens(userId: string, amount: number): Promise<SeasonStateRecord> {
    const result = await this.pool.query(
      `insert into season_state (user_id, season_id, tokens, claimed_tiers, updated_at)
       values ($1, $2, $3, '[]'::jsonb, now())
       on conflict (user_id, season_id) do update
       set tokens = season_state.tokens + excluded.tokens,
           updated_at = now()
       returning user_id as "userId",
                 season_id as "seasonId",
                 tokens,
                 claimed_tiers as "claimedTiers",
                 updated_at as "updatedAt"`,
      [userId, currentSeasonId, amount],
    );
    return result.rows[0];
  }

  async claimTier(input: {
    userId: string;
    characterId: string;
    tier: number;
    rewardGold: number;
    rewardExp: number;
    now: Date;
  }) {
    const client = await this.pool.connect();
    try {
      await client.query("begin");
      const state = await client.query(
        `update season_state
         set claimed_tiers = claimed_tiers || to_jsonb(array[$3::int]),
             updated_at = $4
         where user_id = $1
           and season_id = $2
           and not claimed_tiers @> to_jsonb(array[$3::int])
         returning user_id as "userId",
                   season_id as "seasonId",
                   tokens,
                   claimed_tiers as "claimedTiers",
                   updated_at as "updatedAt"`,
        [input.userId, currentSeasonId, input.tier, input.now],
      );
      if (!state.rows[0]) {
        await client.query("rollback");
        return null;
      }

      const totals = await client.query(
        `update characters
         set gold = gold + $3,
             total_exp = total_exp + $4,
             last_save_at = $5
         where user_id = $1 and id = $2
         returning gold, total_exp as "totalExp"`,
        [
          input.userId,
          input.characterId,
          input.rewardGold,
          input.rewardExp,
          input.now,
        ],
      );
      if (!totals.rows[0]) {
        await client.query("rollback");
        return null;
      }

      await client.query("commit");
      return { state: state.rows[0], totals: totals.rows[0] };
    } catch (error) {
      await client.query("rollback");
      throw error;
    } finally {
      client.release();
    }
  }
}
