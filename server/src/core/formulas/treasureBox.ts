// 보물 상자(Treasure Box / 일일 뽑기) — 서버 parity 미러.
// 하루 1회 무료 보물 상자에서 가중 랜덤 보상을 뽑는 RNG 리텐션 루프.
// RNG(가중 추첨 roll·수량 결정)는 클라 권위(FRandomStream, #71 패턴).
// 서버는 가중치/보상표(TREASURE_POOL)와 결정적 매핑 함수(pickTreasureReward)만
// 1:1 미러링하여 drift 를 방지한다(라우트/DB 없음).
// 클라 TreasureBoxService 가 이 풀과 누적 가중 구간 매핑을 그대로 이식한다.

// 보상 종류. 클라 ETreasureReward 와 1:1(모두 실존 재화, 신규 재화 없음).
export type TreasureReward =
  | "gold"
  | "essence"
  | "consumable"
  | "protectionScroll"
  | "resetCube"
  | "rankCube";

// 보상 풀 엔트리. reward(보상 종류)/weight(가중치)/minAmount~maxAmount(수량 범위).
// 수량은 클라 RNG(RandRange(min,max))가 권위 — 서버는 표만 미러링.
export type TreasurePoolEntry = {
  reward: TreasureReward;
  weight: number;
  minAmount: number;
  maxAmount: number;
};

// 보상 풀(스펙 §3). weight 합 = 100. 흔함=소액/희귀=고액 분포.
// 순서가 곧 누적 가중 구간 순서이므로 클라와 동일 순서를 엄수한다.
// 누적 구간(roll 경계, totalWeight=100 기준):
//   gold            weight 40 → roll  0~39
//   essence         weight 25 → roll 40~64
//   consumable      weight 15 → roll 65~79
//   protectionScroll weight 10 → roll 80~89
//   resetCube       weight  7 → roll 90~96
//   rankCube        weight  3 → roll 97~99
export const TREASURE_POOL: readonly TreasurePoolEntry[] = [
  { reward: "gold", weight: 40, minAmount: 10000, maxAmount: 50000 },
  { reward: "essence", weight: 25, minAmount: 3, maxAmount: 10 },
  { reward: "consumable", weight: 15, minAmount: 1, maxAmount: 2 },
  { reward: "protectionScroll", weight: 10, minAmount: 1, maxAmount: 3 },
  { reward: "resetCube", weight: 7, minAmount: 1, maxAmount: 2 },
  { reward: "rankCube", weight: 3, minAmount: 1, maxAmount: 1 },
] as const;

// 풀 전체 가중치 합(= totalWeight). roll 범위는 0 ~ totalWeight-1.
export function getTotalTreasureWeight(): number {
  let total = 0;
  for (const entry of TREASURE_POOL) {
    total += entry.weight;
  }
  return total;
}

// roll(0 ~ totalWeight-1)을 누적 가중 구간에 매핑하여 보상 종류 반환(결정적).
// 음수 roll 은 0 으로, totalWeight-1 초과는 마지막 구간으로 클램프.
// 클라 pickReward(roll) 와 1:1: 누적 합(cumulative)에 roll 이 처음 미달하는 엔트리.
export function pickTreasureReward(roll: number): TreasureReward {
  const total = getTotalTreasureWeight();
  // 정수화 후 [0, total-1] 로 클램프(음수/초과 안전).
  const clamped = Math.min(Math.max(0, Math.trunc(roll)), total - 1);

  let cumulative = 0;
  for (const entry of TREASURE_POOL) {
    cumulative += entry.weight;
    if (clamped < cumulative) {
      return entry.reward;
    }
  }

  // 도달 불가(클램프로 인해) — 안전상 마지막 보상 반환.
  return TREASURE_POOL[TREASURE_POOL.length - 1].reward;
}
