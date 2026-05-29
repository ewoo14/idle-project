# 칭호(Title) 시스템 구현 계획

> 스펙: [`2026-05-30-title-system-design.md`](../specs/2026-05-30-title-system-design.md). v3 디스패치, 현행 재검증. SaveVer 20→21.

**Goal:** 업적 메트릭 달성으로 칭호 해금 + 1개 장착 패시브 보너스. 신규 메타 레이어.

**Architecture:** 서버 `title.ts` 카탈로그/parity + 클라 TitleService(해금/장착/보너스) + 칭호 패널 UI + 보너스 단일 적용. SaveVer 20→21.

## Task 1: backend (backend)
- [ ] `server/src/core/formulas/title.ts` 신규: `TITLE_CATALOG`(16~20종: id/metric/threshold/bonusType/bonusValue), `getUnlockedTitles(metricValues: Record<metric, number>): string[]`, `getTitleBonus(titleId): {type, value}`. 메트릭 키는 클라 EAchievementMetric과 동일 문자열/순서. 기존 formulas 패턴(Math.fround 등) 모방.
- [ ] `title.test.ts`: 해금 경계(threshold-1 미해금/threshold 해금), 보너스 매핑, 카탈로그 무결성(id 유니크). `cd server; npm run lint && npx vitest run src/core/formulas && npm run build` GREEN.
- [ ] 커밋 `feat: 칭호 카탈로그/해금 backend parity (칭호 시스템)`.

## Task 2: client (character)
- [ ] `GameCore/TitleTypes.h`: `ETitleBonus{None,AllStatPct,GoldPct,ExpPct,CritDmgPct}`, `FTitleDefinition`(TitleId/Name/Metric(EAchievementMetric)/Threshold/BonusType/BonusValue), `FTitleBonus`.
- [ ] `GameCore/TitleService.{h,cpp}`(UObject): `InitializeDefaultTitles`(서버 카탈로그 1:1), `TSet<FString> UnlockedTitleIds`, `FString EquippedTitleId`, `RecomputeUnlocked(const UAchievementService*)`(GetMetricValue>=threshold면 Add), `bool EquipTitle(id)`(해금된 것만), `FTitleBonus GetEquippedTitleBonus() const`, `GetDefinitions`/접근자, `RestoreState(unlocked, equipped)`.
- [ ] **보너스 적용(단일 지점, #72 가드)**: AllStatPct→RefreshDerivedStats 스탯 배수, GoldPct→AddGold, ExpPct→AddExp, CritDmgPct→크리뎀 계산 — 각 1곳, 기존 마스터리/펫 합산 옆. 장착 칭호 1개만.
- [ ] GameInstance: TitleService 보유·초기화, 업적 메트릭 갱신(RecordMetric)/로그인/세이브 시 RecomputeUnlocked, EquipTitle 진입점(→RefreshDerivedStats·Autosave). SaveVer **20→21**(UnlockedTitleIds/EquippedTitleId 직렬화, 기존 Set/FString 저장 패턴 모방). Capture=21/Apply `>=21` 가드. CloudSavePayloadMapper 정합(UStructToJson 자동이면 확인만).
- [ ] **전 세이브 테스트 단언 20→21 일괄 갱신**(grep `(20)`/`V20`/`v20`/`SaveVersion, 20`/`SaveVersion = 20` in Tests/, 레거시 입력 <21 유지).
- [ ] Automation(`TitleServiceTests` 신규): 해금 경계(메트릭 threshold), 장착(해금된 것만·1개), 보너스 적용(AllStat/Gold 등 CP·재화 반영), 미해금 장착 거부, 세이브 v21 라운드트립, parity(서버 카탈로그 임계/보너스). 익명 헬퍼 Title~ prefix.
- [ ] 커밋 `feat: 칭호 TitleService + SaveVer21 (칭호 시스템)`.

## Task 3: UI (designer)
- [ ] 칭호 패널: 해금/미해금 목록(이름·해금 조건·보너스 표시), 장착/해제 버튼, 현재 장착 칭호 강조. 미해금은 조건 진행 표시(선택). ko/en + CsvIntegrity. 표준 jumbo.
- [ ] 커밋 `feat: 칭호 패널 UI (칭호 시스템)`.

## Task 4: balance/docs (balance)
- [ ] `docs/planning/title-system-balance-note.md`: 칭호 16~20종 목록(메트릭/임계/보너스), 보너스 곡선(+3~20%), 장착 1개 제한·파워크리프 분석·median 영향. SaveVer21.
- [ ] [[project_pr_order]]/[[project_session_progress]] 갱신. 커밋 `docs: 칭호 시스템 밸런스 노트 (칭호 시스템)`.

## Task 5: qa (qa)
- [ ] E2E: 메트릭 달성→해금(경계)→장착(보너스 CP/재화 반영)→해제→미해금 장착 거부→세이브 v21→parity. 표준 jumbo + ue-automation.ps1 게이트. PR 스크린샷.
- [ ] 커밋 `test: 칭호 시스템 E2E (칭호 시스템)`.

## Self-Review
- 스펙 §4 전부 매핑 ✓. parity: 서버 title.ts(카탈로그/해금/보너스) ↔ 클라 TitleService 정의 1:1(임계·보너스) — TM cross-check.
- **이중 적용 가드(#72)**: 보너스 타입별 단일 지점, 장착 1개만. 기존 마스터리/펫/길드 버프와 합산 일관.
- SaveVer 20→21 전 테스트 단언 갱신(stale 방지, #83 교훈). 해금/장착 누락=빈(회귀안전).
- jumbo ODR(Title~ prefix). 카탈로그 클라↔서버 동기(메트릭 enum 문자열 일치).

## 매핑: 1→backend, 2→character(메인), 3→designer, 4→balance, 5→qa.
