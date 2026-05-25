import { randomUUID } from "node:crypto";
import type { Pool } from "pg";
import type {
  CharacterCreateInput,
  CharacterRebirthInput,
} from "./character.service.js";

export class CharacterRepoPg {
  constructor(private readonly pool: Pool) {}

  async createCharacter(input: CharacterCreateInput) {
    const result = await this.pool.query(
      `insert into characters (id, user_id, class_id, level, rebirth_count, rebirth_bonus_points, stats, skill_tree, inventory, last_seen_at)
       values ($1, $2, $3, $4, $5, $6, $7::jsonb, $8::jsonb, $9::jsonb, now())
       returning id, user_id as "userId", class_id as "classId", level, rebirth_count as "rebirthCount",
                 rebirth_bonus_points as "rebirthBonusPoints", stats, skill_tree as "skillTree", inventory, gold, total_exp as "totalExp",
                 last_seen_at as "lastSeenAt", last_save_at as "lastSaveAt"`,
      [
        randomUUID(),
        input.userId,
        input.classId,
        input.level,
        input.rebirthCount,
        input.rebirthBonusPoints,
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
              rebirth_bonus_points as "rebirthBonusPoints", stats, skill_tree as "skillTree", inventory, gold, total_exp as "totalExp",
              last_seen_at as "lastSeenAt", last_save_at as "lastSaveAt"
       from characters
       where user_id = $1 and id = $2`,
      [userId, characterId],
    );
    return result.rows[0] ?? null;
  }

  async rebirthCharacter(input: CharacterRebirthInput) {
    const result = await this.pool.query(
      `update characters
       set level = $2,
           rebirth_count = $3,
           rebirth_bonus_points = $4,
           gold = $5,
           total_exp = $6,
           stats = $7::jsonb,
           last_save_at = now()
       where id = $1
         and rebirth_count = $8
         and rebirth_bonus_points = $9
       returning id, user_id as "userId", class_id as "classId", level, rebirth_count as "rebirthCount",
                 rebirth_bonus_points as "rebirthBonusPoints", stats, skill_tree as "skillTree", inventory, gold, total_exp as "totalExp",
                 last_seen_at as "lastSeenAt", last_save_at as "lastSaveAt"`,
      [
        input.characterId,
        input.level,
        input.rebirthCount,
        input.rebirthBonusPoints,
        input.gold,
        input.totalExp,
        JSON.stringify(input.stats),
        input.expectedRebirthCount,
        input.expectedRebirthBonusPoints,
      ],
    );
    return result.rows[0] ?? null;
  }
}
