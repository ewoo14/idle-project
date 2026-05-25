# 멀티 에이전트 PR 루프 v3 — 진영별 리뷰+fix 일체 ping-pong

> 2026-05-25 사용자 명시로 v2 → v3 전환. PR #6 (자동 전투 V1) 부터 정식 적용.
> v2 (이전): TM 종합 → PM 통합 → Codex fix 분리. v3 (현재): **각 진영이 리뷰 + fix 일체** 로 ping-pong.

---

## 1. 단계 정의

### [1] PM 기획 (Plan)
- 주체: **PM (Claude 메인 세션)**
- 산출물:
  - 기획서 (`docs/planning/slices/NN-...md`)
  - PR 생성 (`plan/NN-주제` 브랜치)
  - PR 본문 7개 섹션 + 워크플로우 v3 체크리스트
  - 범위 / DoD / 7파트 분배 / 일정 / 리스크 명시
- 종료 조건: PR 이 열림.

### [2] Codex 개발 + PM 산출 게시 (Implement + Publish)
- 주체: **Codex CLI 7파트** (디자이너 / 스토리 / 퀘스트 / 캐릭터·아이템·능력치 / 밸런스 / 백엔드·DB / QA)
- 호출 방식: 메인 (가장 큰 작업) + 보조 (다른 파트 합동) 분할 가능
- 산출물:
  - PR 브랜치 commits (`codex(<role>): ...` prefix)
  - 푸시 완료
- **단계 [2] 직후 의무**: **PM 이 Codex stdout 결과를 PR 코멘트로 게시**
  - 헤더: `## Codex <역할> 산출 (단계 [2])`
  - 내용: 호출 정보 / commit 목록 / 주요 구현 / 자기 검증 / 알려진 한계
  - 사유: GitHub PR 영구 기록 필요 (Codex stdout 만으로는 보이지 않음)
- 종료 조건: 모든 Codex 호출 완료 + 모든 산출 PR 코멘트 게시.

### [3] Claude 리뷰 + fix (Review + Fix)
- 주체: **Claude TM** (메인 세션 또는 sub-agent)
- 산출물:
  - **Claude TM 종합 1개 코멘트** (개별 7파트 게시 금지)
    - §1 7파트 요약 표 (블로커/중요/권장/질문/칭찬)
    - §2 핵심 지적 (파트별 1~3건, 태그 + 근거 인용)
    - §3 fix 우선순위
    - §4 다음 단계 (Codex 위임 항목 명시)
  - **Claude/PM 직접 처리 가능 fix commit**:
    - 예: 워크플로우 docs 갱신, 작은 코드 정정, 문서 정합성
    - 큰 코드 fix 는 [4] Codex 에 위임
- 종료 조건: 종합 코멘트 게시 + Claude/PM fix commit 완료.

### [4] Codex 리뷰 + fix + PM 산출 게시 (Codex Review + Fix + Publish)
- 주체: **Codex CLI (Codex TM 역할 + fix 에이전트)**
- 호출 task: Claude TM 종합 검토 + 자체 추가 검토 + fix commit
- 산출물:
  - **Codex TM 종합 1개 코멘트** (Claude TM 비교 + 추가 발견)
  - **Codex fix commit** (서버/클라이언트 코드 fix)
- **단계 [4] 직후 의무**: **PM 이 Codex fix 산출 정리 → PR 코멘트 게시**
  - 헤더: `## Codex fix 산출 (단계 [4])`
- 종료 조건: 종합 코멘트 + fix commit + 산출 PR 코멘트.

### [5] Claude 검증 리뷰 (Claude Validation Review)
- 주체: **Claude TM**
- 산출물: Claude TM 검증 1개 코멘트
  - fix 항목별 검증 결과 표 (✅/❌)
  - 신규 발견 이슈
  - CI / 테스트 상태
  - 판정: **fix 없음** / **fix 있음**
- 분기:
  - **fix 없음** → 단계 [N]
  - **fix 있음** → 단계 [3] 또는 [4] 로 루프 (라운드 번호 증가)
- 종료 조건: 판정 코멘트 게시.

### (...) 라운드 반복
- 같은 PR 에서 [3] → [4] → [5] 사이클이 라운드 단위로 반복 가능
- 라운드별 코멘트 헤더에 라운드 번호 명시 (예: `## Claude TM 종합 리뷰 (3차 — 라운드 2)`)

### [N] PM 종합 + 머지 (Final)
- 주체: **PM**
- 사전 조건:
  - 최신 [5] Claude 검증 = "fix 없음"
  - GitHub Actions CI 모두 통과 (녹색)
- 산출물:
  - **PM 종합 소견** 코멘트:
    - 본 PR 달성 / 단계별 진행 / 양 진영 발견 요약 / 후속 PR 위임
  - 머지 실행: `gh pr merge <N> --squash --delete-branch`
- 종료 조건: main 에 머지, 브랜치 삭제, 다음 PR 진행.

---

## 2. 흐름도 (v3)

