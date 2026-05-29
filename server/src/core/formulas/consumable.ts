export const CONSUMABLE_TYPE = {
  AttackTonic: 0,
  GuardTonic: 1,
  AllStatElixir: 2,
  FortuneScroll: 3,
  GoldFeast: 4,
  WisdomBooster: 5,
} as const;

export type ConsumableType =
  (typeof CONSUMABLE_TYPE)[keyof typeof CONSUMABLE_TYPE];

/** 소비 아이템 등급(소/중/대). Standard 가 #73 기본 효과입니다. */
export const CONSUMABLE_GRADE = {
  Lesser: 0,
  Standard: 1,
  Greater: 2,
} as const;

export type ConsumableGrade =
  (typeof CONSUMABLE_GRADE)[keyof typeof CONSUMABLE_GRADE];

const BUFF_DURATION_SEC = 1800;

/** 등급별 효과 % 배수: Lesser=0.5x, Standard=1.0x, Greater=2.0x. */
export function getConsumableGradeMultiplier(grade: number): number {
  switch (grade) {
    case CONSUMABLE_GRADE.Lesser:
      return Math.fround(0.5);
    case CONSUMABLE_GRADE.Standard:
      return Math.fround(1);
    case CONSUMABLE_GRADE.Greater:
      return Math.fround(2);
    default:
      return Math.fround(1);
  }
}

/** Standard 기준 #73 기본 효과 %를 반환합니다. */
function getConsumableBaseBuffPercent(type: number): number {
  switch (type) {
    case CONSUMABLE_TYPE.AttackTonic:
    case CONSUMABLE_TYPE.GuardTonic:
    case CONSUMABLE_TYPE.FortuneScroll:
      return Math.fround(0.3);
    case CONSUMABLE_TYPE.AllStatElixir:
      return Math.fround(0.2);
    case CONSUMABLE_TYPE.GoldFeast:
    case CONSUMABLE_TYPE.WisdomBooster:
      return Math.fround(0.5);
    default:
      return 0;
  }
}

/**
 * 등급별 효과 % (Standard=기본, Lesser=×0.5, Greater=×2.0).
 * grade 미지정 시 Standard 로 동작해 #73 호환을 유지합니다.
 */
export function getConsumableBuffPercent(
  type: number,
  grade: number = CONSUMABLE_GRADE.Standard,
): number {
  if (!isConsumableGrade(grade)) {
    return 0;
  }
  return Math.fround(
    getConsumableBaseBuffPercent(type) * getConsumableGradeMultiplier(grade),
  );
}

/** 지속시간은 등급과 무관하게 타입별 고정값입니다. */
export function getConsumableBuffDurationSec(
  type: number,
  grade: number = CONSUMABLE_GRADE.Standard,
): number {
  return isConsumableType(type) && isConsumableGrade(grade)
    ? BUFF_DURATION_SEC
    : 0;
}

export function isConsumableType(type: number): type is ConsumableType {
  return (
    type === CONSUMABLE_TYPE.AttackTonic ||
    type === CONSUMABLE_TYPE.GuardTonic ||
    type === CONSUMABLE_TYPE.AllStatElixir ||
    type === CONSUMABLE_TYPE.FortuneScroll ||
    type === CONSUMABLE_TYPE.GoldFeast ||
    type === CONSUMABLE_TYPE.WisdomBooster
  );
}

export function isConsumableGrade(grade: number): grade is ConsumableGrade {
  return (
    grade === CONSUMABLE_GRADE.Lesser ||
    grade === CONSUMABLE_GRADE.Standard ||
    grade === CONSUMABLE_GRADE.Greater
  );
}
