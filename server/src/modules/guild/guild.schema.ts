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

// ── PR-G2: 기여/상점 ─────────────────────────────────────────────────────────

const guildShopItemParams = {
  type: "object",
  required: ["id", "itemId"],
  additionalProperties: false,
  properties: { id: uuid, itemId: { type: "string", maxLength: 64 } },
} as const;

// 일일 출석(멤버 본인) — body 는 characterId 만.
export const guildAttendanceSchema = {
  params: guildIdParams,
  body: characterBody,
  response: okResponse,
} as const;

// 재화 헌납 — 클라 권위 골드(서버 미차감). 일일 상한으로 어뷰즈 억제.
export const guildDonateSchema = {
  params: guildIdParams,
  body: {
    type: "object",
    required: ["characterId", "gold"],
    additionalProperties: false,
    properties: {
      characterId: uuid,
      gold: { type: "integer", minimum: 0 },
    },
  },
  response: okResponse,
} as const;

// 전투/던전 자동 기여 델타 플러시 — 주간 상한 검증.
export const guildContributeSchema = {
  params: guildIdParams,
  body: {
    type: "object",
    required: ["characterId", "amount"],
    additionalProperties: false,
    properties: {
      characterId: uuid,
      amount: { type: "integer", minimum: 0 },
    },
  },
  response: okResponse,
} as const;

// 길드 상점 카탈로그 조회(멤버) — querystring 으로 characterId.
export const guildShopSchema = {
  params: guildIdParams,
  querystring: characterBody,
  response: okResponse,
} as const;

// 길드 상점 구매 — 포인트 차감 + 보상 반환(소비형).
export const guildShopBuySchema = {
  params: guildShopItemParams,
  body: characterBody,
  response: okResponse,
} as const;

// ── PR-G3: 길드 보스 / 주간 랭킹 ─────────────────────────────────────────────

// 보스 도전 — CP 는 클라 권위(서버는 신뢰하되 주간 도전 횟수로 어뷰즈 억제).
export const guildBossChallengeSchema = {
  params: guildIdParams,
  body: {
    type: "object",
    required: ["characterId", "cp"],
    additionalProperties: false,
    properties: {
      characterId: uuid,
      cp: { type: "integer", minimum: 0 },
    },
  },
  response: okResponse,
} as const;

// 보스 격파 보상 수령(멤버 본인) — body 는 characterId 만.
export const guildBossClaimSchema = {
  params: guildIdParams,
  body: characterBody,
  response: okResponse,
} as const;

// 보스 상태 조회(멤버) — querystring 으로 characterId.
export const guildBossSchema = {
  params: guildIdParams,
  querystring: characterBody,
  response: okResponse,
} as const;

// 주간 길드 랭킹(누구나) — characterId 지정 시 내 길드 순위도 반환.
export const guildRankingsSchema = {
  querystring: {
    type: "object",
    additionalProperties: false,
    properties: {
      characterId: uuid,
      limit: { type: "integer", minimum: 1, maximum: 100 },
    },
  },
  response: okResponse,
} as const;
