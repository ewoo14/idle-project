const seasonTierResponse = {
  type: "object",
  required: [
    "tier",
    "requiredTokens",
    "rewardType",
    "rewardAmount",
    "unlocked",
    "claimed",
  ],
  properties: {
    tier: { type: "integer", minimum: 1 },
    requiredTokens: { type: "integer", minimum: 0 },
    rewardType: { type: "string", enum: ["gold", "exp"] },
    rewardAmount: { type: "integer", minimum: 0 },
    unlocked: { type: "boolean" },
    claimed: { type: "boolean" },
  },
} as const;

const seasonProgressData = {
  type: "object",
  required: ["seasonId", "tokens", "tiers"],
  properties: {
    seasonId: { type: "integer", minimum: 1 },
    tokens: { type: "integer", minimum: 0 },
    tiers: { type: "array", items: seasonTierResponse },
  },
} as const;

export const seasonProgressSchema = {
  response: {
    200: {
      type: "object",
      required: ["ok", "data"],
      properties: {
        ok: { type: "boolean" },
        data: seasonProgressData,
      },
    },
  },
} as const;

export const seasonAddTokensSchema = {
  body: {
    type: "object",
    required: ["characterId", "amount"],
    additionalProperties: false,
    properties: {
      characterId: { type: "string", format: "uuid" },
      amount: { type: "integer", minimum: 1 },
    },
  },
  response: seasonProgressSchema.response,
} as const;

export const seasonClaimSchema = {
  params: {
    type: "object",
    required: ["tier"],
    additionalProperties: false,
    properties: {
      tier: { type: "integer", minimum: 1 },
    },
  },
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
          ...seasonProgressData,
          required: [
            "seasonId",
            "tokens",
            "tiers",
            "claimedTier",
            "reward",
            "totals",
          ],
          properties: {
            ...seasonProgressData.properties,
            claimedTier: { type: "integer", minimum: 1 },
            reward: {
              type: "object",
              required: ["rewardType", "rewardAmount"],
              properties: {
                rewardType: { type: "string", enum: ["gold", "exp"] },
                rewardAmount: { type: "integer", minimum: 0 },
              },
            },
            totals: {
              type: "object",
              required: ["gold", "totalExp"],
              properties: {
                gold: { type: "integer", minimum: 0 },
                totalExp: { type: "integer", minimum: 0 },
              },
            },
          },
        },
      },
    },
  },
} as const;
