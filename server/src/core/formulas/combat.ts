/**
 * 데미지 계산 - UE5 client CombatFormulas C++ 미러.
 * 최소 보장: ATK x 0.05 (방어가 압도해도 1 이상)
 */
export function computeDamage(atk: number, def: number): number {
  return Math.max(atk * 0.05, atk - def * 0.6);
}
