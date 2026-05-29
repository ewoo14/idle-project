# PR #80 기획서 — 챕터5 콘텐츠 확장

> **PM 자율 진행 (Claude 에이전트 구현팀, Codex 6/1 복구까지)**. #66/#70 멀티챕터 일반화 패턴 클론 — 진행축 확장. TotalChapters 4→5 + ch5 10스테이지(글로벌 41~50) + 약점 + 스토리/퀘스트. **멀티챕터 일반화로 세이브 마이그레이션 불필요**(저위험). client + server 5-team.

## 1. 목표 / DoD
저위험 콘텐츠 확장(검증된 #70 패턴). 진행 가능 챕터 +1.

### DoD
1. `UStageService::TotalChapters` 4→5(StageService.h). ch5 스테이지(글로벌 41~50, 5-5 미니보스/5-10 챕터보스). StageServiceTests "4→5" 기대 갱신.
2. 약점: `getStageWeakElement`(서버 stage.ts) + 클라 미러에 41~50 케이스 추가(5속성 Fire/Ice/Lightning/Holy/Dark 분포, default None 탈출). 클라↔서버 parity.
3. ch5 스토리바이블(`docs/planning/06-story-bible.md`) + Story/StoryText CSV(ko/en) 키. 메인 퀘스트 ch5(DefinitionParity 클라↔서버 quests.ts).
4. 챕터5 HUD 표시(챕터 라벨/전환). ko/en CsvIntegrity.
5. **세이브 변경 없음**(SaveVersion 15 유지, 멀티챕터 일반화 재사용). 기존 챕터1~4 회귀 없음.
6. 테스트: 클라 Automation(스테이지/약점/챕터 진행) + 서버 vitest(stage/quests parity). CI GREEN + **표준 jumbo 빌드 PM 검증**.

## 2. 작업 분배 (Claude 서브에이전트, claude(<role>):)
| 파트 | 작업 |
| --- | --- |
| character (메인) | TotalChapters 4→5, ch5 스테이지/약점 41~50(클라+서버 stage.ts), StageServiceTests 갱신, 챕터 진행 Automation |
| story | ch5 스토리바이블 + Story/StoryText CSV ko/en |
| quest | ch5 메인 퀘스트(클라 BuildDefaultDefinitions=서버 quests.ts DefinitionParity) |
| designer | 챕터5 HUD 라벨/전환 + ko/en + CsvIntegrity |
| balance | 스케일 미변경 확인(#68/#70 교훈, balance-sim median 무변동) |

## 3. 범위 외
신규 속성/보스 기믹(#83 Dark 심화 별도), 챕터6+, ch5 전용 드롭 테이블.

## 4. 워크플로우 v3
Claude 서브에이전트 구현 → PM 리뷰/통합. [5] **표준 jumbo(unity) 빌드 PM 직접 검증**([[reference-ue-headless-verify]] §1-b). 머지 전 CI 그린.

## 5. 리스크
| 리스크 | 완화 |
| --- | --- |
| 약점 41~50 클라↔서버 drift | parity 테스트 |
| 챕터 경계/보스 진행 회귀 | 기존 멀티챕터 일반화 재사용 + StageServiceTests 갱신 |
| 스케일 인플레 | balance 미변경(#68/#70 교훈) + balance-sim 무변동 |
| jumbo ODR | 신규 익명 헬퍼 동명 grep + PM 표준 jumbo 빌드 |
