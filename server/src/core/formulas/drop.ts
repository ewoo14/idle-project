import type { EquipmentBonus, ItemSlot } from "./equipment.js";

export type ItemRarity =
  | "None"
  | "Common"
  | "Rare"
  | "Epic"
  | "Unique"
  | "Legendary"
  | "Transcendent"
  | "Mythic";

export type DropRng = () => number;

const ARMOR_SLOTS = new Set<ItemSlot>([2, 3, 4, 5, 6, 7]);
const toClientFloat = Math.fround;
const AFFIX_KINDS = [
  "CritRate",
  "AtkSpeed",
  "MagicAtk",
  "PhysDef",
  "MagicDef",
  "Hp",
  "CritDmg",
] as const;

type AffixKind = (typeof AFFIX_KINDS)[number];

export interface AffixBonus {
  bonusCritRate: number;
  bonusAtkSpeed: number;
  bonusMagicAtk: number;
  bonusPhysDef: number;
  bonusMagicDef: number;
  bonusAffixHp: number;
  bonusCritDmg: number;
}

export interface BaseItemDefinition {
  baseItemId: string;
  nameKo: string;
  nameEn: string;
  statBias: "physical" | "magic" | "defense" | "speed" | "balanced";
  atkScale: number;
  defScale: number;
  hpScale: number;
}

const BASE_ITEM_STAT_SCALES: Record<
  string,
  Pick<BaseItemDefinition, "atkScale" | "defScale" | "hpScale">
> = {
  longsword: { atkScale: 1.05, defScale: 1, hpScale: 1 },
  greatsword: { atkScale: 1.15, defScale: 0.9, hpScale: 1 },
  dagger: { atkScale: 0.9, defScale: 1, hpScale: 1 },
  bow: { atkScale: 1, defScale: 1, hpScale: 1 },
  staff: { atkScale: 0.9, defScale: 1, hpScale: 1 },
  wand: { atkScale: 0.85, defScale: 1, hpScale: 1 },
  helm: { atkScale: 1, defScale: 1.08, hpScale: 1 },
  hood: { atkScale: 1, defScale: 0.92, hpScale: 1.05 },
  circlet: { atkScale: 1, defScale: 1, hpScale: 1 },
  armor: { atkScale: 1, defScale: 1.1, hpScale: 1.05 },
  robe: { atkScale: 1, defScale: 0.9, hpScale: 1 },
  jacket: { atkScale: 1, defScale: 1, hpScale: 0.95 },
  greaves: { atkScale: 1, defScale: 1.08, hpScale: 1 },
  trousers: { atkScale: 1, defScale: 0.95, hpScale: 0.95 },
  leggings: { atkScale: 1, defScale: 0.9, hpScale: 1.05 },
  boots: { atkScale: 1, defScale: 1, hpScale: 1 },
  shoes: { atkScale: 1, defScale: 0.92, hpScale: 0.9 },
  sandals: { atkScale: 1, defScale: 0.95, hpScale: 1.05 },
  gauntlets: { atkScale: 1, defScale: 1.08, hpScale: 1 },
  gloves: { atkScale: 1, defScale: 0.92, hpScale: 0.95 },
  bracers: { atkScale: 1, defScale: 1, hpScale: 1 },
  cloak: { atkScale: 1, defScale: 1, hpScale: 1 },
  cape: { atkScale: 1, defScale: 1.02, hpScale: 0.95 },
  mantle: { atkScale: 1, defScale: 0.95, hpScale: 1.08 },
  ring: { atkScale: 1, defScale: 1, hpScale: 1 },
  amulet: { atkScale: 0.95, defScale: 1, hpScale: 1.05 },
  talisman: { atkScale: 0.9, defScale: 1.08, hpScale: 1 },
};

type BaseItemCatalogEntry = Omit<
  BaseItemDefinition,
  "atkScale" | "defScale" | "hpScale"
>;

