export type QuestType = "main" | "daily" | "weekly";

export type QuestObjective =
  | "kill_monster"
  | "clear_map"
  | "claim_offline"
  | "enhance"
  | "defeat_boss"
  | "rebirth"
  | "transcend"
  | "climb_tower"
  | "reach_level"
  | "spend_gold"
  | "roll_gear_shop"
  | "feed_pet";

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
    title: "Find the Broken Gate",
    objective: "kill_monster",
    targetCount: 5,
    rewardGold: 150,
    rewardExp: 80,
    chapterMapId: "1-1",
  },
  {
    questId: "main_ch1_002",
    type: "main",
    title: "Secure the Village Field",
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
    title: "Repeating Echoes",
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
    title: "Trace of the Guardian",
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
    title: "Gate of the Winged Legion",
    objective: "kill_monster",
    targetCount: 20,
    rewardGold: 1_200,
    rewardExp: 900,
    prerequisiteQuestId: "main_ch1_004",
    chapterMapId: "1-5",
  },
  {
    questId: "main_ch1_006",
    type: "main",
    title: "Temper the Gate Key",
    objective: "enhance",
    targetCount: 2,
    rewardGold: 1_600,
    rewardExp: 1_200,
    prerequisiteQuestId: "main_ch1_005",
    chapterMapId: "1-5",
  },
  {
    questId: "main_ch1_007",
    type: "main",
    title: "Break the First Seal",
    objective: "defeat_boss",
    targetCount: 1,
    rewardGold: 2_200,
    rewardExp: 1_600,
    prerequisiteQuestId: "main_ch1_006",
    chapterMapId: "1-5",
  },
  {
    questId: "main_ch2_001",
    type: "main",
    title: "Enter the Ashen Road",
    objective: "kill_monster",
    targetCount: 25,
    rewardGold: 2_600,
    rewardExp: 1_900,
    prerequisiteQuestId: "main_ch1_007",
    chapterMapId: "2-1",
  },
  {
    questId: "main_ch2_002",
    type: "main",
    title: "Map the Ember Ruins",
    objective: "clear_map",
    targetCount: 1,
    rewardGold: 3_200,
    rewardExp: 2_300,
    prerequisiteQuestId: "main_ch2_001",
    chapterMapId: "2-2",
  },
  {
    questId: "main_ch2_003",
    type: "main",
    title: "Reach the Watch Spire",
    objective: "reach_level",
    targetCount: 10,
    rewardGold: 3_900,
    rewardExp: 2_800,
    prerequisiteQuestId: "main_ch2_002",
    chapterMapId: "2-3",
  },
  {
    questId: "main_ch2_004",
    type: "main",
    title: "Reforge Through Rebirth",
    objective: "rebirth",
    targetCount: 1,
    rewardGold: 4_800,
    rewardExp: 3_400,
    prerequisiteQuestId: "main_ch2_003",
    chapterMapId: "2-4",
  },
  {
    questId: "main_ch2_005",
    type: "main",
    title: "Defeat the Ember Warden",
    objective: "defeat_boss",
    targetCount: 1,
    rewardGold: 6_200,
    rewardExp: 4_500,
    prerequisiteQuestId: "main_ch2_004",
    chapterMapId: "2-5",
  },
  {
    questId: "main_ch3_001",
    type: "main",
    title: "Enter the Rift Frontier",
    objective: "kill_monster",
    targetCount: 35,
    rewardGold: 7_600,
    rewardExp: 5_400,
    prerequisiteQuestId: "main_ch2_005",
    chapterMapId: "3-1",
  },
  {
    questId: "main_ch3_002",
    type: "main",
    title: "Chart the Shadow Fault",
    objective: "clear_map",
    targetCount: 1,
    rewardGold: 8_800,
    rewardExp: 6_200,
    prerequisiteQuestId: "main_ch3_001",
    chapterMapId: "3-2",
  },
  {
    questId: "main_ch3_003",
    type: "main",
    title: "Strength for the Abyss Gate",
    objective: "reach_level",
    targetCount: 25,
    rewardGold: 10_200,
    rewardExp: 7_300,
    prerequisiteQuestId: "main_ch3_002",
    chapterMapId: "3-4",
  },
  {
    questId: "main_ch3_004",
    type: "main",
    title: "Light the Rift Beacon",
    objective: "climb_tower",
    targetCount: 15,
    rewardGold: 11_800,
    rewardExp: 8_400,
    prerequisiteQuestId: "main_ch3_003",
    chapterMapId: "3-5",
  },
  {
    questId: "main_ch3_005",
    type: "main",
    title: "Silence the Umbral Legion",
    objective: "kill_monster",
    targetCount: 50,
    rewardGold: 13_600,
    rewardExp: 9_800,
    prerequisiteQuestId: "main_ch3_004",
    chapterMapId: "3-8",
  },
  {
    questId: "main_ch3_006",
    type: "main",
    title: "Defeat the Dimension Lord",
    objective: "defeat_boss",
    targetCount: 1,
    rewardGold: 16_000,
    rewardExp: 12_000,
    prerequisiteQuestId: "main_ch3_005",
    chapterMapId: "3-10",
  },
  {
    questId: "daily_kill_monsters",
    type: "daily",
    title: "Daily Hunt",
    objective: "kill_monster",
    targetCount: 30,
    rewardGold: 500,
    rewardExp: 240,
  },
  {
    questId: "daily_claim_offline",
    type: "daily",
    title: "Claim Rest Rewards",
    objective: "claim_offline",
    targetCount: 1,
    rewardGold: 300,
    rewardExp: 180,
  },
  {
    questId: "daily_enhance_gear",
    type: "daily",
    title: "Temper Equipment",
    objective: "enhance",
    targetCount: 3,
    rewardGold: 650,
    rewardExp: 320,
  },
  {
    questId: "daily_reach_level",
    type: "daily",
    title: "Push Your Training",
    objective: "reach_level",
    targetCount: 10,
    rewardGold: 700,
    rewardExp: 360,
  },
  {
    questId: "daily_spend_gold",
    type: "daily",
    title: "Keep the Market Moving",
    objective: "spend_gold",
    targetCount: 1_000,
    rewardGold: 750,
    rewardExp: 380,
  },
  {
    questId: "daily_roll_gear_shop",
    type: "daily",
    title: "Try the Gear Shop",
    objective: "roll_gear_shop",
    targetCount: 1,
    rewardGold: 850,
    rewardExp: 420,
  },
  {
    questId: "daily_feed_pet",
    type: "daily",
    title: "Feed a Companion",
    objective: "feed_pet",
    targetCount: 1,
    rewardGold: 900,
    rewardExp: 450,
  },
  {
    questId: "weekly_defeat_bosses",
    type: "weekly",
    title: "Weekly Boss Breaker",
    objective: "defeat_boss",
    targetCount: 3,
    rewardGold: 5_000,
    rewardExp: 2_500,
  },
  {
    questId: "weekly_rebirth",
    type: "weekly",
    title: "Weekly Rebirth",
    objective: "rebirth",
    targetCount: 1,
    rewardGold: 8_000,
    rewardExp: 4_000,
  },
  {
    questId: "weekly_climb_tower",
    type: "weekly",
    title: "Weekly Tower Push",
    objective: "climb_tower",
    targetCount: 10,
    rewardGold: 7_000,
    rewardExp: 3_600,
  },
  {
    questId: "weekly_spend_gold",
    type: "weekly",
    title: "Weekly Gold Sink",
    objective: "spend_gold",
    targetCount: 10_000,
    rewardGold: 6_500,
    rewardExp: 3_200,
  },
];

export const questById = new Map(
  questDefinitions.map((quest) => [quest.questId, quest]),
);

export const dailyQuestIds = questDefinitions
  .filter((quest) => quest.type === "daily")
  .map((quest) => quest.questId);

export const weeklyQuestIds = questDefinitions
  .filter((quest) => quest.type === "weekly")
  .map((quest) => quest.questId);
