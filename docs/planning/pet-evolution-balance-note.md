# 펫 진화(별/Star) 밸런스 노트 (펫 시스템 심화)

> 대상: 펫 진화 슬라이스 · 작성 2026-05-30 · 스펙 [`2026-05-30-pet-evolution-design.md`](../superpowers/specs/2026-05-30-pet-evolution-design.md) · SaveVer 19→20

## 1. 진화 비용 (골드, 무한 sink)

`GetPetEvolveCost(star) = floor(PET_EVOLVE_BASE * PET_EVOLVE_GROWTH^star)` — BASE 50,000 / GROWTH 1.8.

| star→star+1 | 0→1 | 1→2 | 2→3 | 3→4 | 4→5 | 5→6 |
| --- | --- | --- | --- | --- | --- | --- |
| 골드 | 50,000 | 90,000 | 162,000 | 291,600 | 524,880 | 944,784 |

무한 기하 증가 → 골드 sink([[project_infinite_growth]] "낮은 캡 지양", 별 상한 없음).

## 2. 별 배수 (장착 펫 보너스)

`GetPetStarMultiplier(star) = 1 + PET_STAR_STEP * star` — STEP 0.15(선형 무한).

| star | 0 | 1 | 2 | 3 | 4 | 5 | 10 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 배수 | 1.00 | 1.15 | 1.30 | 1.45 | 1.60 | 1.75 | 2.50 |

- **레벨과 직교**: 펫 보너스 = (정의 BonusPercent × 레벨 배수) × 별 배수. 레벨(FeedPet, 골드 점진)·별(진화, 골드 기하)은 다른 곱 — 이중 계산 없음(#72 교훈, GetEquippedPetStatBonus 최종 1곳·장착 펫만).
- 확정 성공(RNG 없음) → parity 단순, 무한성은 비용 곡선이 담당.

## 3. 경제/페이싱

- 골드 단일 sink(신규 재화 없음). 펫 보너스는 스탯 % (장착 1마리)라 별 배수가 직접 CP에 반영되되 선형이라 완만. 기하 골드 비용이 페이싱 — 초반 별은 저렴, 후반 급증.
- median 5.328h 영향: 선택적 가속(필수 아님). 통합 검증 재측정.
- 수치 초기값(BASE/GROWTH/STEP) — 골드 경제·펫 보너스 분포 데이터로 재튜닝.

## 4. 세이브

- **19 → 20**: `PetStars: TMap<FString,int32>`(펫별 별 등급). 누락=0성(회귀안전), SaveVer<20 가드.