const BASE_ITEMS_BY_SLOT: Partial<Record<ItemSlot, BaseItemCatalogEntry[]>> = {
  1: [
    {
      baseItemId: "longsword",
      nameKo: "장검",
      nameEn: "Longsword",
      statBias: "physical",
    },
    {
      baseItemId: "greatsword",
      nameKo: "대검",
      nameEn: "Greatsword",
      statBias: "physical",
    },
    {
      baseItemId: "dagger",
      nameKo: "단검",
      nameEn: "Dagger",
      statBias: "speed",
    },
    { baseItemId: "bow", nameKo: "활", nameEn: "Bow", statBias: "speed" },
    {
      baseItemId: "staff",
      nameKo: "지팡이",
      nameEn: "Staff",
      statBias: "magic",
    },
    { baseItemId: "wand", nameKo: "마법봉", nameEn: "Wand", statBias: "magic" },
  ],
  2: [
    { baseItemId: "helm", nameKo: "투구", nameEn: "Helm", statBias: "defense" },
    { baseItemId: "hood", nameKo: "후드", nameEn: "Hood", statBias: "magic" },
    {
      baseItemId: "circlet",
      nameKo: "서클릿",
      nameEn: "Circlet",
      statBias: "balanced",
    },
  ],
  3: [
    {
      baseItemId: "armor",
      nameKo: "갑옷",
      nameEn: "Armor",
      statBias: "defense",
    },
    { baseItemId: "robe", nameKo: "로브", nameEn: "Robe", statBias: "magic" },
    {
      baseItemId: "jacket",
      nameKo: "재킷",
      nameEn: "Jacket",
      statBias: "speed",
    },
  ],
  4: [
    {
      baseItemId: "greaves",
      nameKo: "각반",
      nameEn: "Greaves",
      statBias: "defense",
    },
    {
      baseItemId: "trousers",
      nameKo: "전투 바지",
      nameEn: "Battle Trousers",
      statBias: "speed",
    },
    {
      baseItemId: "leggings",
      nameKo: "마력 레깅스",
      nameEn: "Arcane Leggings",
      statBias: "magic",
    },
  ],
  5: [
    {
      baseItemId: "boots",
      nameKo: "장화",
      nameEn: "Boots",
      statBias: "defense",
    },
    { baseItemId: "shoes", nameKo: "신발", nameEn: "Shoes", statBias: "speed" },
    {
      baseItemId: "sandals",
      nameKo: "성화 샌들",
      nameEn: "Blessed Sandals",
      statBias: "magic",
    },
  ],
  6: [
    {
      baseItemId: "gauntlets",
      nameKo: "건틀릿",
      nameEn: "Gauntlets",
      statBias: "physical",
    },
    {
      baseItemId: "gloves",
      nameKo: "장갑",
      nameEn: "Gloves",
      statBias: "speed",
    },
    {
      baseItemId: "bracers",
      nameKo: "팔보호구",
      nameEn: "Bracers",
      statBias: "defense",
    },
  ],
  7: [
    {
      baseItemId: "cloak",
      nameKo: "망토",
      nameEn: "Cloak",
      statBias: "balanced",
    },
    {
      baseItemId: "cape",
      nameKo: "전투 망토",
      nameEn: "Battle Cape",
      statBias: "physical",
    },
    {
      baseItemId: "mantle",
      nameKo: "현자의 외투",
      nameEn: "Sage Mantle",
      statBias: "magic",
    },
  ],
  8: [
    { baseItemId: "ring", nameKo: "반지", nameEn: "Ring", statBias: "magic" },
    {
      baseItemId: "amulet",
      nameKo: "목걸이",
      nameEn: "Amulet",
      statBias: "balanced",
    },
    {
      baseItemId: "talisman",
      nameKo: "부적",
      nameEn: "Talisman",
      statBias: "defense",
    },
  ],
};

export function getRarityStatMultiplier(rarity: ItemRarity): number {
  switch (rarity) {
    case "Common":
      return toClientFloat(1);
    case "Rare":
      return toClientFloat(1.7);
    case "Epic":
      return toClientFloat(2.3);
    case "Unique":
      return toClientFloat(2.75);
    case "Legendary":
      return toClientFloat(3.2);
    case "Transcendent":
      return toClientFloat(3.85);
    case "Mythic":
      return toClientFloat(4.5);
    case "None":
      return toClientFloat(0);
  }
}

