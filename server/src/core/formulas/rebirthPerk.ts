// 환생 특성(Rebirth Perks) — 서버 parity 미러.
// 환생 특성 포인트 적립/분배는 클라 로컬 세이브 권위(RebirthPerkService).
// 서버는 포인트 출처(환생 횟수)와 특성별 보너스 비율(PerkStep)을 1:1 미러링하여
// drift 를 방지한다(라우트/DB 없음).
// perk 키는 클라 ERebirthPerk(None 제외) enum 이름 문자열과 동일.

// 환생 특성 6종. 클라 ERebirthPerk(None 제외)와 1:1.
// GoldPct: 골드 획득 / DropPct: 드롭률 / CritDmgPct: 크리티컬 데미지 /
// AllStatPct: 전체 스탯 / ExpPct: 경험치 / OfflineEffPct: 오프라인 효율.
export type RebirthPerk =
  | "GoldPct"
  | "DropPct"
  | "CritDmgPct"
  | "AllStatPct"
  | "ExpPct"
  | "OfflineEffPct";

// 특성 레벨당 % 보너스 비율(클라 PerkStep parity). 선형 무한 성장.
// 비율 값(0.02 = +2%/레벨). AllStat 은 전 스탯에 곱해지므로 가장 보수적.
export const PERK_STEP: Record<RebirthPerk, number> = {
  GoldPct: 0.02,
  DropPct: 0.02,
  CritDmgPct: 0.03,
  AllStatPct: 0.01,
  ExpPct: 0.02,
  OfflineEffPct: 0.03,
};

// 환생 횟수로 적립되는 총 환생 특성 포인트. 1/환생(무한 누적).
// 음수/소수 환생 횟수는 절삭·0 가드(회귀안전).
export function getTotalRebirthPerkPoints(rebirthCount: number): number {
  return Math.max(0, Math.trunc(rebirthCount));
}

// 특성 perk 를 level 만큼 분배했을 때의 보너스 비율(클라 parity).
// 선형 무한(level0=0), 음수 level 은 0 가드. fround 로 클라 float 경로와 정합.
export function getPerkBonus(perk: RebirthPerk, level: number): number {
  return Math.fround(PERK_STEP[perk] * Math.max(0, level));
}
