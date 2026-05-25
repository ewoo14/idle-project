const petResponse = {
  type: "object",
  required: ["petId", "name", "bonusType", "bonusPercent", "owned", "equipped"],
  properties: {
    petId: { type: "string" },
    name: { type: "string" },
    bonusType: { type: "string", enum: ["gold", "drop"] },
    bonusPercent: { type: "integer", minimum: 0 },
    owned: { type: "boolean" },
    equipped: { type: "boolean" },
  },
} as const;

const petListData = {
  type: "object",
  required: ["equippedPetId", "activeBonus", "pets"],
  properties: {
    equippedPetId: { anyOf: [{ type: "string" }, { type: "null" }] },
    activeBonus: {
      anyOf: [
        {
          type: "object",
          required: ["bonusType", "bonusPercent"],
          properties: {
            bonusType: { type: "string", enum: ["gold", "drop"] },
            bonusPercent: { type: "integer", minimum: 0 },
          },
        },
        { type: "null" },
      ],
    },
    pets: { type: "array", items: petResponse },
  },
} as const;

export const petListSchema = {
  response: {
    200: {
      type: "object",
      required: ["ok", "data"],
      properties: {
        ok: { type: "boolean" },
        data: petListData,
      },
    },
  },
} as const;

export const petEquipSchema = {
  params: {
    type: "object",
    required: ["id"],
    additionalProperties: false,
    properties: {
      id: { type: "string" },
    },
  },
  response: petListSchema.response,
} as const;
