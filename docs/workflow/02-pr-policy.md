# PR 정책

## 1. 브랜치 명명

| 패턴 | 용도 |
| --- | --- |
| `plan/<번호>-<주제>` | PM 기획 PR |
| `feat/<번호>-<주제>` | 기능 슬라이스 |
| `fix/<번호>-<주제>` | 버그 수정 |
| `chore/<주제>` | 잡무 |
| `docs/<주제>` | 문서 전용 |
| `hotfix/<주제>` | 긴급 운영 패치 |

## 2. PR 제목

```
<type>: <한 줄 한글 요약 (50자 이내)>
```

예:
- `feat: 자동 전투 코어 (M1-슬라이스1)`
- `fix: 강화 +12 확률 표시 오류`

## 3. PR 본문

`.github/PULL_REQUEST_TEMPLATE.md` 가 자동 적용. 7개 섹션 모두 채워야 함.

## 4. 라벨

| 라벨 | 의미 |
| --- | --- |
| `type/feat`, `type/fix`, `type/chore`, `type/docs` | 변경 종류 |
| `step/2~step/8` | 워크플로우 진행 단계 |
| `part/designer`, `part/story`, `part/quest`, `part/character`, `part/balance`, `part/backend`, `part/qa` | 변경 파트 |
| `priority/p0` (긴급) ~ `priority/p3` (저) | 우선순위 |
| `risk/high|med|low` | 회귀 리스크 |
| `build/client` | UE5 CI 빌드 트리거 |
| `ready/merge` | PM 머지 직전 표시 |

## 5. 커밋 메시지

- 제목: 영문 prefix + 한글 요약 (50자)
  - `feat: 인벤토리 8슬롯 UI 추가`
  - `codex(backend): 회원가입 핸들러 구현`
  - `tm: 1차 종합 리뷰 반영 fix`
- 본문: 한글, what/why/how. 관련: `#123`.

## 6. 머지 방식

- 기본: **Squash merge** (히스토리 단순화).
- 머지 메시지: PR 제목 + 본문 요약 자동 첨부.
- 머지 후: 브랜치 자동 삭제 (`--delete-branch`).

## 7. 금지

- `git push --force origin main`
- main 으로 직접 push (PR 우회)
- 시크릿 (`.env`, 키) 커밋
- 한글 외 리뷰 (영문 코멘트는 즉시 재작성)
- 워크플로우 단계 건너뛰기

## 8. 리뷰 응답 시간 가이드 (인간 개입 시)

| 우선순위 | 응답 시한 |
| --- | --- |
| p0 | 4시간 |
| p1 | 24시간 |
| p2 | 72시간 |
| p3 | 1주 |

에이전트 작업은 즉시 / 세션 내.
