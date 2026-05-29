import { describe, expect, it } from "vitest";
import {
  ConflictError,
  NotFoundError,
  ValidationError,
} from "../../core/errors.js";
import {
  ATTENDANCE_REWARD,
  AUTO_WEEKLY_CAP,
  DONATION_DAILY_CAP,
  GUILD_BOSS_BASE_HP,
  GUILD_BOSS_DMG_TO_CONTRIB,
  GUILD_CAPACITY,
  GUILD_LEVEL_BASE,
  GUILD_SHOP_CATALOG,
  type GuildBossRecord,
  type GuildJoinMode,
  type GuildJoinRequestRecord,
  type GuildMemberRecord,
  type GuildRank,
  type GuildRankingRow,
  type GuildRecord,
  type GuildRepo,
  GuildService,
  getChallengeDamage,
  getGuildBossHp,
  getGuildBuff,
  getGuildLevel,
  getRankSlotCap,
  isRankUnlocked,
  WEEKLY_GUILD_BOSS_CHALLENGE_LIMIT,
} from "./guild.service.js";

const userId = "00000000-0000-0000-0000-000000000001";
const charA = "00000000-0000-0000-0000-0000000000a1";
const charB = "00000000-0000-0000-0000-0000000000b2";
const charC = "00000000-0000-0000-0000-0000000000c3";

// ── 인메모리 fake repo (season.service.test 의 fake 패턴 확장) ────────────────
class FakeGuildRepo implements GuildRepo {
  guilds = new Map<string, GuildRecord>();
  members = new Map<string, GuildMemberRecord>(); // characterId → member
  requests: GuildJoinRequestRecord[] = [];
  bosses = new Map<string, GuildBossRecord>(); // guildId → boss
  // (guildId|weekId|characterId) → damage
  bossContrib = new Map<string, bigint>();
  // userId → characterId 들. 기본은 모든 charA/B/C 가 userId 소유로 간주.
  ownedCharacters = new Set<string>([charA, charB, charC]);

  async findCharacter(_userId: string, characterId: string) {
    return this.ownedCharacters.has(characterId)
      ? { id: characterId, userId: _userId }
      : null;
  }

  async getGuild(guildId: string) {
    return this.guilds.get(guildId) ?? null;
  }

  async getGuildByName(name: string) {
    for (const g of this.guilds.values()) {
      if (g.name.toLowerCase() === name.toLowerCase()) {
        return g;
      }
    }
    return null;
  }

  async listGuilds(input: { limit: number; offset: number; q?: string }) {
    let all = [...this.guilds.values()];
    if (input.q) {
      all = all.filter((g) => g.name.includes(input.q ?? ""));
    }
    return all.slice(input.offset, input.offset + input.limit);
  }

  async getMembership(characterId: string) {
    return this.members.get(characterId) ?? null;
  }

  async listMembers(guildId: string) {
    return [...this.members.values()].filter((m) => m.guildId === guildId);
  }

  async listRequests(guildId: string) {
    return this.requests.filter((r) => r.guildId === guildId);
  }

  async getRequest(guildId: string, characterId: string) {
    return (
      this.requests.find(
        (r) => r.guildId === guildId && r.characterId === characterId,
      ) ?? null
    );
  }

  async createGuild(input: {
    id: string;
    name: string;
    masterCharacterId: string;
    now: Date;
  }) {
    const guild: GuildRecord = {
      id: input.id,
      name: input.name,
      notice: "",
      joinMode: "open",
      level: 1,
      exp: 0n,
      masterCharacterId: input.masterCharacterId,
      memberCount: 1,
      weekId: null,
      createdAt: input.now,
    };
    this.guilds.set(guild.id, guild);
    this.members.set(input.masterCharacterId, {
      guildId: guild.id,
      characterId: input.masterCharacterId,
      rank: "master",
      joinedAt: input.now,
      weeklyContribution: 0n,
      totalContribution: 0n,
      contributionPoints: 0n,
      lastAttendanceDate: null,
      weeklyAutoContribution: 0n,
      lastDonationDate: null,
      dailyDonation: 0n,
      weeklyResetId: null,
      weeklyBossChallenges: 0,
      bossClaimedCount: 0,
      bossClaimWeekId: null,
    });
    return guild;
  }

  async addMember(input: {
    guildId: string;
    characterId: string;
    capacity: number;
    now: Date;
  }) {
    const guild = this.guilds.get(input.guildId);
    if (!guild || guild.memberCount >= input.capacity) {
      return null;
    }
    if (this.members.has(input.characterId)) {
      return null;
    }
    guild.memberCount += 1;
    const member: GuildMemberRecord = {
      guildId: input.guildId,
      characterId: input.characterId,
      rank: "member",
      joinedAt: input.now,
      weeklyContribution: 0n,
      totalContribution: 0n,
      contributionPoints: 0n,
      lastAttendanceDate: null,
      weeklyAutoContribution: 0n,
      lastDonationDate: null,
      dailyDonation: 0n,
      weeklyResetId: null,
      weeklyBossChallenges: 0,
      bossClaimedCount: 0,
      bossClaimWeekId: null,
    };
    this.members.set(input.characterId, member);
    return member;
  }

  async removeMember(characterId: string) {
    const member = this.members.get(characterId);
    if (!member) {
      return false;
    }
    this.members.delete(characterId);
    const guild = this.guilds.get(member.guildId);
    if (guild) {
      guild.memberCount = Math.max(guild.memberCount - 1, 0);
    }
    return true;
  }

  async insertRequest(input: {
    guildId: string;
    characterId: string;
    now: Date;
  }) {
    if (!(await this.getRequest(input.guildId, input.characterId))) {
      this.requests.push({
        guildId: input.guildId,
        characterId: input.characterId,
        requestedAt: input.now,
      });
    }
  }

  async deleteRequest(guildId: string, characterId: string) {
    const before = this.requests.length;
    this.requests = this.requests.filter(
      (r) => !(r.guildId === guildId && r.characterId === characterId),
    );
    return this.requests.length < before;
  }

