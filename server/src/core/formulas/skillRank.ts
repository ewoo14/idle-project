export const MAX_SKILL_RANK = 50;

export function clampSkillRank(rank: number): number {
  return Math.max(0, Math.min(Math.trunc(rank), MAX_SKILL_RANK));
}

export function getEffectiveDamageCoeff(
  baseCoeff: number,
  rank: number,
): number {
  const rankBonus = f32(f32(clampSkillRank(rank)) * f32(0.1));
  const multiplier = f32(f32(1) + rankBonus);
  return f32(f32(baseCoeff) * multiplier);
}

export function getEffectiveCooldown(
  baseCooldown: number,
  rank: number,
): number {
  if (baseCooldown <= 0) {
    return 0;
  }

  const rankReduction = f32(f32(clampSkillRank(rank)) * f32(0.05));
  const multiplier = f32(f32(1) - rankReduction);
  const reducedCooldown = f32(f32(baseCooldown) * multiplier);
  return f32(Math.max(f32(0.1), reducedCooldown));
}

function f32(value: number): number {
  return Math.fround(value);
}
