const rewardResponse = {
  type: "object",
  required: ["cappedSeconds", "gold", "exp", "timeBonusMultiplier"],
  properties: {
    cappedSeconds: { type: "integer", minimum: 0 },
    gold: { type: "integer", minimum: 0 },
    exp: { type: "integer", minimum: 0 },
    timeBonusMultiplier: { type: "number", minimum: 1 },
  },
} as const;

export const offlinePreviewSchema = {
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
          required: ["characterId", "lastSeenUnixSec", "nowUnixSec", "rewards"],
          properties: {
            characterId: { type: "string", format: "uuid" },
            lastSeenUnixSec: { type: "integer", minimum: 0 },
            nowUnixSec: { type: "integer", minimum: 0 },
            rewards: rewardResponse,
          },
        },
      },
    },
  },
} as const;

export const offlineClaimSchema = {
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
          required: [
            "characterId",
            "lastSeenUnixSec",
            "nowUnixSec",
            "rewards",
            "totals",
          ],
          properties: {
            characterId: { type: "string", format: "uuid" },
            lastSeenUnixSec: { type: "integer", minimum: 0 },
            nowUnixSec: { type: "integer", minimum: 0 },
            rewards: rewardResponse,
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
