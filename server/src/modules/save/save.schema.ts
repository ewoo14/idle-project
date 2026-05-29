export const SAVE_LEVEL_MAX = 1000;
export const SAVE_MAX_EQUIPMENT_GRADE = 7;

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
          level: { type: "integer", minimum: 1, maximum: SAVE_LEVEL_MAX },
          rebirthCount: { type: "integer", minimum: 0 },
          maxEquipmentGrade: {
            type: "integer",
            minimum: 0,
            maximum: SAVE_MAX_EQUIPMENT_GRADE,
          },
          totalExp: { type: "number", minimum: 0 },
          gold: { type: "integer", minimum: 0 },
          lastSeenUnixSec: { type: "integer", minimum: 0 },
          transcendCount: { type: "integer", minimum: 0 },
          towerHighestFloor: { type: "integer", minimum: 0 },
          skillPoints: { type: "integer", minimum: 0 },
          worldPower: { type: "integer", minimum: 0 },
          masteryLevels: {
            type: "array",
            items: { type: "integer", minimum: 0 },
            minItems: 0,
            maxItems: 6,
          },
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
