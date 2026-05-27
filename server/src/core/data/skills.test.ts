import { describe, expect, it } from "vitest";
import {
  archerSkillDefinitions,
  berserkerSkillDefinitions,
  clericSkillDefinitions,
  getSkillDefinitionsForClass,
  mageSkillDefinitions,
  paladinSkillDefinitions,
  type SkillDefinition,
  summonerSkillDefinitions,
  thiefSkillDefinitions,
  warriorSkillDefinitions,
} from "./skills.js";

type ExpectedSkillDefinition = Omit<
  SkillDefinition,
  "statusEffect" | "statusDuration" | "statusMagnitude" | "element"
> &
  Partial<
    Pick<
      SkillDefinition,
      "statusEffect" | "statusDuration" | "statusMagnitude" | "element"
    >
  >;

function withExpectedDefaults(skill: ExpectedSkillDefinition): SkillDefinition {
  return {
    statusEffect: "none",
    statusDuration: 0,
    statusMagnitude: 0,
    element: "none",
    ...skill,
  };
}

function expectedSkillsForClass(classId: number): SkillDefinition[] {
  return (
    expectedSkillDefinitionsByClass.get(classId)?.map(withExpectedDefaults) ??
    []
  );
}

const expectedSkillDefinitionsByClass = new Map<
  number,
  ExpectedSkillDefinition[]
