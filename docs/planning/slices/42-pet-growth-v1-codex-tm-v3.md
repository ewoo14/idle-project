# PR #42 Codex TM v3 Review Notes

Date: 2026-05-26

## Stdout Summary

- character: `FPetLevelFormula` keeps `MaxPetLevel = 10`,
  `GetFeedCost(level) = 500 * (max(0, level) + 1)^2`, max-level cost 0, and
  `GetBonusMultiplier(level) = 1.0 + clamp(level, 0, 10) * 0.1`. `TryFeedPet`
  checks known pet, max level, and gold before spending. Successful feeds spend
  once, increment one level, and broadcast `OnPetFed`.
- backend: server `petLevel.ts` mirrors the client integer cost and float32
  bonus multiplier. v3 fixes `getEffectiveBonusPercent` to float32 the final
  base-percent multiplication too, so Lv1 dog is exactly 22 and Lv1 bird is
  exactly 16.5 instead of leaking JS double precision.
- designer: pet HUD shows level, next feed cost, level-scaled bonus, enabled or
  disabled feed state, and keeps `PetEquip_` and `PetFeed_` hit boxes separate.
  `OnPetFed` is bound and removed with the rest of `AIdleHUD` GameInstance
  delegates.
- balance: pet feed pressure remains 192,500 gold from Lv0 to Lv10. Dog moves
  from 20% to 40%, bird moves from 15% to 30%, and the dog payback stays about
  1.47h at the sampled 654,689 Lv50 gold/hour median.
- qa: v3 adds regression coverage for Lv1 effective percent parity, unknown pet
  no-spend, max-level no-spend, and GameInstance drop-bonus application after
  feeding bird.

## Fixes

- `server/src/core/formulas/petLevel.ts`: final effective bonus percent now uses
  `Math.fround(Math.fround(basePercent) * getBonusMultiplier(level))`.
- `server/src/core/formulas/petLevel.test.ts`: added Lv1 dog/bird effective
  percent cases that failed before the fix.
- `client/Source/IdleProject/Tests/PetSeasonTests.cpp`: added feed guard and
  drop-bonus regression assertions.

## Checklist

- [x] Server/client pet level formula parity reviewed.
- [x] Feed guard no-spend cases covered.
- [x] Lv0 base pet bonus regression preserved by existing tests.
- [x] Lv1 and Lv10 effective bonus percent parity covered.
- [x] HUD hit box and delegate cleanup paths reviewed.
- [x] Balance payback conclusion checked against simulator report.
