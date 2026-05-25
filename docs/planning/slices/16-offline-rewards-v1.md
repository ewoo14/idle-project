# PR #16 기획서 — 오프라인 보상 V1 (원 슬라이스 ID: M3 PR #6)

> 마일스톤 M3(오프라인 & 백엔드). 게임이 꺼져 있어도 진행 — 오프라인 동안 골드/EXP 누적(최대 12h), 재실행 시 환영 모달로 수령. 밸런스 §7(오프라인 = 액티브 70~80% 효율), 아키텍처(서버 공식 = TS, 클라 표현 = C++). 백엔드 V1(PR #2) + 자동전투/스킬 위에 구축.

---

## 1. 목표 / DoD
게임 종료 후 재실행 시, 오프라인 경과 시간에 비례한 골드/EXP 보상(12h 상한, 액티브 대비 ~75% 효율, 시간 보너스 곡선 적용)을 **환영 모달**로 보여주고 수령하면 재화에 반영된다.

### DoD 검증 시나리오
1. 게임 플레이 → 종료(마지막 접속 시각 저장) → N분/시간 후 재실행.
2. 시작 시 오프라인 경과 시간 계산 → 보상(골드/EXP) 산출 → **환영 모달** 표시(경과 시간, 골드, EXP).
3. 수령 버튼 → 골드/EXP 가 GameInstance/세이브에 반영(HUD 갱신).
4. 12h 상한·효율·곡선이 공식대로 적용.
5. 서버 Vitest(공식) + UE 빌드/Automation(클라 모델) GREEN.

---

## 2. 범위 (In Scope)

### 2.1 서버 — 오프라인 보상 공식 (TS, 메인)
- `server/src/core/formulas/offline.ts`:
  - `OFFLINE_CAP_SECONDS = 12 * 3600` (43200).
  - `OFFLINE_EFFICIENCY = 0.75` (액티브 대비, 밸런스 §7 70~80% 중앙).
  - `computeOfflineRewards(input: { level, lastSeenUnixSec, nowUnixSec, rebirthCount? }): { cappedSeconds, gold, exp, timeBonusMultiplier }`
    - `elapsed = max(0, now - lastSeen)`; `capped = min(elapsed, CAP)`.
    - 초당 기준 골드/EXP: 레벨 기반 1차식(기존 level/combat 공식 재사용 또는 단순 `baseGoldPerSec(level)`), × EFFICIENCY × timeBonus.
    - **시간 보너스 곡선**: `timeBonus = 1 + min(cappedHours, 12) * PER_HOUR_BONUS` (PER_HOUR_BONUS 1차값, 예 0.0 → V1 평탄, 또는 소폭) + 환생 보너스 `+0.05 * rebirthCount`(밸런스 §line119). 함수로 분리해 곡선 교체 용이.
  - Vitest: 상한(>12h=12h), 0 경과=0, 효율 적용, 곡선/환생 보너스, 단조 증가.

### 2.2 서버 — 수령 모듈/라우트 (최소)
- `server/src/modules/offline/`: `GET /v1/offline/preview` (lastSeen 기준 미리보기) + `POST /v1/offline/claim` (수령 → 재화 반영 + lastSeen 갱신). save 모듈의 lastSeen 타임스탬프 활용/추가.
- V1 은 인증 사용자 기준. 미인증/로컬 단독도 클라 로컬 계산 폴백 허용(아래 2.3).

### 2.3 클라이언트 — 환영 모달 + 연동 (C++)
- `client/Source/IdleProject/`: 오프라인 보상 표현.
  - 시작 시 lastSeen(로컬 세이브 or 서버) 으로 경과 계산 → 공식 미러(C++ `OfflineRewardFormula`, 서버 TS 와 동일 상수) 로 산출.
  - **환영 모달 UI**(IdleHUD 또는 별도 위젯, V1 단순 패널): 경과 시간 / 골드 / EXP / 수령 버튼.
  - 수령 → `UIdleGameInstance` 골드/EXP 반영(기존 AddExp/골드 경로 재사용) + lastSeen 갱신.
  - 서버 연동 가능 시 `NetworkClient` 로 preview/claim 호출(graceful: 서버 없으면 로컬 계산).
- 클라 공식 미러 순수 로직 Automation 테스트(상한/효율/곡선).

### 2.4 데이터 / 밸런스
- 효율 0.75, 12h cap, 시간 보너스 곡선 1차값을 balance 문서(05) 표에 반영. 시뮬레이터 도착 시 재보정 명시.

### 2.5 테스트
- 서버 Vitest: offline 공식. 클라 Automation: C++ 미러 순수 로직.

---

## 3. 범위 외 (Out of Scope)
- 환생 시스템 자체(보너스 인자만 받음, 환생 PR 후속).
- 오프라인 중 전투 시뮬/아이템 드롭(V1 은 골드/EXP 만).
- 푸시 알림, 오프라인 보상 광고 2배 등 수익화(후속).
- 모달 아트/연출(V1 단순 패널, 아트 후속).

## 4. 7파트 작업 분배 — Codex 호출 계획
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| **백엔드 (메인)** | offline.ts 공식 + offline 모듈/라우트 + Vitest + lastSeen 연동 | ✅ 메인 (`backend`) |
| 캐릭터·전투 | 클라 C++ 공식 미러 + GameInstance 재화 반영 + Automation 테스트 | ✅ 보조 (`character`) |
| 디자이너 | 환영 모달 UI(경과/골드/EXP/수령) | ✅ 보조 (`designer`) |
| 밸런스 | 효율/cap/곡선 수치 + 문서 | ✅ 보조 (`balance`) |
| QA | 오프라인 시나리오(종료→재실행→모달→수령) + IT | ✅ 보조 (`qa`) |
| 스토리/퀘스트 | 해당 없음 | ◻ |

## 5. 호출 순서
1. `backend`(메인) → 공식 + 라우트 + 테스트
2. `character` → 클라 미러 + 재화 반영
3. `designer` → 환영 모달
4. `balance` → 수치/문서 (필요 시 [4] 흡수)
5. `qa` → 시나리오 ([4] 흡수 가능)

## 6. 워크플로우 v3
[1] PM 기획(본 문서)+PR → [2] Codex 개발(+PM 산출 게시) → [3] Claude 리뷰+fix → [4] Codex 리뷰+fix(+PM 산출 게시) → [5] Claude 검증 → [N] CI+PM 종합+머지. **사용자 PIE 검증은 차후 일괄([[feedback-autonomous-slices]])**.

## 7. 리스크
| 리스크 | 완화 |
| --- | --- |
| 서버/클라 공식 불일치 | 상수(cap/효율/곡선) 단일 정의 미러, 양쪽 동일 테스트 |
| lastSeen 신뢰(치트) | V1 서버 권위 우선, 로컬은 폴백. 서버 검증 강화는 후속 |
| 곡선 수치 미검증 | 1차 가이드, 밸런스 시뮬레이터 재보정 명시 |
| 모달 UI 에셋 부재 | V1 단순 패널, 아트 후속 |

## 8. 후속 PR 예고
- 오프라인 전투 시뮬/드롭, 광고 2배 보상, 환생 연동 보너스, 서버 치트 검증 강화, 모달 연출.
