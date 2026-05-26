import { describe, expect, it } from "vitest";
import {
  archerSkillDefinitions,
  clericSkillDefinitions,
  getSkillDefinitionsForClass,
  mageSkillDefinitions,
  thiefSkillDefinitions,
  warriorSkillDefinitions,
} from "./skills.js";

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

describe("mageSkillDefinitions", () => {
  it("mirrors the seven V1 mage skills with server-readable fields", () => {
    expect(mageSkillDefinitions).toHaveLength(7);
    expect(mageSkillDefinitions).toEqual([
      {
        skillId: "arcane_bolt",
        classId: 2,
        displayName: "Arcane Bolt",
        type: "active",
        effectType: "damage_single",
        cooldown: 3,
        damageCoeff: 2.4,
        buffMagnitude: 0,
        buffDuration: 0,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "chain_lightning",
        classId: 2,
        displayName: "Chain Lightning",
        type: "active",
        effectType: "damage_aoe",
        cooldown: 7,
        damageCoeff: 1.7,
        buffMagnitude: 0,
        buffDuration: 0,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "mana_shield",
        classId: 2,
        displayName: "Mana Shield",
        type: "active",
        effectType: "self_buff",
        cooldown: 12,
        damageCoeff: 0,
        buffMagnitude: 0.35,
        buffDuration: 4,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "meteor",
        classId: 2,
        displayName: "Meteor",
        type: "active",
        effectType: "damage_aoe",
        cooldown: 14,
        damageCoeff: 2.8,
        buffMagnitude: 0,
        buffDuration: 0,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "spell_mastery",
        classId: 2,
        displayName: "Spell Mastery",
        type: "passive",
        effectType: "self_buff",
        cooldown: 0,
        damageCoeff: 0,
        buffMagnitude: 0.15,
        buffDuration: 0,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "mana_flow",
        classId: 2,
        displayName: "Mana Flow",
        type: "passive",
        effectType: "self_buff",
        cooldown: 0,
        damageCoeff: 0,
        buffMagnitude: 0.2,
        buffDuration: 0,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "arcane_overload",
        classId: 2,
        displayName: "Arcane Overload",
        type: "ultimate",
        effectType: "damage_aoe",
        cooldown: 0,
        damageCoeff: 5.5,
        buffMagnitude: 0.25,
        buffDuration: 4,
        gaugeGainOnHit: 9,
        gaugeGainOnTakeDamage: 3,
      },
    ]);
  });
});

describe("archerSkillDefinitions", () => {
  it("mirrors the seven V1 archer skills with server-readable fields", () => {
    expect(archerSkillDefinitions).toHaveLength(7);
    expect(archerSkillDefinitions).toEqual([
      {
        skillId: "precision_shot",
        classId: 3,
        displayName: "Precision Shot",
        type: "active",
        effectType: "damage_single",
        cooldown: 3.5,
        damageCoeff: 2.2,
        buffMagnitude: 0,
        buffDuration: 0,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "arrow_rain",
        classId: 3,
        displayName: "Arrow Rain",
        type: "active",
        effectType: "damage_aoe",
        cooldown: 8,
        damageCoeff: 1.6,
        buffMagnitude: 0,
        buffDuration: 0,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "focus",
        classId: 3,
        displayName: "Focus",
        type: "active",
        effectType: "self_buff",
        cooldown: 10,
        damageCoeff: 0,
        buffMagnitude: 0.2,
        buffDuration: 4,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "piercing_arrow",
        classId: 3,
        displayName: "Piercing Arrow",
        type: "active",
        effectType: "dash_damage",
        cooldown: 9,
        damageCoeff: 2,
        buffMagnitude: 0,
        buffDuration: 0,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "critical_eye",
        classId: 3,
        displayName: "Critical Eye",
        type: "passive",
        effectType: "self_buff",
        cooldown: 0,
        damageCoeff: 0,
        buffMagnitude: 0.05,
        buffDuration: 0,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "quick_draw",
        classId: 3,
        displayName: "Quick Draw",
        type: "passive",
        effectType: "self_buff",
        cooldown: 0,
        damageCoeff: 0,
        buffMagnitude: 0.1,
        buffDuration: 0,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "eagle_eye",
        classId: 3,
        displayName: "Eagle Eye",
        type: "ultimate",
        effectType: "damage_single",
        cooldown: 0,
        damageCoeff: 5,
        buffMagnitude: 0.25,
        buffDuration: 4,
        gaugeGainOnHit: 10,
        gaugeGainOnTakeDamage: 2,
      },
    ]);
  });
});

