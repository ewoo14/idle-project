import {
  ConflictError,
  NotFoundError,
  ValidationError,
} from "../../core/errors.js";
import {
  type ClassId,
  defaultPrimaryStats,
  deriveStats,
} from "../../core/formulas/index.js";

export type CharacterCreateInput = {
  userId: string;
  classId: number;
  level: number;
  rebirthCount: number;
  rebirthBonusPoints: number;
  stats: Record<string, unknown>;
  skillTree: Record<string, unknown>;
  inventory: unknown[];
};

export type CharacterRecord = CharacterCreateInput & {
  id: string;
  gold: number;
  totalExp: number;
  lastSeenAt: Date | null;
  lastSaveAt: Date | null;
};

export type CharacterRebirthInput = {
  characterId: string;
  expectedRebirthCount: number;
  expectedRebirthBonusPoints: number;
  level: number;
  rebirthCount: number;
  rebirthBonusPoints: number;
  gold: number;
  totalExp: number;
  stats: Record<string, unknown>;
};

export type CharacterRepo = {
  createCharacter(input: CharacterCreateInput): Promise<CharacterRecord>;
  findByIdForUser(
    userId: string,
    characterId: string,
  ): Promise<CharacterRecord | null>;
  rebirthCharacter(
    input: CharacterRebirthInput,
  ): Promise<CharacterRecord | null>;
};

export class CharacterService {
  constructor(private readonly repo: CharacterRepo) {}

  async create(userId: string, input: { classId: number }) {
    if (!isSupportedClassId(input.classId)) {
      throw new ValidationError("Unsupported character class.", {
        code: "CHARACTER_CLASS_UNAVAILABLE",
      });
    }
    const primary = defaultPrimaryStats(input.classId, 1);
    return this.repo.createCharacter({
      userId,
      classId: input.classId,
      level: 1,
      rebirthCount: 0,
      rebirthBonusPoints: 0,
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

  async rebirth(userId: string, input: { characterId: string }) {
    const character = await this.repo.findByIdForUser(
      userId,
      input.characterId,
    );
    if (!character) {
      throw new NotFoundError("Character not found.", {
        code: "CHARACTER_NOT_FOUND",
      });
    }
    if (character.level < 100) {
      throw new ValidationError("Rebirth requires level 100 or higher.", {
        code: "CHARACTER_REBIRTH_LEVEL_REQUIRED",
      });
    }
    if (!isChapter1BossDefeated(character.stats)) {
      throw new ValidationError("Rebirth requires Chapter 1 boss defeat.", {
        code: "CHARACTER_REBIRTH_BOSS_REQUIRED",
      });
    }

    const level = 1;
    const rebirthCount = character.rebirthCount + 1;
    const rebirthBonusPoints = character.rebirthBonusPoints + 5;
    const primary = defaultPrimaryStats(1, level);
    const stats = {
      ...asRecord(character.stats),
      progress: {
        ...asRecord(asRecord(character.stats).progress),
        bChapter1BossDefeated: false,
      },
      primary,
      derived: deriveStats(primary, level, {}, rebirthBonusPoints),
    };

    const rebirthed = await this.repo.rebirthCharacter({
      characterId: input.characterId,
      expectedRebirthCount: character.rebirthCount,
      expectedRebirthBonusPoints: character.rebirthBonusPoints,
      level,
      rebirthCount,
      rebirthBonusPoints,
      gold: Math.floor(character.gold * 0.1),
      totalExp: 0,
      stats,
    });
    if (!rebirthed) {
      throw new ConflictError("Character rebirth state changed.", {
        code: "CHARACTER_REBIRTH_CONFLICT",
      });
    }
    return rebirthed;
  }
}

function isSupportedClassId(classId: number): classId is ClassId {
  return Number.isInteger(classId) && classId >= 1 && classId <= 8;
}

function isChapter1BossDefeated(stats: Record<string, unknown>) {
  const root = asRecord(stats);
  const progress = asRecord(root.progress);
  return (
    root.bChapter1BossDefeated === true ||
    progress.bChapter1BossDefeated === true
  );
}

function asRecord(value: unknown): Record<string, unknown> {
  return value !== null && typeof value === "object" && !Array.isArray(value)
    ? (value as Record<string, unknown>)
    : {};
}
