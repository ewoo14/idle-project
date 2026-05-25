import { randomUUID } from "node:crypto";
import type { Pool } from "pg";
import type { CharacterCreateInput } from "./character.service.js";

export class CharacterRepoPg {
  constructor(private readonly pool: Pool) {}

  async createCharacter(input: CharacterCreateInput) {
    const result = await this.pool.query(
      `insert into characters (id, user_id, class_id, level, rebirth_count, stats, skill_tree, inventory)
       values ($1, $2, $3, $4, $5, $6::jsonb, $7::jsonb, $8::jsonb)
       returning id, user_id as "userId", class_id as "classId", level, rebirth_count as "rebirthCount", stats, skill_tree as "skillTree", inventory, last_save_at as "lastSaveAt"`,
      [
        randomUUID(),
        input.userId,
        input.classId,
        input.level,
        input.rebirthCount,
        JSON.stringify(input.stats),
        JSON.stringify(input.skillTree),
        JSON.stringify(input.inventory),
      ],
    );
    return result.rows[0];
  }

  async findByIdForUser(userId: string, characterId: string) {
    const result = await this.pool.query(
      `select id, user_id as "userId", class_id as "classId", level, rebirth_count as "rebirthCount",
              stats, skill_tree as "skillTree", inventory, last_save_at as "lastSaveAt"
       from characters
       where user_id = $1 and id = $2`,
      [userId, characterId],
    );
    return result.rows[0] ?? null;
  }
}
