import { describe, expect, it } from "vitest";
import {
  ConflictError,
  NotFoundError,
  ValidationError,
} from "../../core/errors.js";
import {
  GUILD_CAPACITY,
  type GuildJoinMode,
  type GuildJoinRequestRecord,
  type GuildMemberRecord,
  type GuildRank,
  type GuildRecord,
  type GuildRepo,
  GuildService,
  getRankSlotCap,
  isRankUnlocked,
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
        ...(overrides[i - 1] ?? {}),
      });
    }
  }
}

function makeService(repo = new FakeGuildRepo()) {
  return { repo, service: new GuildService(repo, () => new Date(0)) };
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
});
