# M5 Combat Class Differential V1 QA Scenarios

## Scope

PR #26 differentiates Mage and Archer combat without changing the existing
Warrior baseline. QA focuses on physical/magic parity, crit determinism,
regression safety, and evidence that server TypeScript formulas match UE
`FCombatFormulas`.

## Scenario 1: Server and Client Formula Parity

Given the server formula mirror and UE client formula utilities share the same
damage anchors
When `Atk=100, Def=20`, `MagicAtk=80, MagicDef=20`, and `baseDamage=34` crit
anchors are evaluated
Then physical damage is 88, magic damage is 68, non-crit damage remains 34, and
`CritDmg=1.8` crit damage is 61.2 with diff 0 against the documented anchors.

Automation: `server/src/core/formulas/combat.test.ts`,
`client/Source/IdleProject/Tests/CombatTests.cpp`
(`IdleProject.Combat.Formulas.ComputeDamage`)

## Scenario 2: Mage Magic Damage Path

Given a Mage has low `PhysAtk` and high `MagicAtk`
When class damage or a Mage damage skill targets an enemy with separate
`PhysDef` and `MagicDef`
Then the damage calculation uses `MagicAtk` vs `MagicDef` and applies crit only
after magic mitigation.

Automation: `server/src/core/formulas/combat.test.ts`,
`client/Source/IdleProject/Tests/CombatTests.cpp`
(`IdleProject.Combat.Formulas.ClassDamage`,
`IdleProject.Combat.Skills.MagicDamage`)

## Scenario 3: Archer Critical Damage Path

Given an Archer has physical attack, crit rate, and crit damage
When a damage skill resolves with guaranteed crit in the deterministic test
Then physical mitigation runs first and the crit multiplier is applied once to
the mitigated value.

Automation: `server/src/core/formulas/combat.test.ts`,
`client/Source/IdleProject/Tests/CombatTests.cpp`
(`IdleProject.Combat.Skills.CritDamage`)

## Edge Cases

- `critRate=0` must never crit and `critRate=1` must always crit in deterministic
  boundary tests.
- `critDmg < 1` must not reduce damage below the non-crit base value.
- Basic attacks remain physical even for Mage; only Mage damage skills use the
  magic path in V1.
- The 6-argument `InitializeCombat` overload remains compatible for existing
  physical-only callers, while the 8-argument overload initializes `MagicAtk`,
  `MagicDef`, `CritRate`, and `CritDmg`.
- Zero attack and over-defense anchors keep the existing minimum-damage behavior.

## Regression Checks

- Existing auto-battle, skill definition parity, class selection, boss/rebirth,
  and offline reward tests must remain green.
- Server `computeClassDamage` must keep Warrior and Archer on physical defense
  while Mage routes to magic defense.
- Balance documents must keep expected crit DPS as
  `baseDamage * (1 + critRate * (critDmg - 1))`.

## Evidence Required

- Vitest stdout for `src/core/formulas/combat.test.ts` or full `npm test`.
- UE Automation stdout for `IdleProject.Combat.*`.
- `npm run build`, `npm run lint`, and Unreal `Build.bat` stdout before PM/TM
  handoff.
