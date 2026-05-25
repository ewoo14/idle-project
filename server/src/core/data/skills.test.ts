import { describe, expect, it } from "vitest";
import { warriorSkillDefinitions } from "./skills.js";

describe("warriorSkillDefinitions", () => {
  it("mirrors the seven V1 warrior skills with server-readable fields", () => {
    expect(warriorSkillDefinitions).toHaveLength(7);
    expect(warriorSkillDefinitions).toEqual(
      expect.arrayContaining([
        expect.objectContaining({
          skillId: "heavy_strike",
          classId: 1,
          type: "active",
          cooldown: 4,
          damageCoeff: 2.5,
        }),
        expect.objectContaining({
          skillId: "whirlwind",
          classId: 1,
          type: "active",
          effectType: "damage_aoe",
          cooldown: 8,
          damageCoeff: 1.8,
        }),
        expect.objectContaining({
          skillId: "berserkers_fury",
          classId: 1,
          type: "ultimate",
          damageCoeff: 6,
          gaugeGainOnHit: 8,
          gaugeGainOnTakeDamage: 5,
        }),
      ]),
    );
  });
});
