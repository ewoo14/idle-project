import { describe, expect, it } from "vitest";
import {
  getChallengeDamage,
  getReachedMilestones,
  milestoneEssenceReward,
  milestoneGoldReward,
  milestoneThreshold,
  WEEKLY_CHALLENGE_LIMIT,
} from "./weeklyBoss.js";

// 클라 FWeeklyBossFormula(Tests/WeeklyBossTests.cpp FormulaParity)와 동일 앵커를
// 검증해 서버↔클라 공식 drift 를 방지한다.
describe("weeklyBoss parity with FWeeklyBossFormula", () => {
  it("주간 도전 한도는 7", () => {
    expect(WEEKLY_CHALLENGE_LIMIT).toBe(7);
  });

  it("도전 데미지는 CP 절단/음수 클램프", () => {
    expect(getChallengeDamage(1234)).toBe(1234);
    expect(getChallengeDamage(1234.9)).toBe(1234);
    expect(getChallengeDamage(-10)).toBe(0);
  });

  it("마일스톤 임계 앵커 (floor(1000*1.5^(n-1)))", () => {
    expect(milestoneThreshold(0)).toBe(0);
    expect(milestoneThreshold(1)).toBe(1000);
    expect(milestoneThreshold(2)).toBe(1500);
    expect(milestoneThreshold(3)).toBe(2250);
    expect(milestoneThreshold(5)).toBe(5062);
  });

  it("도달 마일스톤 경계", () => {
    expect(getReachedMilestones(999)).toBe(0);
    expect(getReachedMilestones(1000)).toBe(1);
    expect(getReachedMilestones(5062)).toBe(5);
  });

  it("마일스톤 보상 앵커 (골드 1.5^, 에센스 3n)", () => {
    expect(milestoneGoldReward(1)).toBe(5000);
    expect(milestoneGoldReward(5)).toBe(25312);
    expect(milestoneEssenceReward(5)).toBe(15);
  });

  it("마일스톤 골드 보상은 단조 증가", () => {
    expect(milestoneGoldReward(10)).toBeGreaterThan(milestoneGoldReward(9));
  });
});
