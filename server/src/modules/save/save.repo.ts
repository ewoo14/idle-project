import { randomUUID } from "node:crypto";
import type { Pool } from "pg";
import type { SavePayload } from "./save.service.js";

export class SaveRepoPg {
  constructor(private readonly pool: Pool) {}

  async findCharacterByUser(userId: string, characterId: string) {
    const result = await this.pool.query(
      `select id, user_id as "userId", class_id as "classId", level, rebirth_count as "rebirthCount",
              stats, skill_tree as "skillTree", inventory, gold, total_exp as "totalExp",
              last_seen_at as "lastSeenAt", last_save_at as "lastSaveAt"
       from characters where user_id = $1 and id = $2`,
      [userId, characterId],
    );
    return result.rows[0] ?? null;
  }

  async insertSave(input: {
    characterId: string;
    version: number;
    payload: SavePayload;
    serverValidated: boolean;
  }) {
    const client = await this.pool.connect();
    try {
      await client.query("begin");
      const result = await client.query(
        `insert into saves (id, character_id, version, payload, server_validated)
         values ($1, $2, $3, $4::jsonb, $5)
         returning id, character_id as "characterId", version, payload, server_validated as "serverValidated", created_at as "createdAt"`,
        [
          randomUUID(),
          input.characterId,
          input.version,
          JSON.stringify(input.payload),
          input.serverValidated,
        ],
      );
      await client.query(
        `update characters
         set level = greatest(level, $2),
             rebirth_count = greatest(rebirth_count, $3),
             gold = greatest(gold, coalesce(($4::jsonb ->> 'gold')::int, gold)),
             total_exp = greatest(total_exp, coalesce(($4::jsonb ->> 'totalExp')::int, total_exp)),
             last_seen_at = coalesce(to_timestamp(($4::jsonb ->> 'lastSeenUnixSec')::double precision), last_seen_at, now()),
             last_save_at = now()
         where id = $1`,
        [
          input.characterId,
          input.payload.level,
          input.payload.rebirthCount,
          JSON.stringify(input.payload),
        ],
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

  async listHistory(characterId: string, limit: number) {
    const result = await this.pool.query(
      `select id, character_id as "characterId", version, payload, server_validated as "serverValidated", created_at as "createdAt"
       from saves
       where character_id = $1
       order by created_at desc
       limit $2`,
      [characterId, limit],
    );
    return result.rows;
  }
}
