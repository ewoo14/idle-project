# PR #19 기획서 — 챕터1 보스 + 환생 V1 (M4 PR #9)

> M4 마지막. 챕터1 보스 **안개 군주** + **환생 V1**(영구 보너스 + 자원 일부 보존). 기존 전투/스탯/오프라인/퀘스트 위에. 환생 횟수(rebirthCount)는 이미 오프라인 보너스/캐릭터/리더보드에 연동됨 — 이를 실제로 증가시키는 시스템.

## 1. 목표 / DoD
챕터1 5맵 진행 후 **안개 군주 보스**를 격파하고 레벨 100 도달 시 **환생** 가능 — 환생 시 레벨/골드 일부 초기화, 영구 보너스 포인트(능력치) + rebirthCount 증가로 다음 사이클이 더 강해진다.

### DoD 검증
1. 보스 안개 군주(강화 스탯 + 광역 패턴 V1 단순화)가 챕터1 관문에 등장, 자동 전투로 격파 가능.
2. 환생 조건(보스 격파 플래그 + Lv100) 충족 시 환생 가능, 미충족 시 불가.
3. 환생 실행 → rebirthCount++, 영구 보너스 포인트 +5(능력치 영구 가산), 레벨 1 리셋, 골드 일부 보존(비율), 장비 일부 보존.
4. rebirthCount 증가가 오프라인 보너스(+5%/회)·스탯에 반영.
5. 서버 Vitest(환생 검증/리셋/보존/포인트) + UE 빌드/Automation(보스/환생 로직) GREEN.

## 2. 범위 (In Scope)
### 2.1 캐릭터·전투 (메인, C++)
- **보스 안개 군주**: AIdleMonster 변형(또는 bBoss 플래그 + 강화 스탯) — 높은 HP/ATK, 챕터1 관문 spawn(GameMode). V1 광역 패턴은 단순화(주기적 강타) 허용.
- 보스 격파 시 `bChapter1BossDefeated` 플래그(GameInstance/세이브).
- **환생 로직**: `UIdleGameInstance`(또는 RebirthService) — `CanRebirth()`(보스격파 && Lv>=100), `Rebirth()` → rebirthCount++, 영구 포인트 += 5, Level=1, Gold = floor(Gold × 보존율), 장비 일부 보존(규칙), 능력치에 영구 포인트 반영(StatFormulas/RefreshDerivedStats 합산).
- Automation: 환생 조건/리셋/보존/포인트 누적/rebirthCount 반영 순수 로직.

### 2.2 서버 (보조)
- character 모듈: 환생 persist — rebirthCount(기존) + rebirthBonusPoints 컬럼 추가(migration 0004), 환생 검증/적용 라우트(`POST /v1/characters/rebirth`) 또는 save 경유. Vitest.

### 2.3 디자이너 (보조)
- 환생 UI: 조건 충족 시 환생 버튼/패널(현재 rebirthCount, 다음 보너스, 확인), 보스 등장/격파 표시(HUD V1 단순).

### 2.4 밸런스 (보조)
- 환생 포인트 5, 골드 보존율, 장비 보존 규칙, 보스 스탯 1차값. 문서 반영(시뮬레이터 후속).

### 2.5 테스트
- 서버 Vitest + 클라 Automation.

## 3. 범위 외
- 환생 전용 스킬 슬롯 해금(스킬 트리 확장 후속), 스킨/스토리 잠금 해제(후속).
- 챕터 2 이후, 다단계 보스 페이즈 연출(후속).
- 전직(클래스 체인지) — 환생과 별개 후속.

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | 보스 + 환생 로직 + 스탯 반영 + Automation | ✅ 메인 (`character`) |
| 백엔드 | 환생 persist(rebirthBonusPoints, migration) + 검증 라우트 + Vitest | ✅ 보조 (`backend`) |
| 디자이너 | 환생 UI + 보스 표시 | ✅ 보조 (`designer`) |
| 밸런스 | 포인트/보존율/보스 스탯 + 문서 | ✅ 보조 (`balance`) |
| QA | 보스 격파→환생 시나리오 | ✅ 보조 (`qa`) |

## 5. 호출 순서
1. `character`(메인) → 보스 + 환생 로직 + 테스트
2. `backend` → persist/검증
3. `designer` → 환생 UI
4. `qa`/`balance` → 시나리오/수치 ([4] 흡수 가능)

## 6. 워크플로우 v3
[1] 기획+PR → [2] Codex 개발(+게시) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] 머지. 사용자 PIE 차후 일괄([[feedback-autonomous-slices]]).

## 7. 리스크
| 리스크 | 완화 |
| --- | --- |
| 환생 리셋/보존 데이터 일관성 | 서버 권위 적용 + 클라 미러, Vitest 보존율 경계 |
| 보스 난이도 미검증 | 1차 스탯, 시뮬레이터 후속 |
| rebirthCount 기존 연동 회귀 | offline/leaderboard 테스트 유지 + parity |

## 8. 후속
- 환생 전용 스킬 슬롯, 스킨/스토리 해금, 전직, 챕터2, 보스 페이즈 연출.
