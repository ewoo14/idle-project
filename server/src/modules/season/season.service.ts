import {
  currentSeasonId,
  seasonTierByNumber,
  seasonTiers,
} from "../../core/data/season.js";
import { NotFoundError, ValidationError } from "../../core/errors.js";

export type SeasonStateRecord = {
  userId: string;
  seasonId: number;
  tokens: number;
  claimedTiers: number[];
  updatedAt: Date;
};

export type SeasonRepo = {
  findCharacter(
    userId: string,
    characterId: string,
  ): Promise<{ id: string; userId: string } | null>;
  getOrCreateState(userId: string): Promise<SeasonStateRecord>;
  addTokens(userId: string, amount: number): Promise<SeasonStateRecord>;
  claimTier(input: {
    userId: string;
    characterId: string;
    tier: number;
    rewardGold: number;
    rewardExp: number;
    now: Date;
  }): Promise<{
    state: SeasonStateRecord;
    totals: { gold: number; totalExp: number };
  } | null>;
};

export class SeasonService {
  constructor(
    private readonly repo: SeasonRepo,
    private readonly now: () => Date = () => new Date(),
  ) {}

  async progress(userId: string) {
    return this.toResponse(await this.repo.getOrCreateState(userId));
  }

  async addTokens(userId: string, characterId: string, amount: number) {
    this.assertPositiveInteger(amount, "SEASON_TOKEN_AMOUNT_INVALID");
    await this.assertCharacter(userId, characterId);

    return this.toResponse(await this.repo.addTokens(userId, amount));
  }

  async claim(userId: string, characterId: string, tierNumber: number) {
    const tier = seasonTierByNumber.get(tierNumber);
    if (!tier) {
      throw new NotFoundError("Season tier not found.", {
        code: "SEASON_TIER_NOT_FOUND",
      });
    }
    await this.assertCharacter(userId, characterId);

    const state = await this.repo.getOrCreateState(userId);
    if (state.tokens < tier.requiredTokens) {
      throw new ValidationError("Season tier is locked.", {
        code: "SEASON_TIER_LOCKED",
      });
    }
    if (state.claimedTiers.includes(tier.tier)) {
      throw new ValidationError("Season tier reward already claimed.", {
        code: "SEASON_TIER_ALREADY_CLAIMED",
      });
    }

    const claimed = await this.repo.claimTier({
      userId,
      characterId,
      tier: tier.tier,
      rewardGold: tier.rewardType === "gold" ? tier.rewardAmount : 0,
      rewardExp: tier.rewardType === "exp" ? tier.rewardAmount : 0,
      now: this.now(),
    });
    if (!claimed) {
      throw new ValidationError("Season tier reward already claimed.", {
        code: "SEASON_TIER_ALREADY_CLAIMED",
      });
    }

    return {
      ...this.toResponse(claimed.state),
      claimedTier: tier.tier,
      reward: {
        rewardType: tier.rewardType,
        rewardAmount: tier.rewardAmount,
      },
      totals: claimed.totals,
    };
  }

  private toResponse(state: SeasonStateRecord) {
    return {
      seasonId: state.seasonId,
      tokens: state.tokens,
      tiers: seasonTiers.map((tier) => ({
        ...tier,
        unlocked: state.tokens >= tier.requiredTokens,
        claimed: state.claimedTiers.includes(tier.tier),
      })),
    };
  }

  private async assertCharacter(userId: string, characterId: string) {
    const character = await this.repo.findCharacter(userId, characterId);
    if (!character) {
      throw new NotFoundError("Character not found.", {
        code: "SEASON_CHARACTER_NOT_FOUND",
      });
    }
  }

  private assertPositiveInteger(amount: number, code: string) {
    if (!Number.isInteger(amount) || amount <= 0) {
      throw new ValidationError("Amount must be a positive integer.", {
        code,
      });
    }
  }
}

export { currentSeasonId };
