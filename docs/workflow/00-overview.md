# 워크플로우 문서 — 한눈에 보기

본 디렉터리는 **누가 / 언제 / 어떻게** 작업하는지를 정의합니다.

| 문서 | 내용 |
| --- | --- |
| [01-pm-codex-claude-loop.md](01-pm-codex-claude-loop.md) | 핵심 워크플로우 — PM-Codex-Claude 반복 리뷰 루프 |
| [02-pr-policy.md](02-pr-policy.md) | PR 생성/명명/머지 규칙 |
| [03-review-checklist.md](03-review-checklist.md) | 7개 파트별 리뷰 체크리스트 |
| [04-merge-policy.md](04-merge-policy.md) | 머지 조건, CI 게이트, 롤백 |
| [agents/](agents/) | 17개 에이전트 시스템 프롬프트 / 책임 명세 |

---

## 30초 요약

```
[1] PM 기획 PR
[2] Codex 7파트 구현
[3] Claude 7파트 리뷰
[4] TM 종합 + fix
[5] Codex 2차 fix
[6] Claude 2차 검토
[7] TM 2차 종합
[8] fix 없음 → CI 통과 → PM 종합 소견 → 머지
    fix 있음 → [5] 로 루프
```

7개 파트: 디자이너 / 스토리 / 퀘스트 / 캐릭터·아이템·능력치 / 밸런스 / 백엔드·DB / QA.

---

## 원칙 (잊지 말 것)

1. **한글로 구체적 리뷰** — 추상적 코멘트 금지.
2. **모든 산출물 문서화** — 기획, 설계, 운영, API, SDK 모두.
3. **항시 GitHub 커밋/푸쉬** — 작업 중간 백업.
4. **PM 임의 머지 금지** — 반드시 위 흐름.
5. **머지 전 PM 종합 소견 리뷰** 게시 의무.
