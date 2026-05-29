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

// ── 기여도 상수 (클라 UE GuildFormula 와 parity 필수, 스펙 §4) ────────────────
export const ATTENDANCE_REWARD = 50; // 일일 출석 1회 기여
export const DONATION_GOLD_PER_POINT = 1000; // floor(gold/1000) 기여
export const DONATION_DAILY_CAP = 500; // 헌납 일일 기여 상한
export const AUTO_WEEKLY_CAP = 2000; // 전투/던전 자동 기여 주간 상한

// ── 길드 레벨/버프 공식 상수 (클라 parity — 단독 export, 스펙 §4) ─────────────
export const GUILD_LEVEL_BASE = 10_000; // 레벨 1→2 임계 기준값
export const GUILD_LEVEL_GROWTH = 1.6; // 레벨당 임계 성장률
export const GUILD_BUFF_PER_LEVEL = 0.004; // 레벨당 버프 계수(공격/골드 +0.4%)

export type GuildBuff = { attackPct: number; goldPct: number };

/**
 * 누적 EXP → 길드 레벨(무한 기하). 레벨 L 도달 누적 임계는
 * Σ_{i=1}^{L-1} floor(BASE * GROWTH^(i-1)). 레벨은 1 부터 시작한다.
 * 클라(UE) `GetGuildLevel` 과 동일 결과를 보장한다.
 */
export function getGuildLevel(exp: bigint): number {
  if (exp <= 0n) {
    return 1;
  }
  let level = 1;
  let cumulative = 0n;
  // 무한 곡선이지만 임계는 기하급수라 현실적 EXP 에서 수십 레벨 내 수렴.
  while (true) {
    const step = BigInt(
      Math.floor(GUILD_LEVEL_BASE * GUILD_LEVEL_GROWTH ** (level - 1)),
    );
    if (exp < cumulative + step) {
      return level;
    }
    cumulative += step;
    level += 1;
  }
}

/** 길드 레벨 → 영구 버프(공격 +0.4%*L, 골드 +0.4%*L). 클라 `GetGuildBuff` parity. */
export function getGuildBuff(level: number): GuildBuff {
  const pct = GUILD_BUFF_PER_LEVEL * level;
  return { attackPct: pct, goldPct: pct };
}

// ── 길드 보스 공식 상수 (클라 UE GuildBossFormula 와 parity 필수, 스펙 §4) ──────
export const GUILD_BOSS_BASE_HP = 1_000_000; // 첫 격파(defeated=0) 보스 HP
export const GUILD_BOSS_HP_GROWTH = 1.5; // 격파마다 HP 성장률(무한 곡선)
export const WEEKLY_GUILD_BOSS_CHALLENGE_LIMIT = 7; // 멤버당 주간 도전 횟수
export const GUILD_BOSS_DMG_TO_CONTRIB = 10_000; // 보스 데미지 → 기여 환산 제수

/**
 * 격파 횟수 → 다음 보스의 공유 HP(무한 기하). HP = floor(BASE * GROWTH^defeated).
 * 클라(UE) `GetGuildBossHp` 와 동일 결과를 보장한다.
 */
export function getGuildBossHp(defeatedCount: number): number {
  return Math.floor(
    GUILD_BOSS_BASE_HP * GUILD_BOSS_HP_GROWTH ** Math.max(defeatedCount, 0),
  );
}

/**
 * CP → 1회 도전 누적 데미지(#77 GetChallengeDamage 재사용). 음수/소수 방어.
 * 클라(UE) `GetChallengeDamage` parity.
 */
export function getChallengeDamage(cp: number): number {
  if (!Number.isFinite(cp) || cp <= 0) {
    return 0;
  }
  return Math.trunc(cp);
}

// ── 길드 상점 카탈로그 (서버 상수, 소비형 보상) ──────────────────────────────
export type GuildShopItem = {
  id: string;
  name: string; // 한글 표기명(클라 로컬라이즈 키 매핑용)
  price: number; // 개인 기여 포인트 가격
  reward: { type: string; amount: number }; // 소비형 보상(클라가 세이브 반영)
};

