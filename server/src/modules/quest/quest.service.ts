import {
  dailyQuestIds,
  type Quest,
  type QuestObjective,
  questById,
  questDefinitions,
  weeklyQuestIds,
} from "../../core/data/quests.js";
import { NotFoundError, ValidationError } from "../../core/errors.js";

export type QuestProgressRecord = {
  userId: string;
  questId: string;
  progress: number;
  completed: boolean;
  claimed: boolean;
  dailyResetDate: string | null;
  weeklyResetId: string | null;
  updatedAt: Date;
};

export type QuestRepo = {
  findCharacter(
    userId: string,
    characterId: string,
  ): Promise<{ id: string; userId: string } | null>;
  listProgress(userId: string): Promise<QuestProgressRecord[]>;
  resetDailyProgress(
    userId: string,
    dailyResetDate: string,
    questIds: string[],
  ): Promise<void>;
  resetWeeklyProgress(
    userId: string,
    weeklyResetId: string,
    questIds: string[],
  ): Promise<void>;
  upsertProgress(input: {
    userId: string;
    questId: string;
    progress: number;
    completed: boolean;
    dailyResetDate: string | null;
    weeklyResetId: string | null;
  }): Promise<QuestProgressRecord>;
  claimQuest(input: {
    userId: string;
    characterId: string;
    questId: string;
    rewardGold: number;
    rewardExp: number;
    now: Date;
  }): Promise<{
    progress: QuestProgressRecord;
    totals: { gold: number; totalExp: number };
  } | null>;
};

export class QuestService {
  constructor(
    private readonly repo: QuestRepo,
    private readonly now: () => Date = () => new Date(),
  ) {}

  async list(userId: string, characterId: string) {
    const state = await this.buildState(userId, characterId);
    return {
      quests: questDefinitions
        .filter(
          (quest) =>
            quest.type === "daily" ||
            quest.type === "weekly" ||
            state.isUnlocked(quest.questId),
        )
        .map((quest) => ({
          ...quest,
          ...state.progressFor(quest.questId),
        })),
      dailyResetDate: state.today,
      weeklyResetId: state.week,
    };
  }

  async addProgress(
    userId: string,
    characterId: string,
    input: { questId: string; amount: number },
  ) {
    this.assertProgressAmount(input.amount);

    const quest = questById.get(input.questId);
    if (!quest) {
      throw new NotFoundError("Quest not found.", { code: "QUEST_NOT_FOUND" });
    }

    const state = await this.buildState(userId, characterId);
    if (quest.type === "main" && !state.isUnlocked(quest.questId)) {
      throw new ValidationError("Quest is locked.", { code: "QUEST_LOCKED" });
    }

    return this.saveProgress(userId, quest, state, input.amount);
  }

  async addProgressForObjective(
    userId: string,
    characterId: string,
    input: { objective: QuestObjective; amount: number },
  ) {
    this.assertProgressAmount(input.amount);

    const state = await this.buildState(userId, characterId);
    const activeMatches = questDefinitions.filter(
      (quest) =>
        quest.objective === input.objective &&
        (quest.type === "daily" ||
          quest.type === "weekly" ||
          state.isUnlocked(quest.questId)),
    );

    const results = [];
    for (const quest of activeMatches) {
      results.push(await this.saveProgress(userId, quest, state, input.amount));
    }
    return results;
  }

  private assertProgressAmount(amount: number) {
    if (!Number.isInteger(amount) || amount <= 0) {
      throw new ValidationError(
        "Quest progress amount must be a positive integer.",
        {
          code: "QUEST_PROGRESS_AMOUNT_INVALID",
        },
      );
    }
  }

  private async saveProgress(
    userId: string,
    quest: Quest,
    state: Awaited<ReturnType<QuestService["buildState"]>>,
    amount: number,
  ) {
    const current = state.progressFor(quest.questId);
    if (current.claimed) {
      return { ...quest, ...current };
    }

    const progress = Math.min(quest.targetCount, current.progress + amount);
    const saved = await this.repo.upsertProgress({
      userId,
      questId: quest.questId,
      progress,
      completed: progress >= quest.targetCount,
      dailyResetDate: quest.type === "daily" ? state.today : null,
      weeklyResetId: quest.type === "weekly" ? state.week : null,
    });
    return { ...quest, ...saved };
  }