  async approveRequest(input: {
    guildId: string;
    characterId: string;
    capacity: number;
    now: Date;
  }) {
    if (!(await this.deleteRequest(input.guildId, input.characterId))) {
      return null;
    }
    return this.addMember(input);
  }

  async setRank(guildId: string, characterId: string, rank: GuildRank) {
    const member = this.members.get(characterId);
    if (!member || member.guildId !== guildId) {
      return null;
    }
    member.rank = rank;
    return member;
  }

  async updateGuild(
    guildId: string,
    settings: { name?: string; notice?: string; joinMode?: GuildJoinMode },
  ) {
    const guild = this.guilds.get(guildId);
    if (!guild) {
      return null;
    }
    if (settings.name !== undefined) {
      guild.name = settings.name;
    }
    if (settings.notice !== undefined) {
      guild.notice = settings.notice;
    }
    if (settings.joinMode !== undefined) {
      guild.joinMode = settings.joinMode;
    }
    return guild;
  }

  async transferMaster(input: {
    guildId: string;
    fromCharacterId: string;
    toCharacterId: string;
  }) {
    const from = this.members.get(input.fromCharacterId);
    const to = this.members.get(input.toCharacterId);
    if (from) {
      from.rank = "member";
    }
    if (to) {
      to.rank = "master";
    }
    const guild = this.guilds.get(input.guildId);
    if (guild) {
      guild.masterCharacterId = input.toCharacterId;
    }
  }

  async disbandGuild(guildId: string) {
    this.guilds.delete(guildId);
    for (const [id, m] of this.members) {
      if (m.guildId === guildId) {
        this.members.delete(id);
      }
    }
    this.requests = this.requests.filter((r) => r.guildId !== guildId);
  }

  async applyContribution(input: {
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
  }) {
    const member = this.members.get(input.characterId);
    const guild = this.guilds.get(input.guildId);
    if (!member || !guild || member.guildId !== input.guildId) {
      return null;
    }
    // 길드 EXP/레벨/주간 키.
    guild.exp += input.amount;
    guild.level = input.newLevel;
    guild.weekId = input.weekId;
    // 멤버 기여 누적(weeklyReset 면 weekly 0 부터).
    const weeklyBase = input.weeklyReset ? 0n : member.weeklyContribution;
    const autoBase = input.weeklyReset ? 0n : member.weeklyAutoContribution;
    member.contributionPoints += input.amount;
    member.totalContribution += input.amount;
    member.weeklyContribution = weeklyBase + input.amount;
    member.weeklyAutoContribution =
      autoBase + (input.patch.weeklyAutoContributionDelta ?? 0n);
    member.weeklyResetId = input.weekId;
    if (input.patch.lastAttendanceDate !== undefined) {
      member.lastAttendanceDate = input.patch.lastAttendanceDate;
    }
    if (input.patch.dailyDonation !== undefined) {
      member.dailyDonation = input.patch.dailyDonation;
    }
    if (input.patch.lastDonationDate !== undefined) {
      member.lastDonationDate = input.patch.lastDonationDate;
    }
    return member;
  }

  async spendContributionPoints(input: { characterId: string; cost: bigint }) {
    const member = this.members.get(input.characterId);
    if (!member || member.contributionPoints < input.cost) {
      return null;
    }
    member.contributionPoints -= input.cost;
    return member;
  }

  async getBoss(guildId: string) {
    return this.bosses.get(guildId) ?? null;
  }

  async challengeBoss(input: {
    guildId: string;
    characterId: string;
    weekId: string;
    weeklyReset: boolean;
    dmg: bigint;
    newAccumDamage: bigint;
    newDefeatedCount: number;
    newChallengeCount: number;
  }) {
    const member = this.members.get(input.characterId);
    if (!member || member.guildId !== input.guildId) {
      return null;
    }
    const boss: GuildBossRecord = {
      guildId: input.guildId,
      weekId: input.weekId,
      accumDamage: input.newAccumDamage,
      defeatedCount: input.newDefeatedCount,
    };
    this.bosses.set(input.guildId, boss);
    const key = `${input.guildId}|${input.weekId}|${input.characterId}`;
    this.bossContrib.set(key, (this.bossContrib.get(key) ?? 0n) + input.dmg);
    // 멤버 주간 마커 전진 + weeklyReset 면 weekly 컬럼 0 부터(repo 와 동일).
    if (input.weeklyReset) {
      member.weeklyContribution = 0n;
      member.weeklyAutoContribution = 0n;
    }
    member.weeklyResetId = input.weekId;
    member.weeklyBossChallenges = input.newChallengeCount;
    return { boss, member };
  }

  async claimBossReward(input: {
    characterId: string;
    weekId: string;
    fromClaimedCount: number;
    toDefeatedCount: number;
  }) {
    const member = this.members.get(input.characterId);
    if (!member) {
      return null;
    }
    const current =
      member.bossClaimWeekId === input.weekId ? member.bossClaimedCount : 0;
    if (current !== input.fromClaimedCount) {
      return null;
    }
    member.bossClaimedCount = input.toDefeatedCount;
    member.bossClaimWeekId = input.weekId;
    return member;
  }

  async listBossContrib(input: {
    guildId: string;
    weekId: string;
    limit: number;
  }) {
    const rows: { characterId: string; damage: bigint }[] = [];
    for (const [key, damage] of this.bossContrib) {
      const [guildId, weekId, characterId] = key.split("|");
      if (guildId === input.guildId && weekId === input.weekId) {
        rows.push({ characterId: characterId ?? "", damage });
      }
    }
    rows.sort((a, b) =>
      b.damage > a.damage ? 1 : b.damage < a.damage ? -1 : 0,
    );
    return rows.slice(0, input.limit);
  }

  async listGuildRankings(limit: number) {
    return this.computeRankings().slice(0, limit);
  }

  async getGuildRanking(guildId: string) {
    return this.computeRankings().find((r) => r.guildId === guildId) ?? null;
  }

