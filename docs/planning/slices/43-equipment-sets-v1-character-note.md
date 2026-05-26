# PR #43 Character Implementation Note

- Client C++ scope implemented: `EItemSet`, `FItemInstance::ItemSet`,
  `FDropFormula::RollItemSet`, `FSetBonusFormula`, and
  `UInventoryComponent::ComputeEquipmentBonus` set-bonus aggregation.
- V1 flat values: Warrior 2pc `PhysAtk +20`, Warrior 4pc additional
  `PhysAtk +50` and `CritRate +0.05`; Guardian 2pc `PhysDef +15` and
  `Hp +100`, Guardian 4pc additional `PhysDef +35` and `Hp +250`; Arcane 2pc
  `MagicAtk +20`, Arcane 4pc additional `MagicAtk +50` and `CritDmg +0.10`.
- Guardrails preserved: Common/None items roll no set, set bonuses are not
  included in PowerScore, and under-threshold/None set equipment keeps the
  previous per-item equipment total.
- Verification: `Build.bat` succeeded and UE Automation `IdleProject` completed
  with exit code 0; new coverage includes `RollItemSet`,
  `SetBonusFormula.Thresholds`, and `Inventory.Bonus.SetBonus`.
