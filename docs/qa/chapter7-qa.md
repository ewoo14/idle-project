# 챕터7 QA 검증 노트

> 작성 2026-05-30 · 검증: PM · **세이브 무변경**(SaveVer 25)

## 1. 게이트
| 게이트 | 범위 | 결과 |
| --- | --- | --- |
| 서버 vitest | stages 23·quest 53 (전체) | **GREEN** |
| 서버 lint/build | 전체 | **GREEN** |
| UE jumbo 빌드 | 전체(ODR) | **Succeeded** |
| UE Automation | Stage/Quest/Localization(CsvIntegrity)/UI(HUD 포함)/GameCore/Combat | **167/0** |

## 2. 커버리지
- 스테이지 61~70·7-5 엘리트/7-10 보스·스케일 ch6 연속·글로벌 idx. 약점 61~70 비-None(Dark 가중) 클라↔서버 parity.
- ch7 메인 6개 체인(ch6 finale→ch7 순차), DefinitionParity 47→53.
- 로컬라이즈 Story 26+StoryText 23+Quest 6키 ko/en 게이트. HUD 데이터 구동 자동(코드 0).
- **세이브 무변경**(글로벌 idx). #97 교훈 반영: 게이트에 UI.HUD 포함.

## 3. 후속
- ch7 전용 기믹·챕터8+ 후속. PR 본문 ch7 PIE 스크린샷.
