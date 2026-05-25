import { NotFoundError, ValidationError } from "../../core/errors.js";
import { computeOfflineRewards } from "../../core/formulas/index.js";

export type OfflineCharacterRecord = {
  id: string;
  userId: string;
  level: number;
  rebirthCount: number;
  gold: number;
  totalExp: number;
  lastSeenAt: Date | null;
};

export type OfflineRepo = {
  findCharacter(
    userId: string,
    characterId: string,
  ): Promise<OfflineCharacterRecord | null>;
  claim(input: {
    characterId: string;
    gold: number;
    exp: number;
    now: Date;
  }): Promise<{ gold: number; totalExp: number }>;
};

export class OfflineService {
  constructor(
    private readonly repo: OfflineRepo,
    private readonly now: () => Date = () => new Date(),
  ) {}

  async preview(userId: string, characterId: string) {
    const context = await this.buildRewardContext(userId, characterId);
    return {
      characterId,
      lastSeenUnixSec: context.lastSeenUnixSec,
      nowUnixSec: context.nowUnixSec,
      rewards: context.rewards,
    };
  }

  async claim(userId: string, characterId: string) {
    const context = await this.buildRewardContext(userId, characterId);
    if (context.rewards.cappedSeconds <= 0) {
      throw new ValidationError("수령 가능한 오프라인 보상이 없습니다.", {
        code: "OFFLINE_REWARD_EMPTY",
      });
    }

    const totals = await this.repo.claim({
      characterId,
      gold: context.rewards.gold,
      exp: context.rewards.exp,
      now: context.now,
    });

    return {
      characterId,
      lastSeenUnixSec: context.lastSeenUnixSec,
      nowUnixSec: context.nowUnixSec,
      rewards: context.rewards,
      totals,
    };
  }

  private async buildRewardContext(userId: string, characterId: string) {
    const character = await this.repo.findCharacter(userId, characterId);
    if (!character) {
      throw new NotFoundError("캐릭터를 찾을 수 없습니다.", {
        code: "OFFLINE_CHARACTER_NOT_FOUND",
      });
    }

    const now = this.now();
    const nowUnixSec = toUnixSeconds(now);
    const lastSeenUnixSec = character.lastSeenAt
      ? toUnixSeconds(character.lastSeenAt)
      : nowUnixSec;

    return {
      now,
      nowUnixSec,
      lastSeenUnixSec,
      rewards: computeOfflineRewards({
        level: character.level,
        lastSeenUnixSec,
        nowUnixSec,
        rebirthCount: character.rebirthCount,
      }),
    };
  }
}

function toUnixSeconds(date: Date) {
  return Math.floor(date.getTime() / 1_000);
}
