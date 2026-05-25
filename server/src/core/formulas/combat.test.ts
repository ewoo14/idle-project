import { describe, expect, it } from "vitest";
import { computeDamage } from "./combat.js";

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
});
