# 장비 잠재 V2 구현 계획

> 스펙: [`2026-05-30-potential-v2-design.md`](../specs/2026-05-30-potential-v2-design.md). v3 디스패치, 현행 재검증. **SaveVer 무변경**(enum 확장 전방호환).

**Goal:** 잠재(#71)에 Transcendent 등급(4줄) + 신규 옵션 3종(AllStat/GoldFind/DropRate%) 추가.

**Architecture:** 서버 `potential.ts` 값범위/확률 parity + 클라 ItemTypes enum 확장 + PotentialFormula 미러 + 잠재 집계 적용 + Rank 큐브 상승. SaveVer 무변경.

## Task 1: backend (backend)
- [ ] `server/src/core/formulas/potential.ts` 확장(기존 #71 함수 옆): Transcendent 등급(줄 4, 값범위 = Legendary×~1.3), 신규 옵션 AllStatPercent/GoldFindPercent/DropRatePercent 값범위, `getMaxPotentialGrade(itemRarity)`에 Transcendent 허용(고레어도), Rank 큐브 Legendary→Transcendent 확률(예 0.05). 기존 함수/상수 패턴·Math.fround.
- [ ] `potential.test.ts` 확장: Transcendent 줄수 4·값범위 단조, 신규 옵션 값범위, RankCube Transcendent 확률 범위, 아이템등급 상한. `cd server; npm run lint && npx vitest run src/core/formulas && npm run build` GREEN.
- [ ] 커밋 `feat: 잠재 V2 Transcendent/신규옵션 backend parity (잠재 V2)`.

## Task 2: client (character)
- [ ] `ItemSystem/ItemTypes.h`: `EPotentialGrade::Transcendent = 5`, `EPotentialStat += AllStatPercent=9, GoldFindPercent=10, DropRatePercent=11`. (FPotentialLine 구조 불변.)
- [ ] `PotentialFormula.{h,cpp}` 미러(서버 1:1): Transcendent 줄수/값범위, 신규 옵션 값범위, GetMaxPotentialGrade(rarity) Transcendent 허용, Rank 큐브 Transcendent 상승 확률·로직. 전투 8종 기존 유지.
- [ ] **잠재 적용 집계**: 장착 아이템 잠재 줄 합산에서 —
  - 전투 8종: 기존 ComputeEquipmentBonus/derived stat 경로 불변.
  - **AllStatPercent**: derived stat 전 스탯 배수(RefreshDerivedStats, 마스터리/펫/칭호 옆 단일 지점).
  - **GoldFindPercent**: AddGold 골드 배수(펫/칭호/길드 골드 옆 단일 지점).
  - **DropRatePercent**: 드롭 배수(**펫 Drop 보너스 #69 집계 지점에 합류** — 그 지점 grep해 잠재 Drop% 합산).
  - 각 신규 옵션 합산 getter(GetEquipmentPotentialAllStatPct 등)는 단일 지점에서만 소비(이중 적용 금지 #72).
- [ ] Rank 큐브 사용(기존 진입점)에서 Legendary→Transcendent 상승 확률 반영(RNG 클라).
- [ ] **SaveVer 무변경**(22 유지): enum 확장은 전방호환. **단 기존 세이브 라운드트립 회귀 테스트**(v22 캡처/복원에 Transcendent/신규옵션 줄 포함해 보존 확인).
- [ ] Automation(`PotentialFormulaTests`/`InventoryComponent` 등 확장): Transcendent 롤 줄4·값범위, 신규 옵션 3종 값범위, GetMaxPotentialGrade rarity별, Rank Transcendent 상승(확률 경계 1/0 결정적), 신규 옵션 적용(AllStat/Gold/Drop 합산 반영), 세이브 v22 라운드트립(Transcendent 줄 보존), parity(서버 값범위/줄수). 익명 헬퍼 Potential~ prefix.
- [ ] 커밋 `feat: 잠재 V2 Transcendent/신규옵션 클라 (잠재 V2)`.

## Task 3: UI (designer)
- [ ] 잠재 UI: Transcendent 등급 색/표시명(초월) + 신규 옵션 3종 표시명(전체 스탯%/골드 획득%/드롭률%) + Rank 큐브 Transcendent 도달 표시. ko/en + CsvIntegrity. 표준 jumbo.
- [ ] 커밋 `feat: 잠재 V2 UI (잠재 V2)`.

## Task 4: balance/docs (balance)
- [ ] `docs/planning/potential-v2-balance-note.md`: Transcendent 값범위(Legendary×1.3)·신규 옵션 곡선·Rank Transcendent 확률·아이템등급 상한·파워크리프/median. 세이브 무변경.
- [ ] [[project_pr_order]]/[[project_session_progress]] 갱신. 커밋 `docs: 잠재 V2 밸런스 노트 (잠재 V2)`.

## Task 5: qa (qa)
- [ ] E2E: Transcendent 롤(줄4)→신규 옵션 적용(AllStat/Gold/Drop CP·재화 반영)→Rank 상승→아이템등급 상한→기존 세이브 회귀(Transcendent 줄 보존)→parity. 표준 jumbo + ue-automation.ps1 게이트. PR 스크린샷.
- [ ] 커밋 `test: 잠재 V2 E2E (잠재 V2)`.

## Self-Review
- 스펙 §4 전부 매핑 ✓. parity: 서버 potential.ts ↔ 클라 PotentialFormula 값범위/줄수/확률 1:1 — TM cross-check.
- **이중 적용 가드(#72)**: 신규 옵션 AllStat/Gold/Drop 각 단일 집계 지점(펫/칭호/마스터리 옆), 전투 기존 경로 불변. Drop은 펫 #69 집계점 재사용.
- **SaveVer 무변경**(enum 전방호환): 기존 줄 값 불변, 신규 값은 신규 롤만. 기존 세이브 라운드트립 회귀 테스트로 호환 확인(#83 stale 교훈 — bump 없지만 회귀 검증).
- jumbo ODR(Potential~ prefix), RNG 클라 권위(#71).

## 매핑: 1→backend, 2→character(메인), 3→designer, 4→balance, 5→qa.
