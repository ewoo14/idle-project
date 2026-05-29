import {
  bigint,
  boolean,
  date,
  index,
  integer,
  jsonb,
  pgTable,
  primaryKey,
  text,
  timestamp,
  uuid,
  varchar,
} from "drizzle-orm/pg-core";

export const users = pgTable("users", {
  id: uuid("id").primaryKey(),
  email: text("email").notNull().unique(),
  passwordHash: text("password_hash").notNull(),
  nickname: varchar("nickname", { length: 16 }).notNull().unique(),
  createdAt: timestamp("created_at", { withTimezone: true }).defaultNow(),
  lastLoginAt: timestamp("last_login_at", { withTimezone: true }),
});

export const characters = pgTable("characters", {
  id: uuid("id").primaryKey(),
  userId: uuid("user_id")
    .notNull()
    .references(() => users.id, { onDelete: "cascade" }),
  classId: integer("class_id").notNull(),
  level: integer("level").notNull().default(1),
  rebirthCount: integer("rebirth_count").notNull().default(0),
  rebirthBonusPoints: integer("rebirth_bonus_points").notNull().default(0),
  stats: jsonb("stats").notNull().default({}),
  skillTree: jsonb("skill_tree").notNull().default({}),
  inventory: jsonb("inventory").notNull().default([]),
  gold: integer("gold").notNull().default(0),
  totalExp: bigint("total_exp", { mode: "number" }).notNull().default(0),
  lastSeenAt: timestamp("last_seen_at", { withTimezone: true }),
  lastSaveAt: timestamp("last_save_at", { withTimezone: true }),
});

export const saves = pgTable(
  "saves",
  {
    id: uuid("id").primaryKey(),
    characterId: uuid("character_id")
      .notNull()
      .references(() => characters.id, { onDelete: "cascade" }),
    version: integer("version").notNull(),
    payload: jsonb("payload").notNull(),
    serverValidated: boolean("server_validated").notNull().default(false),
    createdAt: timestamp("created_at", { withTimezone: true }).defaultNow(),
  },
  (table) => ({
    savesCharacterCreated: index("saves_character_created").on(
      table.characterId,
      table.createdAt,
    ),
  }),
);

export const leaderboardPower = pgTable(
  "leaderboard_power",
  {
    characterId: uuid("character_id")
      .primaryKey()
      .references(() => characters.id, { onDelete: "cascade" }),
    seasonId: integer("season_id").notNull(),
    powerScore: bigint("power_score", { mode: "bigint" }).notNull().default(0n),
    updatedAt: timestamp("updated_at", { withTimezone: true }).defaultNow(),
  },
  (table) => ({
    leaderboardPowerSeasonScore: index("leaderboard_power_season_score").on(
      table.seasonId,
      table.powerScore,
    ),
  }),
);

export const leaderboardRebirth = pgTable(
  "leaderboard_rebirth",
  {
    characterId: uuid("character_id")
      .primaryKey()
      .references(() => characters.id, { onDelete: "cascade" }),
    seasonId: integer("season_id").notNull(),
    rebirthCount: integer("rebirth_count").notNull().default(0),
    updatedAt: timestamp("updated_at", { withTimezone: true }).defaultNow(),
  },
  (table) => ({
    leaderboardRebirthSeasonCount: index("leaderboard_rebirth_season_count").on(
      table.seasonId,
      table.rebirthCount,
    ),
  }),
);

export const questProgress = pgTable(
  "quest_progress",
  {
    userId: uuid("user_id")
      .notNull()
      .references(() => users.id, { onDelete: "cascade" }),
    questId: varchar("quest_id", { length: 64 }).notNull(),
    progress: integer("progress").notNull().default(0),
    completed: boolean("completed").notNull().default(false),
    claimed: boolean("claimed").notNull().default(false),
    dailyResetDate: date("daily_reset_date"),
    weeklyResetId: varchar("weekly_reset_id", { length: 8 }),
    updatedAt: timestamp("updated_at", { withTimezone: true }).defaultNow(),
  },
  (table) => ({
    questProgressPk: primaryKey({ columns: [table.userId, table.questId] }),
    questProgressUserCompleted: index("quest_progress_user_completed_idx").on(
      table.userId,
      table.completed,
      table.claimed,
    ),
    questProgressDailyReset: index("quest_progress_daily_reset_idx").on(
      table.dailyResetDate,
    ),
    questProgressWeeklyReset: index("quest_progress_weekly_reset_idx").on(
      table.weeklyResetId,
    ),
  }),
);

export const petState = pgTable("pet_state", {
  userId: uuid("user_id")
    .primaryKey()
    .references(() => users.id, { onDelete: "cascade" }),
  ownedPetIds: jsonb("owned_pet_ids").notNull().default(["dog", "bird"]),
  equippedPetId: varchar("equipped_pet_id", { length: 64 }),
  updatedAt: timestamp("updated_at", { withTimezone: true }).defaultNow(),
});

export const seasonState = pgTable(
  "season_state",
  {
    userId: uuid("user_id")
      .notNull()
      .references(() => users.id, { onDelete: "cascade" }),
    seasonId: integer("season_id").notNull().default(1),
    tokens: integer("tokens").notNull().default(0),
    claimedTiers: jsonb("claimed_tiers").notNull().default([]),
    updatedAt: timestamp("updated_at", { withTimezone: true }).defaultNow(),
  },
  (table) => ({
    seasonStatePk: primaryKey({ columns: [table.userId, table.seasonId] }),
    seasonStateTokens: index("season_state_tokens_idx").on(
      table.seasonId,
      table.tokens,
    ),
  }),
);

