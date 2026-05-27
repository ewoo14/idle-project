import { describe, expect, it, vi } from "vitest";
import { ValidationError } from "../../core/errors.js";
import { createCharacterSchema } from "./character.schema.js";
import { CharacterService } from "./character.service.js";

describe("CharacterService", () => {
  it("creates the MVP warrior at level 1 with rebirth defaults", async () => {
    const repo = {
      createCharacter: vi.fn().mockResolvedValue({
        id: "c1",
        classId: 1,
        level: 1,
        rebirthCount: 0,
        rebirthBonusPoints: 0,
      }),
      findByIdForUser: vi.fn(),
      rebirthCharacter: vi.fn(),
    };
    const service = new CharacterService(repo);

    const result = await service.create("u1", { classId: 1 });

    expect(result.level).toBe(1);
    expect(repo.createCharacter).toHaveBeenCalledWith(
      expect.objectContaining({
        classId: 1,
        level: 1,
        rebirthCount: 0,
        rebirthBonusPoints: 0,
      }),
    );
  });

  it("creates each supported expanded class with matching level 1 stats", async () => {
    const repo = {
      createCharacter: vi.fn().mockImplementation((input) =>
        Promise.resolve({
          id: "c1",
          ...input,
        }),
      ),
      findByIdForUser: vi.fn(),
      rebirthCharacter: vi.fn(),
    };
    const service = new CharacterService(repo);

    for (const classId of [1, 2, 3, 4, 5, 6, 7, 8]) {
      await service.create("u1", { classId });
    }

    expect(repo.createCharacter).toHaveBeenCalledTimes(8);
    expect(repo.createCharacter).toHaveBeenLastCalledWith(
      expect.objectContaining({
        classId: 8,
        stats: expect.objectContaining({
          primary: expect.objectContaining({ int: 13, wis: 11 }),
          derived: expect.objectContaining({ magicAtk: 32 }),
        }),
      }),
    );
  });

  it("rejects unsupported classes outside the eight class roster", async () => {
    const service = new CharacterService({
      createCharacter: vi.fn(),
      findByIdForUser: vi.fn(),
      rebirthCharacter: vi.fn(),
    });

    await expect(service.create("u1", { classId: 9 })).rejects.toBeInstanceOf(
      ValidationError,
    );
  });

  it("rejects rebirth when the chapter 1 boss is not defeated", async () => {
    const service = new CharacterService(
      createRepo({
        level: 100,
        stats: { progress: { bChapter1BossDefeated: false } },
      }),
    );

    await expect(
      service.rebirth(userId(), { characterId: characterId() }),
    ).rejects.toBeInstanceOf(ValidationError);
  });

  it("rejects rebirth before level 100 even after boss defeat", async () => {
    const service = new CharacterService(
      createRepo({
        level: 99,
        stats: { progress: { bChapter1BossDefeated: true } },
      }),
    );

    await expect(
      service.rebirth(userId(), { characterId: characterId() }),
    ).rejects.toBeInstanceOf(ValidationError);
  });

  it("applies rebirth reset, permanent bonus points, and derived stat bonus", async () => {
    const repo = createRepo({
      level: 100,
      rebirthCount: 2,
      rebirthBonusPoints: 10,
      gold: 1234,
      totalExp: 999_999,
      stats: { progress: { bChapter1BossDefeated: true } },
    });
    const service = new CharacterService(repo);

    await service.rebirth(userId(), { characterId: characterId() });

    expect(repo.rebirthCharacter).toHaveBeenCalledWith(
      expect.objectContaining({
        characterId: characterId(),
        expectedRebirthCount: 2,
        expectedRebirthBonusPoints: 10,
        level: 1,
        rebirthCount: 3,
        rebirthBonusPoints: 15,
        gold: 123,
        totalExp: 0,
      }),
    );
    const update = vi.mocked(repo.rebirthCharacter).mock.calls[0][0];
    expect(update.stats.progress).toEqual({
      bChapter1BossDefeated: false,
    });
    expect(update.stats.derived.hp).toBe(270);
    expect(update.stats.derived.physAtk).toBe(54);
  });
});

describe("createCharacterSchema", () => {
  it("allows the full eight class roster", () => {
    expect(createCharacterSchema.body.properties.classId).toMatchObject({
      minimum: 1,
      maximum: 8,
    });
  });
});

function createRepo(overrides: Record<string, unknown>) {
  return {
    createCharacter: vi.fn(),
    findByIdForUser: vi.fn().mockResolvedValue({
      id: characterId(),
      userId: userId(),
      classId: 1,
      level: 1,
      rebirthCount: 0,
      rebirthBonusPoints: 0,
      stats: {},
      skillTree: {},
      inventory: [],
      gold: 0,
      totalExp: 0,
      lastSeenAt: null,
      lastSaveAt: null,
      ...overrides,
    }),
    rebirthCharacter: vi.fn().mockImplementation((input) =>
      Promise.resolve({
        id: input.characterId,
        userId: userId(),
        classId: 1,
        skillTree: {},
        inventory: [],
        lastSeenAt: null,
        lastSaveAt: null,
        ...input,
      }),
    ),
  };
}

function userId() {
  return "00000000-0000-0000-0000-000000000001";
}

function characterId() {
  return "00000000-0000-0000-0000-000000000010";
}
