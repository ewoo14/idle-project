export type QuestType = "main" | "daily";

export type QuestObjective =
  | "kill_monster"
  | "clear_map"
  | "claim_offline"
  | "enhance";

export type Quest = {
  questId: string;
  type: QuestType;
  title: string;
  objective: QuestObjective;
  targetCount: number;
  rewardGold: number;
  rewardExp: number;
  prerequisiteQuestId?: string;
  chapterMapId?: string;
};

export const questDefinitions: Quest[] = [
  {
    questId: "main_ch1_001",
    type: "main",
    title: "꺼진 등불을 찾아서",
    objective: "kill_monster",
    targetCount: 5,
    rewardGold: 150,
    rewardExp: 80,
    chapterMapId: "1-1",
  },
  {
    questId: "main_ch1_002",
    type: "main",
    title: "마을 초원 정리",
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
    title: "반복되는 숲길",
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
    title: "수호석의 흔적",
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
    title: "안개 군주의 관문",
    objective: "kill_monster",
    targetCount: 20,
    rewardGold: 1_200,
    rewardExp: 900,
    prerequisiteQuestId: "main_ch1_004",
    chapterMapId: "1-5",
  },
  {
    questId: "daily_kill_monsters",
    type: "daily",
    title: "일일 토벌",
    objective: "kill_monster",
    targetCount: 30,
    rewardGold: 500,
    rewardExp: 240,
  },
  {
    questId: "daily_claim_offline",
    type: "daily",
    title: "휴식 보상 수령",
    objective: "claim_offline",
    targetCount: 1,
    rewardGold: 300,
    rewardExp: 180,
  },
  {
    questId: "daily_enhance_gear",
    type: "daily",
    title: "장비 손보기",
    objective: "enhance",
    targetCount: 3,
    rewardGold: 650,
    rewardExp: 320,
  },
];

export const questById = new Map(
  questDefinitions.map((quest) => [quest.questId, quest]),
);

export const dailyQuestIds = questDefinitions
  .filter((quest) => quest.type === "daily")
  .map((quest) => quest.questId);