export const GUILD_SHOP_CATALOG: readonly GuildShopItem[] = [
  {
    id: "protection_scroll",
    name: "강화 보호서",
    price: 120,
    reward: { type: "protectionScroll", amount: 5 },
  },
  {
    id: "reset_cube",
    name: "잠재 재설정 큐브",
    price: 200,
    reward: { type: "resetCube", amount: 3 },
  },
  {
    id: "gold_pouch",
    name: "골드 주머니",
    price: 150,
    reward: { type: "gold", amount: 100_000 },
  },
  {
    id: "exp_potion",
    name: "지혜의 물약",
    price: 180,
    reward: { type: "expPotion", amount: 5 },
  },
  {
    id: "rank_cube",
    name: "잠재 등급 큐브",
    price: 400,
    reward: { type: "rankCube", amount: 1 },
  },
  {
    id: "essence",
    name: "룬 정수",
    price: 300,
    reward: { type: "essence", amount: 3 },
  },
] as const;

// ── 길드 보스 격파 보상 (서버 상수, 소비형 — 클라가 세이브 반영) ───────────────
export type GuildBossReward = { type: string; amount: number };

/** 보스 1격파당 전원 지급 보상(소비형). 격파 N건 미수령 시 amount * N 누적 지급. */
export const GUILD_BOSS_REWARD_PER_DEFEAT: readonly GuildBossReward[] = [
  { type: "gold", amount: 200_000 },
  { type: "essence", amount: 5 },
] as const;

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
  weeklyAutoContribution: bigint;
  lastDonationDate: string | null;
  dailyDonation: bigint;
  weeklyResetId: string | null;
  weeklyBossChallenges: number;
  bossClaimedCount: number;
  bossClaimWeekId: string | null;
};

/** 공유 보스 상태(길드당 1행, 주간). */
export type GuildBossRecord = {
  guildId: string;
  weekId: string;
  accumDamage: bigint;
  defeatedCount: number;
};

