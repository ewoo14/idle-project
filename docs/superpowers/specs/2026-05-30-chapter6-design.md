# 챕터6 콘텐츠 — 설계 문서

> 작성일: 2026-05-30 · 작성: PM/Claude · 분류: 콘텐츠 확장(#66/#70/#80 멀티챕터 일반화 재사용)
> 대상 PR: 단일 슬라이스 · 상태: PM 자율(사용자 "계속 진행")

## 0. 한 줄 요약
멀티챕터 일반화(#66)를 재사용해 **챕터6(스테이지 51~60)** 추가 — 스토리/약점/메인 퀘스트.
검증된 데이터 구동 패턴이라 저위험·**세이브 무변경**(#80 ch5와 동일).

## 1. 배경
- 챕터: ch1~ch5(#80), TotalChapters 5, StagesPerChapter 10(글로벌 1~50). [[project_content_richness]].
- 멀티챕터 일반화(#66): TotalChapters 상수 + 데이터(stages.ts/quests.ts/StageFormula 약점)만 확장하면 스테이지/엘리트/보스/HUD 자동.
- ch5(#80) "챕터 HUD 데이터 구동 → 신규 챕터 HUD 코드 0" 입증.

## 2. 핵심 결정 (PM)
| 항목 | 결정 |
| --- | --- |
| TotalChapters | 5 → **6** (글로벌 스테이지 51~60) |
| 엘리트/보스 | 6-5 미니보스(idx 55)/6-10 챕터보스(idx 60) — 기존 IsElite/IsBoss 규칙 자동 |
| 약점 | 스테이지 51~60 속성(5속성, Dark 가중 — ch5 패턴) |
| 스토리 | ch6 내러티브(스토리 바이블 + StoryText/CSV) — ch5 "심연 옥좌" 후속 서사 |
| 퀘스트 | ch6 메인 퀘스트 6개(DefinitionParity 확장, 체인) |
| 세이브 | **무변경**(멀티챕터 일반화 재사용, 마이그레이션 불필요) |

## 3. 통합 지점 (5-team)
| 파트 | 작업 |
| --- | --- |
| character | `StageService::TotalChapters` 5→6, 서버 `stages.ts` 스테이지 51~60(이름/적/보상 스케일 ch5 연속), `StageFormula` 약점 51~60, 클라/서버 stage parity. 검증 테스트(ch6 스테이지/엘리트/보스/약점). backend(stages.ts) 흡수. |
| story | 스토리 바이블 ch6 + StoryText/Story CSV(ch6 내러티브). |
| quest | ch6 메인 퀘스트 6개(서버 quests.ts + 클라 QuestService, DefinitionParity, 체인). |
| designer | 챕터6/약점 HUD 자동 표시 확인 + ko/en(챕터6 이름/스토리/퀘스트 로컬라이즈) + CsvIntegrity. 표준 jumbo. |
| balance | `docs/planning/chapter6-balance-note.md`: ch6 스테이지 스케일(ch5 연속 곡선)·약점·페이싱. 세이브 무변경. |
| qa | ch6 스테이지 진행/엘리트/보스/약점/퀘스트 체인/parity. jumbo+게이트. |

## 4. 스코프
**In:** TotalChapters 6 + ch6 10스테이지(약점/엘리트/보스) + 스토리 + 메인 퀘스트 6 + parity. **세이브 무변경**.
**Out:** 신규 기믹/속성(ch6 전용 기믹은 후속), 챕터7+, 사이드 퀘스트.

## 5. 리스크
| 리스크 | 완화 |
| --- | --- |
| 스테이지 스케일 파워크리프 | ch5 연속 곡선 유지(검증된 스케일), median 재측정. |
| 클라↔서버 stage/quest parity | stages.ts/quests.ts ↔ StageService/QuestService 미러, DefinitionParity 테스트. |
| 약점 default None 탈출 | 51~60 약점 명시(#70 교훈). |
| 세이브 호환 | 멀티챕터 일반화 — 진행도 글로벌 idx 기반, 마이그레이션 불필요(#80 입증). |
