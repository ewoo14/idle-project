export const getSaveSchema = {
  querystring: {
    type: "object",
    required: ["characterId"],
    properties: {
      characterId: { type: "string", format: "uuid" },
    },
  },
} as const;

export const putSaveSchema = {
  body: {
    type: "object",
    required: ["characterId", "version", "payload"],
    additionalProperties: false,
    properties: {
      characterId: { type: "string", format: "uuid" },
      version: { type: "integer", minimum: 1 },
      payload: {
        type: "object",
        required: ["level", "rebirthCount", "maxEquipmentGrade"],
        additionalProperties: true,
        properties: {
          level: { type: "integer", minimum: 1, maximum: 200 },
          rebirthCount: { type: "integer", minimum: 0 },
          maxEquipmentGrade: { type: "integer", minimum: 0, maximum: 5 },
          totalExp: { type: "number", minimum: 0 },
          gold: { type: "integer", minimum: 0 },
          lastSeenUnixSec: { type: "integer", minimum: 0 },
        },
      },
    },
  },
} as const;

export const historySchema = {
  querystring: {
    type: "object",
    required: ["characterId"],
    properties: {
      characterId: { type: "string", format: "uuid" },
      limit: { type: "integer", minimum: 1, maximum: 50, default: 10 },
    },
  },
} as const;