  // 길드별 Σ weekly_contribution 집계 + 순위(동점은 createdAt 오름차순).
  private computeRankings(): GuildRankingRow[] {
    const sums = new Map<string, bigint>();
    for (const m of this.members.values()) {
      sums.set(m.guildId, (sums.get(m.guildId) ?? 0n) + m.weeklyContribution);
    }
    const rows = [...this.guilds.values()].map((g) => ({
      guildId: g.id,
      name: g.name,
      level: getGuildLevel(g.exp),
      weeklyContribution: sums.get(g.id) ?? 0n,
      createdAt: g.createdAt,
    }));
    rows.sort((a, b) => {
      if (b.weeklyContribution !== a.weeklyContribution) {
        return b.weeklyContribution > a.weeklyContribution ? 1 : -1;
      }
      return a.createdAt.getTime() - b.createdAt.getTime();
    });
    return rows.map((r, i) => ({
      guildId: r.guildId,
      name: r.name,
      level: r.level,
      weeklyContribution: r.weeklyContribution,
      rank: i + 1,
    }));
  }

  // 테스트 헬퍼: 임의 멤버 직접 주입(정원/계급 시나리오).
  seedMembers(
    guildId: string,
    count: number,
    overrides: Partial<GuildMemberRecord>[] = [],
  ) {
    const guild = this.guilds.get(guildId);
    if (guild) {
      guild.memberCount = count;
    }
    for (let i = this.members.size; i < count; i++) {
      const id = `seed-${guildId}-${i}`;
      this.members.set(id, {
        guildId,
        characterId: id,
        rank: "member",
        joinedAt: new Date(0),
        weeklyContribution: 0n,
        totalContribution: 0n,
        contributionPoints: 0n,
        lastAttendanceDate: null,
        weeklyAutoContribution: 0n,
        lastDonationDate: null,
        dailyDonation: 0n,
        weeklyResetId: null,
        weeklyBossChallenges: 0,
        bossClaimedCount: 0,
        bossClaimWeekId: null,
        ...(overrides[i - 1] ?? {}),
      });
    }
  }
}

function makeService(repo = new FakeGuildRepo(), clock?: () => Date) {
  return {
    repo,
    service: new GuildService(repo, clock ?? (() => new Date(0))),
  };
}

// 가변 시계 — 출석/헌납/주간 리셋 시나리오용.
function mutableClock(initial: Date) {
  let current = initial;
  const fn = () => current;
  fn.set = (next: Date) => {
    current = next;
  };
  return fn;
}

describe("RankUnlock parity (순수 함수)", () => {
  it("vice 는 ≥11, officer 는 ≥21 에서 해금 (경계 10/11/20/21)", () => {
    expect(isRankUnlocked("vice", 10)).toBe(false);
    expect(isRankUnlocked("vice", 11)).toBe(true);
    expect(isRankUnlocked("officer", 20)).toBe(false);
    expect(isRankUnlocked("officer", 21)).toBe(true);
    expect(isRankUnlocked("master", 1)).toBe(true);
    expect(isRankUnlocked("member", 1)).toBe(true);
  });

  it("계급 정원 슬롯: vice 1, officer 3, 나머지 무제한", () => {
    expect(getRankSlotCap("vice")).toBe(1);
    expect(getRankSlotCap("officer")).toBe(3);
    expect(getRankSlotCap("member")).toBe(Number.POSITIVE_INFINITY);
  });
});

describe("GuildService.create", () => {
  it("무소속 캐릭터가 유니크 이름으로 생성(생성자=master, member_count=1)", async () => {
    const { repo, service } = makeService();
    const data = await service.create(userId, charA, "테스트길드");
    expect(data.name).toBe("테스트길드");
    expect(data.masterCharacterId).toBe(charA);
    expect(data.memberCount).toBe(1);
    expect((await repo.getMembership(charA))?.rank).toBe("master");
  });

  it("이미 소속이면 거부", async () => {
    const { service } = makeService();
    await service.create(userId, charA, "길드A");
    await expect(service.create(userId, charA, "길드B")).rejects.toBeInstanceOf(
      ConflictError,
    );
  });

  it("이름 중복 거부", async () => {
    const { service } = makeService();
    await service.create(userId, charA, "중복이름");
    await expect(
      service.create(userId, charB, "중복이름"),
    ).rejects.toBeInstanceOf(ConflictError);
  });

  it("이름 길이(2~16) 검증", async () => {
    const { service } = makeService();
    await expect(service.create(userId, charA, "x")).rejects.toBeInstanceOf(
      ValidationError,
    );
  });

  it("소유하지 않은 캐릭터 거부", async () => {
    const { repo, service } = makeService();
    repo.ownedCharacters.clear();
    await expect(
      service.create(userId, charA, "유효한이름"),
    ).rejects.toBeInstanceOf(NotFoundError);
  });
});

describe("GuildService.join", () => {
  it("open 길드는 즉시 가입(member_count +1)", async () => {
    const { repo, service } = makeService();
    const guild = await service.create(userId, charA, "오픈길드");
    const res = await service.join(userId, charB, guild.id);
    expect(res.status).toBe("joined");
    expect((await repo.getGuild(guild.id))?.memberCount).toBe(2);
  });

  it("approval 길드는 신청만 등록", async () => {
    const { repo, service } = makeService();
    const guild = await service.create(userId, charA, "승인길드");
    await service.updateSettings(userId, charA, guild.id, {
      joinMode: "approval",
    });
    const res = await service.join(userId, charB, guild.id);
    expect(res.status).toBe("requested");
    expect((await repo.listRequests(guild.id)).length).toBe(1);
    expect((await repo.getGuild(guild.id))?.memberCount).toBe(1);
  });

  it("정원 초과 가입 거부", async () => {
    const { repo, service } = makeService();
    const guild = await service.create(userId, charA, "만석길드");
    repo.seedMembers(guild.id, GUILD_CAPACITY);
    await expect(service.join(userId, charB, guild.id)).rejects.toBeInstanceOf(
      ConflictError,
    );
  });

  it("1캐릭터 1길드 — 이미 소속이면 거부", async () => {
    const { service } = makeService();
    const g1 = await service.create(userId, charA, "길드1");
    const g2 = await service.create(userId, charB, "길드2");
    await expect(service.join(userId, charA, g2.id)).rejects.toBeInstanceOf(
      ConflictError,
    );
    expect(g1.id).not.toBe(g2.id);
  });
});

