import { describe, expect, it, vi } from "vitest";
import { AuthError } from "../../core/errors.js";
import { type AuthDeps, AuthService } from "./auth.service.js";

const user = {
  id: "00000000-0000-0000-0000-000000000001",
  email: "hero@example.com",
  nickname: "hero",
  passwordHash: "hash",
};

describe("AuthService", () => {
  it("issues access and refresh tokens when password matches", async () => {
    const service = new AuthService(
      createDeps({
        findByEmail: vi.fn().mockResolvedValue(user),
        compare: vi.fn().mockResolvedValue(true),
      }),
    );

    const result = await service.login({
      email: user.email,
      password: "Password123!",
    });

    expect(result.accessToken).toBe("access");
    expect(result.refreshToken).toBe("refresh");
  });

  it("rejects login when password does not match", async () => {
    const service = new AuthService(
      createDeps({
        findByEmail: vi.fn().mockResolvedValue(user),
        compare: vi.fn().mockResolvedValue(false),
      }),
    );

    await expect(
      service.login({ email: user.email, password: "bad" }),
    ).rejects.toBeInstanceOf(AuthError);
  });

  it("rejects revoked refresh tokens", async () => {
    const service = new AuthService(
      createDeps({
        verifyRefresh: vi.fn().mockResolvedValue({
          sub: user.id,
          jti: "old",
          email: user.email,
          nickname: user.nickname,
          typ: "refresh",
        }),
        isRefreshRevoked: vi.fn().mockResolvedValue(true),
      }),
    );

    await expect(service.refresh("refresh")).rejects.toBeInstanceOf(AuthError);
  });

  it("rejects an access token used as a refresh token", async () => {
    const service = new AuthService(
      createDeps({
        verifyRefresh: vi.fn().mockResolvedValue({
          sub: user.id,
          jti: "access-jti",
          email: user.email,
          nickname: user.nickname,
          typ: "access",
        }),
        isRefreshActive: vi.fn().mockResolvedValue(true),
      }),
    );

    await expect(service.refresh("access-token")).rejects.toBeInstanceOf(
      AuthError,
    );
    await expect(service.logout("access-token")).rejects.toBeInstanceOf(
      AuthError,
    );
  });

  it("rejects refresh when the jti is not active", async () => {
    const service = new AuthService(
      createDeps({
        verifyRefresh: vi.fn().mockResolvedValue({
          sub: user.id,
          jti: "missing-jti",
          email: user.email,
          nickname: user.nickname,
          typ: "refresh",
        }),
        isRefreshActive: vi.fn().mockResolvedValue(false),
      }),
    );

    await expect(service.refresh("refresh")).rejects.toBeInstanceOf(AuthError);
  });
});

function createDeps(
  overrides: {
    findByEmail?: AuthDeps["users"]["findByEmail"];
    compare?: AuthDeps["password"]["compare"];
    verifyRefresh?: AuthDeps["tokens"]["verifyRefresh"];
    isRefreshRevoked?: AuthDeps["tokenStore"]["isRefreshRevoked"];
    isRefreshActive?: AuthDeps["tokenStore"]["isRefreshActive"];
  } = {},
): AuthDeps {
  return {
    users: {
      createUser: vi.fn(),
      findByEmail: overrides.findByEmail ?? vi.fn(),
      touchLastLogin: vi.fn(),
    },
    password: {
      hash: vi.fn(),
      compare: overrides.compare ?? vi.fn(),
    },
    tokens: {
      signAccess: vi.fn().mockResolvedValue("access"),
      signRefresh: vi.fn().mockResolvedValue("refresh"),
      verifyRefresh: overrides.verifyRefresh ?? vi.fn(),
    },
    tokenStore: {
      rememberRefresh: vi.fn(),
      isRefreshRevoked:
        overrides.isRefreshRevoked ?? vi.fn().mockResolvedValue(false),
      isRefreshActive:
        overrides.isRefreshActive ?? vi.fn().mockResolvedValue(true),
      revokeRefresh: vi.fn(),
    },
  };
}
