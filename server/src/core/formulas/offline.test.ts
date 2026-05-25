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