describe("GuildService.approve / reject", () => {
  it("master 가 신청 승인 시 가입 처리", async () => {
    const { repo, service } = makeService();
    const guild = await service.create(userId, charA, "승인A");
    await service.updateSettings(userId, charA, guild.id, {
      joinMode: "approval",
    });
    await service.join(userId, charB, guild.id);
    const res = await service.approve(userId, charA, guild.id, charB);
    expect(res.status).toBe("approved");
    expect((await repo.getMembership(charB))?.guildId).toBe(guild.id);
    expect((await repo.listRequests(guild.id)).length).toBe(0);
  });

  it("권한 없는 일반 멤버의 승인 거부", async () => {
    const { service } = makeService();
    const guild = await service.create(userId, charA, "승인B");
    await service.updateSettings(userId, charA, guild.id, {
      joinMode: "approval",
    });
    await service.join(userId, charB, guild.id); // charB 신청
    // charC 를 일반 멤버로 가입시킨 뒤 승인 시도
    await service.updateSettings(userId, charA, guild.id, { joinMode: "open" });
    await service.join(userId, charC, guild.id);
    await expect(
      service.approve(userId, charC, guild.id, charB),
    ).rejects.toBeInstanceOf(ValidationError);
  });

  it("reject 는 신청 삭제", async () => {
    const { repo, service } = makeService();
    const guild = await service.create(userId, charA, "거절길드");
    await service.updateSettings(userId, charA, guild.id, {
      joinMode: "approval",
    });
    await service.join(userId, charB, guild.id);
    const res = await service.reject(userId, charA, guild.id, charB);
    expect(res.status).toBe("rejected");
    expect((await repo.listRequests(guild.id)).length).toBe(0);
  });
});

describe("GuildService.setRank", () => {
  it("11명 미만에서 vice 승급 거부(해금 X)", async () => {
    const { repo, service } = makeService();
    const guild = await service.create(userId, charA, "소규모");
    await service.join(userId, charB, guild.id); // 2명
    repo.seedMembers(guild.id, 10);
    await expect(
      service.setRank(userId, charA, guild.id, charB, "vice"),
    ).rejects.toBeInstanceOf(ValidationError);
  });

  it("11명에서 vice 승급 성공", async () => {
    const { repo, service } = makeService();
    const guild = await service.create(userId, charA, "중규모");
    await service.join(userId, charB, guild.id);
    repo.seedMembers(guild.id, 11);
    const res = await service.setRank(userId, charA, guild.id, charB, "vice");
    expect(res.rank).toBe("vice");
  });

  it("vice 정원(1) 초과 거부", async () => {
    const { repo, service } = makeService();
    const guild = await service.create(userId, charA, "정원초과");
    await service.join(userId, charB, guild.id);
    await service.join(userId, charC, guild.id);
    repo.seedMembers(guild.id, 11);
    await service.setRank(userId, charA, guild.id, charB, "vice");
    await expect(
      service.setRank(userId, charA, guild.id, charC, "vice"),
    ).rejects.toBeInstanceOf(ValidationError);
  });

  it("officer 는 21명 미만에서 거부, 21명에서 허용", async () => {
    const { repo, service } = makeService();
    const guild = await service.create(userId, charA, "간부길드");
    await service.join(userId, charB, guild.id);
    repo.seedMembers(guild.id, 20);
    await expect(
      service.setRank(userId, charA, guild.id, charB, "officer"),
    ).rejects.toBeInstanceOf(ValidationError);

    repo.seedMembers(guild.id, 21);
    const res = await service.setRank(
      userId,
      charA,
      guild.id,
      charB,
      "officer",
    );
    expect(res.rank).toBe("officer");
  });

  it("master 가 아니면 승급 권한 거부", async () => {
    const { repo, service } = makeService();
    const guild = await service.create(userId, charA, "권한길드");
    await service.join(userId, charB, guild.id);
    await service.join(userId, charC, guild.id);
    repo.seedMembers(guild.id, 11);
    // charB 는 일반 멤버 → 승급 권한 없음(master 아님)
    await expect(
      service.setRank(userId, charB, guild.id, charC, "vice"),
    ).rejects.toBeInstanceOf(ValidationError);
  });
});

describe("GuildService.leave (위임/해산)", () => {
  it("일반 멤버 탈퇴는 단순 삭제", async () => {
    const { repo, service } = makeService();
    const guild = await service.create(userId, charA, "탈퇴길드");
    await service.join(userId, charB, guild.id);
    const res = await service.leave(userId, charB);
    expect(res.status).toBe("left");
    expect(await repo.getMembership(charB)).toBeNull();
    expect((await repo.getGuild(guild.id))?.memberCount).toBe(1);
  });

  it("길드장 탈퇴 — 부길드장 우선 위임", async () => {
    const { repo, service } = makeService();
    const guild = await service.create(userId, charA, "위임길드");
    await service.join(userId, charB, guild.id);
    repo.seedMembers(guild.id, 11);
    await service.setRank(userId, charA, guild.id, charB, "vice");
    const res = await service.leave(userId, charA);
    expect(res.status).toBe("transferred");
    expect((await repo.getMembership(charB))?.rank).toBe("master");
    expect(await repo.getMembership(charA)).toBeNull();
  });

  it("길드장 탈퇴 — 부길드장 없으면 총기여 최다 멤버 위임", async () => {
    const { repo, service } = makeService();
    const guild = await service.create(userId, charA, "기여위임");
    await service.join(userId, charB, guild.id);
    await service.join(userId, charC, guild.id);
    const cMember = await repo.getMembership(charC);
    if (cMember) {
      cMember.totalContribution = 999n;
    }
    const res = await service.leave(userId, charA);
    expect(res.status).toBe("transferred");
    expect((await repo.getMembership(charC))?.rank).toBe("master");
  });

  it("길드장 단독이면 해산", async () => {
    const { repo, service } = makeService();
    const guild = await service.create(userId, charA, "해산길드");
    const res = await service.leave(userId, charA);
    expect(res.status).toBe("disbanded");
    expect(await repo.getGuild(guild.id)).toBeNull();
    expect(await repo.getMembership(charA)).toBeNull();
  });
});

