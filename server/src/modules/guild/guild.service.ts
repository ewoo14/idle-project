import { randomUUID } from "node:crypto";
import {
  ConflictError,
  NotFoundError,
  ValidationError,
} from "../../core/errors.js";

// ── 계급/정원 상수 (클라 UE GuildFormula 와 parity 필수) ─────────────────────
export const GUILD_CAPACITY = 30; // v1 고정 정원 (길드 레벨 확장은 후속)
export const VICE_UNLOCK_AT = 11; // 부길드장 해금 멤버 수
export const OFFICER_UNLOCK_AT = 21; // 간부 해금 멤버 수
export const VICE_SLOT_CAP = 1; // 부길드장 최대 인원
export const OFFICER_SLOT_CAP = 3; // 간부 최대 인원
export const GUILD_NAME_MIN = 2;
export const GUILD_NAME_MAX = 16;

export type GuildRank = "master" | "vice" | "officer" | "member";
export type GuildJoinMode = "open" | "approval";

/**
 * 계급 해금 — 인원 기반 순수 함수(스펙 §4).
 * master/member 는 항상 사용 가능. vice 는 멤버 ≥11, officer 는 멤버 ≥21 부터 해금.
 * 클라(UE) `IsRankUnlocked` 와 동일 임계값을 유지한다.
 */
export function isRankUnlocked(rank: GuildRank, memberCount: number): boolean {
  switch (rank) {
    case "master":
    case "member":
      return true;
    case "vice":
      return memberCount >= VICE_UNLOCK_AT;
    case "officer":
      return memberCount >= OFFICER_UNLOCK_AT;
    default:
      return false;
  }
}

/** 계급별 정원(슬롯 상한). master/member 는 상한 없음(Number.POSITIVE_INFINITY). */
export function getRankSlotCap(rank: GuildRank): number {
  switch (rank) {
    case "vice":
      return VICE_SLOT_CAP;
    case "officer":
      return OFFICER_SLOT_CAP;
    default:
      return Number.POSITIVE_INFINITY;
  }
}

// ── 레코드 타입 ──────────────────────────────────────────────────────────────
export type GuildRecord = {
  id: string;
  name: string;
  notice: string;
  joinMode: GuildJoinMode;
  level: number;
  exp: bigint;
  masterCharacterId: string;
  memberCount: number;
  weekId: string | null;
  createdAt: Date;
};

export type GuildMemberRecord = {
  guildId: string;
  characterId: string;
  rank: GuildRank;
  joinedAt: Date;
  weeklyContribution: bigint;
  totalContribution: bigint;
  contributionPoints: bigint;
  lastAttendanceDate: string | null;
};

export type GuildJoinRequestRecord = {
  guildId: string;
  characterId: string;
  requestedAt: Date;
};

