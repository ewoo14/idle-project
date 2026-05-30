// 아이템 매각가/파워(Item Sell / Power) — 서버 parity 미러.
// P3 자동 장비(auto-gear) 슬라이스용. 인벤토리 자동 매각/등급 비교에 쓰이는
// 매각가·파워 산식을 클라(UE5)와 1:1 미러링한다(라우트/DB 없음, Date/random 미사용).
// trunc/round 가드로 클라 정수 경로와 정합. enum 이름 문자열은 클라 EItemRarity 와 동일.

// 아이템 레어도 7단계(+None). 클라 EItemRarity 와 1:1 문자열.
export type ItemRarity =
  | "None"
  | "Common"
  | "Rare"
  | "Epic"
  | "Unique"
  | "Legendary"
  | "Transcendent"
  | "Mythic";

// 레어도별 매각 기준가(enhance 0 기준). 클라 parity, None 은 매각 불가(0).
// 상위 등급으로 갈수록 기하급수적으로 상승(무한 성장 골드 sink/source).
export const RARITY_SELL_BASE: Record<ItemRarity, number> = {
  None: 0,
  Common: 100,
  Rare: 400,
  Epic: 1500,
  Unique: 6000,
  Legendary: 25000,
  Transcendent: 100000,
  Mythic: 400000,
};

// 아이템 매각가. 강화 레벨당 기준가의 +20% 가산(선형).
// 음수 강화는 0 가드, 알 수 없는/None 레어도는 기준가 0. round 로 정수 정합.
export function computeItemSellValue(
  rarity: ItemRarity,
  enhanceLevel: number,
): number {
  const base = RARITY_SELL_BASE[rarity] ?? 0;
  const level = Math.max(0, Math.trunc(enhanceLevel));
  return Math.round(base * (1 + 0.2 * level));
}

// 아이템 파워 산정 입력. 모든 보너스 필드는 클라 장비 보너스(EquipmentBonus) parity.
export type ItemPowerInput = {
  bonusAtk: number;
  bonusMagicAtk: number;
  bonusDef: number;
  bonusPhysDef: number;
  bonusMagicDef: number;
  bonusHp: number;
  bonusAffixHp: number;
  bonusCritRate: number;
  bonusCritDmg: number;
  enhanceLevel: number;
};

// 아이템 파워(자동 장비 비교용 단일 스칼라). 클라 parity.
// raw = 공/마공/방/물방/마방 합 + (체력+접사체력)/10 + 크확*1000 + 크댐*100.
// 강화 레벨당 +10% 가산(선형, 음수 0 가드). 0 가드 후 round 로 정수 정합.
export function computeItemPower(item: ItemPowerInput): number {
  const raw =
    item.bonusAtk +
    item.bonusMagicAtk +
    item.bonusDef +
    item.bonusPhysDef +
    item.bonusMagicDef +
    (item.bonusHp + item.bonusAffixHp) / 10 +
    item.bonusCritRate * 1000 +
    item.bonusCritDmg * 100;
  const level = Math.max(0, Math.trunc(item.enhanceLevel));
  return Math.round(Math.max(0, raw) * (1 + 0.1 * level));
}