describe("GuildService.updateSettings", () => {
  it("name/joinMode 는 master 만 가능", async () => {
    const { service } = makeService();
    const guild = await service.create(userId, charA, "설정길드");
    await service.join(userId, charB, guild.id);
    // charB 는 일반 멤버 → 권한 없음(master|vice 아님)
    await expect(
      service.updateSettings(userId, charB, guild.id, { name: "새이름" }),
    ).rejects.toBeInstanceOf(ValidationError);
  });

  it("notice 는 vice 도 가능", async () => {
    const { repo, service } = makeService();
    const guild = await service.create(userId, charA, "공지길드");
    await service.join(userId, charB, guild.id);
    repo.seedMembers(guild.id, 11);
    await service.setRank(userId, charA, guild.id, charB, "vice");
    const res = await service.updateSettings(userId, charB, guild.id, {
      notice: "환영합니다",
    });
    expect(res.notice).toBe("환영합니다");
  });
});

describe("GuildService.snapshot", () => {
  it("무소속이면 guild=null", async () => {
    const { service } = makeService();
    const res = await service.snapshot(userId, charA);
    expect(res.guild).toBeNull();
  });

  it("소속이면 길드/내정보/멤버 반환, 관리자만 requests 노출", async () => {
    const { service } = makeService();
    const guild = await service.create(userId, charA, "스냅길드");
    await service.updateSettings(userId, charA, guild.id, {
      joinMode: "approval",
    });
    await service.join(userId, charB, guild.id);
    const masterView = await service.snapshot(userId, charA);
    expect(masterView.guild?.id).toBe(guild.id);
    expect(masterView.me?.rank).toBe("master");
    expect(masterView.requests.length).toBe(1);
  });

  it("스냅샷에 길드 레벨/버프·출석 가능·헌납 잔여 노출", async () => {
    const { service } = makeService();
    const guild = await service.create(userId, charA, "버프스냅");
    const res = await service.snapshot(userId, charA);
    expect(res.guild?.level).toBe(1);
    expect(res.buff?.attackPct).toBeCloseTo(0.004);
    expect(res.buff?.goldPct).toBeCloseTo(0.004);
    expect(res.me?.canAttend).toBe(true);
    expect(res.me?.donationRemaining).toBe(String(DONATION_DAILY_CAP));
    expect(guild.id).toBeTruthy();
  });
});

describe("GuildLevelFormula (순수 함수, 클라 parity)", () => {
  it("exp 0/음수는 레벨 1", () => {
    expect(getGuildLevel(0n)).toBe(1);
    expect(getGuildLevel(-5n)).toBe(1);
  });

  it("레벨 임계 경계값(BASE=10000, GROWTH=1.6 누적)", () => {
    // L2 임계 = 10000, L3 임계 = 10000 + 16000 = 26000.
    const step1 = BigInt(GUILD_LEVEL_BASE); // 10000
    const step2 = BigInt(Math.floor(GUILD_LEVEL_BASE * 1.6)); // 16000
    expect(getGuildLevel(step1 - 1n)).toBe(1);
    expect(getGuildLevel(step1)).toBe(2); // 정확히 임계 → 레벨업
    expect(getGuildLevel(step1 + step2 - 1n)).toBe(2);
    expect(getGuildLevel(step1 + step2)).toBe(3);
  });

  it("버프는 레벨당 +0.4% 공격/골드", () => {
    expect(getGuildBuff(1)).toEqual({ attackPct: 0.004, goldPct: 0.004 });
    const b10 = getGuildBuff(10);
    expect(b10.attackPct).toBeCloseTo(0.04);
    expect(b10.goldPct).toBeCloseTo(0.04);
  });
});

describe("GuildService.attendance", () => {
  it("출석 1회 +50 적립(EXP·포인트 동시)", async () => {
    const clock = mutableClock(new Date("2026-06-01T00:00:00Z"));
    const repo = new FakeGuildRepo();
    const { service } = makeService(repo, clock);
    const guild = await service.create(userId, charA, "출석길드");
    const res = await service.attendance(userId, charA, guild.id);
    expect(res.gained).toBe(String(ATTENDANCE_REWARD));
    const member = await repo.getMembership(charA);
    expect(member?.contributionPoints).toBe(BigInt(ATTENDANCE_REWARD));
    expect(member?.weeklyContribution).toBe(BigInt(ATTENDANCE_REWARD));
    expect((await repo.getGuild(guild.id))?.exp).toBe(
      BigInt(ATTENDANCE_REWARD),
    );
  });

  it("같은 날 두 번째 출석은 거부", async () => {
    const clock = mutableClock(new Date("2026-06-01T03:00:00Z"));
    const repo = new FakeGuildRepo();
    const { service } = makeService(repo, clock);
    const guild = await service.create(userId, charA, "중복출석");
    await service.attendance(userId, charA, guild.id);
    await expect(
      service.attendance(userId, charA, guild.id),
    ).rejects.toBeInstanceOf(ConflictError);
  });

  it("다음 날이면 다시 출석 가능", async () => {
    const clock = mutableClock(new Date("2026-06-01T00:00:00Z"));
    const repo = new FakeGuildRepo();
    const { service } = makeService(repo, clock);
    const guild = await service.create(userId, charA, "이틀출석");
    await service.attendance(userId, charA, guild.id);
    clock.set(new Date("2026-06-02T00:00:00Z"));
    const res = await service.attendance(userId, charA, guild.id);
    expect(res.gained).toBe(String(ATTENDANCE_REWARD));
    const member = await repo.getMembership(charA);
    expect(member?.contributionPoints).toBe(BigInt(ATTENDANCE_REWARD) * 2n);
  });
});