export type GuildRepo = {
  findCharacter(
    userId: string,
    characterId: string,
  ): Promise<{ id: string; userId: string } | null>;
  getGuild(guildId: string): Promise<GuildRecord | null>;
  getGuildByName(name: string): Promise<GuildRecord | null>;
  listGuilds(input: {
    limit: number;
    offset: number;
    q?: string;
  }): Promise<GuildRecord[]>;
  getMembership(characterId: string): Promise<GuildMemberRecord | null>;
  listMembers(guildId: string): Promise<GuildMemberRecord[]>;
  listRequests(guildId: string): Promise<GuildJoinRequestRecord[]>;
  getRequest(
    guildId: string,
    characterId: string,
  ): Promise<GuildJoinRequestRecord | null>;
  /** 길드 생성: guilds insert + master 멤버 insert(member_count=1)를 한 트랜잭션으로. */
  createGuild(input: {
    id: string;
    name: string;
    masterCharacterId: string;
    now: Date;
  }): Promise<GuildRecord>;
  /** 멤버 가입: guild_members insert + member_count +1 원자적. 정원/중복 충돌 시 null. */
  addMember(input: {
    guildId: string;
    characterId: string;
    capacity: number;
    now: Date;
  }): Promise<GuildMemberRecord | null>;
  /** 멤버 탈퇴: guild_members delete + member_count -1 원자적. */
  removeMember(characterId: string): Promise<boolean>;
  /** 승인 신청 등록. 이미 있으면 무시(idempotent). */
  insertRequest(input: {
    guildId: string;
    characterId: string;
    now: Date;
  }): Promise<void>;
  deleteRequest(guildId: string, characterId: string): Promise<boolean>;
  /**
   * 승인: 신청 행 삭제 + 멤버 insert + member_count +1 을 한 트랜잭션으로(정원 검증).
   * 정원 초과/중복/신청 없음 시 null.
   */
  approveRequest(input: {
    guildId: string;
    characterId: string;
    capacity: number;
    now: Date;
  }): Promise<GuildMemberRecord | null>;
  setRank(
    guildId: string,
    characterId: string,
    rank: GuildRank,
  ): Promise<GuildMemberRecord | null>;
  updateGuild(
    guildId: string,
    settings: { name?: string; notice?: string; joinMode?: GuildJoinMode },
  ): Promise<GuildRecord | null>;
  /** 길드장 위임: 신/구 길드장 rank 교체(트랜잭션). */
  transferMaster(input: {
    guildId: string;
    fromCharacterId: string;
    toCharacterId: string;
  }): Promise<void>;
  /** 해산: 길드·멤버·신청 행 전부 삭제(cascade). */
  disbandGuild(guildId: string): Promise<void>;
};

export class GuildService {
  constructor(
    private readonly repo: GuildRepo,
    private readonly now: () => Date = () => new Date(),
  ) {}

  /** 길드 생성: 무소속만·이름 2~16·유니크, 생성자=master. */
  async create(userId: string, characterId: string, rawName: string) {
    await this.assertCharacter(userId, characterId);
    const name = this.normalizeName(rawName);

    const existingMembership = await this.repo.getMembership(characterId);
    if (existingMembership) {
      throw new ConflictError("이미 길드에 소속되어 있습니다.", {
        code: "GUILD_ALREADY_MEMBER",
      });
    }

    const existingName = await this.repo.getGuildByName(name);
    if (existingName) {
      throw new ConflictError("이미 사용 중인 길드 이름입니다.", {
        code: "GUILD_NAME_TAKEN",
      });
    }

    const guild = await this.repo.createGuild({
      id: randomUUID(),
      name,
      masterCharacterId: characterId,
      now: this.now(),
    });
    return this.toGuildResponse(guild);
  }

  /** 길드 가입: 무소속·정원<30·중복 금지. open=즉시 가입 / approval=신청. */
  async join(userId: string, characterId: string, guildId: string) {
    await this.assertCharacter(userId, characterId);

    const existingMembership = await this.repo.getMembership(characterId);
    if (existingMembership) {
      throw new ConflictError("이미 길드에 소속되어 있습니다.", {
        code: "GUILD_ALREADY_MEMBER",
      });
    }

    const guild = await this.assertGuild(guildId);
    if (guild.memberCount >= GUILD_CAPACITY) {
      throw new ConflictError("길드 정원이 가득 찼습니다.", {
        code: "GUILD_FULL",
      });
    }

    if (guild.joinMode === "approval") {
      await this.repo.insertRequest({
        guildId,
        characterId,
        now: this.now(),
      });
      return { status: "requested" as const, guildId };
    }

    const member = await this.repo.addMember({
      guildId,
      characterId,
      capacity: GUILD_CAPACITY,
      now: this.now(),
    });
    if (!member) {
      // 동시성: 정원이 그새 찼거나 중복 가입.
      throw new ConflictError("길드 가입에 실패했습니다(정원 초과/중복).", {
        code: "GUILD_JOIN_FAILED",
      });
    }
    return { status: "joined" as const, guildId };
  }

