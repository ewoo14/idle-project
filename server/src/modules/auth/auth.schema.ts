export const registerSchema = {
  body: {
    type: "object",
    required: ["email", "password", "nickname"],
    additionalProperties: false,
    properties: {
      email: { type: "string", format: "email", maxLength: 254 },
      password: { type: "string", minLength: 12, maxLength: 128 },
      nickname: { type: "string", minLength: 2, maxLength: 16 },
    },
  },
} as const;

export const loginSchema = {
  body: {
    type: "object",
    required: ["email", "password"],
    additionalProperties: false,
    properties: {
      email: { type: "string", format: "email", maxLength: 254 },
      password: { type: "string", minLength: 1, maxLength: 128 },
    },
  },
} as const;

export const refreshSchema = {
  body: {
    type: "object",
    required: ["refreshToken"],
    additionalProperties: false,
    properties: {
      refreshToken: { type: "string", minLength: 20 },
    },
  },
} as const;

export const logoutSchema = refreshSchema;
