import { describe, expect, it } from "vitest";
import {
  baseExpPerSec,
  baseGoldPerSec,
  computeOfflineRewards,
  OFFLINE_CAP_SECONDS,
  OFFLINE_EFFICIENCY,
} from "./offline.js";

describe("computeOfflineRewards", () => {
  it("caps elapsed time at twelve hours", () => {
    const result = computeOfflineRewards({
      level: 10,
      lastSeenUnixSec: 1_000,
      nowUnixSec: 1_000 + OFFLINE_CAP_SECONDS + 60,
    });

    expect(result.cappedSeconds).toBe(OFFLINE_CAP_SECONDS);
  });

  it("keeps one second below cap and clamps one second above cap", () => {
    const underCap = computeOfflineRewards({
      level: 10,
      lastSeenUnixSec: 1_000,
      nowUnixSec: 1_000 + OFFLINE_CAP_SECONDS - 1,
    });
    const overCap = computeOfflineRewards({
      level: 10,
      lastSeenUnixSec: 1_000,
      nowUnixSec: 1_000 + OFFLINE_CAP_SECONDS + 1,
    });

    expect(underCap.cappedSeconds).toBe(43_199);
    expect(overCap.cappedSeconds).toBe(43_200);
  });

  it("returns zero rewards when no time elapsed", () => {
    const result = computeOfflineRewards({
      level: 10,
      lastSeenUnixSec: 2_000,
      nowUnixSec: 1_000,
    });

    expect(result).toMatchObject({
      cappedSeconds: 0,
      gold: 0,
      exp: 0,
    });
  });

  it("accepts unix epoch zero as a valid lastSeen timestamp", () => {
    const result = computeOfflineRewards({
      level: 1,
      lastSeenUnixSec: 0,
      nowUnixSec: 60,
    });

    expect(result.cappedSeconds).toBe(60);
    expect(result.gold).toBe(180);
    expect(result.exp).toBe(180);
  });

  it("returns non-zero gold and exp for a positive elapsed second", () => {
    const result = computeOfflineRewards({
      level: 1,
      lastSeenUnixSec: 1_000,
      nowUnixSec: 1_001,
    });

    expect(result.gold).toBeGreaterThan(0);
    expect(result.exp).toBeGreaterThan(0);
  });

  it("applies offline efficiency to level-based base rates", () => {
    const level = 5;
    const seconds = 60;
    const result = computeOfflineRewards({
      level,
      lastSeenUnixSec: 1_000,
      nowUnixSec: 1_000 + seconds,
    });

    expect(result.gold).toBe(
      Math.round(baseGoldPerSec(level) * seconds * OFFLINE_EFFICIENCY),
    );
    expect(result.exp).toBe(
      Math.round(baseExpPerSec(level) * seconds * OFFLINE_EFFICIENCY),
    );
  });

  it("keeps high-level base rates in numeric parity with client int64", () => {
    const highLevel = 1_000_000_000;

    expect(baseGoldPerSec(highLevel)).toBe(4_000_000_000);
    expect(baseExpPerSec(highLevel)).toBe(4_000_000_000);
  });

  it("increases rewards with rebirth bonus", () => {
    const base = computeOfflineRewards({
      level: 20,
      lastSeenUnixSec: 1_000,
      nowUnixSec: 1_600,
    });
    const rebirthed = computeOfflineRewards({
      level: 20,
      lastSeenUnixSec: 1_000,
      nowUnixSec: 1_600,
      rebirthCount: 2,
    });

    expect(rebirthed.timeBonusMultiplier).toBeGreaterThan(
      base.timeBonusMultiplier,
    );
    expect(rebirthed.gold).toBeGreaterThan(base.gold);
    expect(rebirthed.exp).toBeGreaterThan(base.exp);
  });

  it("increases time bonus for longer capped sessions", () => {
    const oneHour = computeOfflineRewards({
      level: 20,
      lastSeenUnixSec: 1_000,
      nowUnixSec: 1_000 + 3_600,
    });
    const twelveHours = computeOfflineRewards({
      level: 20,
      lastSeenUnixSec: 1_000,
      nowUnixSec: 1_000 + OFFLINE_CAP_SECONDS,
    });

    expect(twelveHours.timeBonusMultiplier).toBeGreaterThan(
      oneHour.timeBonusMultiplier,
    );
  });

  it("rejects invalid level values", () => {
    expect(() =>
      computeOfflineRewards({
        level: 0,
        lastSeenUnixSec: 1_000,
        nowUnixSec: 1_100,
      }),
    ).toThrow("level must be >= 1");
  });
});
