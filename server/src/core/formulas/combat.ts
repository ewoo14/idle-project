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

export type SkillElement = "None" | "Fire" | "Ice" | "Lightning" | "Holy";

export function computeElementMultiplier(
  skillElement: SkillElement,
  targetWeakElement: SkillElement,
): number {
  if (skillElement === "None" || targetWeakElement === "None") {
    return 1;
  }

  if (skillElement === targetWeakElement) {
    return 1.5;
  }

  return isResistedElementPair(skillElement, targetWeakElement) ? 0.5 : 1;
}

function isResistedElementPair(
  skillElement: SkillElement,
  targetWeakElement: SkillElement,
): boolean {
  return (
    (skillElement === "Fire" && targetWeakElement === "Ice") ||
    (skillElement === "Ice" && targetWeakElement === "Fire") ||
    (skillElement === "Lightning" && targetWeakElement === "Holy") ||
    (skillElement === "Holy" && targetWeakElement === "Lightning")
  );
}

function clamp(value: number, min: number, max: number): number {
  return Math.min(Math.max(value, min), max);
}