export function rollRarityForLevel(
  level: number,
  rng: DropRng = Math.random,
): ItemRarity {
  const safeLevel = Math.max(level, 1);
  const levelScale = toClientFloat(
    Math.min(Math.max(toClientFloat(toClientFloat(safeLevel - 1) / 99), 0), 1),
  );

  const noneChance = toClientFloat(0.02);
  const rareChance = toClientFloat(
    toClientFloat(0.28) + toClientFloat(toClientFloat(0.02) * levelScale),
  );
  const epicChance = toClientFloat(toClientFloat(0.06) * levelScale);
  const uniqueChance = toClientFloat(toClientFloat(0.025) * levelScale);
  const legendaryChance = toClientFloat(toClientFloat(0.015) * levelScale);
  const transcendentChance = toClientFloat(toClientFloat(0.007) * levelScale);
  const mythicChance = toClientFloat(toClientFloat(0.005) * levelScale);
  let commonChance = toClientFloat(1 - noneChance);
  commonChance = toClientFloat(commonChance - rareChance);
  commonChance = toClientFloat(commonChance - epicChance);
  commonChance = toClientFloat(commonChance - uniqueChance);
  commonChance = toClientFloat(commonChance - legendaryChance);
  commonChance = toClientFloat(commonChance - transcendentChance);
  commonChance = toClientFloat(commonChance - mythicChance);
  commonChance = toClientFloat(Math.max(0, commonChance));

  const roll = toClientFloat(rng());
  if (roll < noneChance) {
    return "None";
  }
  const commonThreshold = toClientFloat(noneChance + commonChance);
  if (roll < commonThreshold) {
    return "Common";
  }
  const rareThreshold = toClientFloat(commonThreshold + rareChance);
  if (roll < rareThreshold) {
    return "Rare";
  }
  const epicThreshold = toClientFloat(rareThreshold + epicChance);
  if (roll < epicThreshold) {
    return "Epic";
  }
  const uniqueThreshold = toClientFloat(epicThreshold + uniqueChance);
  if (roll < uniqueThreshold) {
    return "Unique";
  }
  const legendaryThreshold = toClientFloat(uniqueThreshold + legendaryChance);
  if (roll < legendaryThreshold) {
    return "Legendary";
  }
  const transcendentThreshold = toClientFloat(
    legendaryThreshold + transcendentChance,
  );
  if (roll < transcendentThreshold) {
    return "Transcendent";
  }
  return "Mythic";
}

export function computeItemBonus(
  slot: ItemSlot,
  level: number,
  rarity: ItemRarity,
  variance: number,
): EquipmentBonus {
  const safeLevel = Math.max(level, 1);
  const safeVariance = toClientFloat(Math.max(variance, 0));
  const baseBonus = toClientFloat(
    toClientFloat(toClientFloat(safeLevel) * safeVariance) *
      getRarityStatMultiplier(rarity),
  );

  if (slot === 1) {
    return {
      bonusAtk: baseBonus,
      bonusDef: 0,
      bonusHp: 0,
      critRate: 0,
      atkSpeed: 0,
      magicAtk: 0,
    };
  }

  if (slot === 8) {
    return {
      bonusAtk: toClientFloat(baseBonus * toClientFloat(0.5)),
      bonusDef: toClientFloat(baseBonus * toClientFloat(0.3)),
      bonusHp: toClientFloat(baseBonus * toClientFloat(2)),
      critRate: 0,
      atkSpeed: 0,
      magicAtk: 0,
    };
  }

  if (ARMOR_SLOTS.has(slot)) {
    return {
      bonusAtk: 0,
      bonusDef: toClientFloat(baseBonus * toClientFloat(0.7)),
      bonusHp: toClientFloat(baseBonus * toClientFloat(3)),
      critRate: 0,
      atkSpeed: 0,
      magicAtk: 0,
    };
  }

  return {
    bonusAtk: 0,
    bonusDef: 0,
    bonusHp: 0,
    critRate: 0,
    atkSpeed: 0,
    magicAtk: 0,
  };
}