describe("GuildService.donate", () => {
  it("floor(gold/1000) 기여(서버 골드 미차감)", async () => {
    const repo = new FakeGuildRepo();
    const { service } = makeService(repo);
    const guild = await service.create(userId, charA, "헌납길드");
    const res = await service.donate(userId, charA, guild.id, 3500);
    expect(res.gained).toBe("3"); // floor(3500/1000)
    expect((await repo.getMembership(charA))?.contributionPoints).toBe(3n);
  });

  it("일일 상한(500) 초과분은 잔여까지만 적립", async () => {
    const repo = new FakeGuildRepo();
    const { service } = makeService(repo);
    const guild = await service.create(userId, charA, "상한헌납");
    // 1,000,000 골드 → 1000 기여지만 상한 500 까지만.
    const res = await service.donate(userId, charA, guild.id, 1_000_000);
    expect(res.gained).toBe(String(DONATION_DAILY_CAP));
    // 상한 도달 후 추가 헌납 거부.
    await expect(
      service.donate(userId, charA, guild.id, 5000),
    ).rejects.toBeInstanceOf(ValidationError);
  });

  it("1000 골드 미만은 거부", async () => {
    const repo = new FakeGuildRepo();
    const { service } = makeService(repo);
    const guild = await service.create(userId, charA, "소액헌납");
    await expect(
      service.donate(userId, charA, guild.id, 500),
    ).rejects.toBeInstanceOf(ValidationError);
  });
});

describe("GuildService.contribute (자동 기여)", () => {
  it("자동 기여 적립(주간)", async () => {
    const repo = new FakeGuildRepo();
    const { service } = makeService(repo);
    const guild = await service.create(userId, charA, "자동길드");
    const res = await service.contribute(userId, charA, guild.id, 300);
    expect(res.gained).toBe("300");
    const member = await repo.getMembership(charA);
    expect(member?.weeklyAutoContribution).toBe(300n);
    expect(member?.contributionPoints).toBe(300n);
  });

  it("주간 자동 상한 초과분은 잔여까지만", async () => {
    const repo = new FakeGuildRepo();
    const { service } = makeService(repo);
    const guild = await service.create(userId, charA, "자동상한");
    const res = await service.contribute(
      userId,
      charA,
      guild.id,
      AUTO_WEEKLY_CAP + 5000,
    );
    expect(res.gained).toBe(String(AUTO_WEEKLY_CAP));
    // 상한 도달 → 추가분 capped.
    const res2 = await service.contribute(userId, charA, guild.id, 100);
    expect(res2.status).toBe("capped");
    expect(res2.gained).toBe("0");
  });
});

describe("GuildService 주간 리셋", () => {
  it("새 주 진입 시 weekly_contribution/auto 0 부터 재집계, exp·points 영속", async () => {
    const clock = mutableClock(new Date("2026-06-01T00:00:00Z")); // 2026-W23
    const repo = new FakeGuildRepo();
    const { service } = makeService(repo, clock);
    const guild = await service.create(userId, charA, "리셋길드");
    await service.contribute(userId, charA, guild.id, 500);
    let member = await repo.getMembership(charA);
    expect(member?.weeklyContribution).toBe(500n);

    // 다음 주(약 +8일)로 이동 후 자동 기여 → weekly 리셋되어 200 만.
    clock.set(new Date("2026-06-09T00:00:00Z")); // 다음 주
    await service.contribute(userId, charA, guild.id, 200);
    member = await repo.getMembership(charA);
    expect(member?.weeklyContribution).toBe(200n); // 리셋됨
    expect(member?.weeklyAutoContribution).toBe(200n);
    // 누적 EXP/포인트는 영속(500 + 200).
    expect(member?.contributionPoints).toBe(700n);
    expect((await repo.getGuild(guild.id))?.exp).toBe(700n);
  });
});

describe("GuildService.shop / shopBuy", () => {
  it("상점 카탈로그 조회", async () => {
    const { service } = makeService();
    const guild = await service.create(userId, charA, "상점길드");
    const res = await service.shop(userId, charA, guild.id);
    expect(res.items.length).toBe(GUILD_SHOP_CATALOG.length);
    expect(res.items[0]).toHaveProperty("price");
  });

  it("포인트 충분하면 구매·차감·보상 반환", async () => {
    const repo = new FakeGuildRepo();
    const { service } = makeService(repo);
    const guild = await service.create(userId, charA, "구매길드");
    // 출석/헌납으로 포인트 적립.
    await service.donate(userId, charA, guild.id, 200_000); // +200 포인트
    const item = GUILD_SHOP_CATALOG[0]; // 카탈로그 첫 항목(가격은 카탈로그 상수에서 가져옴)
    const res = await service.shopBuy(userId, charA, guild.id, item.id);
    expect(res.itemId).toBe(item.id);
    expect(res.reward).toEqual(item.reward);
    expect((await repo.getMembership(charA))?.contributionPoints).toBe(
      200n - BigInt(item.price),
    );
  });

  it("포인트 부족 시 구매 거부", async () => {
    const repo = new FakeGuildRepo();
    const { service } = makeService(repo);
    const guild = await service.create(userId, charA, "부족길드");
    const item = GUILD_SHOP_CATALOG[0];
    await expect(
      service.shopBuy(userId, charA, guild.id, item.id),
    ).rejects.toBeInstanceOf(ConflictError);
  });

  it("존재하지 않는 아이템은 NotFound", async () => {
    const { service } = makeService();
    const guild = await service.create(userId, charA, "없는아이템");
    await expect(
      service.shopBuy(userId, charA, guild.id, "no_such_item"),
    ).rejects.toBeInstanceOf(NotFoundError);
  });
});

