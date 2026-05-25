import { describe, expect, it, vi } from "vitest";
import { ValidationError } from "../../core/errors.js";
import { cumulativeExp } from "../../core/formulas/index.js";
import { type SaveRepo, SaveService } from "./save.service.js";

const character = {
  id: "00000000-0000-0000-0000-000000000010",
  userId: "00000000-0000-0000-0000-000000000001",
  classId: 1,
  level: 10,
  rebirthCount: 2,
  stats: {},
  skillTree: {},
  inventory: [],
  lastSaveAt: null,
};

describe("SaveService", () => {
  it("stores a valid save and marks it server validated", async () => {
    const repo = createRepo();
    const leaderboard = createLeaderboard();
    const service = new SaveService(repo, leaderboard);

    const result = await service.upload(userId(), {
      characterId: character.id,
      version: 1,
      payload: { level: 10, rebirthCount: 2, maxEquipmentGrade: 1 },
    });

    expect(result.id).toBe("save-1");
    expect(repo.insertSave).toHaveBeenCalledWith(
      expect.objectContaining({ serverValidated: true }),
    );
  });

  it("rejects payloads outside level bounds", async () => {
    const service = new SaveService(createRepo(), createLeaderboard());

    await expect(
      service.upload(userId(), {
        characterId: character.id,
        version: 1,
        payload: { level: 201, rebirthCount: 2, maxEquipmentGrade: 1 },
      }),
    ).rejects.toBeInstanceOf(ValidationError);
  });

  it("rejects maxEquipmentGrade 6", async () => {
    const service = new SaveService(createRepo(), createLeaderboard());

    await expect(
      service.upload(userId(), {
        characterId: character.id,
        version: 1,
        payload: { level: 10, rebirthCount: 2, maxEquipmentGrade: 6 },
      }),
    ).rejects.toBeInstanceOf(ValidationError);
  });

  it("rejects totalExp that does not match level cumulative exp", async () => {
    const service = new SaveService(createRepo(), createLeaderboard());

    await expect(
      service.upload(userId(), {
        characterId: character.id,
        version: 1,
        payload: {
          level: 10,
          rebirthCount: 2,
          maxEquipmentGrade: 1,
          totalExp: 1,
        },
      }),
    ).rejects.toBeInstanceOf(ValidationError);
  });

  it("accepts totalExp within one percent of level cumulative exp", async () => {
    const service = new SaveService(createRepo(), createLeaderboard());

    const result = await service.upload(userId(), {
      characterId: character.id,
      version: 1,
      payload: {
        level: 10,
        rebirthCount: 2,
        maxEquipmentGrade: 1,
        totalExp: cumulativeExp(10),
      },
    });

    expect(result.id).toBe("save-1");
  });

  it("updates leaderboard after a successful upload", async () => {
    const leaderboard = createLeaderboard();
    const service = new SaveService(createRepo(), leaderboard);

    await service.upload(userId(), {
      characterId: character.id,
      version: 1,
      payload: { level: 10, rebirthCount: 2, maxEquipmentGrade: 1 },
    });

    expect(leaderboard.updatePower).toHaveBeenCalledWith({
      characterId: character.id,
      seasonId: 1,
      score: 11000n,
    });
    expect(leaderboard.updateRebirth).toHaveBeenCalledWith({
      characterId: character.id,
      seasonId: 1,
      rebirthCount: 2,
    });
  });

  it("normalizes history limit to 1-50", async () => {
    const repo = createRepo();
    const service = new SaveService(repo, createLeaderboard());

    await service.history(userId(), character.id, 100);

    expect(repo.listHistory).toHaveBeenCalledWith(character.id, 50);
  });
});

function createRepo(): SaveRepo {
  return {
    findCharacterByUser: vi.fn().mockResolvedValue(character),
    insertSave: vi.fn().mockResolvedValue({ id: "save-1" }),
    listHistory: vi.fn().mockResolvedValue([]),
  };
}

function createLeaderboard() {
  return {
    updatePower: vi.fn(),
    updateRebirth: vi.fn(),
  };
}

function userId() {
  return "00000000-0000-0000-0000-000000000001";
}
