import { randomUUID } from "node:crypto";
import type { Pool } from "pg";
import type { AuthUser } from "./auth.service.js";

export class AuthRepo {
  constructor(private readonly pool: Pool) {}

  async createUser(input: {
    email: string;
    nickname: string;
    passwordHash: string;
  }): Promise<AuthUser> {
    const result = await this.pool.query(
      `insert into users (id, email, nickname, password_hash)
       values ($1, $2, $3, $4)
       returning id, email, nickname, password_hash as "passwordHash"`,
      [randomUUID(), input.email, input.nickname, input.passwordHash],
    );
    return result.rows[0];
  }

  async findByEmail(email: string): Promise<AuthUser | null> {
    const result = await this.pool.query(
      `select id, email, nickname, password_hash as "passwordHash"
       from users
       where email = $1`,
      [email],
    );
    return result.rows[0] ?? null;
  }

  async touchLastLogin(userId: string) {
    await this.pool.query(
      "update users set last_login_at = now() where id = $1",
      [userId],
    );
  }
}