  async claim(userId: string, characterId: string, questId: string) {
    const quest = questById.get(questId);
    if (!quest) {
      throw new NotFoundError("Quest not found.", { code: "QUEST_NOT_FOUND" });
    }

    const state = await this.buildState(userId, characterId);
    if (quest.type === "main" && !state.isUnlocked(quest.questId)) {
      throw new ValidationError("Quest is locked.", { code: "QUEST_LOCKED" });
    }

    const progress = state.progressFor(quest.questId);
    if (!progress.completed) {
      throw new ValidationError("Quest is not complete.", {
        code: "QUEST_NOT_COMPLETED",
      });
    }
    if (progress.claimed) {
      throw new ValidationError("Quest reward already claimed.", {
        code: "QUEST_ALREADY_CLAIMED",
      });
    }

    const claimed = await this.repo.claimQuest({
      userId,
      characterId,
      questId,
      rewardGold: quest.rewardGold,
      rewardExp: quest.rewardExp,
      now: this.now(),
    });
    if (!claimed) {
      throw new ValidationError("Quest reward already claimed.", {
        code: "QUEST_ALREADY_CLAIMED",
      });
    }

    return {
      quest: { ...quest, ...claimed.progress },
      totals: claimed.totals,
      unlockedQuestIds: questDefinitions
        .filter((candidate) => candidate.prerequisiteQuestId === quest.questId)
        .map((candidate) => candidate.questId),
    };
  }

  private async buildState(userId: string, characterId: string) {
    const character = await this.repo.findCharacter(userId, characterId);
    if (!character) {
      throw new NotFoundError("Character not found.", {
        code: "QUEST_CHARACTER_NOT_FOUND",
      });
    }

    const now = this.now();
    const today = toUtcDate(now);
    const week = toUtcWeek(now);
    const rows = await this.repo.listProgress(userId);
    const hasStaleDaily = rows.some((row) => {
      const quest = questById.get(row.questId);
      return quest?.type === "daily" && row.dailyResetDate !== today;
    });
    if (hasStaleDaily) {
      await this.repo.resetDailyProgress(userId, today, dailyQuestIds);
    }
    const hasStaleWeekly = rows.some((row) => {
      const quest = questById.get(row.questId);
      return quest?.type === "weekly" && row.weeklyResetId !== week;
    });
    if (hasStaleWeekly) {
      await this.repo.resetWeeklyProgress(userId, week, weeklyQuestIds);
    }

    const progress = new Map(
      rows.map((row) => {
        const quest = questById.get(row.questId);
        if (quest?.type === "daily" && row.dailyResetDate !== today) {
          return [
            row.questId,
            {
              ...row,
              progress: 0,
              completed: false,
              claimed: false,
              dailyResetDate: today,
            },
          ];
        }
        if (quest?.type === "weekly" && row.weeklyResetId !== week) {
          return [
            row.questId,
            {
              ...row,
              progress: 0,
              completed: false,
              claimed: false,
              weeklyResetId: week,
            },
          ];
        }
        return [row.questId, row];
      }),
    );

    const progressFor = (questId: string) => {
      const quest = questById.get(questId);
      const row = progress.get(questId);
      return {
        userId,
        questId,
        progress: row?.progress ?? 0,
        completed: row?.completed ?? false,
        claimed: row?.claimed ?? false,
        dailyResetDate:
          quest?.type === "daily" ? (row?.dailyResetDate ?? today) : null,
        weeklyResetId:
          quest?.type === "weekly" ? (row?.weeklyResetId ?? week) : null,
      };
    };

    return {
      today,
      week,
      progressFor,
      isUnlocked: (questId: string) => {
        const quest = questById.get(questId);
        if (!quest || quest.type === "daily" || quest.type === "weekly") {
          return true;
        }
        return quest.prerequisiteQuestId
          ? progressFor(quest.prerequisiteQuestId).claimed
          : true;
      },
    };
  }
}

function toUtcDate(date: Date) {
  return date.toISOString().slice(0, 10);
}

function toUtcWeek(date: Date) {
  const utcDate = new Date(
    Date.UTC(date.getUTCFullYear(), date.getUTCMonth(), date.getUTCDate()),
  );
  const day = utcDate.getUTCDay() || 7;
  utcDate.setUTCDate(utcDate.getUTCDate() + 4 - day);
  const yearStart = new Date(Date.UTC(utcDate.getUTCFullYear(), 0, 1));
  const week = Math.ceil(
    ((utcDate.getTime() - yearStart.getTime()) / 86_400_000 + 1) / 7,
  );
  return `${utcDate.getUTCFullYear()}-W${String(week).padStart(2, "0")}`;
}
