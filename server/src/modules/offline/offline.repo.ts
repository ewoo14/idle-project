import type { Pool } from "pg";

export class OfflineRepoPg {
  constructor(private readonly pool: Pool) {}

  async findCharacter(userId: string, characterId: string) {
    const result = await this.pool.query(
      `select id, user_id as "userId", level, rebirth_count as "rebirthCount",
              gold, total_exp as "totalExp", last_seen_at as "lastSeenAt"
       from characters
       where user_id = $1 and id = $2`,
      [userId, characterId],
    );
    return result.rows[0] ?? null;
  }

  async claim(input: {
    characterId: string;
    gold: number;
    exp: number;
    now: Date;
  }) {
    const client = await this.pool.connect();
    try {
      await client.query("begin");
      const result = await client.query(
        `update characters
         set gold = gold + $2,
             total_exp = total_exp + $3,
             last_seen_at = $4,
             last_save_at = $4
         where id = $1
         returning gold, total_exp as "totalExp"`,
        [input.characterId, input.gold, input.exp, input.now],
      );
      await client.query("commit");
      return result.rows[0];
    } catch (error) {
      await client.query("rollback");
      throw error;
    } finally {
      client.release();
    }
  }
}
