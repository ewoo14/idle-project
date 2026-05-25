export type SkillDefinition = {
  skillId: string;
  classId: number;
  displayName: string;
  type: "active" | "passive" | "ultimate";
  effectType: "damage_single" | "damage_aoe" | "self_buff" | "dash_damage";
  cooldown: number;
  damageCoeff: number;
  buffMagnitude: number;
  buffDuration: number;
  gaugeGainOnHit: number;
  gaugeGainOnTakeDamage: number;
};

export const warriorSkillDefinitions: SkillDefinition[] = [
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
];
