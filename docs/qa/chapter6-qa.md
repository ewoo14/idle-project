# 챕터6 QA 검증 노트

> 작성 2026-05-30 · 검증: PM · **세이브 무변경**(SaveVer 22)

## 1. 자동 검증 게이트

| 게이트 | 범위 | 결과 |
| --- | --- | --- |
| 서버 vitest (`src/core/data`, `src/modules/quest`) | stages 51~60·약점·ch6 퀘스트 parity/체인 | **stages 19 / quest 48** (전체 821 GREEN) |
| 서버 lint/build | 전체 | **GREEN** |
| UE jumbo 빌드 | 전체(ODR) | **Succeeded** |
| UE Automation | Stage/Quest/Localization(CsvIntegrity)/UI/GameCore/Combat | **163/0** |

## 2. 시나리오 커버리지

- **스테이지**: ch6 글로벌 51~60 존재, 6-5 엘리트/6-10 보스, 스케일 ch5 연속, 글로벌 idx 정합.
- **약점**: 51~60 비-None(Dark 가중), 클라 StageFormula ↔ 서버 stage.ts parity.
- **퀘스트**: ch6 메인 6개 체인(ch5 finale→ch6 순차), DefinitionParity 41→47(클라↔서버 1:1), objective/보상 검증.
- **로컬라이즈**: Story 26 + StoryText 23 + Quest 6키 ko/en CsvIntegrity 게이트.
- **HUD**: 챕터6/스테이지/약점 데이터 구동 자동 표시(신규 코드 0).

## 3. 한계 / 후속

- ch6 전용 신규 기믹/속성 없음(콘텐츠 확장 슬라이스). 챕터7+/사이드 퀘스트는 후속.
- `tools/balance-sim` TOTAL_CHAPTERS 갱신(ch5/ch6 미반영) 별도 후속.
- PR 본문 ch6 스테이지/스토리/퀘스트 PIE 스크린샷.
