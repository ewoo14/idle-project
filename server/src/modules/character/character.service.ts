import { NotFoundError, ValidationError } from "../../core/errors.js";
import { defaultPrimaryStats, deriveStats } from "../../core/formulas/index.js";

export type CharacterCreateInput = {
  userId: string;
  classId: number;
  level: number;
  rebirthCount: number;
  stats: Record<string, unknown>;
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
      throw new ValidationError("MVP 단계는 전사 (classId=1) 만 지원합니다.", {
        code: "CHARACTER_CLASS_UNAVAILABLE",
      });
    }
    const primary = defaultPrimaryStats(1, 1);
    return this.repo.createCharacter({
      userId,
      classId: 1,
      level: 1,
      rebirthCount: 0,
      stats: {
        primary,
        derived: deriveStats(primary, 1),
      },
      skillTree: {},
      inventory: [],
    });
  }

  async get(userId: string, characterId: string) {
    const character = await this.repo.findByIdForUser(userId, characterId);
    if (!character) {
      throw new NotFoundError("캐릭터를 찾을 수 없습니다.", {
        code: "CHARACTER_NOT_FOUND",
      });
    }
    return character;
  }
}
