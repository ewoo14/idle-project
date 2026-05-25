export function expToNext(level: number) {
  return Math.floor(100 * level * 1.15 ** Math.max(level - 1, 0));
}

export function baseWarriorStats() {
  return { str: 12, dex: 6, int: 3, luk: 4, hp: 120, mp: 30 };
}
