export const TRANSCEND_REBIRTH_THRESHOLD = 5;

export function getTranscendStatMultiplier(count: number): number {
  return Math.fround(
    Math.fround(1) + Math.fround(Math.max(0, count) * Math.fround(0.25)),
  );
}

export function canTranscend(rebirthCount: number): boolean {
  return rebirthCount >= TRANSCEND_REBIRTH_THRESHOLD;
}
