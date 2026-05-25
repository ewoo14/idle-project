export type SeasonRewardType = "gold" | "exp";

export type SeasonTier = {
  tier: number;
  requiredTokens: number;
  rewardType: SeasonRewardType;
  rewardAmount: number;
};

export const currentSeasonId = 1;

export const seasonTiers: SeasonTier[] = [
  { tier: 1, requiredTokens: 10, rewardType: "gold", rewardAmount: 500 },
  { tier: 2, requiredTokens: 25, rewardType: "gold", rewardAmount: 1_000 },
  { tier: 3, requiredTokens: 45, rewardType: "exp", rewardAmount: 300 },
  { tier: 4, requiredTokens: 70, rewardType: "gold", rewardAmount: 1_800 },
  { tier: 5, requiredTokens: 100, rewardType: "exp", rewardAmount: 650 },
  { tier: 6, requiredTokens: 135, rewardType: "gold", rewardAmount: 3_000 },
  { tier: 7, requiredTokens: 175, rewardType: "exp", rewardAmount: 1_100 },
  { tier: 8, requiredTokens: 220, rewardType: "gold", rewardAmount: 4_800 },
  { tier: 9, requiredTokens: 270, rewardType: "exp", rewardAmount: 1_750 },
  { tier: 10, requiredTokens: 325, rewardType: "gold", rewardAmount: 7_500 },
];

export const seasonTierByNumber = new Map(
  seasonTiers.map((tier) => [tier.tier, tier]),
);