  /** 승인제 신청 승인: 권한(master|vice)·정원 검증 후 가입 처리. */
  async approve(
    userId: string,
    actorCharacterId: string,
    guildId: string,
    targetCharacterId: string,
  ) {
    await this.assertActorRank(userId, actorCharacterId, guildId, [
      "master",
      "vice",
    ]);

    const request = await this.repo.getRequest(guildId, targetCharacterId);
    if (!request) {
      throw new NotFoundError("가입 신청을 찾을 수 없습니다.", {
        code: "GUILD_REQUEST_NOT_FOUND",
      });
    }

    // 이미 다른 길드에 가입한 신청자는 거절 처리하듯 신청만 정리.
    const targetMembership = await this.repo.getMembership(targetCharacterId);
    if (targetMembership) {
      await this.repo.deleteRequest(guildId, targetCharacterId);
      throw new ConflictError("신청자가 이미 다른 길드에 소속되어 있습니다.", {
        code: "GUILD_REQUEST_STALE",
      });
    }

    const member = await this.repo.approveRequest({
      guildId,
      characterId: targetCharacterId,
      capacity: GUILD_CAPACITY,
      now: this.now(),
    });
    if (!member) {
      throw new ConflictError("승인에 실패했습니다(정원 초과/중복).", {
        code: "GUILD_APPROVE_FAILED",
      });
    }
    return { status: "approved" as const, characterId: targetCharacterId };
  }

  /** 승인제 신청 거절: 권한(master|vice) 검증 후 신청 삭제. */
  async reject(
    userId: string,
    actorCharacterId: string,
    guildId: string,
    targetCharacterId: string,
  ) {
    await this.assertActorRank(userId, actorCharacterId, guildId, [
      "master",
      "vice",
    ]);

    const deleted = await this.repo.deleteRequest(guildId, targetCharacterId);
    if (!deleted) {
      throw new NotFoundError("가입 신청을 찾을 수 없습니다.", {
        code: "GUILD_REQUEST_NOT_FOUND",
      });
    }
    return { status: "rejected" as const, characterId: targetCharacterId };
  }

  /**
   * 탈퇴. 일반 멤버는 단순 삭제.
   * 길드장 탈퇴: 부길드장→총기여 최다 멤버 순으로 위임 후 탈퇴.
   * 후보가 0명(혼자)이면 길드 해산.
   */
  async leave(userId: string, characterId: string) {
    await this.assertCharacter(userId, characterId);

    const membership = await this.repo.getMembership(characterId);
    if (!membership) {
      throw new NotFoundError("길드에 소속되어 있지 않습니다.", {
        code: "GUILD_NOT_MEMBER",
      });
    }

    if (membership.rank !== "master") {
      await this.repo.removeMember(characterId);
      return { status: "left" as const, guildId: membership.guildId };
    }

    // 길드장 탈퇴 — 위임 후보 선정.
    const members = await this.repo.listMembers(membership.guildId);
    const successor = this.pickSuccessor(members, characterId);

    if (!successor) {
      // 후보 0 → 해산.
      await this.repo.disbandGuild(membership.guildId);
      return { status: "disbanded" as const, guildId: membership.guildId };
    }

    await this.repo.transferMaster({
      guildId: membership.guildId,
      fromCharacterId: characterId,
      toCharacterId: successor.characterId,
    });
    await this.repo.removeMember(characterId);
    return {
      status: "transferred" as const,
      guildId: membership.guildId,
      newMasterCharacterId: successor.characterId,
    };
  }