describe("thiefSkillDefinitions", () => {
  it("mirrors the seven V1 thief skills with server-readable fields", () => {
    expect(thiefSkillDefinitions).toHaveLength(7);
    expect(thiefSkillDefinitions).toEqual([
      {
        skillId: "shadow_stab",
        classId: 4,
        displayName: "Shadow Stab",
        type: "active",
        effectType: "damage_single",
        cooldown: 3,
        damageCoeff: 2.3,
        buffMagnitude: 0,
        buffDuration: 0,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "smoke_bomb",
        classId: 4,
        displayName: "Smoke Bomb",
        type: "active",
        effectType: "damage_aoe",
        cooldown: 7,
        damageCoeff: 1.5,
        buffMagnitude: 0,
        buffDuration: 0,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "evasion_stance",
        classId: 4,
        displayName: "Evasion Stance",
        type: "active",
        effectType: "self_buff",
        cooldown: 10,
        damageCoeff: 0,
        buffMagnitude: 0.2,
        buffDuration: 4,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "backstab",
        classId: 4,
        displayName: "Backstab",
        type: "active",
        effectType: "dash_damage",
        cooldown: 9,
        damageCoeff: 2.1,
        buffMagnitude: 0,
        buffDuration: 0,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "nimble_hands",
        classId: 4,
        displayName: "Nimble Hands",
        type: "passive",
        effectType: "self_buff",
        cooldown: 0,
        damageCoeff: 0,
        buffMagnitude: 0.05,
        buffDuration: 0,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "lucky_instinct",
        classId: 4,
        displayName: "Lucky Instinct",
        type: "passive",
        effectType: "self_buff",
        cooldown: 0,
        damageCoeff: 0,
        buffMagnitude: 0.05,
        buffDuration: 0,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "assassinate",
        classId: 4,
        displayName: "Assassinate",
        type: "ultimate",
        effectType: "damage_single",
        cooldown: 0,
        damageCoeff: 5.3,
        buffMagnitude: 0.25,
        buffDuration: 4,
        gaugeGainOnHit: 11,
        gaugeGainOnTakeDamage: 1,
      },
    ]);
  });
});

describe("clericSkillDefinitions", () => {
  it("mirrors the seven V1 cleric skills including heal effects", () => {
    expect(clericSkillDefinitions).toHaveLength(7);
    expect(clericSkillDefinitions).toEqual([
      {
        skillId: "holy_smite",
        classId: 5,
        displayName: "Holy Smite",
        type: "active",
        effectType: "damage_single",
        cooldown: 3.2,
        damageCoeff: 2,
        buffMagnitude: 0,
        buffDuration: 0,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "heal",
        classId: 5,
        displayName: "Heal",
        type: "active",
        effectType: "heal",
        cooldown: 6,
        damageCoeff: 0,
        buffMagnitude: 0.2,
        buffDuration: 0,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "blessing",
        classId: 5,
        displayName: "Blessing",
        type: "active",
        effectType: "self_buff",
        cooldown: 10,
        damageCoeff: 0,
        buffMagnitude: 0.15,
        buffDuration: 4,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "purify",
        classId: 5,
        displayName: "Purify",
        type: "active",
        effectType: "self_buff",
        cooldown: 12,
        damageCoeff: 0,
        buffMagnitude: 0.25,
        buffDuration: 4,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "wisdom_training",
        classId: 5,
        displayName: "Wisdom Training",
        type: "passive",
        effectType: "self_buff",
        cooldown: 0,
        damageCoeff: 0,
        buffMagnitude: 0.1,
        buffDuration: 0,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "divine_vitality",
        classId: 5,
        displayName: "Divine Vitality",
        type: "passive",
        effectType: "self_buff",
        cooldown: 0,
        damageCoeff: 0,
        buffMagnitude: 0.2,
        buffDuration: 0,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "sanctuary",
        classId: 5,
        displayName: "Sanctuary",
        type: "ultimate",
        effectType: "heal",
        cooldown: 0,
        damageCoeff: 0,
        buffMagnitude: 0.4,
        buffDuration: 0,
        gaugeGainOnHit: 6,
        gaugeGainOnTakeDamage: 6,
      },
    ]);
  });
});

describe("skill definition parity by class", () => {
  it("returns the V1 skill set for every supported class id", () => {
    expect(getSkillDefinitionsForClass(1)).toBe(warriorSkillDefinitions);
    expect(getSkillDefinitionsForClass(2)).toBe(mageSkillDefinitions);
    expect(getSkillDefinitionsForClass(3)).toBe(archerSkillDefinitions);
    expect(getSkillDefinitionsForClass(4)).toBe(thiefSkillDefinitions);
    expect(getSkillDefinitionsForClass(5)).toBe(clericSkillDefinitions);
  });

  it("keeps every supported class id aligned with its lookup key", () => {
    for (const classId of [1, 2, 3, 4, 5]) {
      for (const skill of getSkillDefinitionsForClass(classId)) {
        expect(skill.classId).toBe(classId);
      }
    }
  });

  it("contains 35 V1 skill definitions across the five supported classes", () => {
    const allSkills = [1, 2, 3, 4, 5].flatMap((classId) =>
      getSkillDefinitionsForClass(classId),
    );

    expect(allSkills).toHaveLength(35);
    expect(new Set(allSkills.map((skill) => skill.skillId)).size).toBe(35);
    expect(new Set(allSkills.map((skill) => skill.effectType))).toContain(
      "heal",
    );
  });
});
