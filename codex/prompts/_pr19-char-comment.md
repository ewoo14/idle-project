## Codex character 산출 (단계 [2] — 메인)
커밋 `9cc8509` `codex(character): add chapter1 boss rebirth v1`
- **보스 안개 군주**(IdleMonster 강화) + 격파 플래그.
- **환생 V1**: 조건(보스격파+Lv100), rebirthCount++, 골드 10% 보존, 영구 포인트 +5, 레벨/EXP 리셋.
- **스탯 반영(C++/TS deriveStats 양쪽)**: 포인트당 HP+10, PhysAtk+2.
- Automation(환생/보스) + 서버 Vitest 추가.
- 검증: 서버 formulas 70 tests passed, UE Build Succeeded, UE Automation **33 tests** exit 0.

→ 다음: backend(환생 persist: rebirthBonusPoints 컬럼/migration + 검증 라우트) + designer(환생 UI/보스 표시).