/** 주간 길드 랭킹 1행(길드별 Σ weekly_contribution). */
export type GuildRankingRow = {
  guildId: string;
  name: string;
  level: number;
  weeklyContribution: bigint;
  rank: number;
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
  /**
   * 기여 누적(트랜잭션, 원자적). 한 호출에서:
   *  - guilds.exp += amount, guilds.level = newLevel, guilds.week_id = weekId
   *  - guild_members 의 기여 누적(contribution_points/weekly_contribution/total_contribution += amount)
   *  - patch 로 지정한 컬럼(주간 자동/일일 헌납/출석일/리셋값) 갱신
   * weeklyReset=true 면 같은 트랜잭션에서 해당 멤버의 weekly_contribution/weekly_auto_contribution 을 먼저 0 으로 리셋한 뒤 누적한다.
   */
  applyContribution(input: {
    guildId: string;
    characterId: string;
    amount: bigint;
    newLevel: number;
    weekId: string;
    weeklyReset: boolean;
    patch: {
      lastAttendanceDate?: string;
      weeklyAutoContributionDelta?: bigint;
      dailyDonation?: bigint;
      lastDonationDate?: string;
    };
  }): Promise<GuildMemberRecord | null>;
  /** 개인 기여 포인트 차감(상점 구매, 조건부 update — 잔액 부족 시 null). */
  spendContributionPoints(input: {
    characterId: string;
    cost: bigint;
  }): Promise<GuildMemberRecord | null>;
  /** 보스 상태 조회(없으면 null). 주간 리셋/생성은 서비스가 판단해 challengeBoss 가 원자적으로 처리. */
  getBoss(guildId: string): Promise<GuildBossRecord | null>;
  /**
   * 보스 도전 누적(한 트랜잭션, 원자적). 한 호출에서:
   *  - guild_boss upsert: 새 주(weeklyReset)면 week_id/accum/defeated 리셋 후 누적,
   *    아니면 accum_damage 를 newAccumDamage 로, defeated_count 를 newDefeatedCount 로 갱신.
   *  - guild_boss_contrib upsert: 멤버 주간 누적 데미지 += dmg(weeklyReset 면 0 부터).
   *  - guild_members: weekly_boss_challenges 갱신(weeklyReset 면 1, 아니면 +1).
   * 도전 후 보스/멤버 상태를 반환. weeklyReset 인자는 멤버 주간 마커 기준(needsWeeklyReset).
   */
  challengeBoss(input: {
    guildId: string;
    characterId: string;
    weekId: string;
    weeklyReset: boolean;
    dmg: bigint;
    newAccumDamage: bigint;
    newDefeatedCount: number;
    newChallengeCount: number;
  }): Promise<{ boss: GuildBossRecord; member: GuildMemberRecord } | null>;
  /**
   * 보스 격파 보상 수령(원자적). boss_claimed_count 를 toDefeatedCount 로 올리고
   * boss_claim_week_id 를 weekId 로 갱신. fromClaimedCount(기대 현재값)와 다르면(동시성) null.
   */
  claimBossReward(input: {
    characterId: string;
    weekId: string;
    fromClaimedCount: number;
    toDefeatedCount: number;
  }): Promise<GuildMemberRecord | null>;
  /** 보스 내부 기여 랭킹(멤버별 누적 데미지 상위 N, 주간). */
  listBossContrib(input: {
    guildId: string;
    weekId: string;
    limit: number;
  }): Promise<{ characterId: string; damage: bigint }[]>;
  /** 주간 길드 랭킹: 길드별 Σ weekly_contribution 상위 N(#76 패턴). */
  listGuildRankings(limit: number): Promise<GuildRankingRow[]>;
  /** 특정 길드의 주간 랭킹 순위(내 길드 표시용). 없으면 null. */
  getGuildRanking(guildId: string): Promise<GuildRankingRow | null>;
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

    const now = this.now();
    const today = toUtcDate(now);
    const guildResponse = this.toGuildResponse(guild);
    const boss = await this.toBossState(membership, membership.guildId);
    return {
      guild: guildResponse,
      me: {
        ...this.toMemberResponse(membership),
        canAttend: membership.lastAttendanceDate !== today,
        donationRemaining: this.donationRemaining(membership, today).toString(),
      },
      buff: guildResponse.buff,
      members: members.map((m) => this.toMemberResponse(m)),
      requests: requests.map((r) => ({
        characterId: r.characterId,
        requestedAt: r.requestedAt.toISOString(),
      })),
      boss,
    };
  }

  /**
   * 일일 출석 기여(+50). UTC date 기준 1일 1회. 이미 출석했으면 ConflictError.
   */
  async attendance(userId: string, characterId: string, guildId: string) {
    const { membership, guild } = await this.assertMember(
      userId,
      characterId,
      guildId,
    );
    const now = this.now();
    const today = toUtcDate(now);
    const weekId = toUtcWeek(now);
    const weeklyReset = this.needsWeeklyReset(membership, weekId);

    if (membership.lastAttendanceDate === today) {
      throw new ConflictError("오늘은 이미 출석했습니다.", {
        code: "GUILD_ATTENDANCE_DONE",
      });
    }

    const amount = BigInt(ATTENDANCE_REWARD);
    const updated = await this.repo.applyContribution({
      guildId,
      characterId,
      amount,
      newLevel: getGuildLevel(guild.exp + amount),
      weekId,
      weeklyReset,
      patch: { lastAttendanceDate: today },
    });
    if (!updated) {
      throw new NotFoundError("길드에 소속되어 있지 않습니다.", {
        code: "GUILD_NOT_MEMBER",
      });
    }
    return {
      status: "attended" as const,
      gained: amount.toString(),
      contributionPoints: updated.contributionPoints.toString(),
    };
  }

  /**
   * 재화 헌납 기여(floor(gold/1000)). 골드는 클라 권위이므로 서버는 차감하지 않고
   * 보고된 gold 를 신뢰하되 일일 상한(DONATION_DAILY_CAP)으로 어뷰즈를 억제한다.
   */
  async donate(
    userId: string,
    characterId: string,
    guildId: string,
    gold: number,
  ) {
    const { membership, guild } = await this.assertMember(
      userId,
      characterId,
      guildId,
    );
    if (!Number.isFinite(gold) || gold < 0) {
      throw new ValidationError("헌납 골드 값이 올바르지 않습니다.", {
        code: "GUILD_DONATE_INVALID",
      });
    }
    const now = this.now();
    const today = toUtcDate(now);
    const weekId = toUtcWeek(now);
    const weeklyReset = this.needsWeeklyReset(membership, weekId);

    // 일일 누적(날짜 변경 시 0 부터). 클라 권위 gold → floor 기여, 잔여 상한으로 캡.
    const dayDonated =
      membership.lastDonationDate === today ? membership.dailyDonation : 0n;
    const remaining = BigInt(DONATION_DAILY_CAP) - dayDonated;
    if (remaining <= 0n) {
      throw new ValidationError("오늘 헌납 기여 상한에 도달했습니다.", {
        code: "GUILD_DONATE_CAP",
      });
    }
    const rawAmount = BigInt(Math.floor(gold / DONATION_GOLD_PER_POINT));
    if (rawAmount <= 0n) {
      throw new ValidationError(
        `헌납은 최소 ${DONATION_GOLD_PER_POINT} 골드부터 가능합니다.`,
        { code: "GUILD_DONATE_TOO_SMALL" },
      );
    }
    const amount = rawAmount < remaining ? rawAmount : remaining;

    const updated = await this.repo.applyContribution({
      guildId,
      characterId,
      amount,
      newLevel: getGuildLevel(guild.exp + amount),
      weekId,
      weeklyReset,
      patch: {
        lastDonationDate: today,
        dailyDonation: dayDonated + amount,
      },
    });
    if (!updated) {
      throw new NotFoundError("길드에 소속되어 있지 않습니다.", {
        code: "GUILD_NOT_MEMBER",
      });
    }
    return {
      status: "donated" as const,
      gained: amount.toString(),
      dailyRemaining: (remaining - amount).toString(),
      contributionPoints: updated.contributionPoints.toString(),
    };
  }

  /**
   * 전투/던전 자동 기여 델타 플러시. 주간 자동 상한(AUTO_WEEKLY_CAP)으로 캡.
   * 클라가 누적한 델타(autoAmount)를 보고하면 잔여 상한만큼 적립한다.
   */
  async contribute(
    userId: string,
    characterId: string,
    guildId: string,
    autoAmount: number,
  ) {
    const { membership, guild } = await this.assertMember(
      userId,
      characterId,
      guildId,
    );
    if (!Number.isFinite(autoAmount) || autoAmount < 0) {
      throw new ValidationError("자동 기여 값이 올바르지 않습니다.", {
        code: "GUILD_CONTRIBUTE_INVALID",
      });
    }
    const now = this.now();
    const weekId = toUtcWeek(now);
    const weeklyReset = this.needsWeeklyReset(membership, weekId);

    // 주간 리셋 대상이면 자동 누적은 0 부터 다시 계산.
    const weekAuto = weeklyReset ? 0n : membership.weeklyAutoContribution;
    const remaining = BigInt(AUTO_WEEKLY_CAP) - weekAuto;
    if (remaining <= 0n) {
      return {
        status: "capped" as const,
        gained: "0",
        weeklyRemaining: "0",
        contributionPoints: membership.contributionPoints.toString(),
      };
    }
    const rawAmount = BigInt(Math.floor(autoAmount));
    const amount = rawAmount < remaining ? rawAmount : remaining;
    if (amount <= 0n) {
      return {
        status: "noop" as const,
        gained: "0",
        weeklyRemaining: remaining.toString(),
        contributionPoints: membership.contributionPoints.toString(),
      };
    }

    const updated = await this.repo.applyContribution({
      guildId,
      characterId,
      amount,
      newLevel: getGuildLevel(guild.exp + amount),
      weekId,
      weeklyReset,
      patch: { weeklyAutoContributionDelta: amount },
    });
    if (!updated) {
      throw new NotFoundError("길드에 소속되어 있지 않습니다.", {
        code: "GUILD_NOT_MEMBER",
      });
    }
    return {
      status: "contributed" as const,
      gained: amount.toString(),
      weeklyRemaining: (remaining - amount).toString(),
      contributionPoints: updated.contributionPoints.toString(),
    };
  }

  /** 길드 상점 카탈로그 조회(멤버). */
  async shop(userId: string, characterId: string, guildId: string) {
    await this.assertMember(userId, characterId, guildId);
    return {
      items: GUILD_SHOP_CATALOG.map((item) => ({
        id: item.id,
        name: item.name,
        price: item.price,
        reward: item.reward,
      })),
    };
  }

  /**
   * 길드 상점 구매. 카탈로그 가격 검증 → 개인 기여 포인트 차감 → 보상 반환(소비형).
   * 보상은 클라가 캐릭터 세이브에 반영한다(테이블 없음).
   */
  async shopBuy(
    userId: string,
    characterId: string,
    guildId: string,
    itemId: string,
  ) {
    await this.assertMember(userId, characterId, guildId);
    const item = GUILD_SHOP_CATALOG.find((i) => i.id === itemId);
    if (!item) {
      throw new NotFoundError("상점 아이템을 찾을 수 없습니다.", {
        code: "GUILD_SHOP_ITEM_NOT_FOUND",
      });
    }
    const updated = await this.repo.spendContributionPoints({
      characterId,
      cost: BigInt(item.price),
    });
    if (!updated) {
      throw new ConflictError("기여 포인트가 부족합니다.", {
        code: "GUILD_SHOP_INSUFFICIENT_POINTS",
      });
    }
    return {
      status: "purchased" as const,
      itemId: item.id,
      reward: item.reward,
      contributionPoints: updated.contributionPoints.toString(),
    };
  }

  /**
   * 공유 HP 풀 길드 보스 도전(#77 길드화, 4번째 기여 발생원).
   * 멤버 검증 → 주간 진입 시 보스/도전 카운트 리셋 → 주간 도전 한도(7) 검증 →
   * dmg = getChallengeDamage(cp) 를 accum_damage 와 멤버 보스 기여에 누적 →
   * **격파 루프**: accum_damage >= getGuildBossHp(defeated) 인 동안 HP 만큼 이월 차감하고
   * defeated_count++ (누적이 여러 HP 임계를 한 번에 넘으면 다중 격파) →
   * 보스 데미지 기여: applyContribution(floor(dmg / DMG_TO_CONTRIB)) 1회만(이중적립 금지).
   */
  async challengeBoss(
    userId: string,
    characterId: string,
    guildId: string,
    cp: number,
  ) {
    const { membership, guild } = await this.assertMember(
      userId,
      characterId,
      guildId,
    );
    const now = this.now();
    const weekId = toUtcWeek(now);
    const weeklyReset = this.needsWeeklyReset(membership, weekId);

    // 주간 도전 한도(멤버당 7). 새 주면 0 부터 다시 카운트.
    const usedChallenges = weeklyReset ? 0 : membership.weeklyBossChallenges;
    if (usedChallenges >= WEEKLY_GUILD_BOSS_CHALLENGE_LIMIT) {
      throw new ConflictError("이번 주 보스 도전 횟수를 모두 사용했습니다.", {
        code: "GUILD_BOSS_CHALLENGE_LIMIT",
      });
    }

    // 보스 누적 상태(새 주면 0 부터 — 보스도 주간 리셋).
    const existingBoss = await this.repo.getBoss(guildId);
    const bossIsCurrentWeek = existingBoss?.weekId === weekId;
    const baseAccum = bossIsCurrentWeek
      ? (existingBoss?.accumDamage ?? 0n)
      : 0n;
    const baseDefeated = bossIsCurrentWeek
      ? (existingBoss?.defeatedCount ?? 0)
      : 0;

    const dmg = BigInt(getChallengeDamage(cp));
    let accum = baseAccum + dmg;
    let defeated = baseDefeated;
    // 격파 루프: 누적이 현재 HP 임계 이상이면 이월 차감 후 격파(다중 격파 가능).
    let hp = BigInt(getGuildBossHp(defeated));
    while (accum >= hp) {
      accum -= hp;
      defeated += 1;
      hp = BigInt(getGuildBossHp(defeated));
    }

    const challenged = await this.repo.challengeBoss({
      guildId,
      characterId,
      weekId,
      // 보스 행 누적/격파는 이미 bossIsCurrentWeek 기준으로 계산된 값을 그대로 권위 기록.
      // weeklyReset 은 멤버 주간 마커 기준(도전 카운트·weekly 컬럼 리셋). 마커도 전진시킨다.
      weeklyReset,
      dmg,
      newAccumDamage: accum,
      newDefeatedCount: defeated,
      newChallengeCount: usedChallenges + 1,
    });
    if (!challenged) {
      throw new NotFoundError("길드에 소속되어 있지 않습니다.", {
        code: "GUILD_NOT_MEMBER",
      });
    }

    // 보스 데미지 → 기여(4번째 발생원). contrib 1회만 적립(이중적립 금지).
    const contribGain = dmg / BigInt(GUILD_BOSS_DMG_TO_CONTRIB);
    let contributionPoints = challenged.member.contributionPoints;
    if (contribGain > 0n) {
      // challengeBoss 가 이미 멤버 주간 마커를 전진·weekly 컬럼을 리셋했으므로
      // 여기선 weeklyReset=false 로 EXP/기여만 누적(이중 리셋 방지).
      const updated = await this.repo.applyContribution({
        guildId,
        characterId,
        amount: contribGain,
        newLevel: getGuildLevel(guild.exp + contribGain),
        weekId,
        weeklyReset: false,
        patch: {},
      });
      if (updated) {
        contributionPoints = updated.contributionPoints;
      }
    }

    const newDefeats = defeated - baseDefeated;
    return {
      status: "challenged" as const,
      damage: dmg.toString(),
      contributionGain: contribGain.toString(),
      contributionPoints: contributionPoints.toString(),
      accumDamage: accum.toString(),
      defeatedCount: defeated,
      newDefeats,
      bossHp: getGuildBossHp(defeated).toString(),
      challengesRemaining:
        WEEKLY_GUILD_BOSS_CHALLENGE_LIMIT - (usedChallenges + 1),
    };
  }

  /**
   * 보스 격파 보상 수령. 현 주 격파분(defeated_count) 중 멤버 미수령분을 전원 지급.
   * 멤버별 수령 마일스톤(boss_claimed_count)을 추적, 주간 리셋(boss_claim_week_id).
   * 보상은 소비형(클라가 세이브 반영). 미수령 0 이면 ConflictError.
   */
  async claimBossReward(userId: string, characterId: string, guildId: string) {
    const { membership } = await this.assertMember(
      userId,
      characterId,
      guildId,
    );
    const now = this.now();
    const weekId = toUtcWeek(now);

    const boss = await this.repo.getBoss(guildId);
    const defeatedCount =
      boss && boss.weekId === weekId ? boss.defeatedCount : 0;

    // 보스 수령 마일스톤은 보스 주(week_id)와 동기화. 다른 주면 0 부터.
    const claimedCount =
      membership.bossClaimWeekId === weekId ? membership.bossClaimedCount : 0;
    const unclaimed = defeatedCount - claimedCount;
    if (unclaimed <= 0) {
      throw new ConflictError("수령할 격파 보상이 없습니다.", {
        code: "GUILD_BOSS_NO_REWARD",
      });
    }

    const updated = await this.repo.claimBossReward({
      characterId,
      weekId,
      fromClaimedCount: claimedCount,
      toDefeatedCount: defeatedCount,
    });
    if (!updated) {
      // 동시성: 그새 다른 요청이 수령. 멱등하게 보상 없음으로 처리.
      throw new ConflictError("수령할 격파 보상이 없습니다.", {
        code: "GUILD_BOSS_NO_REWARD",
      });
    }

    // 격파 N건 × 1격파당 보상.
    const rewards = GUILD_BOSS_REWARD_PER_DEFEAT.map((r) => ({
      type: r.type,
      amount: r.amount * unclaimed,
    }));
    return {
      status: "claimed" as const,
      defeats: unclaimed,
      claimedCount: defeatedCount,
      rewards,
    };
  }

  /** 보스 상태 조회(멤버) — 공유 HP/누적/격파/내 도전 잔여/미수령/내부 기여 랭킹. */
  async getBoss(userId: string, characterId: string, guildId: string) {
    const { membership } = await this.assertMember(
      userId,
      characterId,
      guildId,
    );
    return this.toBossState(membership, guildId);
  }

  /**
   * 주간 길드 랭킹(Σ weekly_contribution 길드별 상위 N, #76 확장).
   * characterId 지정 시 내 길드 순위도 함께 반환(인증 라우트 — 누구나).
   */
  async guildRankings(input: {
    limit?: number;
    userId?: string;
    characterId?: string;
  }) {
    const limit = Math.min(Math.max(input.limit ?? 20, 1), 100);
    const rows = await this.repo.listGuildRankings(limit);
    const top = rows.map((r) => this.toRankingResponse(r));

    let me: ReturnType<typeof this.toRankingResponse> | null = null;
    if (input.userId && input.characterId) {
      await this.assertCharacter(input.userId, input.characterId);
      const membership = await this.repo.getMembership(input.characterId);
      if (membership) {
        const myRank = await this.repo.getGuildRanking(membership.guildId);
        if (myRank) {
          me = this.toRankingResponse(myRank);
        }
      }
    }
    return { rankings: top, me };
  }

  // ── 내부 헬퍼 ───────────────────────────────────────────────────────────────

  /** 멤버십+guildId 로 보스 상태 응답 구성(snapshot/getBoss 공통). */
  private async toBossState(membership: GuildMemberRecord, guildId: string) {
    const now = this.now();
    const weekId = toUtcWeek(now);
    const boss = await this.repo.getBoss(guildId);
    const bossIsCurrentWeek = boss?.weekId === weekId;
    const accumDamage = bossIsCurrentWeek ? (boss?.accumDamage ?? 0n) : 0n;
    const defeatedCount = bossIsCurrentWeek ? (boss?.defeatedCount ?? 0) : 0;

    const usedChallenges =
      membership.weeklyResetId === weekId ? membership.weeklyBossChallenges : 0;
    const claimedCount =
      membership.bossClaimWeekId === weekId ? membership.bossClaimedCount : 0;

    const topContrib = await this.repo.listBossContrib({
      guildId,
      weekId,
      limit: 10,
    });

    return {
      weekId,
      hp: getGuildBossHp(defeatedCount).toString(),
      accumDamage: accumDamage.toString(),
      defeatedCount,
      challengesRemaining: Math.max(
        WEEKLY_GUILD_BOSS_CHALLENGE_LIMIT - usedChallenges,
        0,
      ),
      unclaimedDefeats: Math.max(defeatedCount - claimedCount, 0),
      topContributors: topContrib.map((c) => ({
        characterId: c.characterId,
        damage: c.damage.toString(),
      })),
    };
  }

  private toRankingResponse(row: GuildRankingRow) {
    return {
      guildId: row.guildId,
      name: row.name,
      level: row.level,
      weeklyContribution: row.weeklyContribution.toString(),
      rank: row.rank,
    };
  }

  /**
   * 멤버의 주간 마커가 현재 주와 다르면 주간 리셋이 필요(멤버별 lazy 리셋).
   * 길드 단위가 아니라 멤버 단위로 추적해야 다른 멤버가 먼저 활동해도 누락 없이 리셋된다.
   */
  private needsWeeklyReset(member: GuildMemberRecord, weekId: string): boolean {
    return member.weeklyResetId !== weekId;
  }

  /** 헌납 일일 잔여 상한(오늘 누적 기준). */
  private donationRemaining(member: GuildMemberRecord, today: string): bigint {
    const dayDonated =
      member.lastDonationDate === today ? member.dailyDonation : 0n;
    const remaining = BigInt(DONATION_DAILY_CAP) - dayDonated;
    return remaining > 0n ? remaining : 0n;
  }

  /** 액터가 해당 길드 멤버인지 검증하고 membership+guild 반환(기여 라우트 공통). */
  private async assertMember(
    userId: string,
    characterId: string,
    guildId: string,
  ): Promise<{ membership: GuildMemberRecord; guild: GuildRecord }> {
    await this.assertCharacter(userId, characterId);
    const membership = await this.repo.getMembership(characterId);
    if (!membership || membership.guildId !== guildId) {
      throw new NotFoundError("길드에 소속되어 있지 않습니다.", {
        code: "GUILD_NOT_MEMBER",
      });
    }
    const guild = await this.assertGuild(guildId);
    return { membership, guild };
  }

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
    // level 은 누적 exp 로 재계산(저장 denorm 과 무관하게 항상 권위 일치).
    const level = getGuildLevel(guild.exp);
    return {
      id: guild.id,
      name: guild.name,
      notice: guild.notice,
      joinMode: guild.joinMode,
      level,
      exp: guild.exp.toString(),
      masterCharacterId: guild.masterCharacterId,
      memberCount: guild.memberCount,
      buff: getGuildBuff(level),
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

// ── 주간/일자 키 헬퍼 (quest 모듈과 동일 알고리즘 — ISO week, UTC date) ────────
/** UTC 날짜 문자열(YYYY-MM-DD). 출석/헌납 일일 리셋 기준. */
function toUtcDate(date: Date): string {
  return date.toISOString().slice(0, 10);
}

/** ISO week 문자열(YYYY-Www). 주간 기여/자동 상한 리셋 기준(quest/leaderboard parity). */
function toUtcWeek(date: Date): string {
  const utcDate = new Date(
    Date.UTC(date.getUTCFullYear(), date.getUTCMonth(), date.getUTCDate()),
  );
  const day = utcDate.getUTCDay() || 7;
  utcDate.setUTCDate(utcDate.getUTCDate() + 4 - day);
  const yearStart = new Date(Date.UTC(utcDate.getUTCFullYear(), 0, 1));
  const week = Math.ceil(
    ((utcDate.getTime() - yearStart.getTime()) / 86_400_000 + 1) / 7,
  );
  return `${utcDate.getUTCFullYear()}-W${String(week).padStart(2, "0")}`;
}
