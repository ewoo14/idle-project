# PR #24 기획서 — 로컬라이즈 한/영 V1 (M6)

> M6 자율 항목. 한국어 하드코딩 UI 라벨/스토리 텍스트를 **문자열 테이블(ko/en)** 로 외부화하고 영문 번역을 추가, **언어 전환** 지원. Steam EA 영어권 대응. 사운드/Steam SDK 는 외부 의존 별도 후속.

## 1. 목표 / DoD
HUD/스토리 텍스트가 문자열 테이블(ko/en)을 참조하고, 언어 설정으로 한/영 전환 시 UI/스토리가 해당 언어로 표시된다.

### DoD 검증
1. UE 문자열 테이블(또는 CSV 로컬라이즈) ko/en — UI(스킬/퀘스트/오프라인/환생/펫/직업 HUD 라벨) + Story.
2. 하드코딩 한글 라벨 → 테이블 키 참조로 치환(핵심 HUD).
3. 언어 설정/토글 → 컬처(ko/en) 전환 시 표시 언어 변경.
4. 영문 번역 키 100% 채움(미번역 키 없음).
5. UE 빌드/Automation(로컬라이즈 조회 모델) GREEN + CSV 검증(키 누락/중복 없음).

## 2. 범위 (In Scope)
### 2.1 i18n 인프라 (메인, 클라)
- `client/Content/Localization/Game/{ko,en}/` 문자열 테이블/CSV — 네임스페이스: UI, Story, Skill, Quest. (기존 ko/StoryText.csv 확장 + en 추가.)
- HUD 라벨 헬퍼: `LOCTEXT`/StringTable 참조 or CSV 키 조회. 하드코딩 `FText::FromString("한글")` → 키 참조로 치환(IdleHUD/IdleHUDWidget 핵심 라벨).
- 언어 설정: UIdleGameInstance(또는 Settings) 에 Language(ko/en) + SetCulture 적용 + 저장.
### 2.2 번역 (보조)
- 기존 ko 텍스트 전부 en 번역(UI 라벨 + Story + Skill/Quest 표시명). V1 초안(자연스러운 게임 영어, 후속 교정 여지).
### 2.3 검증
- CSV 키 정합(ko↔en 키 일치, 미번역 0), 로컬라이즈 조회 Automation.

## 3. 범위 외
- 사운드/보이스 로컬라이즈, Steam 스토어 페이지 번역(외부).
- 일본어 등 3언어+(후속). 폰트(영문은 Pretendard/기본으로 충분, CJK 외 추가 폰트 후속).
- 완벽한 전문 번역(V1 초안).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 디자이너 (메인) | 문자열 테이블 인프라 + HUD 라벨 테이블화 + 언어 전환 + Automation | ✅ 메인 (`designer`) |
| 스토리 (보조) | Story/UI/Skill/Quest en 번역 | ✅ 보조 (`story`) |
| QA | 키 정합/미번역/전환 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex designer 메인(+story/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정** + 머지. 사용자 검토 차후(영문 초안 교정 여지)([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 하드코딩 라벨 전수 치환 부담 | V1 은 핵심 HUD + 신규는 테이블, 잔여는 후속 |
| 영문 번역 품질 | V1 초안 명시, 사용자/후속 교정 |
| 키 누락/중복 | CSV 정합 테스트로 게이트 |

## 7. 후속
- 3언어+ 확장, 전문 번역 교정, 사운드/보이스 로컬, Steam 스토어 번역, 폰트 확장.
