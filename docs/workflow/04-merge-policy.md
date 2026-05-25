# 머지 정책

## 1. 머지 조건 (전부 필수)

- [ ] **TM 2차 종합 리뷰** = "fix 없음" 판정
- [ ] **GitHub Actions CI 모두 통과** (필수 워크플로우)
- [ ] **PM 종합 소견 코멘트** 게시 완료
- [ ] **블로커 / 중요 리뷰 모두 해결** (인용 + 해결 commit 링크)
- [ ] **문서 동기화** (필요한 경우)
- [ ] **컨플릭트 없음** (base 와 충돌 없음)

위 모두를 만족하면 **PM 만** 머지 가능.

## 2. 필수 CI 워크플로우

| 워크플로우 | 트리거 | 머지 차단? |
| --- | --- | --- |
| `docs-lint` | `**/*.md` 변경 PR | 경고만 (M2 부터 차단) |
| `server-ci` | `server/**` 변경 PR | 차단 |
| `client-ci` | `client/**` 변경 PR + `build/client` 라벨 | 차단 (M1 부터) |

## 3. 머지 방식

- 기본: **Squash merge** (`gh pr merge --squash --delete-branch`)
- 예외: `chore/big-refactor` 처럼 커밋 히스토리 보존 가치 있는 경우 PM 판단으로 **Rebase merge**
- Merge commit 사용 금지

## 4. 머지 후 절차

1. main 으로 자동 push (squash 결과)
2. 브랜치 자동 삭제
3. PM 이 다음 PR 진행 (마일스톤 순서)
4. 영향이 큰 머지(예: M1 머지)는 Discussion 또는 별도 알림에 한글 릴리즈 노트 등재

## 5. 롤백

- 머지 후 30분 이내 critical 결함 발견 시 PM 단독 결정으로 revert PR 즉시 생성.
- revert PR 도 워크플로우 적용하나, `priority/p0` 라벨로 단계 시간 단축.

## 6. CI 실패 시

- TM 이 실패 원인 코멘트 → Codex 재호출 → fix → 재실행.
- 일시적 인프라 장애로 의심 시 PM 이 재실행 (`gh run rerun`).
- 3회 연속 실패 시 PM 이 별도 디버깅 PR 분리.

## 7. 핫픽스

- 운영 장애 시:
  1. PM 이 `hotfix/<주제>` 브랜치 → 최소 fix 커밋
  2. CI 만 통과 시 PM 단독 머지 가능 (워크플로우 단계 건너뜀 허용)
  3. 24시간 이내 사후 리뷰 PR 필수 — 정식 워크플로우로 검증
