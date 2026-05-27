export const createCharacterSchema = {
  body: {
    type: "object",
    required: ["classId"],
    additionalProperties: false,
    properties: {
      classId: { type: "integer", minimum: 1, maximum: 8 },
    },
  },
} as const;

export const getCharacterSchema = {
  params: {
    type: "object",
    required: ["id"],
    properties: {
      id: { type: "string", format: "uuid" },
    },
  },
} as const;

export const rebirthCharacterSchema = {
  body: {
    type: "object",
    required: ["characterId"],
    additionalProperties: false,
    properties: {
      characterId: { type: "string", format: "uuid" },
    },
  },
} as const;