describe("GuildBossFormula (순수 함수, 클라 parity)", () => {
  it("HP = floor(BASE * 1.5^defeated) (격파마다 상향)", () => {
    expect(getGuildBossHp(0)).toBe(GUILD_BOSS_BASE_HP);
    expect(getGuildBossHp(1)).toBe(Math.floor(GUILD_BOSS_BASE_HP * 1.5));
    expect(getGuildBossHp(2)).toBe(Math.floor(GUILD_BOSS_BASE_HP * 1.5 ** 2));
    // 음수 방어(0 으로 클램프).
    expect(getGuildBossHp(-3)).toBe(GUILD_BOSS_BASE_HP);
  });

  it("getChallengeDamage = trunc(cp), 음수/비정상 0", () => {
    expect(getChallengeDamage(12345)).toBe(12345);
    expect(getChallengeDamage(99.9)).toBe(99);
    expect(getChallengeDamage(0)).toBe(0);
    expect(getChallengeDamage(-50)).toBe(0);
    expect(getChallengeDamage(Number.NaN)).toBe(0);
  });
});

describe("GuildService.challengeBoss", () => {
  it("도전 시 누적·내 기여 데미지 적립, 잔여 횟수 감소", async () => {
    const repo = new FakeGuildRepo();
    const { service } = makeService(repo);
    const guild = await service.create(userId, charA, "보스길드");
    const res = await service.challengeBoss(userId, charA, guild.id, 5000);
    expect(res.damage).toBe("5000");
    expect(res.accumDamage).toBe("5000");
    expect(res.defeatedCount).toBe(0);
    expect(res.challengesRemaining).toBe(WEEKLY_GUILD_BOSS_CHALLENGE_LIMIT - 1);
    const boss = await repo.getBoss(guild.id);
    expect(boss?.accumDamage).toBe(5000n);
  });

  it("누적이 여러 명에 걸쳐 쌓임(비동기 협력)", async () => {
    const repo = new FakeGuildRepo();
    const { service } = makeService(repo);
    const guild = await service.create(userId, charA, "협력길드");
    await service.join(userId, charB, guild.id);
    await service.challengeBoss(userId, charA, guild.id, 100_000);
    const res = await service.challengeBoss(userId, charB, guild.id, 50_000);
    expect(res.accumDamage).toBe("150000");
    expect(res.defeatedCount).toBe(0);
  });

  it("주간 도전 한도(7) 초과 거부", async () => {
    const repo = new FakeGuildRepo();
    const { service } = makeService(repo);
    const guild = await service.create(userId, charA, "한도길드");
    for (let i = 0; i < WEEKLY_GUILD_BOSS_CHALLENGE_LIMIT; i++) {
      await service.challengeBoss(userId, charA, guild.id, 10);
    }
    await expect(
      service.challengeBoss(userId, charA, guild.id, 10),
    ).rejects.toBeInstanceOf(ConflictError);
  });

  it("단일 격파: 누적이 HP 도달 시 defeated++·이월 차감", async () => {
    const repo = new FakeGuildRepo();
    const { service } = makeService(repo);
    const guild = await service.create(userId, charA, "격파길드");
    // BASE_HP(1,000,000) + 1 데미지 → 1격파, 이월 1.
    const res = await service.challengeBoss(
      userId,
      charA,
      guild.id,
      GUILD_BOSS_BASE_HP + 1,
    );
    expect(res.defeatedCount).toBe(1);
    expect(res.newDefeats).toBe(1);
    expect(res.accumDamage).toBe("1"); // 이월
    expect(res.bossHp).toBe(String(getGuildBossHp(1)));
  });

  it("연속 격파: 한 번의 도전이 여러 HP 임계를 넘으면 defeated 다중 증가", async () => {
    const repo = new FakeGuildRepo();
    const { service } = makeService(repo);
    const guild = await service.create(userId, charA, "연속격파");
    // HP0=1,000,000, HP1=1,500,000, HP2=2,250,000. 합 4,750,000.
    // 5,000,000 데미지 → 3격파, 이월 250,000.
    const hp0 = getGuildBossHp(0);
    const hp1 = getGuildBossHp(1);
    const hp2 = getGuildBossHp(2);
    const total = hp0 + hp1 + hp2; // 4,750,000
    const res = await service.challengeBoss(
      userId,
      charA,
      guild.id,
      total + 250_000,
    );
    expect(res.defeatedCount).toBe(3);
    expect(res.newDefeats).toBe(3);
    expect(res.accumDamage).toBe("250000");
  });

  it("보스 데미지 → 기여(floor(dmg/D)) 1회만 적립(이중적립 금지)", async () => {
    const repo = new FakeGuildRepo();
    const { service } = makeService(repo);
    const guild = await service.create(userId, charA, "기여보스");
    // dmg=25,000, D=10,000 → +2 기여.
    const res = await service.challengeBoss(userId, charA, guild.id, 25_000);
    const expected = Math.floor(25_000 / GUILD_BOSS_DMG_TO_CONTRIB);
    expect(res.contributionGain).toBe(String(expected));
    const member = await repo.getMembership(charA);
    expect(member?.contributionPoints).toBe(BigInt(expected));
    expect(member?.weeklyContribution).toBe(BigInt(expected));
    // EXP 도 1회만 반영.
    expect((await repo.getGuild(guild.id))?.exp).toBe(BigInt(expected));
  });

  it("데미지가 D 미만이면 기여 0(누적 데미지는 적립)", async () => {
    const repo = new FakeGuildRepo();
    const { service } = makeService(repo);
    const guild = await service.create(userId, charA, "소액보스");
    const res = await service.challengeBoss(userId, charA, guild.id, 5000);
    expect(res.contributionGain).toBe("0");
    expect(res.accumDamage).toBe("5000");
    expect((await repo.getMembership(charA))?.contributionPoints).toBe(0n);
  });

  it("주간 리셋: 새 주 진입 시 보스 누적·도전 횟수 0 부터", async () => {
    const clock = mutableClock(new Date("2026-06-01T00:00:00Z")); // 2026-W23
    const repo = new FakeGuildRepo();
    const { service } = makeService(repo, clock);
    const guild = await service.create(userId, charA, "보스리셋");
    await service.challengeBoss(userId, charA, guild.id, 300_000);
    expect((await repo.getBoss(guild.id))?.accumDamage).toBe(300_000n);

    clock.set(new Date("2026-06-09T00:00:00Z")); // 다음 주
    const res = await service.challengeBoss(userId, charA, guild.id, 100_000);
    expect(res.accumDamage).toBe("100000"); // 보스 누적 리셋
    expect(res.challengesRemaining).toBe(WEEKLY_GUILD_BOSS_CHALLENGE_LIMIT - 1); // 도전 횟수 리셋
  });

  it("비멤버는 도전 거부", async () => {
    const repo = new FakeGuildRepo();
    const { service } = makeService(repo);
    const guild = await service.create(userId, charA, "비멤버보스");
    await expect(
      service.challengeBoss(userId, charB, guild.id, 100),
    ).rejects.toBeInstanceOf(NotFoundError);
  });
});

