# PR #74 Mastery Local Bonuses QA Notes

## Scope

QA coverage for `docs/planning/slices/74-mastery-local-bonuses.md` section 2.5 / DoD 6 and `docs/superpowers/plans/2026-05-29-mastery-local-bonuses.md` Task 6.

## Given / When / Then

### Formula Anchors

Given the six mastery tracks Combat, Equipment, Abyss, Rune, Beast, and Explore.

When `FMasteryFormula::GetLocalBonus` is called at levels `0, 1, 30, 100`, including negative level input and the largest `int32` input.

Then all tracks return `0` at level `0`, negative input clamps to `0`, positive values are monotonic, level anchors match the server `Math.fround` values, and Equipment uses the same `min(0.50, raw)` cap formula without exceeding `0.50`.

Automation: `IdleProject.Mastery.Formula`.

Vitest: `server/src/core/formulas/mastery.parity.test.ts`.

### Production Hook Application

Given level 1 local mastery for each track through a v14 mastery save.

When production APIs execute kill EXP, equipment enhancement cost, dungeon rewards, rune core derived stats, pet bonus scaling, and quest claim rewards.

Then Combat scales kill EXP once, Equipment reduces the enhancement gold spend by the formula, Abyss scales dungeon rewards by the formula, Rune increases derived stats through the rune core path, Beast scales equipped pet bonuses, and Explore scales quest gold and EXP by the formula.

Automation: `IdleProject.Mastery.LocalBonusApplication`.

### Global And Local Separation

Given #72 global mastery bonuses and PR #74 local mastery bonuses on the same Beast or Abyss track.

When `AddExp`, `GetRuneExpBoostBonus`, `GetRuneGoldFindBonus`, pet gold application, and drop chance application are evaluated.

Then #72 EXP is applied once inside `AddExp`, the EXP getter excludes mastery to prevent double application in kill rewards, the gold find getter includes only the #72 global gold bonus, pet gold application uses only the PR #74 Beast local multiplier, and Abyss global drop add remains separate from Abyss local dungeon reward scaling.

Automation: `IdleProject.Mastery.UtilityBonusExposure`.

### Zero Regression

Given a legacy v12 save with no mastery payload.

When a Warrior refreshes derived stats and local bonus getters are read.

Then all mastery tracks remain level `0`, local bonuses return `0`, core stats and CP remain at the non-mastery baseline, and no economy path receives a local bonus.

Automation: `IdleProject.Mastery.ZeroRegression`.

## Regression Surface

- Track coverage is explicit for all six local bonus ids.
- Edge cases cover `level <= 0`, very large Equipment input, save migration from v12/v13, and reset persistence.
- #72 global mastery paths are kept separate from PR #74 local paths to prevent duplicate EXP, gold, or drop application.
- No Playwright coverage is required for this slice because the HUD display contract is covered by the mastery panel view model and localization tests.

## Verification

- Server lint: `cd server; npm run lint`
- Server targeted tests: `cd server; npm run test -- mastery`
- UE build: `Build.bat IdleProjectEditor Win64 Development -Project=client/IdleProject.uproject -WaitMutex`
- UE Automation: `Automation RunTests IdleProject.Mastery; Quit`