export function rollBaseItem(
  slot: ItemSlot,
  rng: DropRng = Math.random,
): BaseItemDefinition {
  const candidates = BASE_ITEMS_BY_SLOT[slot] ?? BASE_ITEMS_BY_SLOT[1];
  if (!candidates) {
    throw new Error("missing weapon base item catalog");
  }
  const index = Math.min(
    Math.max(Math.floor(rng() * candidates.length), 0),
    candidates.length - 1,
  );
  const selected = candidates[index];
  const statScales = BASE_ITEM_STAT_SCALES[selected.baseItemId] ?? {
    atkScale: 1,
    defScale: 1,
    hpScale: 1,
  };

  return {
    ...selected,
    ...statScales,
  };
}

export function getAffixCount(
  rarity: ItemRarity,
  rng: DropRng = Math.random,
): number {
  switch (rarity) {
    case "Rare":
      return 1;
    case "Epic":
    case "Unique":
      return 2;
    case "Legendary":
    case "Transcendent":
      return rng() < 0.5 ? 2 : 3;
    case "Mythic":
      return 3;
    case "None":
    case "Common":
      return 0;
  }
}

function randRangeInt(min: number, max: number, rng: DropRng): number {
  const roll = Math.floor(rng() * (max - min + 1)) + min;

  return Math.min(Math.max(roll, min), max);
}

function randRangeFloat(min: number, max: number, rng: DropRng): number {
  return toClientFloat(
    toClientFloat(min) + toClientFloat(toClientFloat(max - min) * rng()),
  );
}

function shuffleAffixKinds(rng: DropRng): AffixKind[] {
  const kinds = [...AFFIX_KINDS];
  for (let index = kinds.length - 1; index > 0; index -= 1) {
    const swapIndex = randRangeInt(0, index, rng);
    [kinds[index], kinds[swapIndex]] = [kinds[swapIndex], kinds[index]];
  }

  return kinds;
}

export function rollAffixes(
  rarity: ItemRarity,
  level: number,
  rng: DropRng = Math.random,
): AffixBonus {
  const affixes: AffixBonus = {
    bonusCritRate: 0,
    bonusAtkSpeed: 0,
    bonusMagicAtk: 0,
    bonusPhysDef: 0,
    bonusMagicDef: 0,
    bonusAffixHp: 0,
    bonusCritDmg: 0,
  };
  const affixCount = getAffixCount(rarity, rng);
  if (affixCount <= 0) {
    return affixes;
  }

  const safeLevel = Math.max(level, 1);
  const affixKinds = shuffleAffixKinds(rng);
  for (const affixKind of affixKinds.slice(0, affixCount)) {
    switch (affixKind) {
      case "CritRate":
        affixes.bonusCritRate =
          Math.round(randRangeFloat(0.01, 0.05, rng) * 1000) / 1000;
        break;
      case "AtkSpeed":
        affixes.bonusAtkSpeed =
          Math.round(randRangeFloat(0.05, 0.15, rng) * 1000) / 1000;
        break;
      case "MagicAtk":
        affixes.bonusMagicAtk = Math.round(
          toClientFloat(safeLevel) * randRangeFloat(0.5, 1.5, rng),
        );
        break;
      case "PhysDef":
        affixes.bonusPhysDef = Math.max(
          1,
          Math.round(toClientFloat(safeLevel) * randRangeFloat(0.3, 1.0, rng)),
        );
        break;
      case "MagicDef":
        affixes.bonusMagicDef = Math.max(
          1,
          Math.round(toClientFloat(safeLevel) * randRangeFloat(0.3, 1.0, rng)),
        );
        break;
      case "Hp":
        affixes.bonusAffixHp = Math.max(
          1,
          Math.round(toClientFloat(safeLevel) * randRangeFloat(2.0, 5.0, rng)),
        );
        break;
      case "CritDmg":
        affixes.bonusCritDmg =
          Math.round(randRangeFloat(0.05, 0.2, rng) * 1000) / 1000;
        break;
    }
  }

  return affixes;
}