describe("GuildService.claimBossReward", () => {
  it("격파분 미수령 보상 전원 지급(격파 N건 × 보상)", async () => {
    const repo = new FakeGuildRepo();
    const { service } = makeService(repo);
    const guild = await service.create(userId, charA, "보상길드");
    await service.join(userId, charB, guild.id);
    // 2격파 발생.
    const hp0 = getGuildBossHp(0);
    const hp1 = getGuildBossHp(1);
    await service.challengeBoss(userId, charA, guild.id, hp0 + hp1);
    // charB(미도전 멤버)도 전원 보상 수령 가능.
    const res = await service.claimBossReward(userId, charB, guild.id);
    expect(res.status).toBe("claimed");
    expect(res.defeats).toBe(2);
    expect(res.claimedCount).toBe(2);
    // 재수령 시 미수령 없음.
    await expect(
      service.claimBossReward(userId, charB, guild.id),
    ).rejects.toBeInstanceOf(ConflictError);
  });

  it("격파 0건이면 수령할 보상 없음", async () => {
    const repo = new FakeGuildRepo();
    const { service } = makeService(repo);
    const guild = await service.create(userId, charA, "무보상");
    await service.challengeBoss(userId, charA, guild.id, 100); // 격파 X
    await expect(
      service.claimBossReward(userId, charA, guild.id),
    ).rejects.toBeInstanceOf(ConflictError);
  });

  it("추가 격파 후 차분만 수령(마일스톤 추적)", async () => {
    const repo = new FakeGuildRepo();
    const { service } = makeService(repo);
    const guild = await service.create(userId, charA, "차분보상");
    await service.challengeBoss(userId, charA, guild.id, getGuildBossHp(0));
    const first = await service.claimBossReward(userId, charA, guild.id);
    expect(first.defeats).toBe(1);
    // 추가 격파 1건.
    await service.challengeBoss(userId, charA, guild.id, getGuildBossHp(1));
    const second = await service.claimBossReward(userId, charA, guild.id);
    expect(second.defeats).toBe(1); // 차분만
    expect(second.claimedCount).toBe(2);
  });
});

describe("GuildService.guildRankings", () => {
  it("길드별 Σ weekly_contribution 내림차순 정렬", async () => {
    const repo = new FakeGuildRepo();
    const { service } = makeService(repo);
    const g1 = await service.create(userId, charA, "랭킹1");
    const g2 = await service.create(userId, charB, "랭킹2");
    // g2 가 더 많은 주간 기여.
    await service.contribute(userId, charA, g1.id, 100);
    await service.contribute(userId, charB, g2.id, 500);
    const res = await service.guildRankings({ limit: 10 });
    expect(res.rankings[0].guildId).toBe(g2.id);
    expect(res.rankings[0].rank).toBe(1);
    expect(res.rankings[1].guildId).toBe(g1.id);
  });

  it("동점은 생성 순(createdAt 오름차순)으로 안정 정렬", async () => {
    const clock = mutableClock(new Date("2026-06-01T00:00:00Z"));
    const repo = new FakeGuildRepo();
    const { service } = makeService(repo, clock);
    const g1 = await service.create(userId, charA, "동점A");
    clock.set(new Date("2026-06-01T01:00:00Z"));
    const g2 = await service.create(userId, charB, "동점B");
    // 둘 다 기여 0(동점).
    const res = await service.guildRankings({ limit: 10 });
    expect(res.rankings[0].guildId).toBe(g1.id); // 먼저 생성
    expect(res.rankings[1].guildId).toBe(g2.id);
  });

  it("characterId 지정 시 내 길드 순위 동봉", async () => {
    const repo = new FakeGuildRepo();
    const { service } = makeService(repo);
    const g1 = await service.create(userId, charA, "내랭킹1");
    const g2 = await service.create(userId, charB, "내랭킹2");
    await service.contribute(userId, charB, g2.id, 500);
    const res = await service.guildRankings({
      limit: 10,
      userId,
      characterId: charA,
    });
    expect(res.me?.guildId).toBe(g1.id);
    expect(res.me?.rank).toBe(2);
  });

  it("무소속이면 me=null", async () => {
    const repo = new FakeGuildRepo();
    const { service } = makeService(repo);
    await service.create(userId, charB, "타길드");
    const res = await service.guildRankings({
      limit: 10,
      userId,
      characterId: charA, // 무소속
    });
    expect(res.me).toBeNull();
  });
});

describe("GuildService.snapshot (보스 상태)", () => {
  it("스냅샷에 보스 상태(HP/누적/도전 잔여/미수령) 포함", async () => {
    const repo = new FakeGuildRepo();
    const { service } = makeService(repo);
    const guild = await service.create(userId, charA, "보스스냅");
    await service.challengeBoss(userId, charA, guild.id, getGuildBossHp(0) + 7);
    const snap = await service.snapshot(userId, charA);
    expect(snap.boss?.defeatedCount).toBe(1);
    expect(snap.boss?.unclaimedDefeats).toBe(1);
    expect(snap.boss?.accumDamage).toBe("7");
    expect(snap.boss?.challengesRemaining).toBe(
      WEEKLY_GUILD_BOSS_CHALLENGE_LIMIT - 1,
    );
    expect(snap.boss?.hp).toBe(String(getGuildBossHp(1)));
  });
});
