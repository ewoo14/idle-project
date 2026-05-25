import { NotFoundError, ValidationError } from "../../core/errors.js";
import { cumulativeExp } from "../../core/formulas/index.js";
import type { LeaderboardService } from "../leaderboard/leaderboard.service.js";

export type CharacterRecord = {
  id: string;
  userId: string;
  classId: number;
  level: number;
  rebirthCount: number;
  stats: unknown;
  skillTree: unknown;
  inventory: unknown;
  gold: number;
  totalExp: number;
  lastSeenAt: Date | null;
  lastSaveAt: Date | null;
};

export type SavePayload = {
  level: number;
  rebirthCount: number;
  maxEquipmentGrade: number;
  totalExp?: number;
  gold?: number;
  lastSeenUnixSec?: number;
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

export type SaveLogger = {
  warn(input: unknown, message?: string): void;
};

export class SaveService {
  constructor(
    private readonly repo: SaveRepo,
    private readonly leaderboard: Pick<
      LeaderboardService,
      "updatePower" | "updateRebirth"
    >,
    private readonly logger: SaveLogger = { warn: () => undefined },
  ) {}

  async upload(
    userId: string,
    input: { characterId: string; version: number; payload: SavePayload },
  ) {
    const character = await this.repo.findCharacterByUser(
      userId,
      input.characterId,
    );
    if (!character) {
      throw new NotFoundError("캐릭터를 찾을 수 없습니다.", {
        code: "SAVE_CHARACTER_NOT_FOUND",
      });
    }
    validateSavePayload(input.payload, character, {
      userId,
      characterId: input.characterId,
      logger: this.logger,
    });
    const save = await this.repo.insertSave({
      characterId: input.characterId,
      version: input.version,
      payload: input.payload,
      serverValidated: true,
    });
    await this.leaderboard.updatePower({
      characterId: input.characterId,
      seasonId: 1,
      score: computePowerScore(input.payload, character),
    });
    await this.leaderboard.updateRebirth({
      characterId: input.characterId,
      seasonId: 1,
      rebirthCount: input.payload.rebirthCount,
    });
    return save;
  }

  async history(userId: string, characterId: string, limit = 10) {
    const character = await this.repo.findCharacterByUser(userId, characterId);
    if (!character) {
      throw new NotFoundError("캐릭터를 찾을 수 없습니다.", {
        code: "SAVE_CHARACTER_NOT_FOUND",
      });
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
  context?: {
    userId: string;
    characterId: string;
    logger: SaveLogger;
  },
) {
  if (
    !Number.isInteger(payload.level) ||
    payload.level < 1 ||
    payload.level > 200
  ) {
    rejectSave(context, payload, "level must be between 1 and 200");
  }
  if (!Number.isInteger(payload.rebirthCount) || payload.rebirthCount < 0) {
    rejectSave(context, payload, "rebirthCount must be >= 0");
  }
  if (
    !Number.isInteger(payload.maxEquipmentGrade) ||
    payload.maxEquipmentGrade < 0 ||
    payload.maxEquipmentGrade > 5
  ) {
    rejectSave(context, payload, "maxEquipmentGrade must be between 0 and 5");
  }
  if (payload.totalExp !== undefined) {
    const expected = cumulativeExp(payload.level);
    const tolerance = Math.max(expected * 0.01, 1);
    if (
      typeof payload.totalExp !== "number" ||
      !Number.isFinite(payload.totalExp) ||
      Math.abs(payload.totalExp - expected) > tolerance
    ) {
      rejectSave(context, payload, "totalExp does not match level");
    }
  }
  if (
    payload.gold !== undefined &&
    (!Number.isInteger(payload.gold) || payload.gold < character.gold)
  ) {
    rejectSave(context, payload, "gold regresses server currency");
  }
  if (
    payload.lastSeenUnixSec !== undefined &&
    (!Number.isInteger(payload.lastSeenUnixSec) || payload.lastSeenUnixSec < 0)
  ) {
    rejectSave(context, payload, "lastSeenUnixSec must be a unix timestamp");
  }
  if (
    payload.level < character.level ||
    payload.rebirthCount < character.rebirthCount
  ) {
    rejectSave(context, payload, "payload regresses server progress");
  }
}

export function computePowerScore(
  payload: SavePayload,
  _character: CharacterRecord,
) {
  return BigInt(payload.level * 100 + payload.rebirthCount * 5000);
}

function rejectSave(
  context:
    | {
        userId: string;
        characterId: string;
        logger: SaveLogger;
      }
    | undefined,
  payload: SavePayload,
  reason: string,
): never {
  context?.logger.warn(
    {
      userId: context.userId,
      characterId: context.characterId,
      reason,
      payloadSnapshot: {
        level: payload.level,
        rebirthCount: payload.rebirthCount,
        maxEquipmentGrade: payload.maxEquipmentGrade,
        totalExp: payload.totalExp,
        gold: payload.gold,
        lastSeenUnixSec: payload.lastSeenUnixSec,
      },
    },
    "save payload rejected",
  );
  throw new ValidationError("세이브 payload 검증에 실패했습니다.", {
    code: "SAVE_VALIDATION_FAILED",
  });
}
