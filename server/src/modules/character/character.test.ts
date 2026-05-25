import { describe, expect, it, vi } from "vitest";
import { CharacterService } from "./character.service.js";

describe("CharacterService", () => {
  it("전사 캐릭터를 level 1, rebirth 0 기본 스탯으로 생성한다", async () => {
    const repo = {
      createCharacter: vi
        .fn()
        .mockResolvedValue({ id: "c1", classId: 1, level: 1, rebirthCount: 0 }),
      findByIdForUser: vi.fn(),
    };
    const service = new CharacterService(repo);

    const result = await service.create("u1", { classId: 1 });

    expect(result.level).toBe(1);
    expect(repo.createCharacter).toHaveBeenCalledWith(
      expect.objectContaining({
        classId: 1,
        level: 1,
        rebirthCount: 0,
      }),
    );
  });

  it("PR #2 범위에서는 전사 외 직업 생성을 거절한다", async () => {
    const service = new CharacterService({
      createCharacter: vi.fn(),
      findByIdForUser: vi.fn(),
    });

    await expect(service.create("u1", { classId: 2 })).rejects.toThrow("전사");
  });
});
