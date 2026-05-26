# PR #31 기획서 — 스테이지 진행 시스템 (심화)

> 메이플스토리 키우기류 방치형의 **핵심 백본**. 현재 몬스터는 고정 위치·고정 스탯으로 무한 리스폰만 함(진행 구조 부재). 챕터1 맵(1-1~1-5)을 **스테이지로 자동 진행**하고, 스테이지별 몬스터 스케일링 + 맵별 속성 약점(#30 연계) + 보스 스테이지를 추가해 방치 진행 루프를 완성한다.

## 1. 목표 / DoD
몬스터를 일정 수 처치하면 다음 스테이지로 자동 진행하고, 스테이지가 오를수록 몬스터가 강해지며, 마지막 스테이지(1-5)에서 보스를 처치하면 챕터 보스 격파로 기록된다.

### DoD 검증
1. 스테이지 = (챕터, 스테이지) — 챕터1: 1-1 ~ 1-5. 각 일반 스테이지는 `KillsToAdvance`(예 10) 처치 시 다음으로 진행. 마지막(1-5)은 보스 스테이지.
2. 몬스터 스탯이 글로벌 스테이지 인덱스로 스케일(HP/Atk × (1 + idx×k)). 스테이지별 속성 약점(맵 테마) 부여.
3. 1-5 보스 처치 → 다음 진행 + `MarkChapter1BossDefeated()`(환생 게이트 기존 연동). V1은 챕터1 끝에서 보스 반복(다음 챕터 후속).
4. HUD 에 현재 스테이지 + 진행도("스테이지 1-3 · 7/10") 표시.
5. 서버 StageDB/공식 미러 + parity. 서버 Vitest + UE 빌드/Automation GREEN. 기존 전투/리스폰 회귀 없음.

## 2. 범위 (In Scope)
### 2.1 스테이지 서비스 (메인, C++)
- `UStageService`(기존 Quest/Pet/Season 서비스 패턴): `CurrentChapter`/`CurrentStage`/`KillsThisStage`/`KillsToAdvance`. `RecordKill(bWasBoss)` → 카운트 증가, 임계 도달 시 `AdvanceStage()`. `GetGlobalStageIndex()`(=(챕터-1)*StagesPerChapter+(스테이지-1)). `GetCurrentStageInfo()`(약점 속성/보스 여부/진행도).
- `StageFormula.h/.cpp` 순수 함수: `ComputeMonsterStatMultiplier(GlobalStageIndex)`, `ComputeRewardMultiplier(GlobalStageIndex)`, `IsBossStage(Stage)`, `GetStageWeakElement(GlobalStageIndex)`(맵 테마: 1-1 초원=None, 1-2 늑대숲=None, 1-3 안개동굴=Ice, 1-4 망령폐허=Holy, 1-5 신전=Fire 등 1차).
- `UIdleGameInstance`: StageService 보유 + 게터(+ EnsureStageService, 테스트 초기화). `MarkChapter1BossDefeated` 연계.
### 2.2 GameMode 통합 (메인, C++)
- `SpawnMonsterAt`: StageService 스테이지 정보로 몬스터 HP/Atk 스케일 + WeakElement 설정 + 보스 스테이지면 보스 스폰.
- `ScheduleRespawn`(OnDeath): `StageService->RecordKill(bWasBoss)` + 기존 `RecordMonsterKilled()`(퀘스트) 호출. 스테이지 진행 시 신규 스케일로 리스폰.
### 2.3 UI (디자이너)
- HUD 스테이지 인디케이터: "스테이지 {챕터}-{스테이지} · {kills}/{toAdvance}" + 보스 스테이지 강조. ViewModel + DrawHUD. 로컬라이즈 키(ko/en).
### 2.4 서버 (백엔드)
- `server/src/core/data/stages.ts`(챕터1 5스테이지 정의: killsToAdvance/boss/weakElement) + `server/src/core/formulas/stage.ts`(statMultiplier/rewardMultiplier 클라 미러) + parity 테스트.
### 2.5 데이터/밸런스
- 스케일 계수 k, KillsToAdvance, 맵별 약점 + 문서(05-balance-philosophy / story-bible 맵 연계).
### 2.6 테스트
- 서버 Vitest(공식/미러/parity) + 클라 Automation(진행·스케일·보스·약점·HUD 뷰모델).

## 3. 범위 외
- 다중 챕터(2+) / 챕터 전환 연출(V1 챕터1 반복), 스테이지 선택/되돌리기, 스테이지 보상 상자/드롭 테이블 차등(후속).
- 맵별 배경 아트/BGM(외부 의존).
- 서버 권위 스테이지 동기화(클라 권위 V1, 서버는 공식/정의 미러만).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | StageService + GameMode 통합 + StageFormula + 스케일/약점/보스 + Automation | ✅ 메인 (`character`) |
| 백엔드 | stages.ts + stage.ts 공식 미러 + parity | ✅ 보조 (`backend`) |
| 디자이너 | 스테이지 HUD 인디케이터 + 로컬라이즈 | ✅ 보조 (`designer`) |
| 밸런스 | 스케일 계수/KillsToAdvance/맵 약점 + 문서 | ✅ 보조 (`balance`) |
| QA | 진행/스케일/보스/약점 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/designer/balance/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정** + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]]). **Codex 커밋/푸시 누락 PM 보완**([[reference-codex-invoke]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 진행 상태/리스폰 정합(스테이지 전환 시 기존 몬스터 처리) | RecordKill→Advance→다음 스폰만 스케일 적용, 결정적 StageService 단위 테스트 |
| 보스 스테이지 무한 반복/진행 잠김 | IsBossStage + 챕터 끝 처리 명확, 보스 처치 시 MarkChapter1BossDefeated 1회 |
| 스케일 폭주(HP/Atk 과성장) | 선형 계수 k 보수적 + 밸런스 시뮬(tools/balance-sim) 참고, 상한 검토 |
| 서버↔클라 공식 parity | StageFormula DefinitionParity 확장 |
| 기존 전투/리스폰 회귀 | 스테이지 미초기화 시 기본(idx 0, 배수 1.0)로 기존 동작 보존 |

## 7. 후속
- 다중 챕터 + 챕터 전환 연출, 스테이지 보상 상자/드롭 차등, 서버 권위 스테이지, 맵 배경/BGM(외부).