  /** 계급 승강: master 만·인원 해금·정원(vice≤1, officer≤3) 검증. */
  async setRank(
    userId: string,
    actorCharacterId: string,
    guildId: string,
    targetCharacterId: string,
    rank: GuildRank,
  ) {
    await this.assertActorRank(userId, actorCharacterId, guildId, ["master"]);

    if (rank === "master") {
      throw new ValidationError("길드장 계급은 직접 지정할 수 없습니다.", {
        code: "GUILD_RANK_MASTER_FORBIDDEN",
      });
    }

    const guild = await this.assertGuild(guildId);
    const members = await this.repo.listMembers(guildId);
    const target = members.find((m) => m.characterId === targetCharacterId);
    if (!target) {
      throw new NotFoundError("대상 멤버를 찾을 수 없습니다.", {
        code: "GUILD_MEMBER_NOT_FOUND",
      });
    }
    if (target.rank === "master") {
      throw new ValidationError("길드장 계급은 변경할 수 없습니다.", {
        code: "GUILD_RANK_MASTER_IMMUTABLE",
      });
    }
    if (target.rank === rank) {
      throw new ValidationError("이미 해당 계급입니다.", {
        code: "GUILD_RANK_UNCHANGED",
      });
    }

    if (!isRankUnlocked(rank, guild.memberCount)) {
      throw new ValidationError("아직 해금되지 않은 계급입니다.", {
        code: "GUILD_RANK_LOCKED",
      });
    }

    // 정원 검증 — 자신을 제외한 현재 해당 계급 인원.
    const cap = getRankSlotCap(rank);
    if (Number.isFinite(cap)) {
      const occupied = members.filter(
        (m) => m.rank === rank && m.characterId !== targetCharacterId,
      ).length;
      if (occupied >= cap) {
        throw new ValidationError("해당 계급의 정원이 가득 찼습니다.", {
          code: "GUILD_RANK_SLOT_FULL",
        });
      }
    }

    const updated = await this.repo.setRank(guildId, targetCharacterId, rank);
    if (!updated) {
      throw new NotFoundError("대상 멤버를 찾을 수 없습니다.", {
        code: "GUILD_MEMBER_NOT_FOUND",
      });
    }
    return { status: "ranked" as const, characterId: targetCharacterId, rank };
  }

  /**
   * 설정 변경. name/joinMode 는 master 만, notice 는 master|vice.
   */
  async updateSettings(
    userId: string,
    actorCharacterId: string,
    guildId: string,
    settings: { name?: string; notice?: string; joinMode?: GuildJoinMode },
  ) {
    const membership = await this.assertActorRank(
      userId,
      actorCharacterId,
      guildId,
      ["master", "vice"],
    );

    const patch: { name?: string; notice?: string; joinMode?: GuildJoinMode } =
      {};

    if (settings.name !== undefined) {
      if (membership.rank !== "master") {
        throw new ValidationError("이름 변경은 길드장만 가능합니다.", {
          code: "GUILD_SETTINGS_NAME_FORBIDDEN",
        });
      }
      const name = this.normalizeName(settings.name);
      const existing = await this.repo.getGuildByName(name);
      if (existing && existing.id !== guildId) {
        throw new ConflictError("이미 사용 중인 길드 이름입니다.", {
          code: "GUILD_NAME_TAKEN",
        });
      }
      patch.name = name;
    }

    if (settings.joinMode !== undefined) {
      if (membership.rank !== "master") {
        throw new ValidationError("가입 모드 변경은 길드장만 가능합니다.", {
          code: "GUILD_SETTINGS_JOINMODE_FORBIDDEN",
        });
      }
      patch.joinMode = settings.joinMode;
    }

    if (settings.notice !== undefined) {
      patch.notice = settings.notice;
    }

    if (Object.keys(patch).length === 0) {
      throw new ValidationError("변경할 설정이 없습니다.", {
        code: "GUILD_SETTINGS_EMPTY",
      });
    }

    const updated = await this.repo.updateGuild(guildId, patch);
    if (!updated) {
      throw new NotFoundError("길드를 찾을 수 없습니다.", {
        code: "GUILD_NOT_FOUND",
      });
    }
    return this.toGuildResponse(updated);
  }

  /** 공개 길드 조회. */
  async getById(guildId: string) {
    const guild = await this.assertGuild(guildId);
    const members = await this.repo.listMembers(guildId);
    return {
      ...this.toGuildResponse(guild),
      members: members.map((m) => this.toMemberResponse(m)),
    };
  }

  /** 길드 목록/검색(페이지). */
  async list(input: { limit?: number; offset?: number; q?: string }) {
    const limit = Math.min(Math.max(input.limit ?? 20, 1), 100);
    const offset = Math.max(input.offset ?? 0, 0);
    const guilds = await this.repo.listGuilds({ limit, offset, q: input.q });
    return guilds.map((g) => this.toGuildResponse(g));
  }

