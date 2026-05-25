import type { Pool } from "pg";
import { defaultOwnedPetIds } from "../../core/data/pets.js";
import type { PetStateRecord } from "./pet.service.js";

export class PetRepoPg {
  constructor(private readonly pool: Pool) {}

  async getOrCreateState(userId: string): Promise<PetStateRecord> {
    const result = await this.pool.query(
      `insert into pet_state (user_id, owned_pet_ids, equipped_pet_id, updated_at)
       values ($1, $2::jsonb, $3, now())
       on conflict (user_id) do update
       set user_id = excluded.user_id
       returning user_id as "userId",
                 owned_pet_ids as "ownedPetIds",
                 equipped_pet_id as "equippedPetId",
                 updated_at as "updatedAt"`,
      [userId, JSON.stringify(defaultOwnedPetIds), defaultOwnedPetIds[0]],
    );
    return result.rows[0];
  }

  async equipPet(userId: string, petId: string): Promise<PetStateRecord> {
    const result = await this.pool.query(
      `update pet_state
       set equipped_pet_id = $2,
           updated_at = now()
       where user_id = $1
       returning user_id as "userId",
                 owned_pet_ids as "ownedPetIds",
                 equipped_pet_id as "equippedPetId",
                 updated_at as "updatedAt"`,
      [userId, petId],
    );
    return result.rows[0];
  }
}
