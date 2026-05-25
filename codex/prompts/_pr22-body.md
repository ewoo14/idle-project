## 개요 ([1] PM 기획)
M5 마지막 — 펫 2종(강아지 골드+/새 드롭+) 장착·보너스 + 시즌 패스 무료 트랙 베타(티어/시즌토큰←퀘스트/보상 수령). 퀘스트·골드·오프라인 연동.

기획서: `docs/planning/slices/22-pets-season-pass-v1.md`.

## 설계
- backend(메인): pets.ts/season.ts + pet/season 모듈(장착·보너스·티어·토큰·수령 + migration) + Vitest.
- character: 펫 보너스 골드/드롭 반영 + 시즌 토큰 적립. designer: 펫/시즌 UI. balance: %/보상.

## 워크플로우 v3
[1]→[2] Codex backend 메인+character/designer/balance/qa →[3] Claude TM →[4] Codex TM+fix →[5] 검증 →[N] **CI 그린 확정** 후 머지. 사용자 PIE 차후 일괄.

🤖 Generated with [Claude Code](https://claude.com/claude-code)
