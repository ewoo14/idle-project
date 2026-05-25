import { NotFoundError, ValidationError } from "../../core/errors.js";

export type CharacterRecord = {
  id: string;
  userId: string;
  classId: number;
  level: number;
  rebirthCount: number;
  stats: unknown;
  skillTree: unknown;
  inventory: unknown;
  lastSaveAt: Date | null;
};

export type SavePayload = {
  level: number;
  rebirthCount: number;
  maxEquipmentGrade: number;
  [key: string]: unknown;
};

export type SaveRepo = {
  findCharacterByUser(
    userId: string,
    characterId: string,
  ): Promise<CharacterRecord | null>;
  insertSave(input: {
    characterId: string;
    version: number;
    payload: SavePayload;
    serverValidated: boolean;
  }): Promise<{ id: string }>;
  listHistory(characterId: string, limit: number): Promise<unknown[]>;
};

export class SaveService {
  constructor(private readonly repo: SaveRepo) {}

  async upload(
    userId: string,
    input: { characterId: string; version: number; payload: SavePayload },
  ) {
    const character = await this.repo.findCharacterByUser(
      userId,
      input.characterId,
    );
    if (!character) {
      throw new NotFoundError("캐릭터를 찾을 수 없습니다.");
    }
    validateSavePayload(input.payload, character);
    return this.repo.insertSave({
      characterId: input.characterId,
      version: input.version,
      payload: input.payload,
      serverValidated: true,
    });
  }

  async history(userId: string, characterId: string, limit = 10) {
    const character = await this.repo.findCharacterByUser(userId, characterId);
    if (!character) {
      throw new NotFoundError("캐릭터를 찾을 수 없습니다.");
    }
    const normalizedLimit = Math.min(Math.max(limit, 1), 50);
    return this.repo.listHistory(characterId, normalizedLimit);
  }

  async current(userId: string, characterId: string) {
    return this.history(userId, characterId, 1);
  }
}

export function validateSavePayload(
  payload: SavePayload,
  character: CharacterRecord,
) {
  if (
    !Number.isInteger(payload.level) ||
    payload.level < 1 ||
    payload.level > 200
  ) {
    throw new ValidationError("level은 1~200 범위여야 합니다.");
  }
  if (!Number.isInteger(payload.rebirthCount) || payload.rebirthCount < 0) {
    throw new ValidationError("rebirthCount는 0 이상이어야 합니다.");
  }
  if (
    !Number.isInteger(payload.maxEquipmentGrade) ||
    payload.maxEquipmentGrade < 0 ||
    payload.maxEquipmentGrade > 10
  ) {
    throw new ValidationError("maxEquipmentGrade는 0~10 범위여야 합니다.");
  }
  if (
    payload.level < character.level ||
    payload.rebirthCount < character.rebirthCount
  ) {
    throw new ValidationError(
      "서버 진행도보다 낮은 세이브는 허용하지 않습니다.",
    );
  }
}
