import { describe, expect, it, vi } from "vitest";
import { seasonTiers } from "../../core/data/season.js";
import { NotFoundError, ValidationError } from "../../core/errors.js";
import type { SeasonRepo, SeasonStateRecord } from "./season.service.js";
import { SeasonService } from "./season.service.js";

const userId = "00000000-0000-0000-0000-000000000001";
const characterId = "00000000-0000-0000-0000-000000000010";

describe("SeasonService", () => {
  it("lists token progress, tier unlocks, and claimed rewards", async () => {
    const service = new SeasonService(
      createRepo({
        state: stateRecord({ tokens: 30, claimedTiers: [1] }),
      }),
    );

    const result = await service.progress(userId);

    expect(result.seasonId).toBe(1);
    expect(result.tokens).toBe(30);
    expect(result.tiers.slice(0, 3)).toEqual([
      expect.objectContaining({ tier: 1, unlocked: true, claimed: true }),
      expect.objectContaining({ tier: 2, unlocked: true, claimed: false }),
      expect.objectContaining({ tier: 3, unlocked: false, claimed: false }),
    ]);
  });

  it("adds quest-reported season tokens after character ownership check", async () => {
    const repo = createRepo({ state: stateRecord({ tokens: 8 }) });
    repo.addTokens.mockResolvedValue(stateRecord({ tokens: 13 }));
    const service = new SeasonService(repo);

    const result = await service.addTokens(userId, characterId, 5);

    expect(repo.findCharacter).toHaveBeenCalledWith(userId, characterId);
    expect(repo.addTokens).toHaveBeenCalledWith(userId, 5);
    expect(result.tokens).toBe(13);
  });

  it("claims an unlocked tier once and applies the reward to character totals", async () => {
    const repo = createRepo({
      state: stateRecord({ tokens: seasonTiers[1].requiredTokens }),
    });
    repo.claimTier.mockResolvedValue({
      state: stateRecord({
        tokens: seasonTiers[1].requiredTokens,
        claimedTiers: [2],
      }),
      totals: { gold: 1_500, totalExp: 300 },
    });
    const service = new SeasonService(repo);

    const result = await service.claim(userId, characterId, 2);

    expect(repo.claimTier).toHaveBeenCalledWith({
      userId,
      characterId,
      tier: 2,
      rewardGold: seasonTiers[1].rewardAmount,
      rewardExp: 0,
      now: expect.any(Date),
    });
    expect(result.claimedTier).toBe(2);
    expect(result.totals).toEqual({ gold: 1_500, totalExp: 300 });
  });

  it("rejects invalid token amounts, locked tiers, duplicate claims, and missing characters", async () => {
    const service = new SeasonService(createRepo());

    await expect(
      service.addTokens(userId, characterId, 0),
    ).rejects.toBeInstanceOf(ValidationError);

    await expect(service.claim(userId, characterId, 10)).rejects.toBeInstanceOf(
      ValidationError,
    );

    const duplicate = new SeasonService(
      createRepo({ state: stateRecord({ tokens: 10, claimedTiers: [1] }) }),
    );
    await expect(
      duplicate.claim(userId, characterId, 1),
    ).rejects.toBeInstanceOf(ValidationError);

    const missingCharacter = new SeasonService(
      createRepo({ characterExists: false }),
    );
    await expect(
      missingCharacter.addTokens(userId, characterId, 1),
    ).rejects.toBeInstanceOf(NotFoundError);
  });
});

function stateRecord(overrides: Partial<SeasonStateRecord> = {}) {
  return {
    userId,
    seasonId: 1,
    tokens: 0,
    claimedTiers: [],
    updatedAt: new Date("2026-05-26T00:00:00.000Z"),
    ...overrides,
  } satisfies SeasonStateRecord;
}

function createRepo(
  options: { state?: SeasonStateRecord; characterExists?: boolean } = {},
) {
  const state = options.state ?? stateRecord();
  const repo = {
    findCharacter: vi
      .fn()
      .mockResolvedValue(
        options.characterExists === false ? null : { id: characterId, userId },
      ),
    getOrCreateState: vi.fn().mockResolvedValue(state),
    addTokens: vi.fn(async (_userId: string, amount: number) => ({
      ...state,
      tokens: state.tokens + amount,
    })),
    claimTier: vi.fn(),
  } satisfies SeasonRepo;
  return repo;
}
