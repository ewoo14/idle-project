import { describe, expect, it } from "vitest";
import {
  getTotalTreasureWeight,
  pickTreasureReward,
  TREASURE_POOL,
  type TreasureReward,
} from "./index.js";

describe("treasure box formula", () => {
  it("sums the pool weights for client parity (= 100)", () => {
    const total = TREASURE_POOL.reduce((acc, entry) => acc + entry.weight, 0);
    expect(getTotalTreasureWeight()).toBe(total);
    expect(getTotalTreasureWeight()).toBe(100);
  });

  it("exposes the spec §3 pool table (reward/weight/min/max)", () => {
    expect(TREASURE_POOL).toEqual([
      { reward: "gold", weight: 40, minAmount: 10000, maxAmount: 50000 },
      { reward: "essence", weight: 25, minAmount: 3, maxAmount: 10 },
      { reward: "consumable", weight: 15, minAmount: 1, maxAmount: 2 },
      { reward: "protectionScroll", weight: 10, minAmount: 1, maxAmount: 3 },
      { reward: "resetCube", weight: 7, minAmount: 1, maxAmount: 2 },
      { reward: "rankCube", weight: 3, minAmount: 1, maxAmount: 1 },
    ]);
  });

  it("maps roll 0 to the first reward (gold)", () => {
    expect(pickTreasureReward(0)).toBe("gold");
  });

  it("maps each cumulative boundary to the exact reward", () => {
    // 누적 구간(totalWeight=100): gold 0~39 / essence 40~64 / consumable 65~79 /
    // protectionScroll 80~89 / resetCube 90~96 / rankCube 97~99.
    // gold 구간 경계
    expect(pickTreasureReward(0)).toBe("gold");
    expect(pickTreasureReward(39)).toBe("gold");
    // essence 구간 경계
    expect(pickTreasureReward(40)).toBe("essence");
    expect(pickTreasureReward(64)).toBe("essence");
    // consumable 구간 경계
    expect(pickTreasureReward(65)).toBe("consumable");
    expect(pickTreasureReward(79)).toBe("consumable");
    // protectionScroll 구간 경계
    expect(pickTreasureReward(80)).toBe("protectionScroll");
    expect(pickTreasureReward(89)).toBe("protectionScroll");
    // resetCube 구간 경계
    expect(pickTreasureReward(90)).toBe("resetCube");
    expect(pickTreasureReward(96)).toBe("resetCube");
    // rankCube 구간 경계
    expect(pickTreasureReward(97)).toBe("rankCube");
    expect(pickTreasureReward(99)).toBe("rankCube");
  });

  it("covers the full roll range 0..99 with valid pool rewards", () => {
    // 각 roll 이 정확히 해당 누적 구간 reward 로 떨어지는지 풀 정의로 재계산 검증.
    const total = getTotalTreasureWeight();
    for (let roll = 0; roll < total; roll += 1) {
      let cumulative = 0;
      let expected: TreasureReward = TREASURE_POOL[0].reward;
      for (const entry of TREASURE_POOL) {
        cumulative += entry.weight;
        if (roll < cumulative) {
          expected = entry.reward;
          break;
        }
      }
      expect(pickTreasureReward(roll)).toBe(expected);
    }
  });

  it("clamps negative and out-of-range rolls to the first/last reward", () => {
    // 음수 → 첫 보상(gold), totalWeight-1 초과 → 마지막 보상(rankCube).
    expect(pickTreasureReward(-1)).toBe("gold");
    expect(pickTreasureReward(-1000)).toBe("gold");
    expect(pickTreasureReward(100)).toBe("rankCube");
    expect(pickTreasureReward(99999)).toBe("rankCube");
  });

  it("truncates fractional rolls before mapping", () => {
    expect(pickTreasureReward(39.9)).toBe("gold");
    expect(pickTreasureReward(40.1)).toBe("essence");
  });

  it("keeps the pool internally consistent (weight>0, min<=max, unique rewards)", () => {
    const validRewards: readonly TreasureReward[] = [
      "gold",
      "essence",
      "consumable",
      "protectionScroll",
      "resetCube",
      "rankCube",
    ];
    const seen = new Set<TreasureReward>();
    for (const entry of TREASURE_POOL) {
      expect(entry.weight).toBeGreaterThan(0);
      expect(entry.minAmount).toBeLessThanOrEqual(entry.maxAmount);
      expect(entry.minAmount).toBeGreaterThan(0);
      expect(validRewards).toContain(entry.reward);
      // reward 중복 없음(엔트리 id 유일).
      expect(seen.has(entry.reward)).toBe(false);
      seen.add(entry.reward);
    }
  });
});
