// 길드 라우트 Fastify JSON 스키마(요청 검증 중심). 응답은 {ok,data} 래퍼.

const uuid = { type: "string", format: "uuid" } as const;

const characterBody = {
  type: "object",
  required: ["characterId"],
  additionalProperties: false,
  properties: { characterId: uuid },
} as const;

const guildIdParams = {
  type: "object",
  required: ["id"],
  additionalProperties: false,
  properties: { id: uuid },
} as const;

const guildMemberParams = {
  type: "object",
  required: ["id", "charId"],
  additionalProperties: false,
  properties: { id: uuid, charId: uuid },
} as const;

const okResponse = {
  200: {
    type: "object",
    required: ["ok"],
    properties: { ok: { type: "boolean" } },
  },
} as const;

export const guildCreateSchema = {
  body: {
    type: "object",
    required: ["characterId", "name"],
    additionalProperties: false,
    properties: {
      characterId: uuid,
      name: { type: "string", minLength: 2, maxLength: 16 },
    },
  },
  response: okResponse,
} as const;

export const guildSnapshotSchema = {
  querystring: characterBody,
  response: okResponse,
} as const;

export const guildGetSchema = {
  params: guildIdParams,
  response: okResponse,
} as const;

export const guildListSchema = {
  querystring: {
    type: "object",
    additionalProperties: false,
    properties: {
      q: { type: "string", maxLength: 32 },
      limit: { type: "integer", minimum: 1, maximum: 100 },
      offset: { type: "integer", minimum: 0 },
    },
  },
  response: okResponse,
} as const;

export const guildJoinSchema = {
  params: guildIdParams,
  body: characterBody,
  response: okResponse,
} as const;

export const guildLeaveSchema = {
  params: guildIdParams,
  body: characterBody,
  response: okResponse,
} as const;

export const guildRequestActionSchema = {
  params: guildMemberParams,
  body: characterBody,
  response: okResponse,
} as const;

export const guildUpdateSchema = {
  params: guildIdParams,
  body: {
    type: "object",
    required: ["characterId"],
    additionalProperties: false,
    properties: {
      characterId: uuid,
      name: { type: "string", minLength: 2, maxLength: 16 },
      notice: { type: "string", maxLength: 500 },
      joinMode: { type: "string", enum: ["open", "approval"] },
    },
  },
  response: okResponse,
} as const;

export const guildRankSchema = {
  params: guildMemberParams,
  body: {
    type: "object",
    required: ["characterId", "rank"],
    additionalProperties: false,
    properties: {
      characterId: uuid,
      rank: { type: "string", enum: ["vice", "officer", "member"] },
    },
  },
  response: okResponse,
} as const;
