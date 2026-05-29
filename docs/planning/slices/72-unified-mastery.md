# PR #72 기획서 — 통합 마스터리 (Unified Mastery)

> **PM 자율 진행**. 브레인스토밍→스펙→계획으로 확정한 교차 메타 시스템 슬라이스. 각 시스템을 플레이하면 자동 적립되는 6개 무한 "숙련 트랙"을 추가하고, 그 합인 **월드 파워**가 환생·초월로 초기화되지 않는 **영구** 전역 보너스를 능력치·드롭·골드·EXP에 부여한다. 무한 성장([[project-infinite-growth]]) + 콘텐츠 풍부화([[project-content-richness]]). client + server 멀티시스템(5-team).
>
> 상세 스펙: [`docs/superpowers/specs/2026-05-29-unified-mastery-design.md`](../../superpowers/specs/2026-05-29-unified-mastery-design.md)
> 구현 계획(TDD): [`docs/superpowers/plans/2026-05-29-unified-mastery.md`](../../superpowers/plans/2026-05-29-unified-mastery.md)

## 1. 목표 / DoD

엔드게임 성장 우물 — 던전·장비·룬 등을 마스터한 고수가 "각 시스템을 계속 써서" 무한히 더 쌓을 수 있는 영구 상위 축. 환생·초월(리셋형)·전투력(표시 지표)·무한의 탑(콘텐츠)과 직교한다.

### DoD 검증
1. **무한 숙련 트랙(6)**: 전투/장비/심연/룬/야성/탐험. 활동 시 트랙 XP 자동 적립, `XpToNext(L)=floor(100·1.15^L)` 무한 기하 곡선. 클라 `FMasteryFormula` ↔ 서버 `mastery.ts` parity.
2. **전역 보너스(월드 파워)**: 코어 스탯 배수(전투+장비+탐험, `1+0.02·ln(1+합)`)·크리 가산(룬)·드롭 가산(심연)·골드/EXP %(야성). 코어 배수는 `RefreshDerivedStats` 단일 지점에서 transcend×tower×achievement 곱에 **별도 항**으로 합류(이중 계산 없음). 드롭/골드/EXP는 기존 펫 보너스 경로에 합류. CP 자동 반영.
3. **영구(리셋 생존)**: 환생·초월 리셋 대상에서 마스터리 명시 제외. 리셋 후 트랙 XP/월드 파워 불변.
4. **자동 적립 훅**: 처치/스테이지·강화/세트·던전/탑·룬·펫·퀘스트/업적 record 경로에서 트랙 XP 적립.
5. **저장**: `UIdleSaveGame`에 `TArray<FMasterySaveEntry> Mastery` 추가, SaveVersion **12→13** 마이그레이션(구 세이브 = 전 트랙 0). 클라우드 payload 선택 필드(`worldPower`/`masteryLevels`, 미필수=v12 호환). 라운드트립 회귀안전.
6. **UI**: 숙련 패널(트랙 6행 — 레벨/XP바/현재 보너스) + 헤더 월드 파워(전투력 CP 옆). ko/en 로컬라이즈 `CsvIntegrity` 정합.
7. **테스트**: 클라 Automation(공식·서비스·적립 훅·능력치/경제 보너스 단조 증가·리셋 생존·v12→v13·CP 증가) + 서버 vitest(mastery 공식/save payload) + parity. UE Build/Automation + server-ci **GREEN**.

## 2. 범위 (In Scope)
### 2.1 공식 (character + backend)
`FMasteryFormula`/`mastery.ts` 미러 — XP 곡선·레벨 환산·전역 보너스(코어/크리/드롭/골드/EXP)·월드 파워. `Math.fround` 경계 매칭.
### 2.2 서비스 + 적립 (character)
`UMasteryService`(AchievementService 선례) — 트랙 누적 XP·레벨·전역 보너스·레벨업 델리게이트. GameInstance 보유 + 기존 record 훅에서 `AddXp`.
### 2.3 능력치/경제 적용 (character)
`RefreshDerivedStats:180` 코어 배수(곱)+크리(가산) 합류. 골드/드롭/EXP는 펫 보너스 경로 합류.
### 2.4 저장/마이그레이션 (character + backend) — DoD 5.
### 2.5 UI (designer)
숙련 패널 + 월드 파워 HUD + 로컬라이즈 ko/en.
### 2.6 밸런스 (balance)
XP/보너스 계수 근거 + 초기/중반/엔드 성장 예시 + 인플레 점검 문서.
### 2.7 테스트 (qa) — DoD 7.

## 3. 범위 외 (후속)
- **트랙별 로컬 보너스**(강화 성공률 보정 등 시스템별 심화) — V1.5 별도 PR(계획 Task 12).
- 숙련 전용 재화/상점, 마스터리 기반 콘텐츠 게이팅/랭킹.
- 서버 권위 마스터리(V1은 클라 + 서버 공식 미러).
- 숙련 단계 연출/그래프.

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 |
| --- | --- |
| character (메인) | MasteryTypes/FMasteryFormula, UMasteryService, GameInstance 연결·XP 훅, RefreshDerivedStats 코어/크리, 드롭/골드/EXP 경로, 세이브 v12→v13, 클라 Automation |
| backend | mastery.ts 공식 미러 + index.ts export + vitest, save.schema.ts payload 선택 필드 + save.test.ts |
| balance | XP/보너스 계수 근거 + 성장 단계 예시 + 인플레 점검 (`docs/planning/mastery-v1-balance-note.md`) |
| designer | 숙련 패널 + 월드 파워 HUD + 잠금/툴팁 + 로컬라이즈 ko/en + CsvIntegrity |
| qa | 시나리오(적립/리셋 생존/마이그레이션/parity/회귀) + 회귀 체크리스트 |
| (story/quest) | 해당 없음 (기존 RecordQuestProgress/RecordMonsterKilled 훅 재사용) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/balance/designer/qa) → PM 산출 게시 → [3] Claude TM 종합+fix → [4] Codex TM 종합+fix+PM 산출 게시 → [5] 검증(UE Automation 직접 구동, #68 교훈) → [N] **CI 그린 확정** + PM 종합 소견 + 머지. PM 자율([[feedback-autonomous-slices]], [[feedback-ci-before-merge]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 초월/업적 배수와 이중 계산 | 마스터리는 별도 곱(코어)+별도 가산(크리)만, transcend/achievement 항 불간섭 + parity |
| 무한 보너스 인플레 | 로그 체감 곡선(엔드 ~1.11배 수준) + balance 인플레 점검 문서 |
| XP 훅 누락(특정 시스템 미반영) | record 훅 일괄 삽입 + qa 단조 증가 검증 |
| 리셋 시 마스터리 소실 버그 | Rebirth/Transcend 리셋 블록에서 명시 제외(주석) + 리셋 생존 Automation |
| 서버↔클라 공식 drift | `Math.fround` 미러 + 경계 parity(transcend/CP 선례) |
| 세이브 v12→v13 회귀 | 누락 필드=0 마이그레이션 + 라운드트립 + 클라우드 payload 테스트 |
| V1 스코프 비대 | 트랙 로컬 보너스는 V1.5 분리, V1은 전역 보너스 우선 |

## 7. 후속
- 트랙별 로컬 보너스(V1.5), 숙련 전용 재화/상점, 마스터리 기반 게이팅/랭킹, 서버 권위 마스터리, 숙련 연출.