// 길드 본체. member_count 는 멤버 수 denorm(가입/탈퇴 트랜잭션에서 원자적 ±1).
export const guilds = pgTable("guilds", {
  id: uuid("id").primaryKey(),
  name: varchar("name", { length: 16 }).notNull().unique(),
  notice: text("notice").notNull().default(""),
  joinMode: text("join_mode").notNull().default("open"),
  level: integer("level").notNull().default(1),
  exp: bigint("exp", { mode: "bigint" }).notNull().default(0n),
  masterCharacterId: uuid("master_character_id")
    .notNull()
    .references(() => characters.id, { onDelete: "cascade" }),
  memberCount: integer("member_count").notNull().default(1),
  weekId: text("week_id"),
  createdAt: timestamp("created_at", { withTimezone: true }).defaultNow(),
});

// 멤버십·기여. character_id PK = 1캐릭터 1길드 보장.
export const guildMembers = pgTable(
  "guild_members",
  {
    guildId: uuid("guild_id")
      .notNull()
      .references(() => guilds.id, { onDelete: "cascade" }),
    characterId: uuid("character_id")
      .primaryKey()
      .references(() => characters.id, { onDelete: "cascade" }),
    rank: text("rank").notNull().default("member"),
    joinedAt: timestamp("joined_at", { withTimezone: true }).defaultNow(),
    weeklyContribution: bigint("weekly_contribution", { mode: "bigint" })
      .notNull()
      .default(0n),
    totalContribution: bigint("total_contribution", { mode: "bigint" })
      .notNull()
      .default(0n),
    contributionPoints: bigint("contribution_points", { mode: "bigint" })
      .notNull()
      .default(0n),
    lastAttendanceDate: text("last_attendance_date"),
    // 자동 기여 주간 상한 추적(새 week_id 진입 시 0 으로 리셋).
    weeklyAutoContribution: bigint("weekly_auto_contribution", {
      mode: "bigint",
    })
      .notNull()
      .default(0n),
    // 헌납 일일 상한 추적(UTC date 변경 시 0 으로 리셋).
    lastDonationDate: text("last_donation_date"),
    dailyDonation: bigint("daily_donation", { mode: "bigint" })
      .notNull()
      .default(0n),
    // 멤버별 주간 리셋 마커(ISO week). 멤버 활동 시 주가 바뀌면 weekly 컬럼을 lazy 리셋.
    weeklyResetId: text("weekly_reset_id"),
    // PR-G3: 주간 보스 도전 횟수(weekly_reset_id 기준 lazy 리셋).
    weeklyBossChallenges: integer("weekly_boss_challenges")
      .notNull()
      .default(0),
    // PR-G3: 보스 격파 보상 수령 마일스톤(boss_claim_week_id 가 현재 주와 다르면 0 부터).
    bossClaimedCount: integer("boss_claimed_count").notNull().default(0),
    bossClaimWeekId: text("boss_claim_week_id"),
  },
  (table) => ({
    guildMembersGuild: index("guild_members_guild_idx").on(table.guildId),
  }),
);

// PR-G3: 공유 HP 풀 길드 보스(길드당 1행, 주간 단위 누적/격파).
export const guildBoss = pgTable("guild_boss", {
  guildId: uuid("guild_id")
    .primaryKey()
    .references(() => guilds.id, { onDelete: "cascade" }),
  weekId: text("week_id").notNull(),
  accumDamage: bigint("accum_damage", { mode: "bigint" }).notNull().default(0n),
  defeatedCount: integer("defeated_count").notNull().default(0),
  updatedAt: timestamp("updated_at", { withTimezone: true }).defaultNow(),
});

// PR-G3: 보스 데미지 기여(주간 내부 랭킹용).
export const guildBossContrib = pgTable(
  "guild_boss_contrib",
  {
    guildId: uuid("guild_id")
      .notNull()
      .references(() => guilds.id, { onDelete: "cascade" }),
    weekId: text("week_id").notNull(),
    characterId: uuid("character_id")
      .notNull()
      .references(() => characters.id, { onDelete: "cascade" }),
    damage: bigint("damage", { mode: "bigint" }).notNull().default(0n),
  },
  (table) => ({
    guildBossContribPk: primaryKey({
      columns: [table.guildId, table.weekId, table.characterId],
    }),
    guildBossContribGuildWeek: index("guild_boss_contrib_guild_week").on(
      table.guildId,
      table.weekId,
      table.damage,
    ),
  }),
);

// 승인제(approval) 가입 신청 큐.
export const guildJoinRequests = pgTable(
  "guild_join_requests",
  {
    guildId: uuid("guild_id")
      .notNull()
      .references(() => guilds.id, { onDelete: "cascade" }),
    characterId: uuid("character_id")
      .notNull()
      .references(() => characters.id, { onDelete: "cascade" }),
    requestedAt: timestamp("requested_at", { withTimezone: true }).defaultNow(),
  },
  (table) => ({
    guildJoinRequestsPk: primaryKey({
      columns: [table.guildId, table.characterId],
    }),
    guildJoinRequestsCharacter: index("guild_join_requests_character_idx").on(
      table.characterId,
    ),
  }),
);
