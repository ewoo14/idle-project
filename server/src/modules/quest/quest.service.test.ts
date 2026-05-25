import { describe, expect, it, vi } from "vitest";
import { questDefinitions } from "../../core/data/quests.js";
import { NotFoundError, ValidationError } from "../../core/errors.js";
import type { QuestProgressRecord, QuestRepo } from "./quest.service.js";
import { QuestService } from "./quest.service.js";

const userId = "00000000-0000-0000-0000-000000000001";
const characterId = "00000000-0000-0000-0000-000000000010";

describe("QuestService", () => {
  it("lists unlocked main quests and resets stale daily progress on UTC date change", async () => {
    const repo = createRepo({
      progress: [
        progressRecord({
          questId: "daily_kill_monsters",
          progress: 9,
          completed: true,
          claimed: true,
          dailyResetDate: "2026-05-25",
        }),
      ],
    });
    const service = new QuestService(
      repo,
      () => new Date("2026-05-26T00:10:00.000Z"),
    );

    const result = await service.list(userId, characterId);

    expect(repo.resetDailyProgress).toHaveBeenCalledWith(userId, "2026-05-26", [
      "daily_kill_monsters",
      "daily_claim_offline",
      "daily_enhance_gear",
    ]);
    expect(result.quests.map((quest) => quest.questId)).toContain(
      "main_ch1_001",
    );
    expect(result.quests.map((quest) => quest.questId)).not.toContain(
      "main_ch1_002",
    );
    expect(
      result.quests.find((quest) => quest.questId === "daily_kill_monsters"),
    ).toMatchObject({
      progress: 0,
      completed: false,
      claimed: false,
      dailyResetDate: "2026-05-26",
    });
  });

  it("increments progress up to the target and marks the quest complete", async () => {
    const repo = createRepo({
      progress: [progressRecord({ questId: "main_ch1_001", progress: 4 })],
    });
    repo.upsertProgress.mockResolvedValue(
      progressRecord({ questId: "main_ch1_001", progress: 5, completed: true }),
    );
    const service = new QuestService(repo);

    const result = await service.addProgress(userId, characterId, {
      questId: "main_ch1_001",
      amount: 3,
    });

    expect(repo.upsertProgress).toHaveBeenCalledWith({
      userId,
      questId: "main_ch1_001",
      progress: 5,
      completed: true,
      dailyResetDate: null,
    });
    expect(result).toMatchObject({
      questId: "main_ch1_001",
      progress: 5,
      completed: true,
      claimed: false,
    });
  });

  it("advances unlocked active quests by objective without progressing claimed rewards", async () => {
    const repo = createRepo({
      progress: [
        progressRecord({
          questId: "daily_kill_monsters",
          progress: 29,
          dailyResetDate: "2026-05-26",
        }),
        progressRecord({
          questId: "daily_claim_offline",
          progress: 1,
          completed: true,
          claimed: true,
          dailyResetDate: "2026-05-26",
        }),
        progressRecord({
          questId: "daily_enhance_gear",
          progress: 2,
          dailyResetDate: "2026-05-26",
        }),
      ],
    });
    const service = new QuestService(
      repo,
      () => new Date("2026-05-26T02:00:00.000Z"),
    );

    const killResults = await service.addProgressForObjective(
      userId,
      characterId,
      { objective: "kill_monster", amount: 1 },
    );
    const claimedOffline = await service.addProgressForObjective(
      userId,
      characterId,
      { objective: "claim_offline", amount: 1 },
    );
    const enhanceResults = await service.addProgressForObjective(
      userId,
      characterId,
      { objective: "enhance", amount: 1 },
    );

    expect(killResults.map((quest) => quest.questId)).toEqual([
      "main_ch1_001",
      "daily_kill_monsters",
    ]);
    expect(killResults).toEqual(
      expect.arrayContaining([
        expect.objectContaining({
          questId: "main_ch1_001",
          progress: 1,
          completed: false,
        }),
        expect.objectContaining({
          questId: "daily_kill_monsters",
          progress: 30,
          completed: true,
        }),
      ]),
    );
    expect(claimedOffline).toEqual([
      expect.objectContaining({
        questId: "daily_claim_offline",
        progress: 1,
        completed: true,
        claimed: true,
      }),
    ]);
    expect(enhanceResults).toEqual([
      expect.objectContaining({
        questId: "daily_enhance_gear",
        progress: 3,
        completed: true,
      }),
    ]);
    expect(repo.upsertProgress).not.toHaveBeenCalledWith(
      expect.objectContaining({ questId: "daily_claim_offline" }),
    );
  });

  it("keeps listed quest data aligned with the canonical quest definitions", async () => {
    const repo = createRepo({
      progress: questDefinitions
        .filter((quest) => quest.prerequisiteQuestId)
        .map((quest) =>
          progressRecord({
            questId: quest.prerequisiteQuestId,
            progress: quest.targetCount,
            completed: true,
            claimed: true,
          }),
        ),
    });
    const service = new QuestService(repo);

    const result = await service.list(userId, characterId);

    expect(
      result.quests.map((quest) => ({
        questId: quest.questId,
        targetCount: quest.targetCount,
        rewardGold: quest.rewardGold,
        rewardExp: quest.rewardExp,
        objective: quest.objective,
        type: quest.type,
        prerequisiteQuestId: quest.prerequisiteQuestId,
        chapterMapId: quest.chapterMapId,
      })),
    ).toEqual(
      questDefinitions.map((quest) => ({
        questId: quest.questId,
        targetCount: quest.targetCount,
        rewardGold: quest.rewardGold,
        rewardExp: quest.rewardExp,
        objective: quest.objective,
        type: quest.type,
        prerequisiteQuestId: quest.prerequisiteQuestId,
        chapterMapId: quest.chapterMapId,
      })),
    );
  });

  it("rejects progress for locked main quests", async () => {
    const repo = createRepo();
    const service = new QuestService(repo);

    await expect(
      service.addProgress(userId, characterId, {
        questId: "main_ch1_002",
        amount: 1,
      }),
    ).rejects.toBeInstanceOf(ValidationError);
    expect(repo.upsertProgress).not.toHaveBeenCalled();
  });

  it("claims rewards once and unlocks the next main quest", async () => {
    const repo = createRepo({
      progress: [
        progressRecord({
          questId: "main_ch1_001",
          progress: 5,
          completed: true,
        }),
      ],
    });
    repo.claimQuest.mockResolvedValue({
      progress: progressRecord({
        questId: "main_ch1_001",
        progress: 5,
        completed: true,
        claimed: true,
      }),
      totals: { gold: 250, totalExp: 120 },
    });
    const service = new QuestService(repo);

    const result = await service.claim(userId, characterId, "main_ch1_001");

    expect(repo.claimQuest).toHaveBeenCalledWith({
      userId,
      characterId,
      questId: "main_ch1_001",
      rewardGold: 150,
      rewardExp: 80,
      now: expect.any(Date),
    });
    expect(result.totals).toEqual({ gold: 250, totalExp: 120 });
    expect(result.unlockedQuestIds).toEqual(["main_ch1_002"]);
  });

  it("rejects claims for incomplete, already claimed, or unknown quests", async () => {
    const incomplete = new QuestService(
      createRepo({
        progress: [progressRecord({ questId: "main_ch1_001", progress: 1 })],
      }),
    );
    await expect(
      incomplete.claim(userId, characterId, "main_ch1_001"),
    ).rejects.toBeInstanceOf(ValidationError);

    const alreadyClaimed = new QuestService(
      createRepo({
        progress: [
          progressRecord({
            questId: "main_ch1_001",
            progress: 5,
            completed: true,
            claimed: true,
          }),
        ],
      }),
    );
    await expect(
      alreadyClaimed.claim(userId, characterId, "main_ch1_001"),
    ).rejects.toBeInstanceOf(ValidationError);

    const unknown = new QuestService(createRepo());
    await expect(
      unknown.claim(userId, characterId, "missing_quest"),
    ).rejects.toBeInstanceOf(NotFoundError);
  });
});

function progressRecord(
  overrides: Partial<QuestProgressRecord>,
): QuestProgressRecord {
  return {
    userId,
    questId: "main_ch1_001",
    progress: 0,
    completed: false,
    claimed: false,
    dailyResetDate: null,
    updatedAt: new Date("2026-05-26T00:00:00.000Z"),
    ...overrides,
  };
}

function createRepo(options: { progress?: QuestProgressRecord[] } = {}) {
  const progress = options.progress ?? [];
  const repo = {
    findCharacter: vi.fn().mockResolvedValue({ id: characterId, userId }),
    listProgress: vi.fn().mockResolvedValue(progress),
    resetDailyProgress: vi.fn().mockResolvedValue(undefined),
    upsertProgress: vi.fn(async (input) =>
      progressRecord({
        questId: input.questId,
        progress: input.progress,
        completed: input.completed,
        dailyResetDate: input.dailyResetDate,
      }),
    ),
    claimQuest: vi.fn(),
  } satisfies QuestRepo;
  return repo;
}
