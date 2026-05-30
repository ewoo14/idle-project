import { describe, expect, it } from "vitest";
import { type DecideOnClearInput, decideOnClear } from "./automation.js";

describe("automation decideOnClear (progression on clear)", () => {
  const base: DecideOnClearInput = {
    mode: "Advance",
    clearedGlobalStage: 5,
    highestClearedGlobalStage: 5,
    farmLockStage: 3,
    autoBossChallenge: true,
    nextIsBoss: false,
  };

  it("advances to the next stage on a normal clear", () => {
    expect(decideOnClear(base)).toEqual({
      action: "advance",
      targetGlobalStage: 6,
    });
  });

  it("holds before a boss when auto boss challenge is disabled", () => {
    expect(
      decideOnClear({ ...base, nextIsBoss: true, autoBossChallenge: false }),
    ).toEqual({ action: "hold", targetGlobalStage: 5 });
  });

  it("advances into a boss when auto boss challenge is enabled", () => {
    expect(
      decideOnClear({ ...base, nextIsBoss: true, autoBossChallenge: true }),
    ).toEqual({ action: "advance", targetGlobalStage: 6 });
  });

  it("holds at the farm lock stage in FarmLock mode", () => {
    expect(decideOnClear({ ...base, mode: "FarmLock" })).toEqual({
      action: "hold",
      targetGlobalStage: 3,
    });
  });

  it("clamps the farm lock stage to highest+1", () => {
    expect(
      decideOnClear({ ...base, mode: "FarmLock", farmLockStage: 99 }),
    ).toEqual({ action: "hold", targetGlobalStage: 6 });
  });

  it("advances on clear in AutoRetreat mode (retreat only triggers on death)", () => {
    expect(decideOnClear({ ...base, mode: "AutoRetreat" })).toEqual({
      action: "advance",
      targetGlobalStage: 6,
    });
  });
});
