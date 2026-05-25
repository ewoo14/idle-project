import { describe, expect, it, vi } from "vitest";
import { AuthError } from "../../core/errors.js";
import { AuthService } from "./auth.service.js";

const user = {
  id: "00000000-0000-0000-0000-000000000001",
  email: "hero@example.com",
  nickname: "전사",
  passwordHash: "hash",
};

describe("AuthService", () => {
  it("비밀번호가 맞으면 access/refresh 토큰을 발급한다", async () => {
    const service = new AuthService({
      users: {
        createUser: vi.fn(),
        findByEmail: vi.fn().mockResolvedValue(user),
        touchLastLogin: vi.fn(),
      },
      password: {
        hash: vi.fn(),
        compare: vi.fn().mockResolvedValue(true),
      },
      tokens: {
        signAccess: vi.fn().mockResolvedValue("access"),
        signRefresh: vi.fn().mockResolvedValue("refresh"),
        verifyRefresh: vi.fn(),
      },
      tokenStore: {
        rememberRefresh: vi.fn(),
        isRefreshRevoked: vi.fn(),
        revokeRefresh: vi.fn(),
      },
    });

    const result = await service.login({
      email: user.email,
      password: "Password123!",
    });

    expect(result.accessToken).toBe("access");
    expect(result.refreshToken).toBe("refresh");
  });

  it("비밀번호가 틀리면 401을 던진다", async () => {
    const service = new AuthService({
      users: {
        createUser: vi.fn(),
        findByEmail: vi.fn().mockResolvedValue(user),
        touchLastLogin: vi.fn(),
      },
      password: {
        hash: vi.fn(),
        compare: vi.fn().mockResolvedValue(false),
      },
      tokens: {
        signAccess: vi.fn(),
        signRefresh: vi.fn(),
        verifyRefresh: vi.fn(),
      },
      tokenStore: {
        rememberRefresh: vi.fn(),
        isRefreshRevoked: vi.fn(),
        revokeRefresh: vi.fn(),
      },
    });

    await expect(
      service.login({ email: user.email, password: "bad" }),
    ).rejects.toBeInstanceOf(AuthError);
  });

  it("폐기된 refresh 토큰은 갱신하지 않는다", async () => {
    const service = new AuthService({
      users: {
        createUser: vi.fn(),
        findByEmail: vi.fn(),
        touchLastLogin: vi.fn(),
      },
      password: {
        hash: vi.fn(),
        compare: vi.fn(),
      },
      tokens: {
        signAccess: vi.fn(),
        signRefresh: vi.fn(),
        verifyRefresh: vi.fn().mockResolvedValue({
          sub: user.id,
          jti: "old",
          email: user.email,
          nickname: user.nickname,
        }),
      },
      tokenStore: {
        rememberRefresh: vi.fn(),
        isRefreshRevoked: vi.fn().mockResolvedValue(true),
        revokeRefresh: vi.fn(),
      },
    });

    await expect(service.refresh("refresh")).rejects.toBeInstanceOf(AuthError);
  });
});
