# Boss/Rebirth V1 Balance Note

Date: 2026-05-26

| Item | V1 value | Code |
| --- | ---: | --- |
| Chapter 1 boss HP | 500 | `AIdleMonster::BossMaxHp` |
| Chapter 1 boss ATK | 24 | `AIdleMonster::BossAttack` |
| Chapter 1 boss DEF | 12 | `AIdleMonster::BeginPlay()` |
| Rebirth required level | 100 | `UIdleGameInstance::CanRebirth()` |
| Rebirth bonus points per rebirth | 5 | `UIdleGameInstance::Rebirth()` |
| Gold retained after rebirth | `floor(gold * 0.1)` | `UIdleGameInstance::Rebirth()` |
| Rebirth stat bonus | `HP +10`, `PhysAtk +2` per point | `FStatFormulas::DeriveStats()` / `deriveStats()` |
| Offline rebirth bonus | `+5%` per rebirth count | `OfflineRewardFormula` / `offline.ts` |

This note records the implemented PR #19 constants until the main balance and GDD documents are re-encoded or normalized.
