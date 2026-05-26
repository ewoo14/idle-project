import type { DerivedStats } from "./stats.js";

/**
 * 데미지 계산 - UE5 client CombatFormulas C++ 미러.
 * 최소 보장: ATK x 0.05 (방어가 압도해도 1 이상)
 */
export function computeDamage(atk: number, def: number): number {
  return Math.max(atk * 0.05, atk - def * 0.6);
}

export function computeMagicDamage(magicAtk: number, magicDef: number): number {
  return computeDamage(magicAtk, magicDef);
}

export function computeClassDamage(
  attackerStats: Pick<DerivedStats, "physAtk" | "magicAtk">,
  classId: number,
  physDef: number,
  magicDef = physDef,
): number {
  return classId === 2 || classId === 5
    ? computeMagicDamage(attackerStats.magicAtk, magicDef)
    : computeDamage(attackerStats.physAtk, physDef);
}

export function rollCrit(
  critRate: number,
  rng: () => number = Math.random,
): boolean {
  return rng() < clamp(critRate, 0, 1);
}

export function applyCrit(
  baseDamage: number,
  isCrit: boolean,
  critDmg: number,
): number {
  return isCrit ? baseDamage * Math.max(1, critDmg) : baseDamage;
}

function clamp(value: number, min: number, max: number): number {
  return Math.min(Math.max(value, min), max);
}
