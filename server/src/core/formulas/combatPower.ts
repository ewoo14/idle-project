import type { DerivedStats } from "./stats.js";

export function computeCombatPower(stats: DerivedStats): number {
  const raw =
    stats.physAtk +
    stats.magicAtk +
    stats.physDef * 2 +
    stats.magicDef * 2 +
    stats.hp * 0.1 +
    stats.critRate * 500 +
    stats.critDmg * 100 +
    stats.atkSpeed * 200;

  return Math.max(0, Math.round(raw));
}
