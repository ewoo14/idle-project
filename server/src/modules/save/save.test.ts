import { describe, expect, it, vi } from "vitest";
import { ValidationError } from "../../core/errors.js";
import { SaveService } from "./save.service.js";

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
  it("정상 세이브를 저장하고 서버 검증 플래그를 남긴다", async () => {
    const repo = {
      findCharacterByUser: vi.fn().mockResolvedValue(character),
      insertSave: vi.fn().mockResolvedValue({ id: "save-1" }),
      listHistory: vi.fn(),
    };
    const service = new SaveService(repo);

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

  it("레벨 경계를 벗어난 조작 payload를 거절한다", async () => {
    const service = new SaveService({
      findCharacterByUser: vi.fn().mockResolvedValue(character),
      insertSave: vi.fn(),
      listHistory: vi.fn(),
    });

    await expect(
      service.upload(userId(), {
        characterId: character.id,
        version: 1,
        payload: { level: 201, rebirthCount: 2, maxEquipmentGrade: 1 },
      }),
    ).rejects.toBeInstanceOf(ValidationError);
  });

  it("history limit은 1~50 범위로 보정한다", async () => {
    const repo = {
      findCharacterByUser: vi.fn().mockResolvedValue(character),
      insertSave: vi.fn(),
      listHistory: vi.fn().mockResolvedValue([]),
    };
    const service = new SaveService(repo);

    await service.history(userId(), character.id, 100);

    expect(repo.listHistory).toHaveBeenCalledWith(character.id, 50);
  });
});

function userId() {
  return "00000000-0000-0000-0000-000000000001";
}
