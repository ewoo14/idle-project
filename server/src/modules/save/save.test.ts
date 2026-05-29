import { readFileSync } from "node:fs";
import { describe, expect, it, vi } from "vitest";
import { ValidationError } from "../../core/errors.js";
import { cumulativeExp } from "../../core/formulas/index.js";
import { putSaveSchema } from "./save.schema.js";
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
  gold: 100,
  totalExp: cumulativeExp(10),
  lastSeenAt: null,
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

  it("accepts high-level cloud saves up to the infinite-growth server cap", async () => {
    const service = new SaveService(createRepo(), createLeaderboard());

    const result = await service.upload(userId(), {
      characterId: character.id,
      version: 1,
      payload: { level: 1000, rebirthCount: 2, maxEquipmentGrade: 1 },
    });

    expect(result.id).toBe("save-1");
  });

  it("rejects payloads outside level bounds", async () => {
    const service = new SaveService(createRepo(), createLeaderboard());

    await expect(
      service.upload(userId(), {
        characterId: character.id,
        version: 1,
        payload: { level: 1001, rebirthCount: 2, maxEquipmentGrade: 1 },
      }),
    ).rejects.toBeInstanceOf(ValidationError);
  });

  it.each([
    0, 1, 2, 3, 4, 5, 6, 7,
  ])("accepts maxEquipmentGrade %i across the v7 rarity range", async (maxEquipmentGrade) => {
    const service = new SaveService(createRepo(), createLeaderboard());

    const result = await service.upload(userId(), {
      characterId: character.id,
      version: 1,
      payload: { level: 10, rebirthCount: 2, maxEquipmentGrade },
    });

    expect(result.id).toBe("save-1");
  });

  it("accepts Mythic maxEquipmentGrade 7", async () => {
    const service = new SaveService(createRepo(), createLeaderboard());

    const result = await service.upload(userId(), {
      characterId: character.id,
      version: 1,
      payload: { level: 10, rebirthCount: 2, maxEquipmentGrade: 7 },
    });

    expect(result.id).toBe("save-1");
  });

  it("rejects maxEquipmentGrade below None", async () => {
    const service = new SaveService(createRepo(), createLeaderboard());

    await expect(
      service.upload(userId(), {
        characterId: character.id,
        version: 1,
        payload: { level: 10, rebirthCount: 2, maxEquipmentGrade: -1 },
      }),
    ).rejects.toBeInstanceOf(ValidationError);
  });

  it("rejects maxEquipmentGrade 8", async () => {
    const service = new SaveService(createRepo(), createLeaderboard());

    await expect(
      service.upload(userId(), {
        characterId: character.id,
        version: 1,
        payload: { level: 10, rebirthCount: 2, maxEquipmentGrade: 8 },
      }),
    ).rejects.toBeInstanceOf(ValidationError);
  });

  it("rejects totalExp below the cumulative exp required for the payload level", async () => {
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

  it("accepts totalExp above the current level floor for rebirth and transcend history", async () => {
    const service = new SaveService(createRepo(), createLeaderboard());

    const result = await service.upload(userId(), {
      characterId: character.id,
      version: 1,
      payload: {
        level: 10,
        rebirthCount: 2,
        maxEquipmentGrade: 1,
        totalExp: cumulativeExp(10) * 3,
      },
    });

    expect(result.id).toBe("save-1");
  });

  it.each([
    ["transcendCount", -1],
    ["towerHighestFloor", -1],
    ["skillPoints", -1],
  ])("rejects negative %s extension fields", async (field, value) => {
    const service = new SaveService(createRepo(), createLeaderboard());

    await expect(
      service.upload(userId(), {
        characterId: character.id,
        version: 1,
        payload: {
          level: 10,
          rebirthCount: 2,
          maxEquipmentGrade: 1,
          [field]: value,
        },
      }),
    ).rejects.toBeInstanceOf(ValidationError);
  });

  it("accepts non-negative cloud progress extension fields", async () => {
    const service = new SaveService(createRepo(), createLeaderboard());

    const result = await service.upload(userId(), {
      characterId: character.id,
      version: 1,
      payload: {
        level: 10,
        rebirthCount: 2,
        maxEquipmentGrade: 1,
        transcendCount: 3,
        towerHighestFloor: 25,
        skillPoints: 12,
        worldPower: 21,
        masteryLevels: [1, 2, 3, 4, 5, 6],
        consumables: [{ type: 4, count: 2, buffEndUnixSec: 1_234_567_890 }],
        customClientField: "kept",
      },
    });

    expect(result.id).toBe("save-1");
  });

  it("rejects level or rebirth progress regression below the server character", async () => {
    const service = new SaveService(createRepo(), createLeaderboard());

    await expect(
      service.upload(userId(), {
        characterId: character.id,
        version: 1,
        payload: { level: 9, rebirthCount: 2, maxEquipmentGrade: 1 },
      }),
    ).rejects.toBeInstanceOf(ValidationError);

    await expect(
      service.upload(userId(), {
        characterId: character.id,
        version: 1,
        payload: { level: 10, rebirthCount: 1, maxEquipmentGrade: 1 },
      }),
    ).rejects.toBeInstanceOf(ValidationError);
  });

  it("keeps the Fastify save upload schema aligned with cloud payload bounds", () => {
    const payloadSchema = putSaveSchema.body.properties.payload;

    expect(payloadSchema.properties.level.maximum).toBe(1000);
    expect(payloadSchema.properties.maxEquipmentGrade.maximum).toBe(7);
    expect(payloadSchema.properties.transcendCount).toEqual({
      type: "integer",
      minimum: 0,
    });
    expect(payloadSchema.properties.towerHighestFloor).toEqual({
      type: "integer",
      minimum: 0,
    });
    expect(payloadSchema.properties.skillPoints).toEqual({
      type: "integer",
      minimum: 0,
    });
    expect(payloadSchema.properties.worldPower).toEqual({
      type: "integer",
      minimum: 0,
    });
    expect(payloadSchema.properties.masteryLevels).toEqual({
      type: "array",
      items: { type: "integer", minimum: 0 },
      minItems: 0,
      maxItems: 6,
    });
    expect(payloadSchema.properties.consumables).toEqual({
      type: "array",
      items: {
        type: "object",
        required: ["type", "count", "buffEndUnixSec"],
        additionalProperties: false,
        properties: {
          type: { type: "integer", minimum: 0, maximum: 5 },
          count: { type: "integer", minimum: 0 },
          buffEndUnixSec: { type: "integer", minimum: 0 },
        },
      },
    });
    expect(payloadSchema.additionalProperties).toBe(true);
  });

  it("keeps character totalExp storage on the bigint path for level 1000 saves", () => {
    const repoSource = readFileSync(
      new URL("./save.repo.ts", import.meta.url),
      "utf8",
    );
    const schemaSource = readFileSync(
      new URL("../../db/schema.ts", import.meta.url),
      "utf8",
    );

    expect(cumulativeExp(1000)).toBeGreaterThan(2_147_483_647);
    expect(repoSource).toContain("totalExp')::bigint");
    expect(repoSource).not.toContain("totalExp')::int");
    expect(schemaSource).toContain(
      'totalExp: bigint("total_exp", { mode: "number" })',
    );
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
