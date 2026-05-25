# PR #18 기획서 — 퀘스트 시스템 V1 (M4 PR #8 본체)

> 스토리 바이블 V1(PR #17) 위에 퀘스트 시스템 구축. **메인 퀘스트 라인(챕터 1) + 일일 퀘스트 3개 + 진행 추적/리셋/수령**. 서버 권위(offline 모듈 패턴) + 클라 진행 훅/로그 UI. 자동 전투(킬)·맵·오프라인과 연동.

## 1. 목표 / DoD
자동 전투 진행에 따라 퀘스트 진행도가 누적되고, 완료 시 보상(골드/EXP)을 수령하며, 일일 퀘스트는 매일 리셋된다. 퀘스트 로그 UI 로 활성 퀘스트/진행도/수령을 확인한다.

### DoD 검증
1. 메인 퀘스트(챕터1) — 맵별 목표(예: 1-1 슬라임 N 처치) 진행 → 완료 → 수령 → 다음 퀘스트 해금.
2. 일일 퀘스트 3종(예: 몬스터 N 처치 / 오프라인 보상 수령 / 장비 강화) 진행·수령, **자정(서버 기준) 리셋**.
3. 퀘스트 로그 UI: 활성 퀘스트 목록 + 진행도 바 + 수령 버튼.
4. 서버 Vitest(퀘스트 공식/리셋/진행) + UE 빌드/Automation(클라 진행·UI 모델) GREEN.

## 2. 범위 (In Scope)

### 2.1 서버 — 퀘스트 모듈 (메인)
- `server/src/core/data/quests.ts`: 퀘스트 정의(읽기 전용) — 메인(챕터1, 스토리 바이블 §5.1 훅 기반) + 일일 3. 필드: questId, type(main|daily), objective(killMonster/clearMap/claimOffline/enhance), targetCount, rewardGold, rewardExp, prerequisiteQuestId(메인 체인).
- `server/src/modules/quest/`: (offline 모듈 패턴 repo/routes/schema/service/test)
  - 진행 상태: 유저별 questId→progress, completed, claimed, daily 리셋 기준 시각.
  - 라우트: `GET /v1/quests`(활성+진행), `POST /v1/quests/:id/progress`(증가; 서버 권위는 후속, V1 은 클라 보고 수용+검증), `POST /v1/quests/:id/claim`(보상 수령), 일일 리셋(자정 기준 lazy 리셋: 조회 시 날짜 바뀌면 daily 초기화).
  - DB schema + drizzle migration(quest_progress).
- Vitest: 진행 누적/완료/수령/일일 리셋(날짜 경계)/메인 체인 해금.

### 2.2 클라이언트 — 진행 훅 + 로그 UI (C++)
- 진행 훅: 자동 전투 몬스터 처치(IdleMonster::HandleDeath 또는 GameInstance) → 활성 퀘스트 objective 진행 증가. 오프라인 수령/강화 시에도 해당 objective 증가.
- `UIdleGameInstance` 또는 신규 `UQuestComponent`/서비스: 활성 퀘스트 보유·진행·완료·수령(재화 반영). 서버 graceful(로컬 진행 후 동기화, 서버 없으면 로컬).
- 퀘스트 로그 UI(IdleHUD 확장 또는 위젯): 활성 퀘스트명/진행(n/target)/수령. 단축키 Q(기존 GDD §단축키).
- Automation: 클라 진행/완료/수령/리셋 순수 로직 + UI 모델.

### 2.3 데이터 / 밸런스
- 메인 퀘스트 목표/보상, 일일 3종 목표/보상 1차값. 밸런스 문서 반영(시뮬레이터 후속 보정).

### 2.4 테스트
- 서버 Vitest + 클라 Automation.

## 3. 범위 외
- 업적(Achievement) 시스템(후속).
- 주간/길드 퀘스트, 일일 던전(후속).
- 서버 권위 진행 검증 강화(V1 클라 보고 수용, 후속 강화).
- 컷씬/대사 연출(스토리 텍스트 키만 참조).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 백엔드 (메인) | quests.ts 데이터 + quest 모듈/라우트/리셋 + Vitest + migration | ✅ 메인 (`backend`) |
| 퀘스트 | 메인 체인(챕터1)·일일 3 정의/목표/보상 설계 | ✅ 보조 (`quest`) |
| 캐릭터 | 클라 진행 훅(킬 등) + 수령 재화 반영 + Automation | ✅ 보조 (`character`) |
| 디자이너 | 퀘스트 로그 UI(목록/진행/수령, Q 단축키) | ✅ 보조 (`designer`) |
| QA | 진행/완료/리셋/수령 시나리오 | ✅ 보조 (`qa`) |

## 5. 호출 순서
1. `backend`(메인) → 모듈/데이터/라우트/리셋/테스트
2. `quest` → 정의 정밀화(메인 체인/일일)
3. `character` → 클라 진행 훅 + 수령
4. `designer` → 로그 UI
5. `qa` → 시나리오 ([4] 흡수 가능)

## 6. 워크플로우 v3
[1] 기획(본 문서)+PR → [2] Codex 개발(+PM 산출 게시) → [3] Claude TM → [4] Codex TM+fix → [5] Claude 검증 → [N] CI+PM 종합+머지. 사용자 PIE 차후 일괄([[feedback-autonomous-slices]]).

## 7. 리스크
| 리스크 | 완화 |
| --- | --- |
| 서버/클라 진행 동기화 복잡 | V1 클라 로컬 진행 + 서버 graceful 동기화, 권위 검증 후속 |
| 일일 리셋 타임존 | 서버 UTC 기준 자정 lazy 리셋, 문서 명시 |
| 진행 objective 종류 확장 | enum + 데이터 주도, killMonster 우선 |
| 메인 체인 ↔ 스토리/맵 정합 | 스토리 바이블 §5.1 맵 훅 인용 |

## 8. 후속
- 업적, 주간/길드 퀘스트, 서버 권위 진행 검증, 퀘스트 대사 컷씬, 일일 던전.
