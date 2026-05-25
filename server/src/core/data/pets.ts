export type PetBonusType = "gold" | "drop";

export type PetDefinition = {
  petId: string;
  name: string;
  bonusType: PetBonusType;
  bonusPercent: number;
};

export const petDefinitions: PetDefinition[] = [
  {
    petId: "dog",
    name: "강아지",
    bonusType: "gold",
    bonusPercent: 20,
  },
  {
    petId: "bird",
    name: "새",
    bonusType: "drop",
    bonusPercent: 15,
  },
];

export const defaultOwnedPetIds = petDefinitions.map((pet) => pet.petId);

export const petById = new Map(petDefinitions.map((pet) => [pet.petId, pet]));
