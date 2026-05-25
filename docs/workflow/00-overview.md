# 워크플로우 문서 — 한눈에 보기

본 디렉터리는 **누가 / 언제 / 어떻게** 작업하는지를 정의합니다.

| 문서 | 내용 |
| --- | --- |
| [01-pm-codex-claude-loop.md](01-pm-codex-claude-loop.md) | 핵심 워크플로우 — PM-Codex-Claude 반복 리뷰 루프 (v2) |
| [02-pr-policy.md](02-pr-policy.md) | PR 생성/명명/머지 규칙 |
| [03-review-checklist.md](03-review-checklist.md) | 7개 파트별 리뷰 체크리스트 |
| [04-merge-policy.md](04-merge-policy.md) | 머지 조건, CI 게이트, 롤백 |
| [agents/](agents/) | 17개 에이전트 시스템 프롬프트 / 책임 명세 |

---

## 30초 요약 (v2 — TM 단일 종합)

```
[1] PM 기획 → PR
[2] Codex 구현 (7파트 커밋, 개별 코멘트 X)
[3] 두 라인 병렬 종합:
    [3a] Claude TM → 1개 코멘트
    [3b] Codex TM  → 1개 코멘트 (Claude TM 과 비교 포함)
[4] PM 통합 → fix 지시 1개 코멘트
[5] Codex 2차 fix
[6] [6a] Claude TM 재검토 → [6b] Codex TM 재검토 (각 1개)
[7] PM 2차 통합 → fix 없음 → [8] / fix 있음 → [5]
[8] CI 통과 + PM 종합 소견 → 머지
```

7개 파트: 디자이너 / 스토리 / 퀘스트 / 캐릭터·아이템·능력치 / 밸런스 / 백엔드·DB / QA.

---

## 핵심 원칙 (잊지 말 것)

1. **TM 만 외부 코멘트** — 각 파트 에이전트가 직접 게시 금지. 위반 시 TM 즉시 삭제.
2. **두 TM 평행** — Claude TM, Codex TM 각자 1개 종합. PM 이 비교 / 통합.
3. **한글로 구체적 리뷰** — 추상적 코멘트 금지.
4. **모든 산출물 문서화** — 기획, 설계, 운영, API, SDK 모두.
5. **항시 GitHub 커밋/푸쉬** — 작업 중간 백업.
6. **PM 임의 머지 금지** — 반드시 위 흐름.
7. **머지 전 PM 종합 소견 리뷰** 게시 의무.
