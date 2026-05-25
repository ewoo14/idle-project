# 워크플로우 문서 — 한눈에 보기 (v3)

> 2026-05-25 사용자 명시로 v2 → v3 전환. PR #6 부터 정식 적용.

| 문서 | 내용 |
| --- | --- |
| [01-pm-codex-claude-loop.md](01-pm-codex-claude-loop.md) | 핵심 워크플로우 v3 (Claude 리뷰+fix / Codex 리뷰+fix 일체 ping-pong) |
| [02-pr-policy.md](02-pr-policy.md) | PR 생성/명명/머지 규칙 |
| [03-review-checklist.md](03-review-checklist.md) | 7개 파트별 리뷰 체크리스트 |
| [04-merge-policy.md](04-merge-policy.md) | 머지 조건, CI 게이트, 롤백 |
| [agents/](agents/) | 에이전트 시스템 프롬프트 / 책임 명세 |

---

## 30초 요약 (v3)

```
[1] PM 기획 → PR
[2] Codex 개발 (메인 + 보조)
    → PM 이 Codex 산출 정리 → PR 코멘트
[3] Claude 리뷰 + fix
    → Claude TM 종합 코멘트 1개
    → Claude/PM 직접 fix commit
[4] Codex 리뷰 + fix
    → Codex TM 종합 코멘트 1개 (Claude TM 비교)
    → Codex fix commit
    → PM 이 Codex fix 산출 → PR 코멘트
[5] Claude 검증 리뷰
    → fix 없음 → [N] / fix 있음 → [3] 또는 [4] 루프
[N] CI 통과 + PM 종합 소견 + 머지
```

7개 파트: 디자이너 / 스토리 / 퀘스트 / 캐릭터·아이템·능력치 / 밸런스 / 백엔드·DB / QA.

---

## 핵심 원칙 (잊지 말 것)

1. **각 진영이 리뷰 + fix 일체** (v3) — 단계 [3] Claude, 단계 [4] Codex 가 ping-pong.
2. **TM 만 외부 코멘트** — 각 파트 에이전트가 직접 게시 금지.
3. **PM 이 Codex 산출 PR 코멘트로 게시** — 단계 [2], [4] 직후 의무.
4. **PM 중간 통합 코멘트 없음** — 마지막 단계 [N] PM 종합 소견만.
5. **한글로 구체적 리뷰** — 추상적 코멘트 금지.
6. **모든 산출물 문서화** — 기획, 설계, 운영, API, SDK 모두.
7. **항시 GitHub 커밋/푸쉬** — 작업 중간 백업.
8. **PM 임의 머지 금지** — 반드시 위 흐름.
9. **머지 전 PM 종합 소견** 게시 의무.
