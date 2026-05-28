export const RUNE_CODEX_TOTAL_CELLS = 54;
export const RUNE_CODEX_CORE_CELLS = 30;
export const RUNE_CODEX_UTIL_CELLS = 24;
export const PER_CELL_CORE_BONUS = 0.004;
export const CORE_CATEGORY_BONUS = 0.05;
export const UTIL_CATEGORY_CAP_EXTENSION = 0.1;

export interface RuneCodexCompletion {
  unlockedCells: number;
  rowComplete: boolean[];
  coreCategoryComplete: boolean;
  utilCategoryComplete: boolean;
}

export interface RuneCodexBonus {
  coreStatAdd: number;
  utilCapExtension: number;
}

export function getRowCompletionBonus(rarity: number): number {
  switch (rarity) {
    case 1:
      return Math.fround(0.01);
    case 2:
      return Math.fround(0.02);
    case 3:
      return Math.fround(0.03);
    case 4:
      return Math.fround(0.05);
    case 5:
      return Math.fround(0.08);
    case 6:
      return Math.fround(0.12);
    default:
      return 0;
  }
}

export function computeRuneCodexBonus(
  completion: RuneCodexCompletion,
): RuneCodexBonus {
  const unlockedCells = Math.min(
    RUNE_CODEX_TOTAL_CELLS,
    Math.max(0, Math.trunc(completion.unlockedCells)),
  );
  let coreStatAdd = unlockedCells * PER_CELL_CORE_BONUS;

  for (
    let rowIndex = 0;
    rowIndex < completion.rowComplete.length && rowIndex < 6;
    rowIndex++
  ) {
    if (completion.rowComplete[rowIndex]) {
      coreStatAdd += getRowCompletionBonus(rowIndex + 1);
    }
  }

  if (completion.coreCategoryComplete) {
    coreStatAdd += CORE_CATEGORY_BONUS;
  }

  return {
    coreStatAdd: Math.fround(coreStatAdd),
    utilCapExtension: completion.utilCategoryComplete
      ? Math.fround(UTIL_CATEGORY_CAP_EXTENSION)
      : 0,
  };
}