>([
  [
    1,
    [
      {
        skillId: "heavy_strike",
        classId: 1,
        displayName: "강타",
        type: "active",
        effectType: "damage_single",
        cooldown: 4,
        damageCoeff: 2.5,
        buffMagnitude: 0,
        buffDuration: 0,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "whirlwind",
        classId: 1,
        displayName: "회전베기",
        type: "active",
        effectType: "damage_aoe",
        cooldown: 8,
        damageCoeff: 1.8,
        buffMagnitude: 0,
        buffDuration: 0,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "shield_up",
        classId: 1,
        displayName: "방패 올리기",
        type: "active",
        effectType: "self_buff",
        cooldown: 12,
        damageCoeff: 0,
        buffMagnitude: 0.5,
        buffDuration: 4,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "charge",
        classId: 1,
        displayName: "돌진",
        type: "active",
        effectType: "dash_damage",
        cooldown: 10,
        damageCoeff: 2,
        buffMagnitude: 0,
        buffDuration: 0,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "weapon_mastery",
        classId: 1,
        displayName: "무기 숙련",
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
        skillId: "toughness",
        classId: 1,
        displayName: "강인함",
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
        skillId: "berserkers_fury",
        classId: 1,
        displayName: "광전사의 분노",
        type: "ultimate",
        effectType: "damage_single",
        cooldown: 0,
        damageCoeff: 6,
        buffMagnitude: 0.3,
        buffDuration: 4,
        gaugeGainOnHit: 8,
        gaugeGainOnTakeDamage: 5,
      },
    ],
  ],
  [
    2,
    [
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
        statusEffect: "burn",
        statusDuration: 3,
        statusMagnitude: 4,
        element: "fire",
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
        element: "lightning",
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
        statusEffect: "freeze",
        statusDuration: 2,
        statusMagnitude: 0.25,
        element: "ice",
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
    ],
  ],
  [
    3,
    [
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
    ],
  ],
  [
    4,
    [
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
        statusEffect: "poison",
        statusDuration: 3,
        statusMagnitude: 3,
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
        statusEffect: "poison",
        statusDuration: 3,
        statusMagnitude: 2,
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
    ],
  ],
  [
    5,
    [
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
        element: "holy",
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
    ],
  ],
  [
    6,
    [
      {
        skillId: "holy_verdict",
        classId: 6,
        displayName: "Holy Verdict",
        type: "active",
        effectType: "damage_single",
        cooldown: 4,
        damageCoeff: 2.3,
        buffMagnitude: 0,
        buffDuration: 0,
        gaugeGainOnHit: 1,
        gaugeGainOnTakeDamage: 1,
        element: "holy",
      },
      {
        skillId: "radiant_sweep",
        classId: 6,
        displayName: "Radiant Sweep",
        type: "active",
        effectType: "damage_aoe",
        cooldown: 8,
        damageCoeff: 1.6,
        buffMagnitude: 0,
        buffDuration: 0,
        gaugeGainOnHit: 1,
        gaugeGainOnTakeDamage: 1,
        element: "holy",
      },
      {
        skillId: "guardian_aegis",
        classId: 6,
        displayName: "Guardian Aegis",
        type: "active",
        effectType: "self_buff",
        cooldown: 12,
        damageCoeff: 0,
        buffMagnitude: 0.45,
        buffDuration: 5,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 2,
      },
      {
        skillId: "lay_on_hands",
        classId: 6,
        displayName: "Lay on Hands",
        type: "active",
        effectType: "heal",
        cooldown: 10,
        damageCoeff: 0,
        buffMagnitude: 0.18,
        buffDuration: 0,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "sacred_oath",
        classId: 6,
        displayName: "Sacred Oath",
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
        skillId: "bulwark_training",
        classId: 6,
        displayName: "Bulwark Training",
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
        skillId: "divine_bastion",
        classId: 6,
        displayName: "Divine Bastion",
        type: "ultimate",
        effectType: "self_buff",
        cooldown: 0,
        damageCoeff: 0,
        buffMagnitude: 0.35,
        buffDuration: 5,
        gaugeGainOnHit: 5,
        gaugeGainOnTakeDamage: 8,
      },
    ],
  ],
  [
    7,
    [
      {
        skillId: "rage_cleave",
        classId: 7,
        displayName: "Rage Cleave",
        type: "active",
        effectType: "damage_single",
        cooldown: 3.5,
        damageCoeff: 2.8,
        buffMagnitude: 0,
        buffDuration: 0,
        gaugeGainOnHit: 2,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "blood_surge",
        classId: 7,
        displayName: "Blood Surge",
        type: "active",
        effectType: "damage_aoe",
        cooldown: 7.5,
        damageCoeff: 1.9,
        buffMagnitude: 0,
        buffDuration: 0,
        gaugeGainOnHit: 2,
        gaugeGainOnTakeDamage: 0,
        statusEffect: "burn",
        statusDuration: 2,
        statusMagnitude: 3,
        element: "fire",
      },
      {
        skillId: "frenzy_stance",
        classId: 7,
        displayName: "Frenzy Stance",
        type: "active",
        effectType: "self_buff",
        cooldown: 11,
        damageCoeff: 0,
        buffMagnitude: 0.3,
        buffDuration: 4,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "savage_leap",
        classId: 7,
        displayName: "Savage Leap",
        type: "active",
        effectType: "dash_damage",
        cooldown: 9,
        damageCoeff: 2.4,
        buffMagnitude: 0,
        buffDuration: 0,
        gaugeGainOnHit: 2,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "blood_frenzy",
        classId: 7,
        displayName: "Blood Frenzy",
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
        skillId: "pain_to_power",
        classId: 7,
        displayName: "Pain to Power",
        type: "passive",
        effectType: "self_buff",
        cooldown: 0,
        damageCoeff: 0,
        buffMagnitude: 0.08,
        buffDuration: 0,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "berserk_apex",
        classId: 7,
        displayName: "Berserk Apex",
        type: "ultimate",
        effectType: "damage_single",
        cooldown: 0,
        damageCoeff: 6.5,
        buffMagnitude: 0.35,
        buffDuration: 4,
        gaugeGainOnHit: 12,
        gaugeGainOnTakeDamage: 2,
      },
    ],
  ],
  [
    8,
    [
      {
        skillId: "spirit_bolt",
        classId: 8,
        displayName: "Spirit Bolt",
        type: "active",
        effectType: "damage_single",
        cooldown: 3.2,
        damageCoeff: 2.2,
        buffMagnitude: 0,
        buffDuration: 0,
        gaugeGainOnHit: 1,
        gaugeGainOnTakeDamage: 0,
        statusEffect: "poison",
        statusDuration: 3,
        statusMagnitude: 2.5,
      },
      {
        skillId: "familiar_swarm",
        classId: 8,
        displayName: "Familiar Swarm",
        type: "active",
        effectType: "damage_aoe",
        cooldown: 7,
        damageCoeff: 1.6,
        buffMagnitude: 0,
        buffDuration: 0,
        gaugeGainOnHit: 1,
        gaugeGainOnTakeDamage: 0,
        statusEffect: "poison",
        statusDuration: 4,
        statusMagnitude: 2,
      },
      {
        skillId: "arcane_binding",
        classId: 8,
        displayName: "Arcane Binding",
        type: "active",
        effectType: "self_buff",
        cooldown: 10,
        damageCoeff: 0,
        buffMagnitude: 0.22,
        buffDuration: 4,
        gaugeGainOnHit: 0,
        gaugeGainOnTakeDamage: 0,
      },
      {
        skillId: "void_call",
        classId: 8,
        displayName: "Void Call",
        type: "active",
        effectType: "damage_aoe",
        cooldown: 12,
        damageCoeff: 2.3,
        buffMagnitude: 0,
        buffDuration: 0,
        gaugeGainOnHit: 1.5,
        gaugeGainOnTakeDamage: 0,
        statusEffect: "freeze",
        statusDuration: 2,
        statusMagnitude: 0.2,
        element: "ice",
      },
      {
        skillId: "pact_mastery",
        classId: 8,
        displayName: "Pact Mastery",
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
        skillId: "spirit_reservoir",
        classId: 8,
        displayName: "Spirit Reservoir",
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
        skillId: "grand_familiar",
        classId: 8,
        displayName: "Grand Familiar",
        type: "ultimate",
        effectType: "damage_aoe",
        cooldown: 0,
        damageCoeff: 5.7,
        buffMagnitude: 0.25,
        buffDuration: 4,
        gaugeGainOnHit: 10,
        gaugeGainOnTakeDamage: 3,
        statusEffect: "poison",
        statusDuration: 5,
        statusMagnitude: 4,
        element: "lightning",
      },
    ],
  ],
]);

