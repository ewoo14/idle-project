export const leaderboardQuerySchema = {
  querystring: {
    type: "object",
    required: ["season"],
    properties: {
      season: { type: "integer", minimum: 1 },
      limit: { type: "integer", minimum: 1, maximum: 100, default: 100 },
    },
  },
} as const;

export const myRankQuerySchema = {
  querystring: {
    type: "object",
    required: ["season", "characterId"],
    properties: {
      season: { type: "integer", minimum: 1 },
      characterId: { type: "string", format: "uuid" },
    },
  },
} as const;

export const weeklyQuerySchema = {
  querystring: {
    type: "object",
    required: ["week"],
    properties: {
      week: { type: "string", minLength: 1, maxLength: 32 },
      limit: { type: "integer", minimum: 1, maximum: 100, default: 100 },
    },
  },
} as const;

export const weeklyMyRankQuerySchema = {
  querystring: {
    type: "object",
    required: ["week", "characterId"],
    properties: {
      week: { type: "string", minLength: 1, maxLength: 32 },
      characterId: { type: "string", format: "uuid" },
    },
  },
} as const;
