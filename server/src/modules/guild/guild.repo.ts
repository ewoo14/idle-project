import type { Pool, PoolClient } from "pg";
import type {
  GuildJoinMode,
  GuildJoinRequestRecord,
  GuildMemberRecord,
  GuildRank,
  GuildRecord,
  GuildRepo,
} from "./guild.service.js";

const GUILD_COLUMNS = `id,
  name,
  notice,
  join_mode as "joinMode",
  level,
  exp,
  master_character_id as "masterCharacterId",
  member_count as "memberCount",
  week_id as "weekId",
  created_at as "createdAt"`;

const MEMBER_COLUMNS = `guild_id as "guildId",
  character_id as "characterId",
  rank,
  joined_at as "joinedAt",
  weekly_contribution as "weeklyContribution",
  total_contribution as "totalContribution",
  contribution_points as "contributionPoints",
  last_attendance_date as "lastAttendanceDate",
  weekly_auto_contribution as "weeklyAutoContribution",
  last_donation_date as "lastDonationDate",
  daily_donation as "dailyDonation",
  weekly_reset_id as "weeklyResetId"`;

function mapGuild(row: Record<string, unknown>): GuildRecord {
  return {
    id: row.id as string,
    name: row.name as string,
    notice: row.notice as string,
    joinMode: row.joinMode as GuildJoinMode,
    level: Number(row.level),
    exp: BigInt(row.exp as string),
    masterCharacterId: row.masterCharacterId as string,
    memberCount: Number(row.memberCount),
    weekId: (row.weekId as string | null) ?? null,
    createdAt: row.createdAt as Date,
  };
}

function mapMember(row: Record<string, unknown>): GuildMemberRecord {
  return {
    guildId: row.guildId as string,
    characterId: row.characterId as string,
    rank: row.rank as GuildRank,
    joinedAt: row.joinedAt as Date,
    weeklyContribution: BigInt(row.weeklyContribution as string),
    totalContribution: BigInt(row.totalContribution as string),
    contributionPoints: BigInt(row.contributionPoints as string),
    lastAttendanceDate: (row.lastAttendanceDate as string | null) ?? null,
    weeklyAutoContribution: BigInt(row.weeklyAutoContribution as string),
    lastDonationDate: (row.lastDonationDate as string | null) ?? null,
    dailyDonation: BigInt(row.dailyDonation as string),
    weeklyResetId: (row.weeklyResetId as string | null) ?? null,
  };
}

export class GuildRepoPg implements GuildRepo {
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

  async getGuild(guildId: string): Promise<GuildRecord | null> {
    const result = await this.pool.query(
      `select ${GUILD_COLUMNS} from guilds where id = $1`,
      [guildId],
    );
    return result.rows[0] ? mapGuild(result.rows[0]) : null;
  }

  async getGuildByName(name: string): Promise<GuildRecord | null> {
    const result = await this.pool.query(
      `select ${GUILD_COLUMNS} from guilds where lower(name) = lower($1)`,
      [name],
    );
    return result.rows[0] ? mapGuild(result.rows[0]) : null;
  }

  async listGuilds(input: {
    limit: number;
    offset: number;
    q?: string;
  }): Promise<GuildRecord[]> {
    const hasQuery = input.q !== undefined && input.q.trim() !== "";
    const result = hasQuery
      ? await this.pool.query(
          `select ${GUILD_COLUMNS}
           from guilds
           where name ilike $1
           order by member_count desc, created_at asc
           limit $2 offset $3`,
          [`%${input.q?.trim()}%`, input.limit, input.offset],
        )
      : await this.pool.query(
          `select ${GUILD_COLUMNS}
           from guilds
           order by member_count desc, created_at asc
           limit $1 offset $2`,
          [input.limit, input.offset],
        );
    return result.rows.map(mapGuild);
  }

  async getMembership(characterId: string): Promise<GuildMemberRecord | null> {
    const result = await this.pool.query(
      `select ${MEMBER_COLUMNS} from guild_members where character_id = $1`,
      [characterId],
    );
    return result.rows[0] ? mapMember(result.rows[0]) : null;
  }

  async listMembers(guildId: string): Promise<GuildMemberRecord[]> {
    const result = await this.pool.query(
      `select ${MEMBER_COLUMNS}
       from guild_members
       where guild_id = $1
       order by total_contribution desc, joined_at asc`,
      [guildId],
    );
    return result.rows.map(mapMember);
  }

  async listRequests(guildId: string): Promise<GuildJoinRequestRecord[]> {
    const result = await this.pool.query(
      `select guild_id as "guildId",
              character_id as "characterId",
              requested_at as "requestedAt"
       from guild_join_requests
       where guild_id = $1
       order by requested_at asc`,
      [guildId],
    );
    return result.rows.map((row) => ({
      guildId: row.guildId,
      characterId: row.characterId,
      requestedAt: row.requestedAt,
    }));
  }

