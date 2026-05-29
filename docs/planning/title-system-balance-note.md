# 칭호(Title) 시스템 밸런스 노트

> 대상: 칭호 시스템 슬라이스 · 작성 2026-05-30 · 스펙 [`2026-05-30-title-system-design.md`](../superpowers/specs/2026-05-30-title-system-design.md) · SaveVer 20→21

## 1. 칭호 카탈로그 (18종, 업적 메트릭 해금)

| id | 해금(메트릭≥) | 보너스 |
| --- | --- | --- |
| monster_hunter | MonstersKilled 10,000 | 전체 스탯 +3% |
| monster_annihilator | MonstersKilled 1,000,000 | 전체 스탯 +10% |
| boss_slayer | BossesKilled 500 | 크리뎀 +10% |
| boss_executioner | BossesKilled 5,000 | 크리뎀 +18% |
| rebirth_master | RebirthCount 10 | 전체 스탯 +5% |
| transcendent | TranscendCount 5 | 전체 스탯 +8% |
| stage_conqueror | StagesCleared 1,000 | 경험치 +12% |
| level_legend | HighestLevelReached 500 | 전체 스탯 +6% |
| tower_conqueror | TowerHighestFloor 100 | 골드 +15% |
| tower_overlord | TowerHighestFloor 300 | 크리뎀 +15% |
| collector | ItemsCollected 5,000 | 골드 +10% |
| unique_seeker | UniqueItemsFound 100 | 전체 스탯 +7% |
| gold_king | GoldEarned 1e9 | 골드 +20% |
| pet_whisperer | HighestPetLevel 50 | 경험치 +8% |
| quest_champion | QuestsCompleted 200 | 경험치 +10% |
| enhance_artisan | HighestEnhanceLevel 20 | 전체 스탯 +5% |
| enhance_grandmaster | HighestEnhanceLevel 35 | 크리뎀 +12% |
| veteran | DaysPlayed 100 | 골드 +10% |

카테고리: 전투4/진행4/탑2/수집2/경제1/펫1/퀘스트1/강화2/메타1. 보너스 차원 AllStat7/CritDmg4/Gold4/Exp3.

## 2. 보너스 (장착 1개, 비율)

- `ETitleBonus`: AllStatPct(RefreshDerivedStats 스탯 배수) / GoldPct(AddGold) / ExpPct(AddExp) / CritDmgPct(크리뎀 가산). **각 타입 단일 적용 지점**(이중 적용 가드, #72), **장착 1개만**.
- bonusValue는 비율(0.03~0.20 = +3~20%). 기존 마스터리/펫/길드 버프와 합산.
- 해금은 영구(업적 메트릭 `GetMetricValue >= threshold`), 장착은 자유 교체.

## 3. 경제/페이싱

- 보너스 보수적(+3~20%) + 장착 1개 제한 → 파워크리프 억제. 강한 칭호(전체 스탯 +10% 등)는 후반 메트릭(MonstersKilled 1e6 등) 게이트.
- 신규 재화 없음(성취 기반 무료 보너스 — 수집/장기 플레이 동기). median 영향: 선택적 보너스, 통합 검증 재측정.
- 수치 초기값(임계·보너스) — 메트릭 누적 속도 데이터로 재튜닝.

## 4. 세이브

- **20 → 21**: `UnlockedTitleIds`(Set) + `EquippedTitleId`(FString). 누락=빈/없음(회귀안전), SaveVer<21 가드.
