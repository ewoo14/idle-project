# PR #70 기획서 — 챕터4 콘텐츠 (진행축 확장)

> **PM 자율 진행**(사용자 "PM 자율로 계속"). #66(챕터3 + 전역 10스테이지) 위 진행축 확장. TotalChapters 3→4, ch4 10스테이지(미니보스/챕터보스) + 신규 약점 + ch4 스토리/퀘스트. 멀티챕터 일반화(#66)로 대부분 자동. 콘텐츠 볼륨([[project-content-richness]]). client + server 멀티시스템(7파트).

## 1. 목표 / DoD
챕터가 4개(각 10스테이지)로 확장되고, ch4(글로벌 idx 31~40) 약점·미니보스·챕터보스·스토리·퀘스트가 연결된다. 기존 스테이지(#31/#37/#66)·전투/상성(#30/#66 Dark)·저장(#52)·레어도(#65)와 정합.

### DoD 검증
1. **스테이지 확장**: `TotalChapters` 3→4(StagesPerChapter 10 유지). ch4 4-1~4-10(글로벌 idx 31~40), 4-5 미니보스/4-10 챕터보스. 3-10 보스 → 4-1 진입, 4-10 보스 → 최종 동결(기존 OnChapterBossDefeated 경로). 클라 `StageService`/`StageFormula` + 서버 `stage`/`stages` 미러.
2. **약점**: `GetStageWeakElement` idx 31~40 약점 5속성(Fire/Ice/Lightning/Holy/Dark) 조합 추가(현재 default None). 클라/서버 parity.
3. **신규 보스**: 4-10 챕터보스(스토리 ch4). 보스 페이즈(#35) 자동.
4. **스토리/퀘스트**: `06-story-bible.md` ch4 본문(스토리 §5.3 "차원 군주의 그림자" 이후 전개). 메인 퀘스트 ch4 신규(#56 ch1 7+ch2 5+ch3 → ch4 추가) + 로컬라이즈 ko/en.
5. **저장**: 기존 StageChapter/StageStage 저장(#52) 그대로 ch4 수용(TotalChapters만 증가, 마이그레이션 불필요·회귀안전). SaveVersion bump 불필요.
6. **레어도 연계(자동)**: ch4 고레벨 → #65 유니크/초월 드롭 자동(drop 곡선 레벨 비례).
7. **테스트**: 클라 Automation(ch4 진행·idx 31~40 약점·미니보스/보스·챕터 전환·ch4 퀘스트) + 서버 vitest(stage/stages/quests ch4 parity). UE Build/Automation + 서버 build/lint/test **GREEN**, server-ci 그린.

## 2. 범위 (In Scope)
### 2.1 스테이지 (character + backend)
TotalChapters 4 + ch4 약점 31~40 + 미니보스/보스. 서버 stage/stages 미러.
### 2.2 스토리 (story) / 퀘스트 (quest)
스토리바이블 ch4 + 메인 퀘스트 ch4 + 서버 quests 미러.
### 2.3 UI (designer)
챕터4 HUD 표시/전환 + 로컬라이즈 ko/en.
### 2.4 밸런스 (balance)
ch4(idx 31~40) 스케일/보상 + 기존 페이싱 영향 시뮬.
### 2.5 테스트 — DoD 7.

## 3. 범위 외 (후속)
- 챕터5+ · ch4 전용 신규 시스템 · 챕터 선택/리플레이.

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 |
| --- | --- |
| character (메인) | TotalChapters 4 + 약점 31~40 + 미니보스/보스 + 서버 stage/stages 미러 + Automation |
| story | 06-story-bible ch4 본문 |
| quest | 메인 퀘스트 ch4 + 서버 quests 미러 |
| designer | 챕터4 HUD/전환 + 로컬라이즈 ko/en |
| balance | ch4 스케일/보상 + 페이싱 시뮬 |
| (backend/qa) | character 흡수, [3] Claude TM parity·커버리지 |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+story/quest/designer/balance) → [3] Claude TM → [4] fix(필요시) → [5] 검증(UE Automation 직접, #68 교훈) → [N] **CI 그린 확정** + PM 종합 소견 + 머지. PM 자율.

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| ch4 약점 idx 31~40 누락(default None) | GetStageWeakElement 31~40 추가 + Automation 경계 |
| 글로벌 idx/챕터 전환 오류 | 멀티챕터 일반화(#66) 재사용 + Automation(3-10→4-1, 4-10 동결) |
| 클라/서버 ch4 불일치 | stage/stages/quests parity + DefinitionParity |
| 저장 회귀(기존 진행) | TotalChapters만 증가, StageStage/Chapter 보존, 마이그레이션 불필요 + 라운드트립 |
| balance 페이싱(스테이지 40개) | ch4 스케일 시뮬, median 영향 점검 |

## 7. 후속
- 챕터5, ch4 전용 콘텐츠, 챕터 리플레이.
