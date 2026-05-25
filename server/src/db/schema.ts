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
  stats: jsonb("stats").notNull().default({}),
  skillTree: jsonb("skill_tree").notNull().default({}),
  inventory: jsonb("inventory").notNull().default([]),
  gold: integer("gold").notNull().default(0),
  totalExp: integer("total_exp").notNull().default(0),
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
  }),
);
