# 룬 리롤 + 전송 구현 계획

> 스펙: [`2026-05-30-rune-reroll-transfer-design.md`](../specs/2026-05-30-rune-reroll-transfer-design.md). v3 디스패치, 현행 재검증. **SaveVer 변경 없음**(기존 FRuneSaveEntry 필드만 변경).

**Goal:** 보유 룬 세트 리롤 + 등급 상승 시도(확률) + 강화 레벨 전송. 룬 정수/골드 소비형 메타 심화.

**Architecture:** 서버 `rune.ts` 비용/확률 함수(parity 미러) + 클라 RuneFormula 미러 + RuneService 메서드(RNG 클라 권위) + 룬 패널 UI. #61/#63 parity 패턴 재사용.

## Task 1: backend (backend)
- [ ] `server/src/core/formulas/rune.ts` 확장(기존 getEnhance*/getDisenchant* 옆): `getRerollSetEssenceCost(rarity)`, `getRarityUpgradeEssenceCost(rarity)`, `getRarityUpgradeGoldCost(rarity)`, `getRarityUpgradeChance(rarity)`(등급↑ 확률↓, clamp), `getTransferEssenceCost(sourceLevel)`. Mythic 등급 상승 불가(상한 가드). `Math.fround` 일관(기존 패턴).
- [ ] `rune.test.ts` 확장: 각 비용 단조성, 등급 상승 확률 범위·Mythic 0/불가, 전송 비용 레벨 비례. `npm run lint && npx vitest run src/core/formulas && npm run build` GREEN.
- [ ] 커밋 `feat: 룬 리롤/전송 비용·확률 backend parity (룬 확장4)`.

## Task 2: client (character)
- [ ] `RuneFormula.{h,cpp}` 미러: `GetRerollSetEssenceCost(EItemRarity)`, `GetRarityUpgradeEssenceCost`, `GetRarityUpgradeGoldCost`, `GetRarityUpgradeChance(EItemRarity)→float`, `GetTransferEssenceCost(int32 SourceLevel)`. 서버 값 1:1. Mythic 상한.
- [ ] `RuneService`:
  - `RerollRuneSet(int32 OwnedIndex)`: 유효 인덱스·정수 충분 검증, 룬 정수 차감(GameInstance 경유 신호 or 반환 비용), `OwnedRunes[idx].RuneSet = 랜덤(Offense~Fortune)`(RNG 클라).
  - `bool TryUpgradeRuneRarity(int32 OwnedIndex)`: Mythic 미만·정수+골드 충분, RNG(GetRarityUpgradeChance) 성공 시 `Rarity = 다음 단계`, 실패 시 불변(자원만 소모). 결과 반환.
  - `TransferEnhancement(int32 SrcIndex, int32 DstIndex)`: 서로 다른 유효 인덱스·정수 충분, `Dst.EnhanceLevel = FMath::Max(Dst.EnhanceLevel, Src.EnhanceLevel)`, source 삭제(인덱스 재정렬), 장착 슬롯 인덱스 참조 갱신.
  - 자원 차감/검증은 GameInstance(룬 정수 GetRuneEssence/골드 GetGold)와 협조 — 기존 EnhanceRune 자원 흐름 패턴 따름.
- [ ] GameInstance: 리롤/등급상승/전송 진입점(자원 검증·차감·RefreshDerivedStats·RequestAutosave). 기존 EnhanceRune/Disenchant 패턴.
- [ ] Automation(`RuneServiceTests.cpp` 또는 신규): 세트 리롤(정수 차감·세트 변경), 등급 상승(성공 시 +1·실패 시 불변·Mythic 거부·자원부족 거부), 전송(레벨 max 이전·source 삭제·인덱스 정합·자원), 비용 parity(서버 값). **RNG 결정성**: 테스트는 시드 고정 또는 확률 경계(chance=1/0) 케이스로 결정적 검증.
- [ ] 익명 헬퍼 Rune~ prefix(jumbo ODR). 커밋 `feat: 룬 리롤/전송 RuneService + Formula (룬 확장4)`.

## Task 3: UI (designer)
- [ ] 룬 패널 선택 룬 액션: **세트 리롤**(비용·현재 세트 표시), **등급 상승 시도**(비용·성공률 % 표시·결과 피드백), **전송**(source/target 선택·비용). 자원 부족/Mythic 시 비활성. ko/en + CsvIntegrity. 표준 jumbo.
- [ ] 커밋 `feat: 룬 리롤/전송 UI (룬 확장4)`.

## Task 4: balance/docs (balance)
- [ ] `docs/planning/rune-reroll-transfer-balance-note.md`: 세트 리롤 비용, 등급 상승 비용/확률 곡선(무한 sink), 전송 비용. 파워크리프/median 영향. 세이브 무변경 명시.
- [ ] [[project_pr_order]]/[[project_session_progress]] 갱신. 커밋 `docs: 룬 리롤/전송 밸런스 노트 (룬 확장4)`.

## Task 5: qa (qa)
- [ ] E2E: 세트 리롤(소비·랜덤 결과)→등급 상승(성공/실패/Mythic 상한/자원부족 거부)→전송(레벨 이전·source 삭제·장착 참조 정합)→parity. 표준 jumbo + ue-automation.ps1 게이트. PR 스크린샷.
- [ ] 커밋 `test: 룬 리롤/전송 E2E (룬 확장4)`.

## Self-Review
- 스펙 §4 전부 매핑 ✓. **SaveVer 변경 없음**(기존 필드만 변경 — 마이그레이션/세이브 테스트 불필요, #79 류).
- parity: 서버 `getRerollSetEssenceCost`/`getRarityUpgrade*`/`getTransferEssenceCost` ↔ 클라 `GetRerollSetEssenceCost`/`GetRarityUpgrade*`/`GetTransferEssenceCost` — TM cross-check(#61 RNG parity 교훈: 클라 권위 RNG + 서버 함수 parity).
- 등급 상한(Mythic) 가드·실패 시 불변·전송 source 삭제 인덱스 정합 = 핵심 엣지.
- jumbo ODR(Rune~ prefix), 자원 이중 차감 금지(단일 지점).

## 매핑: 1→backend, 2→character(메인), 3→designer, 4→balance, 5→qa.