  /** 내 길드 스냅샷(GET /me) — 클라 캐시 소스. 무소속이면 guild=null. */
  async snapshot(userId: string, characterId: string) {
    await this.assertCharacter(userId, characterId);

    const membership = await this.repo.getMembership(characterId);
    if (!membership) {
      return { guild: null, me: null, members: [], requests: [] };
    }

    const guild = await this.repo.getGuild(membership.guildId);
    if (!guild) {
      // denorm 정합 이상 — 멤버십은 cascade 로 정리되므로 일반적으로 도달 불가.
      return { guild: null, me: null, members: [], requests: [] };
    }

    const members = await this.repo.listMembers(membership.guildId);
    const canManage =
      membership.rank === "master" || membership.rank === "vice";
    const requests = canManage
      ? await this.repo.listRequests(membership.guildId)
      : [];

    return {
      guild: this.toGuildResponse(guild),
      me: this.toMemberResponse(membership),
      members: members.map((m) => this.toMemberResponse(m)),
      requests: requests.map((r) => ({
        characterId: r.characterId,
        requestedAt: r.requestedAt.toISOString(),
      })),
    };
  }

  // ── 내부 헬퍼 ───────────────────────────────────────────────────────────────

  /** 위임 후보 선정: 부길드장 우선, 없으면 총기여 최다 멤버. 본인 제외. */
  private pickSuccessor(
    members: GuildMemberRecord[],
    masterCharacterId: string,
  ): GuildMemberRecord | null {
    const others = members.filter((m) => m.characterId !== masterCharacterId);
    if (others.length === 0) {
      return null;
    }
    const vice = others.find((m) => m.rank === "vice");
    if (vice) {
      return vice;
    }
    return others.reduce((best, current) =>
      current.totalContribution > best.totalContribution ? current : best,
    );
  }

  private normalizeName(rawName: string): string {
    const name = rawName.trim();
    if (name.length < GUILD_NAME_MIN || name.length > GUILD_NAME_MAX) {
      throw new ValidationError(
        `길드 이름은 ${GUILD_NAME_MIN}~${GUILD_NAME_MAX}자여야 합니다.`,
        { code: "GUILD_NAME_INVALID" },
      );
    }
    return name;
  }

  private async assertCharacter(userId: string, characterId: string) {
    const character = await this.repo.findCharacter(userId, characterId);
    if (!character) {
      throw new NotFoundError("캐릭터를 찾을 수 없습니다.", {
        code: "GUILD_CHARACTER_NOT_FOUND",
      });
    }
  }

  private async assertGuild(guildId: string): Promise<GuildRecord> {
    const guild = await this.repo.getGuild(guildId);
    if (!guild) {
      throw new NotFoundError("길드를 찾을 수 없습니다.", {
        code: "GUILD_NOT_FOUND",
      });
    }
    return guild;
  }

  /** 액터가 해당 길드의 멤버이고 allowed 계급에 속하는지 검증, 멤버십 반환. */
  private async assertActorRank(
    userId: string,
    actorCharacterId: string,
    guildId: string,
    allowed: GuildRank[],
  ): Promise<GuildMemberRecord> {
    await this.assertCharacter(userId, actorCharacterId);

    const membership = await this.repo.getMembership(actorCharacterId);
    if (!membership || membership.guildId !== guildId) {
      throw new NotFoundError("길드에 소속되어 있지 않습니다.", {
        code: "GUILD_NOT_MEMBER",
      });
    }
    if (!allowed.includes(membership.rank)) {
      throw new ValidationError("권한이 없습니다.", {
        code: "GUILD_FORBIDDEN",
      });
    }
    return membership;
  }

  private toGuildResponse(guild: GuildRecord) {
    return {
      id: guild.id,
      name: guild.name,
      notice: guild.notice,
      joinMode: guild.joinMode,
      level: guild.level,
      exp: guild.exp.toString(),
      masterCharacterId: guild.masterCharacterId,
      memberCount: guild.memberCount,
    };
  }

  private toMemberResponse(member: GuildMemberRecord) {
    return {
      characterId: member.characterId,
      rank: member.rank,
      weeklyContribution: member.weeklyContribution.toString(),
      totalContribution: member.totalContribution.toString(),
      contributionPoints: member.contributionPoints.toString(),
    };
  }
}
