# PR #56 기획서 — 퀘스트 대폭 확장 (콘텐츠 볼륨)

> **사용자 지시([[project-content-richness]]): 퀘스트를 최대한 많고 다양하게.** 현재 메인 5(ch1)+일일 3 = 8개, 목표 4종(KillMonster/ClearMap/ClaimOffline/Enhance)뿐. 챕터2(#37)·환생(#46)·초월(#47)·탑(#50)·펫(#42)·상점(#38)·강화(#44) 등 풍부한 시스템이 있으나 퀘스트가 이를 안 다룸. **메인 퀘스트 다수(ch1+ch2) + 주간 퀘스트 신규 + 목표 종류 대폭 다양화 + 한글 로컬라이즈**. server + client 멀티시스템(+designer/qa).

## 1. 목표 / DoD
플레이어가 다양한 행위(처치/보스/클리어/강화/환생/초월/탑/펫/상점/레벨/골드 등)에 대해 풍부한 메인·일일·주간 퀘스트를 진행·수령한다. 한글 노출.

### DoD 검증
1. **퀘스트 종류 확장**: `EQuestType` {Main, Daily} → **+ Weekly**(서버 type enum "weekly" 동기). 메인 퀘스트 ch1 보강 + **ch2 메인 라인 추가**(#37 챕터2), 일일 다양화, 주간 신규(큰 보상). 총 수십 개(많고 다양하게).
2. **목표 다양화**: `EQuestObjective` {KillMonster, ClearMap, ClaimOffline, Enhance} → **+ DefeatBoss, Rebirth, Transcend, ClimbTower, ReachLevel, SpendGold, RollGearShop, FeedPet** 등(서버 objective enum 동기). 각 목표가 실제 게임 경로에서 RecordQuestProgress 훅으로 진행.
3. **진행 훅**: 신규 목표를 기존 이벤트(보스 처치/환생/초월/ClimbTower/레벨업/골드 소비/상점 뽑기/펫 먹이기)에서 `RecordQuestProgress(objective, amount)` 호출. 업적(#55) RecordMetric 과 공존(중복 호출 OK, 별개 시스템).
4. **로컬라이즈(한글)**: 퀘스트 제목/설명 ko/en CSV(Quest.csv 또는 기존 Story/UI CSV). 현재 영문 하드코딩 제목 → 로컬라이즈 키. CsvIntegrity ko↔en 정합. (Korean Docs Rule.)
5. **클라/서버 미러**: 클라 BuildDefaultDefinitions ↔ 서버 quest 정의(quest.data/service) 동기 — questId/type/objective/targetCount/reward/prerequisite/chapterMapId DefinitionParity 테스트. 서버 schema enum(type/objective) 확장.
6. **일일/주간 리셋**: 일일=UTC 날짜, 주간=ISO 주(또는 7일 경계) 리셋. 기존 ResetDailyQuestsIfNeeded 패턴 확장(주간 리셋 추가). 저장(#52~#54)·클라우드 영속(quest 상태 이미 #53 저장 대상).
7. **테스트**: 클라 Automation(정의 다수·prerequisite 체인·신규 목표 진행·주간 리셋·DefinitionParity) + 서버 vitest(정의·schema enum·진행/수령·주간) + CsvIntegrity. UE 빌드/Automation + 서버 build/test/lint GREEN.

## 2. 범위 (In Scope)
### 2.1 정의/열거 (character + backend 미러)
- `EQuestType` +Weekly, `EQuestObjective` +다수. 클라 BuildDefaultDefinitions 대폭 확장(메인 ch1+ch2 다수, 일일 다양, 주간 신규). 서버 quest 정의 + schema enum 동기 + DefinitionParity.
### 2.2 진행/서비스 (character)
- UQuestService: 신규 목표 진행 처리, 주간 리셋(ResetWeeklyQuestsIfNeeded), 주간 상태 보관. UIdleGameInstance: 보스/환생/초월/탑/레벨/골드소비/상점/펫 경로에서 RecordQuestProgress 훅 추가.
### 2.3 UI (디자이너)
- 퀘스트 로그 HUD 확장: Main/Daily/Weekly 탭 또는 섹션 구분, 다수 항목 스크롤/정리, 주간 표시. 로컬라이즈 제목/설명.
### 2.4 테스트 (character/backend) — 위 DoD 7.

## 3. 범위 외 (후속)
- 퀘스트 분기/선택지, 스토리 컷신, 업적 연동 보상(현 별개), 길드/협동 퀘스트, 이벤트 한정 퀘스트.

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | EQuestType/Objective 확장 + BuildDefaultDefinitions 대폭 확장 + UQuestService 주간/신규목표 + GameInstance 훅 + 로컬라이즈 연동 + Automation | ✅ 메인 (`character`) |
| 백엔드 | 서버 quest 정의/schema enum 동기 + 주간 + vitest/parity | ✅ 보조 (`backend`) |
| 디자이너 | 퀘스트 로그 HUD(Main/Daily/Weekly) + 로컬라이즈 ko/en | ✅ 보조 (`designer`) |
| QA | 메인/일일/주간 진행·수령·리셋·다양 목표 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/designer/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증(UE+서버) → [N] **CI 그린 확정**(server-ci 포함) + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]]). [[project-content-richness]].

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 클라/서버 정의 불일치 | DefinitionParity 테스트(questId/type/objective/targetCount/reward) + schema enum 동기 |
| 신규 목표 진행 훅 누락 | 보스/환생/초월/탑/레벨/골드/상점/펫 전 경로 RecordQuestProgress + Automation 진행 케이스 |
| 영문 제목 잔존(로컬 위반) | 전 퀘스트 ko/en CSV 로컬라이즈 + CsvIntegrity |
| 주간 리셋 경계 오류 | ISO 주/7일 경계 결정적 함수 + 저장 날짜 비교 + Automation 리셋 케이스 |
| 기존 퀘스트/저장 회귀 | 기존 questId/진행 보존, 저장(#53) 호환, prerequisite 체인 유지 |
| 다수 항목 HUD 성능/가독성 | 탭/섹션 구분 + 스크롤, 활성 퀘스트만 표시 |

## 7. 후속 (콘텐츠 확장 계속 — 사용자 지시)
- **다음**: 캐릭터 직업 추가(현 5직업→확장), 아이템 종류 확대(ID/세트/고유옵션). 이후 퀘스트 분기/이벤트 퀘스트.
