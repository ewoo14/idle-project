import type { Pool } from "pg";

export class QuestRepoPg {
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

  async listProgress(userId: string) {
    const result = await this.pool.query(
      `select user_id as "userId", quest_id as "questId", progress, completed,
              claimed, daily_reset_date as "dailyResetDate",
              weekly_reset_id as "weeklyResetId", updated_at as "updatedAt"
       from quest_progress
       where user_id = $1`,
      [userId],
    );
    return result.rows;
  }

  async resetDailyProgress(
    userId: string,
    dailyResetDate: string,
    questIds: string[],
  ) {
    await this.pool.query(
      `insert into quest_progress (user_id, quest_id, progress, completed, claimed, daily_reset_date, updated_at)
       select $1, quest_id, 0, false, false, $2::date, now()
       from unnest($3::text[]) as quest_id
       on conflict (user_id, quest_id) do update
       set progress = 0,
           completed = false,
           claimed = false,
           daily_reset_date = excluded.daily_reset_date,
           updated_at = now()
       where quest_progress.daily_reset_date is distinct from excluded.daily_reset_date`,
      [userId, dailyResetDate, questIds],
    );
  }

  async resetWeeklyProgress(
    userId: string,
    weeklyResetId: string,
    questIds: string[],
  ) {
    await this.pool.query(
      `insert into quest_progress (user_id, quest_id, progress, completed, claimed, weekly_reset_id, updated_at)
       select $1, quest_id, 0, false, false, $2, now()
       from unnest($3::text[]) as quest_id
       on conflict (user_id, quest_id) do update
       set progress = 0,
           completed = false,
           claimed = false,
           weekly_reset_id = excluded.weekly_reset_id,
           updated_at = now()
       where quest_progress.weekly_reset_id is distinct from excluded.weekly_reset_id`,
      [userId, weeklyResetId, questIds],
    );
  }

  async upsertProgress(input: {
    userId: string;
    questId: string;
    progress: number;
    completed: boolean;
    dailyResetDate: string | null;
    weeklyResetId: string | null;
  }) {
    const result = await this.pool.query(
      `insert into quest_progress (user_id, quest_id, progress, completed, claimed, daily_reset_date, weekly_reset_id, updated_at)
       values ($1, $2, $3, $4, false, $5::date, $6, now())
       on conflict (user_id, quest_id) do update
       set progress = excluded.progress,
           completed = excluded.completed,
           daily_reset_date = excluded.daily_reset_date,
           weekly_reset_id = excluded.weekly_reset_id,
           updated_at = now()
       returning user_id as "userId", quest_id as "questId", progress, completed,
                 claimed, daily_reset_date as "dailyResetDate",
                 weekly_reset_id as "weeklyResetId", updated_at as "updatedAt"`,
      [
        input.userId,
        input.questId,
        input.progress,
        input.completed,
        input.dailyResetDate,
        input.weeklyResetId,
      ],
    );
    return result.rows[0];
  }

  async claimQuest(input: {
    userId: string;
    characterId: string;
    questId: string;
    rewardGold: number;
    rewardExp: number;
    now: Date;
  }) {
    const client = await this.pool.connect();
    try {
      await client.query("begin");
      const progress = await client.query(
        `update quest_progress
         set claimed = true, updated_at = $3
         where user_id = $1 and quest_id = $2 and completed = true and claimed = false
         returning user_id as "userId", quest_id as "questId", progress, completed,
                   claimed, daily_reset_date as "dailyResetDate",
                   weekly_reset_id as "weeklyResetId", updated_at as "updatedAt"`,
        [input.userId, input.questId, input.now],
      );
      if (!progress.rows[0]) {
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
      return { progress: progress.rows[0], totals: totals.rows[0] };
    } catch (error) {
      await client.query("rollback");
      throw error;
    } finally {
      client.release();
    }
  }
}