  async getRequest(
    guildId: string,
    characterId: string,
  ): Promise<GuildJoinRequestRecord | null> {
    const result = await this.pool.query(
      `select guild_id as "guildId",
              character_id as "characterId",
              requested_at as "requestedAt"
       from guild_join_requests
       where guild_id = $1 and character_id = $2`,
      [guildId, characterId],
    );
    const row = result.rows[0];
    return row
      ? {
          guildId: row.guildId,
          characterId: row.characterId,
          requestedAt: row.requestedAt,
        }
      : null;
  }

  async createGuild(input: {
    id: string;
    name: string;
    masterCharacterId: string;
    now: Date;
  }): Promise<GuildRecord> {
    return this.withTransaction(async (client) => {
      const guildResult = await client.query(
        `insert into guilds (id, name, notice, join_mode, level, exp, master_character_id, member_count, created_at)
         values ($1, $2, '', 'open', 1, 0, $3, 1, $4)
         returning ${GUILD_COLUMNS}`,
        [input.id, input.name, input.masterCharacterId, input.now],
      );
      await client.query(
        `insert into guild_members (guild_id, character_id, rank, joined_at)
         values ($1, $2, 'master', $3)`,
        [input.id, input.masterCharacterId, input.now],
      );
      return mapGuild(guildResult.rows[0]);
    });
  }

  async addMember(input: {
    guildId: string;
    characterId: string;
    capacity: number;
    now: Date;
  }): Promise<GuildMemberRecord | null> {
    return this.withTransaction(async (client) => {
      // 정원 검증 + member_count +1 을 원자적으로(조건부 update). 미충족 시 0행.
      const bumped = await client.query(
        `update guilds
         set member_count = member_count + 1
         where id = $1 and member_count < $2
         returning id`,
        [input.guildId, input.capacity],
      );
      if (!bumped.rows[0]) {
        return null;
      }
      const member = await client.query(
        `insert into guild_members (guild_id, character_id, rank, joined_at)
         values ($1, $2, 'member', $3)
         on conflict (character_id) do nothing
         returning ${MEMBER_COLUMNS}`,
        [input.guildId, input.characterId, input.now],
      );
      if (!member.rows[0]) {
        // 이미 어딘가에 소속(중복) → 롤백 위해 에러로 트랜잭션 취소.
        throw new GuildConflictRollback();
      }
      return mapMember(member.rows[0]);
    }).catch((error) => {
      if (error instanceof GuildConflictRollback) {
        return null;
      }
      throw error;
    });
  }

  async removeMember(characterId: string): Promise<boolean> {
    return this.withTransaction(async (client) => {
      const deleted = await client.query(
        `delete from guild_members
         where character_id = $1
         returning guild_id as "guildId"`,
        [characterId],
      );
      const row = deleted.rows[0];
      if (!row) {
        return false;
      }
      await client.query(
        `update guilds
         set member_count = greatest(member_count - 1, 0)
         where id = $1`,
        [row.guildId],
      );
      return true;
    });
  }

  async insertRequest(input: {
    guildId: string;
    characterId: string;
    now: Date;
  }): Promise<void> {
    await this.pool.query(
      `insert into guild_join_requests (guild_id, character_id, requested_at)
       values ($1, $2, $3)
       on conflict (guild_id, character_id) do nothing`,
      [input.guildId, input.characterId, input.now],
    );
  }

  async deleteRequest(guildId: string, characterId: string): Promise<boolean> {
    const result = await this.pool.query(
      `delete from guild_join_requests
       where guild_id = $1 and character_id = $2
       returning guild_id`,
      [guildId, characterId],
    );
    return result.rows.length > 0;
  }

  async approveRequest(input: {
    guildId: string;
    characterId: string;
    capacity: number;
    now: Date;
  }): Promise<GuildMemberRecord | null> {
    return this.withTransaction(async (client) => {
      const removed = await client.query(
        `delete from guild_join_requests
         where guild_id = $1 and character_id = $2
         returning guild_id`,
        [input.guildId, input.characterId],
      );
      if (!removed.rows[0]) {
        return null;
      }
      const bumped = await client.query(
        `update guilds
         set member_count = member_count + 1
         where id = $1 and member_count < $2
         returning id`,
        [input.guildId, input.capacity],
      );
      if (!bumped.rows[0]) {
        throw new GuildConflictRollback();
      }
      const member = await client.query(
        `insert into guild_members (guild_id, character_id, rank, joined_at)
         values ($1, $2, 'member', $3)
         on conflict (character_id) do nothing
         returning ${MEMBER_COLUMNS}`,
        [input.guildId, input.characterId, input.now],
      );
      if (!member.rows[0]) {
        throw new GuildConflictRollback();
      }
      return mapMember(member.rows[0]);
    }).catch((error) => {
      if (error instanceof GuildConflictRollback) {
        return null;
      }
      throw error;
    });
  }

