## [4] Codex TM 종합 검토 + fix

커밋: `codex(qa): harden infinity tower parity`

**종합 검토(Claude TM [3] 대조)**
- `FTowerFormula` / `tower.ts` 요구 CP 공식은 `round(100 * pow(1.15, floor - 1))` double 계산 후 int64 clamp로 일치.
- `TryClimbTower`는 CP 부족 시 0 보상, 동일 CP 재호출 보상 없음, 1회 `MaxClimbPerCall=100` 상한 유지.
- HUD는 `TowerClimb` HitBox -> `UIdleGameInstance::ClimbTower()` -> `UTowerService::TryClimbTower()` -> `OnTowerClimbed` 피드백 경로가 연결되어 있음.

**fix**
1. `UIdleGameInstance::AddGold`를 포화 덧셈으로 수정해 tower reward 적용 시 int64 경계에서 0/음수 래핑을 차단.
2. C++ Automation에 floor 10/50/100 parity anchor 및 floor 500 int64 max clamp 검증 추가.
3. 서버 `tower.test.ts`에 floor 100 anchor 추가.
4. `docs/planning/05-balance-philosophy.md`에 PR #50 anchor/guardrail 보강.
5. `docs/qa/scenarios/M8-infinity-tower-hud-v1.md`에 gold overflow QA 시나리오 추가.

**검증(Codex)**
- RED 확인: `IdleProject.GameCore.IdleGameInstance.TowerHooks` 실패, `Gold grant saturates... expected MAX_int64, was 0`.
- UE Build.bat: `Result: Succeeded`.
- UE Automation full: `Found 130 automation tests based on 'IdleProject'`, `TEST COMPLETE. EXIT CODE: 0`.
- 서버: `npm test` 376 passed, 1 skipped / `npm run build` exit 0 / `npm run lint` 112 files, no fixes.
- markdownlint: 변경 문서 2개 0 errors.
- `git diff --check`: clean.

**잔여 판단**
- 추가 로직 결함 없음. CI green 확인 후 다음 루프/머지 판단 가능.
