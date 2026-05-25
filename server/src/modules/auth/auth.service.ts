import { randomUUID } from "node:crypto";
import { AuthError, ConflictError } from "../../core/errors.js";

export type AuthUser = {
  id: string;
  email: string;
  nickname: string;
  passwordHash: string;
};

export type AuthDeps = {
  users: {
    createUser(input: {
      email: string;
      nickname: string;
      passwordHash: string;
    }): Promise<AuthUser>;
    findByEmail(email: string): Promise<AuthUser | null>;
    touchLastLogin(userId: string): Promise<void>;
  };
  password: {
    hash(password: string): Promise<string>;
    compare(password: string, hash: string): Promise<boolean>;
  };
  tokens: {
    signAccess(user: AuthUser): Promise<string>;
    signRefresh(user: AuthUser, jti: string): Promise<string>;
    verifyRefresh(
      token: string,
    ): Promise<{ sub: string; jti: string; email: string; nickname: string }>;
  };
  tokenStore: {
    rememberRefresh(
      userId: string,
      jti: string,
      ttlSeconds: number,
    ): Promise<void>;
    isRefreshRevoked(jti: string): Promise<boolean>;
    revokeRefresh(jti: string, ttlSeconds: number): Promise<void>;
  };
};

export class AuthService {
  constructor(private readonly deps: AuthDeps) {}

  async register(input: { email: string; password: string; nickname: string }) {
    const exists = await this.deps.users.findByEmail(input.email.toLowerCase());
    if (exists) {
      throw new ConflictError("이미 가입된 이메일입니다.");
    }
    const passwordHash = await this.deps.password.hash(input.password);
    const user = await this.deps.users.createUser({
      email: input.email.toLowerCase(),
      nickname: input.nickname,
      passwordHash,
    });
    return this.issueTokens(user);
  }

  async login(input: { email: string; password: string }) {
    const user = await this.deps.users.findByEmail(input.email.toLowerCase());
    if (!user) {
      throw new AuthError("이메일 또는 비밀번호가 올바르지 않습니다.");
    }
    const ok = await this.deps.password.compare(
      input.password,
      user.passwordHash,
    );
    if (!ok) {
      throw new AuthError("이메일 또는 비밀번호가 올바르지 않습니다.");
    }
    await this.deps.users.touchLastLogin(user.id);
    return this.issueTokens(user);
  }

  async refresh(refreshToken: string) {
    const payload = await this.deps.tokens.verifyRefresh(refreshToken);
    if (await this.deps.tokenStore.isRefreshRevoked(payload.jti)) {
      throw new AuthError("폐기된 refresh 토큰입니다.");
    }
    await this.deps.tokenStore.revokeRefresh(payload.jti, refreshTtlSeconds);
    const user: AuthUser = {
      id: payload.sub,
      email: payload.email,
      nickname: payload.nickname,
      passwordHash: "",
    };
    return this.issueTokens(user);
  }

  async logout(refreshToken: string) {
    const payload = await this.deps.tokens.verifyRefresh(refreshToken);
    await this.deps.tokenStore.revokeRefresh(payload.jti, refreshTtlSeconds);
    return { loggedOut: true };
  }

  private async issueTokens(user: AuthUser) {
    const jti = randomUUID();
    const [accessToken, refreshToken] = await Promise.all([
      this.deps.tokens.signAccess(user),
      this.deps.tokens.signRefresh(user, jti),
    ]);
    await this.deps.tokenStore.rememberRefresh(user.id, jti, refreshTtlSeconds);
    return {
      user: { id: user.id, email: user.email, nickname: user.nickname },
      accessToken,
      refreshToken,
      tokenType: "Bearer",
      expiresIn: 900,
    };
  }
}

export const refreshTtlSeconds = 60 * 60 * 24 * 30;