  async setRank(
    guildId: string,
    characterId: string,
    rank: GuildRank,
  ): Promise<GuildMemberRecord | null> {
    const result = await this.pool.query(
      `update guild_members
       set rank = $3
       where guild_id = $1 and character_id = $2
       returning ${MEMBER_COLUMNS}`,
      [guildId, characterId, rank],
    );
    return result.rows[0] ? mapMember(result.rows[0]) : null;
  }

  async updateGuild(
    guildId: string,
    settings: { name?: string; notice?: string; joinMode?: GuildJoinMode },
  ): Promise<GuildRecord | null> {
    const result = await this.pool.query(
      `update guilds
       set name = coalesce($2, name),
           notice = coalesce($3, notice),
           join_mode = coalesce($4, join_mode)
       where id = $1
       returning ${GUILD_COLUMNS}`,
      [
        guildId,
        settings.name ?? null,
        settings.notice ?? null,
        settings.joinMode ?? null,
      ],
    );
    return result.rows[0] ? mapGuild(result.rows[0]) : null;
  }

  async transferMaster(input: {
    guildId: string;
    fromCharacterId: string;
    toCharacterId: string;
  }): Promise<void> {
    await this.withTransaction(async (client) => {
      await client.query(
        `update guild_members set rank = 'member'
         where guild_id = $1 and character_id = $2`,
        [input.guildId, input.fromCharacterId],
      );
      await client.query(
        `update guild_members set rank = 'master'
         where guild_id = $1 and character_id = $2`,
        [input.guildId, input.toCharacterId],
      );
      await client.query(
        `update guilds set master_character_id = $2 where id = $1`,
        [input.guildId, input.toCharacterId],
      );
      return undefined;
    });
  }

  async disbandGuild(guildId: string): Promise<void> {
    // guild_members / guild_join_requests 는 FK cascade 로 함께 삭제된다.
    await this.pool.query(`delete from guilds where id = $1`, [guildId]);
  }

  async applyContribution(input: {
    guildId: string;
    characterId: string;
    amount: bigint;
    newLevel: number;
    weekId: string;
    weeklyReset: boolean;
    patch: {
      lastAttendanceDate?: string;
      weeklyAutoContributionDelta?: bigint;
      dailyDonation?: bigint;
      lastDonationDate?: string;
    };
  }): Promise<GuildMemberRecord | null> {
    return this.withTransaction(async (client) => {
      // 1) 길드 EXP/레벨/주간 키 갱신(EXP·레벨은 영속).
      await client.query(
        `update guilds
         set exp = exp + $2, level = $3, week_id = $4
         where id = $1`,
        [input.guildId, input.amount.toString(), input.newLevel, input.weekId],
      );

      // 2) 멤버 기여 누적. weeklyReset 면 weekly 컬럼을 0 부터(=리셋 후 누적).
      //    weekly_contribution/weekly_auto_contribution 은 리셋 분기로 계산.
      const weeklyBase = input.weeklyReset ? "0" : "weekly_contribution";
      const autoBase = input.weeklyReset ? "0" : "weekly_auto_contribution";
      const autoDelta = input.patch.weeklyAutoContributionDelta ?? 0n;

      const result = await client.query(
        `update guild_members
         set contribution_points = contribution_points + $2,
             total_contribution = total_contribution + $2,
             weekly_contribution = ${weeklyBase} + $2,
             weekly_auto_contribution = ${autoBase} + $3,
             weekly_reset_id = $4,
             last_attendance_date = coalesce($5, last_attendance_date),
             daily_donation = coalesce($6, daily_donation),
             last_donation_date = coalesce($7, last_donation_date)
         where character_id = $1 and guild_id = $8
         returning ${MEMBER_COLUMNS}`,
        [
          input.characterId,
          input.amount.toString(),
          autoDelta.toString(),
          input.weekId,
          input.patch.lastAttendanceDate ?? null,
          input.patch.dailyDonation?.toString() ?? null,
          input.patch.lastDonationDate ?? null,
          input.guildId,
        ],
      );
      return result.rows[0] ? mapMember(result.rows[0]) : null;
    });
  }

  async spendContributionPoints(input: {
    characterId: string;
    cost: bigint;
  }): Promise<GuildMemberRecord | null> {
    // 조건부 update — 잔액 부족 시 0행(원자적 차감).
    const result = await this.pool.query(
      `update guild_members
       set contribution_points = contribution_points - $2
       where character_id = $1 and contribution_points >= $2
       returning ${MEMBER_COLUMNS}`,
      [input.characterId, input.cost.toString()],
    );
    return result.rows[0] ? mapMember(result.rows[0]) : null;
  }

  private async withTransaction<T>(
    fn: (client: PoolClient) => Promise<T>,
  ): Promise<T> {
    const client = await this.pool.connect();
    try {
      await client.query("begin");
      const result = await fn(client);
      await client.query("commit");
      return result;
    } catch (error) {
      await client.query("rollback");
      throw error;
    } finally {
      client.release();
    }
  }
}

/** 트랜잭션 내부 충돌 신호용 내부 에러(정원/중복). 외부로 새지 않는다. */
class GuildConflictRollback extends Error {}
