import { describe, expect, it, vi } from "vitest";
import {
  dailyQuestIds,
  questDefinitions,
  weeklyQuestIds,
} from "../../core/data/quests.js";
import { NotFoundError, ValidationError } from "../../core/errors.js";
import { questListSchema } from "./quest.schema.js";
import type { QuestProgressRecord, QuestRepo } from "./quest.service.js";
import { QuestService } from "./quest.service.js";

const userId = "00000000-0000-0000-0000-000000000001";
const characterId = "00000000-0000-0000-0000-000000000010";

describe("QuestService", () => {
  it("mirrors the expanded client quest definition table", () => {
    expect(
      questDefinitions.map((quest) => ({
        questId: quest.questId,
        type: quest.type,
        objective: quest.objective,
        targetCount: quest.targetCount,
        rewardGold: quest.rewardGold,
        rewardExp: quest.rewardExp,
        prerequisiteQuestId: quest.prerequisiteQuestId,
        chapterMapId: quest.chapterMapId,
      })),
    ).toEqual([
      {
        questId: "main_ch1_001",
        type: "main",
        objective: "kill_monster",
        targetCount: 5,
        rewardGold: 150,
        rewardExp: 80,
        chapterMapId: "1-1",
      },
      {
        questId: "main_ch1_002",
        type: "main",
        objective: "clear_map",
        targetCount: 1,
        rewardGold: 220,
        rewardExp: 140,
        prerequisiteQuestId: "main_ch1_001",
        chapterMapId: "1-1",
      },
      {
        questId: "main_ch1_003",
        type: "main",
        objective: "kill_monster",
        targetCount: 12,
        rewardGold: 420,
        rewardExp: 300,
        prerequisiteQuestId: "main_ch1_002",
        chapterMapId: "1-2",
      },
      {
        questId: "main_ch1_004",
        type: "main",
        objective: "clear_map",
        targetCount: 1,
        rewardGold: 700,
        rewardExp: 520,
        prerequisiteQuestId: "main_ch1_003",
        chapterMapId: "1-3",
      },
      {
        questId: "main_ch1_005",
        type: "main",
        objective: "kill_monster",
        targetCount: 20,
        rewardGold: 1200,
        rewardExp: 900,
        prerequisiteQuestId: "main_ch1_004",
        chapterMapId: "1-5",
      },
      {
        questId: "main_ch1_006",
        type: "main",
        objective: "enhance",
        targetCount: 2,
        rewardGold: 1600,
        rewardExp: 1200,
        prerequisiteQuestId: "main_ch1_005",
        chapterMapId: "1-5",
      },
      {
        questId: "main_ch1_007",
        type: "main",
        objective: "defeat_boss",
        targetCount: 1,
        rewardGold: 2200,
        rewardExp: 1600,
        prerequisiteQuestId: "main_ch1_006",
        chapterMapId: "1-5",
      },
      {
        questId: "main_ch2_001",
        type: "main",
        objective: "kill_monster",
        targetCount: 25,
        rewardGold: 2600,
        rewardExp: 1900,
        prerequisiteQuestId: "main_ch1_007",
        chapterMapId: "2-1",
      },
      {
        questId: "main_ch2_002",
        type: "main",
        objective: "clear_map",
        targetCount: 1,
        rewardGold: 3200,
        rewardExp: 2300,
        prerequisiteQuestId: "main_ch2_001",
        chapterMapId: "2-2",
      },
      {
        questId: "main_ch2_003",
        type: "main",
        objective: "reach_level",
        targetCount: 10,
        rewardGold: 3900,
        rewardExp: 2800,
        prerequisiteQuestId: "main_ch2_002",
        chapterMapId: "2-3",
      },
      {
        questId: "main_ch2_004",
        type: "main",
        objective: "rebirth",
        targetCount: 1,
        rewardGold: 4800,
        rewardExp: 3400,
        prerequisiteQuestId: "main_ch2_003",
        chapterMapId: "2-4",
      },
      {
        questId: "main_ch2_005",
        type: "main",
        objective: "defeat_boss",
        targetCount: 1,
        rewardGold: 6200,
        rewardExp: 4500,
        prerequisiteQuestId: "main_ch2_004",
        chapterMapId: "2-5",
      },
      {
        questId: "main_ch3_001",
        type: "main",
        objective: "kill_monster",
        targetCount: 35,
        rewardGold: 7600,
        rewardExp: 5400,
        prerequisiteQuestId: "main_ch2_005",
        chapterMapId: "3-1",
      },
      {
        questId: "main_ch3_002",
        type: "main",
        objective: "clear_map",
        targetCount: 1,
        rewardGold: 8800,
        rewardExp: 6200,
        prerequisiteQuestId: "main_ch3_001",
        chapterMapId: "3-2",
      },
      {
        questId: "main_ch3_003",
        type: "main",
        objective: "reach_level",
        targetCount: 25,
        rewardGold: 10200,
        rewardExp: 7300,
        prerequisiteQuestId: "main_ch3_002",
        chapterMapId: "3-4",
      },
      {
        questId: "main_ch3_004",
        type: "main",
        objective: "climb_tower",
        targetCount: 15,
        rewardGold: 11800,
        rewardExp: 8400,
        prerequisiteQuestId: "main_ch3_003",
        chapterMapId: "3-5",
      },
      {
        questId: "main_ch3_005",
        type: "main",
        objective: "kill_monster",
        targetCount: 50,
        rewardGold: 13600,
        rewardExp: 9800,
        prerequisiteQuestId: "main_ch3_004",
        chapterMapId: "3-8",
      },
      {
        questId: "main_ch3_006",
        type: "main",
        objective: "defeat_boss",
        targetCount: 1,
        rewardGold: 16000,
        rewardExp: 12000,
        prerequisiteQuestId: "main_ch3_005",
        chapterMapId: "3-10",
      },
      {
        questId: "main_ch4_001",
        type: "main",
        objective: "kill_monster",
        targetCount: 65,
        rewardGold: 18400,
        rewardExp: 13800,
        prerequisiteQuestId: "main_ch3_006",
        chapterMapId: "4-1",
      },
      {
        questId: "main_ch4_002",
        type: "main",
        objective: "clear_map",
        targetCount: 1,
        rewardGold: 21000,
        rewardExp: 15800,
        prerequisiteQuestId: "main_ch4_001",
        chapterMapId: "4-2",
      },
      {
        questId: "main_ch4_003",
        type: "main",
        objective: "reach_level",
        targetCount: 40,
        rewardGold: 24000,
        rewardExp: 18000,
        prerequisiteQuestId: "main_ch4_002",
        chapterMapId: "4-4",
      },
      {
        questId: "main_ch4_004",
        type: "main",
        objective: "climb_tower",
        targetCount: 25,
        rewardGold: 27500,
        rewardExp: 20500,
        prerequisiteQuestId: "main_ch4_003",
        chapterMapId: "4-5",
      },
      {
        questId: "main_ch4_005",
        type: "main",
        objective: "kill_monster",
        targetCount: 80,
        rewardGold: 31500,
        rewardExp: 23600,
        prerequisiteQuestId: "main_ch4_004",
        chapterMapId: "4-8",
      },
      {
        questId: "main_ch4_006",
        type: "main",
        objective: "defeat_boss",
        targetCount: 1,
        rewardGold: 38000,
        rewardExp: 28500,
        prerequisiteQuestId: "main_ch4_005",
        chapterMapId: "4-10",
      },
      {
        questId: "main_ch5_001",
        type: "main",
        objective: "kill_monster",
        targetCount: 95,
        rewardGold: 44000,
        rewardExp: 33000,
        prerequisiteQuestId: "main_ch4_006",
        chapterMapId: "5-1",
      },
      {
        questId: "main_ch5_002",
        type: "main",
        objective: "clear_map",
        targetCount: 1,
        rewardGold: 50000,
        rewardExp: 37500,
        prerequisiteQuestId: "main_ch5_001",
        chapterMapId: "5-2",
      },
      {
        questId: "main_ch5_003",
        type: "main",
        objective: "reach_level",
        targetCount: 60,
        rewardGold: 57000,
        rewardExp: 42500,
        prerequisiteQuestId: "main_ch5_002",
        chapterMapId: "5-4",
      },
      {
        questId: "main_ch5_004",
        type: "main",
        objective: "climb_tower",
        targetCount: 35,
        rewardGold: 64500,
        rewardExp: 48000,
        prerequisiteQuestId: "main_ch5_003",
        chapterMapId: "5-5",
      },
      {
        questId: "main_ch5_005",
        type: "main",
        objective: "kill_monster",
        targetCount: 110,
        rewardGold: 73000,
        rewardExp: 54500,
        prerequisiteQuestId: "main_ch5_004",
        chapterMapId: "5-8",
      },
      {
        questId: "main_ch5_006",
        type: "main",
        objective: "defeat_boss",
        targetCount: 1,
        rewardGold: 88000,
        rewardExp: 66000,
        prerequisiteQuestId: "main_ch5_005",
        chapterMapId: "5-10",
      },
      {
        questId: "daily_kill_monsters",
        type: "daily",
        objective: "kill_monster",
        targetCount: 30,
        rewardGold: 500,
        rewardExp: 240,
      },
      {
        questId: "daily_claim_offline",
        type: "daily",
        objective: "claim_offline",
        targetCount: 1,
        rewardGold: 300,
        rewardExp: 180,
      },
      {
        questId: "daily_enhance_gear",
        type: "daily",
        objective: "enhance",
        targetCount: 3,
        rewardGold: 650,
        rewardExp: 320,
      },
      {
        questId: "daily_reach_level",
        type: "daily",
        objective: "reach_level",
        targetCount: 10,
        rewardGold: 700,
        rewardExp: 360,
      },
      {
        questId: "daily_spend_gold",
        type: "daily",
        objective: "spend_gold",
        targetCount: 1000,
        rewardGold: 750,
        rewardExp: 380,
      },
      {
        questId: "daily_roll_gear_shop",
        type: "daily",
        objective: "roll_gear_shop",
        targetCount: 1,
        rewardGold: 850,
        rewardExp: 420,
      },
      {
        questId: "daily_feed_pet",
        type: "daily",
        objective: "feed_pet",
        targetCount: 1,
        rewardGold: 900,
        rewardExp: 450,
      },
      {
        questId: "weekly_defeat_bosses",
        type: "weekly",
        objective: "defeat_boss",
        targetCount: 3,
        rewardGold: 5000,
        rewardExp: 2500,
      },
      {
        questId: "weekly_rebirth",
        type: "weekly",
        objective: "rebirth",
        targetCount: 1,
        rewardGold: 8000,
        rewardExp: 4000,
      },
      {
        questId: "weekly_climb_tower",
        type: "weekly",
        objective: "climb_tower",
        targetCount: 10,
        rewardGold: 7000,
        rewardExp: 3600,
      },
      {
        questId: "weekly_spend_gold",
        type: "weekly",
        objective: "spend_gold",
        targetCount: 10000,
        rewardGold: 6500,
        rewardExp: 3200,
      },
    ]);
  });

  it("exposes schema enums for weekly quests and expanded objectives", () => {
    const questItemSchema =
      questListSchema.response[200].properties.data.properties.quests.items;

    expect(questItemSchema.properties.type.enum).toEqual([
      "main",
      "daily",
      "weekly",
    ]);
    expect(questItemSchema.properties.objective.enum).toEqual([
      "kill_monster",
      "clear_map",
      "claim_offline",
      "enhance",
      "defeat_boss",
      "rebirth",
      "transcend",
      "climb_tower",
      "reach_level",
      "spend_gold",
      "roll_gear_shop",
      "feed_pet",
    ]);
  });

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

    expect(repo.resetDailyProgress).toHaveBeenCalledWith(
      userId,
      "2026-05-26",
      dailyQuestIds,
    );
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

  it("lists and resets weekly progress on ISO week change", async () => {
    const repo = createRepo({
      progress: [
        progressRecord({
          questId: "weekly_defeat_bosses",
          progress: 3,
          completed: true,
          claimed: true,
          weeklyResetId: "2026-W21",
        }),
      ],
    });
    const service = new QuestService(
      repo,
      () => new Date("2026-05-27T02:00:00.000Z"),
    );

    const result = await service.list(userId, characterId);

    expect(repo.resetWeeklyProgress).toHaveBeenCalledWith(
      userId,
      "2026-W22",
      weeklyQuestIds,
    );
    expect(result.weeklyResetId).toBe("2026-W22");
    expect(
      result.quests.find((quest) => quest.questId === "weekly_defeat_bosses"),
    ).toMatchObject({
      progress: 0,
      completed: false,
      claimed: false,
      weeklyResetId: "2026-W22",
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
      weeklyResetId: null,
    });
    expect(result).toMatchObject({
      questId: "main_ch1_001",
      progress: 5,
      completed: true,
      claimed: false,
    });
  });

  it("records reach level progress as the highest reached level instead of accumulating levels", async () => {
    const repo = createRepo({
      progress: [
        progressRecord({
          questId: "daily_reach_level",
          progress: 4,
          dailyResetDate: "2026-05-26",
        }),
      ],
    });
    const service = new QuestService(
      repo,
      () => new Date("2026-05-26T02:00:00.000Z"),
    );

    const lowerResults = await service.addProgressForObjective(
      userId,
      characterId,
      { objective: "reach_level", amount: 3 },
    );
    const targetResults = await service.addProgressForObjective(
      userId,
      characterId,
      { objective: "reach_level", amount: 10 },
    );

    expect(lowerResults).toEqual([
      expect.objectContaining({
        questId: "daily_reach_level",
        progress: 4,
        completed: false,
      }),
    ]);
    expect(targetResults).toEqual([
      expect.objectContaining({
        questId: "daily_reach_level",
        progress: 10,
        completed: true,
      }),
    ]);
    expect(repo.upsertProgress).toHaveBeenNthCalledWith(1, {
      userId,
      questId: "daily_reach_level",
      progress: 4,
      completed: false,
      dailyResetDate: "2026-05-26",
      weeklyResetId: null,
    });
    expect(repo.upsertProgress).toHaveBeenNthCalledWith(2, {
      userId,
      questId: "daily_reach_level",
      progress: 10,
      completed: true,
      dailyResetDate: "2026-05-26",
      weeklyResetId: null,
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

  it("advances weekly quests through expanded objectives", async () => {
    const repo = createRepo({
      progress: [
        progressRecord({
          questId: "weekly_climb_tower",
          progress: 9,
          weeklyResetId: "2026-W22",
        }),
      ],
    });
    const service = new QuestService(
      repo,
      () => new Date("2026-05-27T02:00:00.000Z"),
    );

    const results = await service.addProgressForObjective(userId, characterId, {
      objective: "climb_tower",
      amount: 2,
    });

    expect(results).toEqual([
      expect.objectContaining({
        questId: "weekly_climb_tower",
        progress: 10,
        completed: true,
        weeklyResetId: "2026-W22",
      }),
    ]);
    expect(repo.upsertProgress).toHaveBeenCalledWith({
      userId,
      questId: "weekly_climb_tower",
      progress: 10,
      completed: true,
      dailyResetDate: null,
      weeklyResetId: "2026-W22",
    });
  });

  it("unlocks and progresses chapter three main quests through existing objective hooks", async () => {
    const repo = createRepo({
      progress: [
        progressRecord({
          questId: "main_ch2_005",
          progress: 1,
          completed: true,
          claimed: true,
        }),
      ],
    });
    const service = new QuestService(repo);

    const killResults = await service.addProgressForObjective(
      userId,
      characterId,
      { objective: "kill_monster", amount: 40 },
    );

    expect(killResults).toEqual([
      expect.objectContaining({
        questId: "main_ch1_001",
        progress: 5,
        completed: true,
      }),
      expect.objectContaining({
        questId: "main_ch3_001",
        progress: 35,
        completed: true,
        chapterMapId: "3-1",
      }),
      expect.objectContaining({
        questId: "daily_kill_monsters",
        progress: 30,
        completed: true,
      }),
    ]);
    expect(repo.upsertProgress).toHaveBeenCalledWith({
      userId,
      questId: "main_ch3_001",
      progress: 35,
      completed: true,
      dailyResetDate: null,
      weeklyResetId: null,
    });
  });

  it("unlocks and progresses chapter four main quests through level, tower, and boss hooks", async () => {
    const repo = createRepo({
      progress: [
        progressRecord({
          questId: "main_ch3_006",
          progress: 1,
          completed: true,
          claimed: true,
        }),
      ],
    });
    const service = new QuestService(repo);

    const killResults = await service.addProgressForObjective(
      userId,
      characterId,
      { objective: "kill_monster", amount: 65 },
    );

    expect(killResults).toContainEqual(
      expect.objectContaining({
        questId: "main_ch4_001",
        progress: 65,
        completed: true,
        chapterMapId: "4-1",
      }),
    );

    const levelRepo = createRepo({
      progress: [
        progressRecord({
          questId: "main_ch3_006",
          progress: 1,
          completed: true,
          claimed: true,
        }),
        progressRecord({
          questId: "main_ch4_001",
          progress: 65,
          completed: true,
          claimed: true,
        }),
        progressRecord({
          questId: "main_ch4_002",
          progress: 1,
          completed: true,
          claimed: true,
        }),
      ],
    });
    const levelService = new QuestService(levelRepo);

    const towerRepo = createRepo({
      progress: [
        progressRecord({
          questId: "main_ch4_003",
          progress: 40,
          completed: true,
          claimed: true,
        }),
      ],
    });
    const towerService = new QuestService(towerRepo);

    const bossRepo = createRepo({
      progress: [
        progressRecord({
          questId: "main_ch4_004",
          progress: 25,
          completed: true,
          claimed: true,
        }),
        progressRecord({
          questId: "main_ch4_005",
          progress: 80,
          completed: true,
          claimed: true,
        }),
      ],
    });
    const bossService = new QuestService(bossRepo);

    const levelResults = await levelService.addProgressForObjective(
      userId,
      characterId,
      { objective: "reach_level", amount: 40 },
    );
    const towerResults = await towerService.addProgressForObjective(
      userId,
      characterId,
      { objective: "climb_tower", amount: 25 },
    );
    const bossResults = await bossService.addProgressForObjective(
      userId,
      characterId,
      { objective: "defeat_boss", amount: 1 },
    );

    expect(levelResults).toContainEqual(
      expect.objectContaining({
        questId: "main_ch4_003",
        progress: 40,
        completed: true,
        chapterMapId: "4-4",
      }),
    );
    expect(towerResults).toContainEqual(
      expect.objectContaining({
        questId: "main_ch4_004",
        progress: 25,
        completed: true,
        chapterMapId: "4-5",
      }),
    );
    expect(bossResults).toContainEqual(
      expect.objectContaining({
        questId: "main_ch4_006",
        progress: 1,
        completed: true,
        chapterMapId: "4-10",
      }),
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
    weeklyResetId: null,
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
    resetWeeklyProgress: vi.fn().mockResolvedValue(undefined),
    upsertProgress: vi.fn(async (input) =>
      progressRecord({
        questId: input.questId,
        progress: input.progress,
        completed: input.completed,
        dailyResetDate: input.dailyResetDate,
        weeklyResetId: input.weeklyResetId,
      }),
    ),
    claimQuest: vi.fn(),
  } satisfies QuestRepo;
  return repo;
}
