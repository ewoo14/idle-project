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
