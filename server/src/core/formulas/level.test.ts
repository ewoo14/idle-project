import { describe, expect, it } from "vitest";
import {
  cumulativeExp,
  enhanceResourceCost,
  enhanceSuccessRate,
  expToNext,
  LEVEL_CAP,
} from "./level.js";

describe("level formulas", () => {
  it("10레벨 다음 레벨 필요 경험치는 약 3,500이다", () => {
    expect(expToNext(10)).toBeCloseTo(3500, -2);
  });

  it("100레벨 다음 레벨 필요 경험치는 약 261,000이다", () => {
    expect(expToNext(100)).toBeCloseTo(261000, -3);
  });

  it("1 미만 레벨은 거절한다", () => {
    expect(() => expToNext(0)).toThrow("level must be >= 1");
  });

  it("레벨 캡 앵커 경험치를 반환한다", () => {
    expect(LEVEL_CAP).toBe(200);
    expect(expToNext(200)).toBe(832291);
  });

  it("누적 경험치는 레벨 1에서 해당 레벨까지 필요한 경험치 합산이다", () => {
    expect(cumulativeExp(1)).toBe(0);
    expect(cumulativeExp(4)).toBe(expToNext(1) + expToNext(2) + expToNext(3));
  });

  it.each([
    [0, 1],
    [5, 1],
    [6, 0.9],
    [10, 0.9],
    [11, 0.7],
    [12, 0.6],
    [13, 0.5],
    [14, 0.4],
    [15, 0.3],
  ])("+%i 강화 성공률을 계산한다", (stage, expected) => {
    expect(enhanceSuccessRate(stage)).toBe(expected);
  });

  it.each([
    [5, { gold: 500, reinforcementStone: 0, mysticStone: 0 }],
    [6, { gold: 3000, reinforcementStone: 1, mysticStone: 0 }],
    [11, { gold: 5000, reinforcementStone: 3, mysticStone: 0 }],
    [13, { gold: 12000, reinforcementStone: 8, mysticStone: 1 }],
    [15, { gold: 35000, reinforcementStone: 18, mysticStone: 3 }],
  ])("+%i 강화 자원 소비를 표와 1:1로 계산한다", (stage, expected) => {
    expect(enhanceResourceCost(stage)).toEqual(expected);
  });

  it("강화 단계 범위를 벗어나면 거절한다", () => {
    expect(() => enhanceSuccessRate(16)).toThrow("currentStage must be 0~15");
    expect(() => enhanceResourceCost(-1)).toThrow("currentStage must be 0~15");
  });
});
