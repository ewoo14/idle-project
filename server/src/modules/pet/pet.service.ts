import {
  defaultOwnedPetIds,
  petById,
  petDefinitions,
} from "../../core/data/pets.js";
import { NotFoundError, ValidationError } from "../../core/errors.js";

export type PetStateRecord = {
  userId: string;
  ownedPetIds: string[];
  equippedPetId: string | null;
  updatedAt: Date;
};

export type PetRepo = {
  getOrCreateState(userId: string): Promise<PetStateRecord>;
  equipPet(userId: string, petId: string): Promise<PetStateRecord>;
};

export class PetService {
  constructor(private readonly repo: PetRepo) {}

  async list(userId: string) {
    const state = await this.repo.getOrCreateState(userId);
    return this.toResponse(state);
  }

  async equip(userId: string, petId: string) {
    const pet = petById.get(petId);
    if (!pet) {
      throw new NotFoundError("Pet not found.", { code: "PET_NOT_FOUND" });
    }

    const state = await this.repo.getOrCreateState(userId);
    if (!state.ownedPetIds.includes(petId)) {
      throw new ValidationError("Pet is not owned.", {
        code: "PET_NOT_OWNED",
      });
    }

    return this.toResponse(await this.repo.equipPet(userId, pet.petId));
  }

  private toResponse(state: PetStateRecord) {
    const ownedPetIds = state.ownedPetIds.length
      ? state.ownedPetIds
      : defaultOwnedPetIds;
    const activePet = state.equippedPetId
      ? petById.get(state.equippedPetId)
      : undefined;
    return {
      equippedPetId: state.equippedPetId,
      activeBonus: activePet
        ? {
            bonusType: activePet.bonusType,
            bonusPercent: activePet.bonusPercent,
          }
        : null,
      pets: petDefinitions.map((pet) => ({
        ...pet,
        owned: ownedPetIds.includes(pet.petId),
        equipped: state.equippedPetId === pet.petId,
      })),
    };
  }
}
