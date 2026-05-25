import type {
  FastifyError,
  FastifyInstance,
  FastifyReply,
  FastifyRequest,
} from "fastify";

export type ErrorCode =
  | "VALIDATION_ERROR"
  | "AUTH_ERROR"
  | "NOT_FOUND"
  | "RATE_LIMITED"
  | "CONFLICT"
  | "INTERNAL_ERROR"
  | (string & {});

export type AppErrorOptions = {
  code?: ErrorCode;
};

export class AppError extends Error {
  constructor(
    message: string,
    public readonly statusCode = 500,
    public readonly code: ErrorCode = "INTERNAL_ERROR",
  ) {
    super(message);
  }
}

export class ValidationError extends AppError {
  constructor(
    message = "요청 값이 올바르지 않습니다.",
    options: AppErrorOptions = {},
  ) {
    super(message, 400, options.code ?? "VALIDATION_ERROR");
  }
}

export class AuthError extends AppError {
  constructor(message = "인증에 실패했습니다.", options: AppErrorOptions = {}) {
    super(message, 401, options.code ?? "AUTH_ERROR");
  }
}

export class NotFoundError extends AppError {
  constructor(
    message = "대상을 찾을 수 없습니다.",
    options: AppErrorOptions = {},
  ) {
    super(message, 404, options.code ?? "NOT_FOUND");
  }
}

export class RateLimitError extends AppError {
  constructor(
    message = "요청 한도를 초과했습니다.",
    options: AppErrorOptions = {},
  ) {
    super(message, 429, options.code ?? "RATE_LIMITED");
  }
}

export class ConflictError extends AppError {
  constructor(
    message = "이미 존재하는 리소스입니다.",
    options: AppErrorOptions = {},
  ) {
    super(message, 409, options.code ?? "CONFLICT");
  }
}

export function sendError(
  reply: FastifyReply,
  error: AppError,
  requestId?: string,
) {
  return reply.status(error.statusCode).send({
    ok: false,
    error: {
      code: error.code,
      message: error.message,
      requestId,
    },
  });
}

export function registerErrorHandler(app: FastifyInstance) {
  app.setErrorHandler(
    (
      error: FastifyError | AppError,
      request: FastifyRequest,
      reply: FastifyReply,
    ) => {
      const requestId = request.id;
      if (error instanceof AppError) {
        request.log.warn({ err: error, requestId }, "domain error");
        return sendError(reply, error, requestId);
      }

      const statusCode = error.statusCode ?? 500;
      const appError =
        statusCode === 400
          ? new ValidationError(error.message)
          : statusCode === 429
            ? new RateLimitError(error.message, { code: "AUTH_RATE_LIMITED" })
            : new AppError(
                statusCode >= 500 ? "서버 오류가 발생했습니다." : error.message,
                statusCode,
                statusCode >= 500 ? "SYSTEM_INTERNAL" : "INTERNAL_ERROR",
              );
      request.log.error({ err: error, requestId }, "unhandled error");
      return sendError(reply, appError, requestId);
    },
  );
}