```
   ┌──────────────────────────┐
   │ [1] PM 기획 → PR         │
   └────────────┬─────────────┘
                ▼
   ┌──────────────────────────────────────┐
   │ [2] Codex 개발 + PM 산출 PR 코멘트   │
   └────────────┬─────────────────────────┘
                ▼
   ┌──────────────────────────────────────┐
   │ [3] Claude 리뷰 + fix                │ ◀──┐
   │  · Claude TM 종합 코멘트              │   │
   │  · 직접 처리 가능 fix commit          │   │
   └────────────┬─────────────────────────┘   │
                ▼                              │
   ┌──────────────────────────────────────┐   │
   │ [4] Codex 리뷰 + fix + PM 산출 게시  │   │
   │  · Codex TM 종합 코멘트               │   │
   │  · Codex fix commit                   │   │
   │  · PM 이 산출 PR 코멘트              │   │
   └────────────┬─────────────────────────┘   │
                ▼                              │
   ┌──────────────────────────────────────┐   │
   │ [5] Claude 검증 리뷰                  │   │
   │  · Claude TM 판정 (fix 없음/있음)     │   │
   └────────────┬─────────────────────────┘   │
                │ fix?                          │
                ├─── 있음 (라운드 +1) ──────────┘
                ▼ 없음
   ┌──────────────────────────────────────┐
   │ [N] CI 통과 + PM 종합 소견 + 머지     │
   └──────────────────────────────────────┘
```

**핵심 규칙**:
- 외부에 보이는 PR 코멘트는 단계당 **1개 (해당 진영의 TM 종합 + PM 의 Codex 산출 게시)** — 개별 7파트 코멘트 금지.
- 라운드 누적 시 코멘트 헤더에 라운드 번호 명시.
- PM 의 중간 통합 코멘트 없음 (v3 차이) — 마지막 종합 소견만.

---

## 3. 운영 규칙

### 3.1 동시성
- Codex 7파트는 메인 + 보조 합동 호출로 분할 가능 (동일 브랜치 commit)
- Claude 리뷰는 메인 세션이 직접 수행 (sub-agent 분리 가능하나 비용 절약 시 메인)
- Codex TM 호출은 매번 단순화된 프롬프트로 (PR #2 학습 — 복잡하면 안정성 이슈)

### 3.2 라벨 (v3 갱신)
- `step/2-codex-impl`, `step/3-claude-review-fix`, `step/4-codex-review-fix`, `step/5-claude-validate`, `step/N-pm-merge`
- 라벨은 단계 전환 시 PM 이 갱신
- 추가: `round/1`, `round/2`, ... (라운드 누적 시)

### 3.3 시간 제한 (가이드)
- 한 단계 24~48시간 정체 시 PM 개입 (재지시 / 파트 단순화)
- 라운드 [3]-[4]-[5] 가 3회 이상 돌면 PM 이 스코프 축소 검토 또는 fix 분할 후속 PR 위임

### 3.4 PR 분할
- 7파트 중 일부만 변경되는 PR 도 허용 (단, v3 단계 [3]~[5] 는 동일 적용)
- chore / hotfix PR 은 단순화 (PM 단독 가능, CONTRIBUTING.md §3 참고)

### 3.5 핫픽스 (v3 단순화)
- 운영 장애 시 PM 단독 `hotfix/<주제>` 브랜치 → CI 통과 후 즉시 머지 가능
- 머지 후 24h 이내 사후 리뷰 PR 의무 (v3 풀 단계)

---

## 4. v2 → v3 마이그레이션 차이 요약

| 항목 | v2 | v3 |
| --- | --- | --- |
| 리뷰와 fix | 분리 ([3a/3b] 리뷰만 → [4] PM 통합 → [5] Codex fix) | **일체** ([3] Claude 리뷰+fix → [4] Codex 리뷰+fix) |
| 진행 패턴 | 평행 (Claude TM + Codex TM 동시) | **순차 ping-pong** |
| PM 중간 통합 | 매 라운드 통합 코멘트 ([4], [7]) | **없음** — 마지막 [N] PM 종합 소견만 |
| Codex 산출 PR 게시 | (없음) | **단계 [2], [4] 후 PM 이 코멘트** ([[feedback-codex-results-to-pr]]) |
| TM 단일 종합 (개별 게시 금지) | 적용 | **유지** ([[feedback-tm-consolidates]]) |

---

## 5. 도구 호출

### 5.1 Codex CLI 호출
```powershell
.\codex\scripts\invoke.ps1 -Role <part> -Task "..."
# 또는 직접:
codex exec --dangerously-bypass-approvals-and-sandbox `
  -C "C:\game\idle game\repo" --skip-git-repo-check `
  "$(Get-Content codex\prompts\pr<N>-<role>.md -Raw)"
```

### 5.2 Codex TM 호출 (단계 [4])
```powershell
codex exec ... "$(cat codex/prompts/tm.md) ## 검토 대상 PR #N..."
# stdout 의 종합 마크다운을 추출 → gh pr comment N
```

### 5.3 Claude TM (단계 [3], [5])
- Claude Code 메인 세션이 직접 수행 (또는 sub-agent)
- 코드/문서 read → 종합 코멘트 작성 → `gh pr comment N --body-file -`

---

## 6. 문서 / 메모리 동기화
- 본 워크플로우 변경 시:
  1. 본 문서 갱신 (v 번호 증가)
  2. `CLAUDE.md`, `README.md`, `PULL_REQUEST_TEMPLATE.md` 동시 갱신
  3. 메모리 `project_workflow.md` + `feedback_workflow_v3.md` 갱신
  4. PR 본문에 변경 영향 명시

---

## 7. 위반 시
- 개별 7파트 코멘트는 TM 이 즉시 삭제 (TM 단일 종합 규칙 위반)
- PM 임의 머지는 리버트 + 재머지 PR 필수
- 한글 외 리뷰는 무효 처리 (재작성 요구)
- Codex 산출 PR 코멘트 누락 시 PM 이 재게시 의무
