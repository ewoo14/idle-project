# PR #37 기획서 — 챕터2 + 챕터 전환 (심화)

> 스테이지 시스템(#31)은 멀티챕터를 염두에 두고 GlobalStageIndex 기반으로 만들었지만, **챕터1 보스(1-5) 클리어 시 동결**된다. 챕터 전환을 일반화해 **챕터2(2-1~2-5)** 로 이어지게 한다. 몬스터 스케일(#31)·처치 보상(#32)·드롭 희귀도(#36)·속성 약점(#30)이 모두 GlobalStageIndex 기반이라 ch2 는 자동으로 더 깊어진다. 환생 게이트(ch1 보스)는 분리 유지한다.

## 1. 목표 / DoD
챕터1 보스를 처치하면 챕터2(2-1)로 진행하고, 최종 챕터 보스까지 클리어하면 진행이 멈춘다. 환생은 여전히 챕터1 보스 격파로 열린다.

### DoD 검증
1. 챕터 전환: 챕터 마지막 스테이지(N-5) 보스 처치 → 다음 챕터(있으면) 1스테이지로 진행(CurrentChapter+1, Stage=1). 최종 챕터(TotalChapters=2) 보스 처치 → 진행 동결.
2. 환생 게이트: 챕터1 보스 처치 시 `MarkChapter1BossDefeated()`(기존 환생 조건) 1회 발동. 이후 챕터 진행과 무관하게 환생 가능.
3. ch2 스케일: GlobalStageIndex(ch2 = 5~9)로 몬스터 HP/Atk·보상·드롭 희귀도 자동 상향. ch2 맵 속성 약점 부여.
4. HUD 챕터 표시(2-3 등) + 챕터 전환 피드백. 환생 시 챕터/스테이지 리셋(1-1).
5. 서버 stages.ts ch2 정의 + parity. 서버 Vitest + UE 빌드/Automation GREEN. 기존 ch1 진행 회귀 없음.

## 2. 범위 (In Scope)
### 2.1 스테이지 멀티챕터 일반화 (메인, C++)
- `UStageService`: `static constexpr int32 TotalChapters = 2`. `AdvanceStage()` 챕터 종료(CurrentStage==StagesPerChapter) 처리:
  - 이 시점은 **현재 챕터 보스 처치** → `OnChapterBossDefeated.Broadcast(CurrentChapter)` + `HighestClearedChapter = max(.., CurrentChapter)`.
  - CurrentChapter < TotalChapters → ++CurrentChapter, CurrentStage=1, KillsThisStage=0(다음 챕터 진행).
  - else → `bFinalChapterCleared=true`(동결), KillsThisStage=GetKillsToAdvance().
  - OnStageChanged.Broadcast.
  - 기존 `bCurrentChapterBossCleared`(전역 동결) → `bFinalChapterCleared`(최종 챕터만 동결)로 의미 변경. RecordKill 의 early-return 은 bFinalChapterCleared 기준.
- `OnChapterBossDefeated(int32 ClearedChapter)` 델리게이트 추가. `GetHighestClearedChapter()`/`HasClearedChapterBoss(int32 Chapter)` 게터.
- 환생 리셋(Rebirth/InitializeDefaultStages 경로): CurrentChapter=1/Stage=1/KillsThisStage=0/bFinalChapterCleared=false/HighestClearedChapter=0.
### 2.2 환생 게이트 분리 (메인, C++)
- `UIdleGameInstance`: EnsureStageService 에서 StageService->OnChapterBossDefeated 구독 → `ClearedChapter == 1` 일 때 `MarkChapter1BossDefeated()` 호출. **GameMode 의 인라인 MarkChapter1BossDefeated 호출은 제거**(StageService 델리게이트로 일원화) — 또는 GameMode 가 chapter==1 게이트. 한 경로로 통일(중복 호출 금지).
### 2.3 챕터 스케일/약점 (메인, C++)
- `FStageFormula::GetStageWeakElement(GlobalStageIndex)`: ch2(idx 5~9) 맵 테마 약점 추가(예 2-1 None / 2-2 Lightning / 2-3 Ice / 2-4 Fire / 2-5 Holy 등 1차). ComputeMonsterStatMultiplier/ComputeRewardMultiplier 는 idx 기반이라 그대로 ch2 상향(검증만).
### 2.4 UI (디자이너)
- HUD 스테이지 인디케이터가 챕터2(2-3 등) 표시 정상 + 챕터 전환 시 짧은 알림("챕터 2 진입" 등). 로컬라이즈 ko/en.
### 2.5 콘텐츠/스토리
- `docs/planning/06-story-bible.md` 의 챕터2 시놉시스/맵(2-1~2-5) 1차 작성(스토리 바이블 챕터 시놉시스 연계). ch2 맵 약점 테마와 정합.
### 2.6 서버 (백엔드)
- `server/src/core/data/stages.ts`: 챕터2 5스테이지 정의(killsToAdvance/boss/weakElement) + ch2 weakElement parity. TotalChapters/챕터 진행 관련 상수 정합.
### 2.7 데이터/밸런스
- ch2 난이도(HP/Atk idx 5~9)·보상·드롭 곡선 점검 + 문서.
### 2.8 테스트
- 서버 Vitest(ch2 정의/약점/parity) + 클라 Automation(챕터 전환, ch1 보스→OnChapterBossDefeated(1), 최종 챕터 동결, ch2 GlobalStageIndex/약점, 환생 리셋).

## 3. 범위 외
- 챕터3+ (TotalChapters 확장만으로 가능, 콘텐츠는 후속), 챕터 전환 컷신/연출(외부), 챕터별 고유 몬스터 메시/보스 메커닉.
- 환생 게이트를 ch2 보스로 상향(V1 ch1 유지).
- 서버 권위 챕터 진행(클라 권위 V1).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | StageService 멀티챕터 + OnChapterBossDefeated + 환생 게이트 분리 + StageFormula ch2 약점 + Automation | ✅ 메인 (`character`) |
| 백엔드 | stages.ts ch2 정의 + parity | ✅ 보조 (`backend`) |
| 디자이너 | HUD 챕터2 표시/전환 알림 + 로컬라이즈 | ✅ 보조 (`designer`) |
| 밸런스/콘텐츠 | ch2 곡선 점검 + 스토리 바이블 ch2 시놉시스 + 문서 | ✅ 보조 (`balance`) |
| QA | 챕터 전환/환생 게이트/ch2 스케일 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/designer/balance/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정** + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]]). **Codex 커밋/푸시 누락 PM 보완**([[reference-codex-invoke]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 환생 게이트 중복/누락(MarkChapter1BossDefeated) | StageService OnChapterBossDefeated(1) 단일 경로로 일원화, GameMode 인라인 제거, 중복 호출 가드(MarkChapter1BossDefeated idempotent) |
| 챕터 전환 시 진행/동결 정합 | bFinalChapterCleared(최종만) + 결정적 StageService 단위 테스트(ch1→ch2 전환, 최종 동결) |
| 환생 리셋 누락(챕터 잔존) | Rebirth 경로에서 챕터/동결/HighestCleared 리셋 + 테스트 |
| ch1 진행 회귀 | TotalChapters=2 로 ch1 동작 보존, ch1 보스→환생 게이트 기존과 동일 |
| 서버↔클라 ch2 정의 parity | stages.ts ch2 + DefinitionParity 확장 |

## 7. 후속
- 챕터3+ 콘텐츠, 챕터별 고유 몬스터/보스 메커닉, 챕터 전환 연출(외부), 환생 게이트 상향, 서버 권위 챕터 진행.
