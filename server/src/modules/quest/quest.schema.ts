const questProgressResponse = {
  type: "object",
  required: [
    "questId",
    "type",
    "title",
    "objective",
    "targetCount",
    "rewardGold",
    "rewardExp",
    "progress",
    "completed",
    "claimed",
    "dailyResetDate",
    "weeklyResetId",
  ],
  properties: {
    questId: { type: "string" },
    type: { type: "string", enum: ["main", "daily", "weekly"] },
    title: { type: "string" },
    objective: {
      type: "string",
      enum: [
        "kill_monster",
        "clear_map",
        "claim_offline",
        "enhance",
        "defeat_boss",
        "rebirth",
        "transcend",
        "climb_tower",
        "reach_level",
        "spend_gold",
        "roll_gear_shop",
        "feed_pet",
      ],
    },
    targetCount: { type: "integer", minimum: 1 },
    rewardGold: { type: "integer", minimum: 0 },
    rewardExp: { type: "integer", minimum: 0 },
    prerequisiteQuestId: { type: "string" },
    chapterMapId: { type: "string" },
    progress: { type: "integer", minimum: 0 },
    completed: { type: "boolean" },
    claimed: { type: "boolean" },
    dailyResetDate: { anyOf: [{ type: "string" }, { type: "null" }] },
    weeklyResetId: { anyOf: [{ type: "string" }, { type: "null" }] },
  },
} as const;

export const questListSchema = {
  querystring: {
    type: "object",
    required: ["characterId"],
    additionalProperties: false,
    properties: {
      characterId: { type: "string", format: "uuid" },
    },
  },
  response: {
    200: {
      type: "object",
      required: ["ok", "data"],
      properties: {
        ok: { type: "boolean" },
        data: {
          type: "object",
          required: ["quests", "dailyResetDate", "weeklyResetId"],
          properties: {
            quests: { type: "array", items: questProgressResponse },
            dailyResetDate: { type: "string" },
            weeklyResetId: { type: "string" },
          },
        },
      },
    },
  },
} as const;

export const questProgressSchema = {
  params: {
    type: "object",
    required: ["id"],
    additionalProperties: false,
    properties: {
      id: { type: "string" },
    },
  },
  body: {
    type: "object",
    required: ["characterId", "amount"],
    additionalProperties: false,
    properties: {
      characterId: { type: "string", format: "uuid" },
      amount: { type: "integer", minimum: 1 },
    },
  },
  response: {
    200: {
      type: "object",
      required: ["ok", "data"],
      properties: {
        ok: { type: "boolean" },
        data: questProgressResponse,
      },
    },
  },
} as const;

export const questClaimSchema = {
  params: questProgressSchema.params,
  body: {
    type: "object",
    required: ["characterId"],
    additionalProperties: false,
    properties: {
      characterId: { type: "string", format: "uuid" },
    },
  },
  response: {
    200: {
      type: "object",
      required: ["ok", "data"],
      properties: {
        ok: { type: "boolean" },
        data: {
          type: "object",
          required: ["quest", "totals", "unlockedQuestIds"],
          properties: {
            quest: questProgressResponse,
            totals: {
              type: "object",
              required: ["gold", "totalExp"],
              properties: {
                gold: { type: "integer", minimum: 0 },
                totalExp: { type: "integer", minimum: 0 },
              },
            },
            unlockedQuestIds: { type: "array", items: { type: "string" } },
          },
        },
      },
    },
  },
} as const;