describe("warriorSkillDefinitions", () => {
  it("mirrors the seven V1 warrior skills with server-readable fields", () => {
    expect(warriorSkillDefinitions).toHaveLength(7);
    expect(warriorSkillDefinitions).toEqual(expectedSkillsForClass(1));
  });
});

describe("mageSkillDefinitions", () => {
  it("mirrors the seven V1 mage skills with server-readable fields", () => {
    expect(mageSkillDefinitions).toHaveLength(7);
    expect(mageSkillDefinitions).toEqual(expectedSkillsForClass(2));
  });
});

describe("archerSkillDefinitions", () => {
  it("mirrors the seven V1 archer skills with server-readable fields", () => {
    expect(archerSkillDefinitions).toHaveLength(7);
    expect(archerSkillDefinitions).toEqual(expectedSkillsForClass(3));
  });
});

describe("thiefSkillDefinitions", () => {
  it("mirrors the seven V1 thief skills with server-readable fields", () => {
    expect(thiefSkillDefinitions).toHaveLength(7);
    expect(thiefSkillDefinitions).toEqual(expectedSkillsForClass(4));
  });
});

describe("clericSkillDefinitions", () => {
  it("mirrors the seven V1 cleric skills including heal effects", () => {
    expect(clericSkillDefinitions).toHaveLength(7);
    expect(clericSkillDefinitions).toEqual(expectedSkillsForClass(5));
  });
});

describe("paladinSkillDefinitions", () => {
  it("mirrors the seven class expansion paladin skills including holy tank effects", () => {
    expect(paladinSkillDefinitions).toHaveLength(7);
    expect(paladinSkillDefinitions).toEqual(expectedSkillsForClass(6));
  });
});

describe("berserkerSkillDefinitions", () => {
  it("mirrors the seven class expansion berserker skills including rage effects", () => {
    expect(berserkerSkillDefinitions).toHaveLength(7);
    expect(berserkerSkillDefinitions).toEqual(expectedSkillsForClass(7));
  });
});

describe("summonerSkillDefinitions", () => {
  it("mirrors the seven class expansion summoner skills including status effects", () => {
    expect(summonerSkillDefinitions).toHaveLength(7);
    expect(summonerSkillDefinitions).toEqual(expectedSkillsForClass(8));
  });
});

describe("skill definition parity by class", () => {
  it("returns the V1 skill set for every supported class id", () => {
    expect(getSkillDefinitionsForClass(1)).toBe(warriorSkillDefinitions);
    expect(getSkillDefinitionsForClass(2)).toBe(mageSkillDefinitions);
    expect(getSkillDefinitionsForClass(3)).toBe(archerSkillDefinitions);
    expect(getSkillDefinitionsForClass(4)).toBe(thiefSkillDefinitions);
    expect(getSkillDefinitionsForClass(5)).toBe(clericSkillDefinitions);
    expect(getSkillDefinitionsForClass(6)).toBe(paladinSkillDefinitions);
    expect(getSkillDefinitionsForClass(7)).toBe(berserkerSkillDefinitions);
    expect(getSkillDefinitionsForClass(8)).toBe(summonerSkillDefinitions);
  });

  it("keeps every supported class id aligned with its lookup key", () => {
    for (const classId of [1, 2, 3, 4, 5, 6, 7, 8]) {
      for (const skill of getSkillDefinitionsForClass(classId)) {
        expect(skill.classId).toBe(classId);
      }
    }
  });

  it("contains 56 skill definitions across the eight supported classes", () => {
    const allSkills = [1, 2, 3, 4, 5, 6, 7, 8].flatMap((classId) =>
      getSkillDefinitionsForClass(classId),
    );

    expect(allSkills).toHaveLength(56);
    expect(new Set(allSkills.map((skill) => skill.skillId)).size).toBe(56);
    expect(new Set(allSkills.map((skill) => skill.effectType))).toContain(
      "heal",
    );
  });

  it("keeps every class definition in exact parity with the V1 balance table", () => {
    for (const [classId] of expectedSkillDefinitionsByClass) {
      expect(getSkillDefinitionsForClass(classId)).toEqual(
        expectedSkillsForClass(classId),
      );
    }
  });
});
