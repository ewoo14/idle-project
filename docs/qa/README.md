# docs/qa/ — QA 산출물

> QA 시나리오, 회귀 체크리스트, 버그 리포트 템플릿.

## 구조 (점진 확장)

| 파일/디렉터리 | 내용 |
| --- | --- |
| `scenarios/` | 마일스톤별 시나리오 (Given/When/Then) |
| `regression-checklist.md` | 매 PR 마다 점검할 회귀 체크 |
| `bug-report-template.md` | 버그 리포트 양식 (이슈 템플릿과 별개) |

## 시나리오 작성 원칙

- 한글 Given / When / Then
- 정상 + 엣지 + 실패 회복 ≥ 3개
- 자동화 가능 항목은 테스트 코드 링크 추가
- 마일스톤 별 디렉터리: `scenarios/M1-...`, `scenarios/M2-...`

QA 슬라이스 시작 (PR #3 이후) 시 본격 작성합니다.
