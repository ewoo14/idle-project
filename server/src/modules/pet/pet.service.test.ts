import { describe, expect, it, vi } from "vitest";
import { petDefinitions } from "../../core/data/pets.js";
import { NotFoundError, ValidationError } from "../../core/errors.js";
import type { PetRepo, PetStateRecord } from "./pet.service.js";
import { PetService } from "./pet.service.js";

const userId = "00000000-0000-0000-0000-000000000001";

describe("PetService", () => {
  it("lists V1 pets with owned and equipped state plus active bonus", async () => {
    const repo = createRepo({
      state: {
        userId,
        ownedPetIds: ["dog", "cat"],
        equippedPetId: "dog",
        updatedAt: new Date("2026-05-26T00:00:00.000Z"),
      },
    });
    const service = new PetService(repo);

    const result = await service.list(userId);

    expect(result.pets).toHaveLength(2);
    expect(result.pets).toEqual(
      petDefinitions.map((pet) => ({
        ...pet,
        owned: true,
        equipped: pet.petId === "dog",
      })),
    );
    expect(result.activeBonus).toEqual({
      bonusType: "gold",
      bonusPercent: 20,
    });
  });

  it("equips one owned pet and returns the updated active bonus", async () => {
    const repo = createRepo();
    const service = new PetService(repo);

    const result = await service.equip(userId, "cat");

    expect(repo.equipPet).toHaveBeenCalledWith(userId, "cat");
    expect(result.equippedPetId).toBe("cat");
    expect(result.activeBonus).toEqual({
      bonusType: "drop",
      bonusPercent: 15,
    });
  });

  it("rejects unknown or unowned pets", async () => {
    const service = new PetService(createRepo());

    await expect(service.equip(userId, "missing")).rejects.toBeInstanceOf(
      NotFoundError,
    );

    const unowned = new PetService(
      createRepo({
        state: {
          userId,
          ownedPetIds: ["dog"],
          equippedPetId: "dog",
          updatedAt: new Date("2026-05-26T00:00:00.000Z"),
        },
      }),
    );
    await expect(unowned.equip(userId, "cat")).rejects.toBeInstanceOf(
      ValidationError,
    );
  });
});

function createRepo(options: { state?: PetStateRecord } = {}) {
  const state =
    options.state ??
    ({
      userId,
      ownedPetIds: ["dog", "cat"],
      equippedPetId: "dog",
      updatedAt: new Date("2026-05-26T00:00:00.000Z"),
    } satisfies PetStateRecord);
  const repo = {
    getOrCreateState: vi.fn().mockResolvedValue(state),
    equipPet: vi.fn(async (_userId: string, petId: string) => ({
      ...state,
      equippedPetId: petId,
    })),
  } satisfies PetRepo;
  return repo;
}
