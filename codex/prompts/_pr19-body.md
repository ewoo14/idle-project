## 개요 ([1] PM 기획)
M4 마지막 — 챕터1 보스 **안개 군주** + **환생 V1**(조건 보스격파+Lv100 → rebirthCount++/영구 포인트 5/레벨 리셋/골드·장비 일부 보존). rebirthCount 는 이미 오프라인 보너스/리더보드 연동 — 이를 실제 증가시키는 시스템.

기획서: `docs/planning/slices/19-boss-rebirth-v1.md`.

## 설계
- character(메인): 보스(IdleMonster 강화 변형) + 환생 로직(CanRebirth/Rebirth) + 영구 포인트 스탯 반영 + Automation.
- backend: rebirthBonusPoints persist(migration 0004) + 검증 라우트 + Vitest.
- designer: 환생 UI/보스 표시. balance: 포인트/보존율/보스 스탯.

## 워크플로우 v3
[1]→[2] Codex character 메인+backend/designer/balance/qa →[3] Claude TM →[4] Codex TM+fix →[5] 검증 →[N] 머지. 사용자 PIE 차후 일괄.

🤖 Generated with [Claude Code](https://claude.com/claude-code)
