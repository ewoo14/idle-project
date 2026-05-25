import { NotFoundError, ValidationError } from "../../core/errors.js";

export type CharacterCreateInput = {
  userId: string;
  classId: number;
  level: number;
  rebirthCount: number;
  stats: Record<string, number>;
  skillTree: Record<string, unknown>;
  inventory: unknown[];
};

export type CharacterRecord = CharacterCreateInput & {
  id: string;
};

export type CharacterRepo = {
  createCharacter(input: CharacterCreateInput): Promise<CharacterRecord>;
  findByIdForUser(userId: string, characterId: string): Promise<unknown | null>;
};

export class CharacterService {
  constructor(private readonly repo: CharacterRepo) {}

  async create(userId: string, input: { classId: number }) {
    if (input.classId !== 1) {
      throw new ValidationError(
        "PR #2 범위에서는 전사(class_id=1)만 생성할 수 있습니다.",
      );
    }
    return this.repo.createCharacter({
      userId,
      classId: 1,
      level: 1,
      rebirthCount: 0,
      stats: { str: 12, dex: 6, int: 3, luk: 4, hp: 120, mp: 30 },
      skillTree: {},
      inventory: [],
    });
  }

  async get(userId: string, characterId: string) {
    const character = await this.repo.findByIdForUser(userId, characterId);
    if (!character) {
      throw new NotFoundError("캐릭터를 찾을 수 없습니다.");
    }
    return character;
  }
}
