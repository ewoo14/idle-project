# 챕터7 콘텐츠 — 설계 문서

> 작성일: 2026-05-30 · 작성: PM/Claude · 분류: 콘텐츠 확장(#66/#80/#93 멀티챕터 일반화 재사용)
> 대상 PR: 단일 슬라이스 · 상태: PM 자율(사용자 "계속")

## 0. 한 줄 요약
멀티챕터 일반화(#66)를 재사용해 **챕터7(스테이지 61~70)** 추가 — 스토리/약점/메인 퀘스트.
검증된 데이터 구동 패턴(#93 ch6 동형)·저위험·**세이브 무변경**.

## 1. 배경
- 챕터: ch1~ch6(#93), TotalChapters 6, StagesPerChapter 10(글로벌 1~60). [[project_content_richness]].
- 멀티챕터 일반화(#66): TotalChapters 상수 + 데이터(stages.ts/quests.ts/약점/StoryText) 확장만으로 자동.
- ch6(#93) "무너지는 근원의 균열" 후속 서사.

## 2. 핵심 결정 (PM)
| 항목 | 결정 |
| --- | --- |
| TotalChapters | 6 → **7** (글로벌 스테이지 61~70) |
| 엘리트/보스 | 7-5 미니보스(idx 65)/7-10 챕터보스(idx 70) — 기존 IsElite/IsBoss 자동 |
| 약점 | 스테이지 61~70 속성(5속성, Dark 가중 ch6 패턴) |
| 스토리 | ch7 내러티브(ch6 붕괴 후속 — 균열 너머/새 차원 등) |
| 퀘스트 | ch7 메인 퀘스트 6개(DefinitionParity 확장, 체인) |
| 세이브 | **무변경**(멀티챕터 일반화 재사용, 글로벌 idx, 마이그레이션 불필요) |

## 3. 통합 지점 (5-team)
| 파트 | 작업 |
| --- | --- |
| character+backend | `StageService::TotalChapters` 6→7, 서버 `stages.ts` 스테이지 61~70(ch6 연속 스케일), 약점 61~70(클라 StageFormula↔서버 stage.ts parity), 검증 테스트. |
| story | 스토리 바이블 ch7 + StoryText/Story CSV(ch7 내러티브). ko/en. |
| quest | 서버 quests.ts + 클라 QuestService ch7 메인 퀘스트 6개(체인, DefinitionParity 47→53). |
| designer | LocalizationTests ch7 필수키 게이트 + HUD 자동 표시 확인(코드 0) + ko/en CsvIntegrity. 표준 jumbo. |
| balance/qa | `docs/planning/chapter7-balance-note.md`(스케일·약점) + QA E2E(ch7 진행/엘리트/보스/약점/퀘스트 체인/parity). jumbo+게이트. |

## 4. 스코프
**In:** TotalChapters 7 + ch7 10스테이지(약점/엘리트/보스) + 스토리 + 메인 퀘스트 6 + parity. **세이브 무변경**.
**Out:** ch7 전용 신규 기믹, 챕터8+, 사이드 퀘스트.

## 5. 리스크
| 리스크 | 완화 |
| --- | --- |
| 스테이지 스케일 파워크리프 | ch6 연속 곡선(검증 스케일), median 재측정. |
| 클라↔서버 stage/quest parity | stages.ts/quests.ts ↔ StageService/QuestService 미러, DefinitionParity 테스트. |
| 약점 default None 탈출 | 61~70 약점 명시(#70 교훈). |
| LocalizationTests 게이트 | ch7 필수키 추가(#93 ch6 패턴 — 실제 커밋 키와 일치). |
| 세이브 호환 | 멀티챕터 일반화(글로벌 idx, #93 입증), SaveVer 무변경. |
