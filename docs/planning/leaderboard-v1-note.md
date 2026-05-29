# Leaderboard V1 Balance Note

Date: 2026-05-29

This note covers PR #76 Task 4 for the leaderboard client integration. It
records the V1 score contracts for Power and Rebirth leaderboards, the current
season boundary, and the balance guardrails for later rank-tier rewards.

## Summary

Leaderboard V1 is a display and ranking surface over already-uploaded save
state. It does not change level curves, monster stats, rewards, rebirth timing,
or progression costs.

```text
Power score   = computePowerScore(savePayload, character)
              = level * 100 + rebirthCount * 5000

Rebirth score = rebirthCount
```

The Power formula is intentionally treated as the current server-side CP proxy,
not as a new balance multiplier. The PR #76 client should display the score it
receives from `/leaderboard/power`; it should not re-score the player locally or
mix the score into combat, rewards, saves, or offline rewards.

## Power Score Contract

`SaveService.upload` writes the Power leaderboard after save validation by
calling `computePowerScore`. The current implementation uses only validated
save progress:

| Input | Weight | Balance interpretation |
| --- | ---: | --- |
| `level` | 100 per level | Progress proxy for the current saved character level. |
| `rebirthCount` | 5,000 per rebirth | Prestige proxy so a rebirthed character sorts above a same-level first-run character. |

Example anchors:

| Level | Rebirth count | Power score |
| ---: | ---: | ---: |
| 1 | 0 | 100 |
| 50 | 0 | 5,000 |
| 100 | 0 | 10,000 |
| 1 | 1 | 5,100 |
| 100 | 1 | 15,000 |
| 100 | 5 | 35,000 |

This keeps V1 ranking stable while the richer client-side combat power work is
still evolving. A future PR can replace the proxy with canonical combat power
only after the server has the same source data and formula parity as the client
(`FCombatPowerFormula::ComputeCombatPower` / `computeCombatPower`).

## Rebirth Score Contract

The Rebirth leaderboard ranks the validated `rebirthCount` from the uploaded
save. It is a count leaderboard, not a point-value leaderboard:

```text
Rebirth rank score = savePayload.rebirthCount
```

This avoids coupling the leaderboard to rebirth reward-point tuning. If rebirth
point rewards change later, the Rebirth board still answers the player-facing
question: "how many resets has this character completed?"

## Season Model

PR #76 uses the current hard-coded season id from `SaveService.upload`:

```text
seasonId = 1
```

The client should read the current season from `SeasonService` for request
construction, but V1 server writes still land in season `1`. Season creation,
closeout, backfill, and archival policy are follow-up operator features. Until
that work exists, do not tune scores differently by season.

## Ranking Semantics

Backend `/me` rank lookup should use PostgreSQL `rank()` window semantics:

- Higher score sorts first.
- Ties share the same rank.
- Missing rows return rank `0` and score `"0"`.
- Redis is allowed for top-N reads, but `/me` rank should use the PostgreSQL
  rank window so the answer is exact even when the cache is partial.

This makes the displayed top-N list and the "my rank" row predictable without
requiring client-side reconstruction.

## Rank-Tier Reward Follow-Up

Rank-tier claims are intentionally out of PR #76. When rewards are added, keep
the reward system separate from the ranking formula:

- Persist claims by `(seasonId, characterId, boardKind, tier)` so reconnects
  and repeated client calls cannot duplicate rewards.
- Resolve tier eligibility from a finalized season snapshot, not from a live
  top-N response that can move while a player is claiming.
- Keep Power and Rebirth tiers separate; do not let one board's rank claim the
  other board's reward.
- Prefer bounded cosmetic, title, currency, or account-progress rewards before
  adding direct combat multipliers.
- Re-run `npm run balance:sim` if a rank reward enters first-rebirth pacing,
  gold/hour, EXP/hour, drop rate, or combat power before the first reset.

## Balance Simulator Interpretation

Leaderboard V1 is report-only for balance. It stores and displays existing
save-derived scores, so it should not be injected into `tools/balance-sim`.

The current simulator report remains the review baseline:

| Metric | Hours |
| --- | ---: |
| p10 | 4.919 |
| median | 5.328 |
| p90 | 5.751 |
| min | 4.564 |
| max | 6.144 |

The median remains inside the 5-10h target, and every sampled run remains
inside the 3-20h review band. Because PR #76 adds no reward, stat, monster,
level, or economy changes, there is no expected delta in the 1000-run first
rebirth distribution.

## Guardrails

- Do not modify `LevelCurveDB.csv`, `MonsterDB.csv`, or reward formulas for PR
  #76 leaderboard display work.
- Do not compute a new client-only Power rank from UI-visible combat power.
  Display the server score until server/client formula parity is explicitly
  promoted into the leaderboard write path.
- Do not use leaderboard rank as a save merge tiebreaker. Cloud-save merge
  policy should continue to compare save-owned progress fields.
- Do not add rank-tier rewards without claim persistence and season-finalized
  snapshots.
- Keep `/leaderboard/power`, `/leaderboard/rebirth`, `/power/me`, and
  `/rebirth/me` additive to the existing save upload path.

## Review Evidence

- Score source: `server/src/modules/save/save.service.ts`
- Ranking source: `server/src/modules/leaderboard/leaderboard.repo.ts`
- API contract: `docs/api/leaderboard.md`
- Simulator source: `tools/balance-sim/index.ts`
- Generated report: `tools/balance-sim/reports/balance-sim-report.md`
- Verification command: `cd server; npm run balance:sim`
