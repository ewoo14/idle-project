export const WEEKLY_CHALLENGE_LIMIT = 7;

const BASE_MILESTONE_THRESHOLD = Math.fround(1000);
const BASE_MILESTONE_GOLD_REWARD = Math.fround(5000);
const MILESTONE_GROWTH = Math.fround(1.5);

export function getChallengeDamage(combatPower: number): number {
  return Math.max(0, Math.trunc(combatPower));
}

export function milestoneThreshold(n: number): number {
  const milestone = Math.trunc(n);
  if (milestone < 1) {
    return 0;
  }

  return Math.floor(
    Math.fround(
      BASE_MILESTONE_THRESHOLD *
        Math.fround(MILESTONE_GROWTH ** (milestone - 1)),
    ),
  );
}

export function getReachedMilestones(accumDamage: number): number {
  const damage = Math.max(0, Math.trunc(accumDamage));
  let reached = 0;
  let next = 1;

  while (damage >= milestoneThreshold(next)) {
    reached = next;
    next += 1;
    if (next > 10_000) {
      break;
    }
  }

  return reached;
}

export function milestoneGoldReward(n: number): number {
  const milestone = Math.trunc(n);
  if (milestone < 1) {
    return 0;
  }

  return Math.floor(
    Math.fround(
      BASE_MILESTONE_GOLD_REWARD *
        Math.fround(MILESTONE_GROWTH ** (milestone - 1)),
    ),
  );
}

export function milestoneEssenceReward(n: number): number {
  return Math.floor(3 * Math.max(0, Math.trunc(n)));
}
