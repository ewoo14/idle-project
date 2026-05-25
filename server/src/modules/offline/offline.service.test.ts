import { describe, expect, it, vi } from "vitest";
import { NotFoundError, ValidationError } from "../../core/errors.js";
import type { OfflineRepo } from "./offline.service.js";
import { OfflineService } from "./offline.service.js";

const character = {
  id: "00000000-0000-0000-0000-000000000010",
  userId: "00000000-0000-0000-0000-000000000001",
  level: 10,
  rebirthCount: 2,
  gold: 100,
  totalExp: 200,
  lastSeenAt: new Date("2026-05-26T00:00:00.000Z"),
};

describe("OfflineService", () => {
  it("previews rewards from the character lastSeenAt without mutating state", async () => {
    const repo = createRepo();
    const service = new OfflineService(
      repo,
      () => new Date("2026-05-26T01:00:00.000Z"),
    );

    const result = await service.preview(userId(), character.id);

    expect(result.rewards.cappedSeconds).toBe(3_600);
    expect(result.lastSeenUnixSec).toBe(1_779_753_600);
    expect(repo.claim).not.toHaveBeenCalled();
  });

  it("claims rewards atomically and advances lastSeenAt", async () => {
    const repo = createRepo();
    const service = new OfflineService(
      repo,
      () => new Date("2026-05-26T01:00:00.000Z"),
    );

    const result = await service.claim(userId(), character.id);

    expect(repo.claim).toHaveBeenCalledWith({
      characterId: character.id,
      gold: result.rewards.gold,
      exp: result.rewards.exp,
      now: new Date("2026-05-26T01:00:00.000Z"),
    });
    expect(result.totals).toEqual({ gold: 1100, totalExp: 1200 });
  });

  it("uses now as lastSeen when a character has no previous lastSeenAt", async () => {
    const repo = createRepo({ ...character, lastSeenAt: null });
    const service = new OfflineService(
      repo,
      () => new Date("2026-05-26T01:00:00.000Z"),
    );

    const result = await service.preview(userId(), character.id);

    expect(result.rewards.cappedSeconds).toBe(0);
    expect(result.rewards.gold).toBe(0);
    expect(result.rewards.exp).toBe(0);
  });

  it("rejects claims with no elapsed offline time", async () => {
    const repo = createRepo({
      ...character,
      lastSeenAt: new Date("2026-05-26T01:00:00.000Z"),
    });
    const service = new OfflineService(
      repo,
      () => new Date("2026-05-26T01:00:00.000Z"),
    );

    await expect(service.claim(userId(), character.id)).rejects.toBeInstanceOf(
      ValidationError,
    );
  });

  it("rejects unknown or foreign characters", async () => {
    const service = new OfflineService(createRepo(null));

    await expect(
      service.preview(userId(), character.id),
    ).rejects.toBeInstanceOf(NotFoundError);
  });
});

function createRepo(
  record: Awaited<ReturnType<OfflineRepo["findCharacter"]>> = character,
): OfflineRepo {
  return {
    findCharacter: vi.fn().mockResolvedValue(record),
    claim: vi.fn().mockResolvedValue({ gold: 1100, totalExp: 1200 }),
  };
}

function userId() {
  return "00000000-0000-0000-0000-000000000001";
}
