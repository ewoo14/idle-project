import { describe, expect, it } from "vitest";
import {
  applyCrit,
  computeDamage,
  computeMagicDamage,
  rollCrit,
} from "./combat.js";

const clientServerDamageAnchors = [
  { atk: 100, def: 20, clientResult: 88 },
  { atk: 10, def: 100, clientResult: 0.5 },
  { atk: 50, def: 0, clientResult: 50 },
  { atk: 0, def: 0, clientResult: 0 },
  { atk: 1000, def: 500, clientResult: 700 },
];

describe("combat formulas", () => {
  it.each(
    clientServerDamageAnchors,
  )("ATK $atk DEF $def 피해량을 클라이언트 기준값과 동일하게 계산한다", ({
    atk,
    def,
    clientResult,
  }) => {
    const serverResult = computeDamage(atk, def);

    expect(serverResult).toBe(clientResult);
    expect(serverResult - clientResult).toBe(0);
  });

  it.each(
    clientServerDamageAnchors,
  )("ATK $atk DEF $def cross-validation diff 0 anchor", ({
    atk,
    def,
    clientResult,
  }) => {
    expect(computeDamage(atk, def) - clientResult).toBe(0);
  });

  it("applies critical multiplier only when the roll crits", () => {
    expect(applyCrit(34, false, 1.8)).toBe(34);
    expect(applyCrit(34, true, 1.8)).toBe(61.2);
  });

  it("rolls deterministic critical boundaries", () => {
    const rng = () => 0.5;

    expect(rollCrit(0, rng)).toBe(false);
    expect(rollCrit(1, rng)).toBe(true);
  });

  it("computes magic damage with the same curve using MagicAtk vs MagicDef", () => {
    expect(computeMagicDamage(80, 20)).toBe(68);
  });
});
