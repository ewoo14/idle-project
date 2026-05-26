export const MAX_SKILL_RANK = 50;

export function clampSkillRank(rank: number): number {
  return Math.max(0, Math.min(Math.trunc(rank), MAX_SKILL_RANK));
}

export function getEffectiveDamageCoeff(baseCoeff: number, rank: number): number {
  return Math.fround(baseCoeff * (1 + clampSkillRank(rank) * 0.1));
}

export function getEffectiveCooldown(baseCooldown: number, rank: number): number {
  if (baseCooldown <= 0) {
    return 0;
  }

  return Math.fround(
    Math.max(0.1, baseCooldown * (1 - clampSkillRank(rank) * 0.05)),
  );
}
