# 보물 상자 (일일 뽑기) 구현 계획

> 스펙: [`2026-05-30-treasure-box-design.md`](../specs/2026-05-30-treasure-box-design.md). v3 디스패치, 현행 재검증. SaveVer 24→25.

**Goal:** 하루 1회 무료 보물 상자 → 가중 랜덤 보상(실존 재화). RNG 리텐션 루프.

**Architecture:** 서버 `treasureBox.ts` 가중치 parity + 클라 TreasureBoxService(추첨/지급) + GameInstance + 보물 상자 패널 UI. SaveVer 24→25. RNG 클라 권위(#71 패턴).

## Task 1: backend (backend)
- [ ] `server/src/core/formulas/treasureBox.ts` 신규: `TreasureReward` 타입(gold/essence/consumable/protectionScroll/resetCube/rankCube), `TREASURE_POOL: readonly {reward, weight, minAmount, maxAmount}[]`(스펙 §3), `getTotalTreasureWeight()`, `pickTreasureReward(roll: number)`(0~totalWeight-1 → 누적 가중 인덱스 → reward, **결정적**). 기존 formulas 패턴.
- [ ] `treasureBox.test.ts`: 가중 누적 경계(roll 0/경계값→정확한 reward), getTotalWeight 합, 풀 무결성(weight>0, min≤max), 전 roll 커버. `cd server; npm run lint && npx vitest run src/core/formulas && npm run build` GREEN.
- [ ] 커밋 `feat: 보물 상자 가중 보상 backend parity (보물 상자)`.

## Task 2: client (character)
- [ ] `GameCore/TreasureBoxTypes.h`: `ETreasureReward{Gold,Essence,Consumable,ProtectionScroll,ResetCube,RankCube}`, `FTreasurePoolEntry`(Reward/Weight/MinAmount/MaxAmount), `FTreasureReward`(Reward/Amount).
- [ ] `GameCore/TreasureBoxService.{h,cpp}`(UObject): `FString LastDrawDate`, `int64 TotalDraws`, 보상 풀 미러(서버 TREASURE_POOL 1:1), `bool CanDrawToday(const FString& Date)`(Date != LastDrawDate), `FTreasureReward DrawTreasure(const FString& Date, FRandomStream& Rng)`(가중 추첨: `pickTreasureReward(Rng.RandRange(0, totalWeight-1))` → reward, 수량 `Rng.RandRange(min,max)`; LastDrawDate=Date·TotalDraws++; CanDraw 아니면 빈 보상), `GetTotalWeight()`/`PickReward(roll)`(서버 parity 미러), `RestoreState(date, total)`.
- [ ] GameInstance: TreasureBoxService 보유·초기화, `DrawTreasureBox()` 진입점(CanDrawToday 검증 → DrawTreasure(RNG) → **보상 지급 단일**: Gold→AddGold/Essence→RuneEssence/Consumable→AddConsumable/ProtectionScroll→ProtectionScrolls+=/ResetCube→ResetCubes+=/RankCube→RankCubes+= → RequestAutosave → 결과 반환). getter lazy-ensure(#91). UTC date(GetCurrentUtcDateString).
- [ ] SaveVer **24→25**: LastTreasureDrawDate/TotalTreasureDraws 직렬화. Capture=25/Apply `>=25` 가드(RestoreState). CloudSavePayloadMapper 정합. **전 세이브 테스트 단언 24→25 일괄 갱신**(grep `(24)`/`V24`/`v24`/`SaveVersion, 24`/`SaveVersion = 24` in Tests/, 레거시 <25 유지).
- [ ] Automation(`TreasureBoxServiceTests` 신규): PickReward 가중 경계(roll 주입 결정적), CanDrawToday(1일1회·날짜 변경), DrawTreasure(보상 결정+수량 범위, TotalDraws++), 보상 지급(GameInstance 통해 각 재화 반영), 세이브 v25 라운드트립, parity(서버 풀 weight/min/max). 익명 헬퍼 Treasure~ prefix.
- [ ] 커밋 `feat: 보물 상자 TreasureBoxService + SaveVer25 (보물 상자)`.

## Task 3: UI (designer)
- [ ] 보물 상자 패널: 오늘 뽑기 상태(가능/완료)·뽑기 버튼(DrawTreasureBox)·최근 결과(보상 종류·수량)·누적 뽑기 횟수. ko/en + CsvIntegrity. 표준 jumbo.
- [ ] 커밋 `feat: 보물 상자 패널 UI (보물 상자)`.

## Task 4: balance/docs (balance)
- [ ] `docs/planning/treasure-box-balance-note.md`: 가중치/수량/기대값(일일 던전 수준)·경제 영향. SaveVer25.
- [ ] [[project_pr_order]]/[[project_session_progress]] 갱신. 커밋 `docs: 보물 상자 밸런스 노트`.

## Task 5: qa (qa)
- [ ] E2E: 뽑기(1일1회·중복 거부·날짜 변경)→가중 분포(roll 경계)→수량 범위→보상 지급(실존 재화)→세이브 v25→parity. 표준 jumbo + ue-automation.ps1 게이트. PR 스크린샷.
- [ ] 커밋 `test: 보물 상자 E2E`.

## Self-Review
- 스펙 §4 전부 매핑 ✓. parity: 서버 treasureBox.ts ↔ 클라 TreasureBoxService(풀/가중/pickReward) 1:1 — TM cross-check. RNG 클라 권위(#71).
- 보상 단일 지급(이중 지급 금지). 1일1회(UTC date, #91/#94 일관). 실존 재화만(신규 재화 없음).
- SaveVer 24→25 전 테스트 단언 갱신(stale 방지, #83). getter lazy-ensure(#91). jumbo ODR(Treasure~ prefix).

## 매핑: 1→backend, 2→character(메인), 3→designer, 4→balance, 5→qa.
