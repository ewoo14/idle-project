#include "Misc/AutomationTest.h"

#include "CombatSystem/BattleAIComponent.h"
#include "CombatSystem/BossPhaseFormula.h"
#include "CombatSystem/CombatComponent.h"
#include "CombatSystem/CombatFormulas.h"
#include "CombatSystem/SkillComponent.h"
#include "CombatSystem/StatusElementTypes.h"
#include "CharacterSystem/CombatPowerFormula.h"
#include "CharacterSystem/IdleCharacter.h"
#include "CharacterSystem/LevelFormulas.h"
#include "BossSpecialAttackTestReceiver.h"
#include "Components/SceneComponent.h"
#include "DamageReceivedTestReceiver.h"
#include "Engine/World.h"
#include "GameCore/IdleGameInstance.h"
#include "GameCore/TowerFormula.h"
#include "GameCore/TowerMilestoneFormula.h"
#include "GameCore/TowerService.h"
#include "CharacterSystem/IdleMonster.h"
#include "Internationalization/IdleLocalization.h"
#include "ItemSystem/InventoryComponent.h"
#include "UI/IdleHUD.h"

namespace
{
int32 CountSkillsByType(const USkillComponent& Skills, ESkillType Type)
{
	return Skills.Skills.FilterByPredicate([Type](const FSkillDefinition& Skill)
	{
		return Skill.Type == Type;
	}).Num();
}

bool HasSkill(const USkillComponent& Skills, FName SkillId)
{
	return Skills.Skills.ContainsByPredicate([SkillId](const FSkillDefinition& Skill)
	{
		return Skill.SkillId == SkillId;
	});
}

struct FExpectedSkillDefinition
{
	FName SkillId;
	EClassId ClassId;
	ESkillType Type;
	ESkillEffectType EffectType;
	float Cooldown;
	float DamageCoeff;
	float BuffMagnitude;
	float BuffDuration;
	float GaugeGainOnHit;
	float GaugeGainOnTakeDamage;
	ESkillStatusEffect StatusEffect = ESkillStatusEffect::None;
	float StatusDuration = 0.0f;
	float StatusMagnitude = 0.0f;
	ESkillElement Element = ESkillElement::None;
};

bool TestSkillDefinitionParity(FAutomationTestBase& Test, const USkillComponent& Skills, const FExpectedSkillDefinition& Expected)
{
	const FSkillDefinition* Actual = Skills.Skills.FindByPredicate([&Expected](const FSkillDefinition& Skill)
	{
		return Skill.SkillId == Expected.SkillId;
	});

	Test.TestNotNull(*FString::Printf(TEXT("%s exists"), *Expected.SkillId.ToString()), Actual);
	if (!Actual)
	{
		return false;
	}

	Test.TestEqual(*FString::Printf(TEXT("%s ClassId"), *Expected.SkillId.ToString()), static_cast<int32>(Actual->ClassId), static_cast<int32>(Expected.ClassId));
	Test.TestEqual(*FString::Printf(TEXT("%s Type"), *Expected.SkillId.ToString()), static_cast<int32>(Actual->Type), static_cast<int32>(Expected.Type));
	Test.TestEqual(*FString::Printf(TEXT("%s EffectType"), *Expected.SkillId.ToString()), static_cast<int32>(Actual->EffectType), static_cast<int32>(Expected.EffectType));
	Test.TestEqual(*FString::Printf(TEXT("%s Cooldown"), *Expected.SkillId.ToString()), Actual->Cooldown, Expected.Cooldown);
	Test.TestEqual(*FString::Printf(TEXT("%s DamageCoeff"), *Expected.SkillId.ToString()), Actual->DamageCoeff, Expected.DamageCoeff);
	Test.TestEqual(*FString::Printf(TEXT("%s BuffMagnitude"), *Expected.SkillId.ToString()), Actual->BuffMagnitude, Expected.BuffMagnitude);
	Test.TestEqual(*FString::Printf(TEXT("%s BuffDuration"), *Expected.SkillId.ToString()), Actual->BuffDuration, Expected.BuffDuration);
	Test.TestEqual(*FString::Printf(TEXT("%s GaugeGainOnHit"), *Expected.SkillId.ToString()), Actual->GaugeGainOnHit, Expected.GaugeGainOnHit);
	Test.TestEqual(*FString::Printf(TEXT("%s GaugeGainOnTakeDamage"), *Expected.SkillId.ToString()), Actual->GaugeGainOnTakeDamage, Expected.GaugeGainOnTakeDamage);
	Test.TestEqual(*FString::Printf(TEXT("%s StatusEffect"), *Expected.SkillId.ToString()), static_cast<int32>(Actual->StatusEffect), static_cast<int32>(Expected.StatusEffect));
	Test.TestEqual(*FString::Printf(TEXT("%s StatusDuration"), *Expected.SkillId.ToString()), Actual->StatusDuration, Expected.StatusDuration);
	Test.TestEqual(*FString::Printf(TEXT("%s StatusMagnitude"), *Expected.SkillId.ToString()), Actual->StatusMagnitude, Expected.StatusMagnitude);
	Test.TestEqual(*FString::Printf(TEXT("%s Element"), *Expected.SkillId.ToString()), static_cast<int32>(Actual->Element), static_cast<int32>(Expected.Element));
	return true;
}

FItemInstance MakeCombatPowerTestItem(EItemSlot Slot, EItemSet ItemSet, float Atk, float Def, float Hp)
{
	FItemInstance Item;
	Item.ItemId = FName(*FString::Printf(TEXT("cp_%d"), static_cast<int32>(Slot)));
	Item.Slot = Slot;
	Item.Rarity = EItemRarity::Mythic;
	Item.ItemSet = ItemSet;
	Item.BonusAtk = Atk;
	Item.BonusDef = Def;
	Item.BonusHp = Hp;
	Item.BonusCritRate = 0.02f;
	Item.BonusAtkSpeed = 0.1f;
	Item.BonusMagicAtk = 8.0f;
	return Item;
}

void TestCurrentCombatPowerParity(FAutomationTestBase& Test, const TCHAR* Label, const AIdleCharacter& Character)
{
	const FDerivedStats CurrentDerived = Character.GetCurrentDerivedStats();
	Test.TestEqual(Label, Character.GetCombatPower(), FCombatPowerFormula::ComputeCombatPower(CurrentDerived));
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBossPhaseFormulaTest,
	"IdleProject.Combat.BossPhase.Formula",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBossPhaseFormulaTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("HP ratio 0.70 is phase 1"), FBossPhaseFormula::GetBossPhase(0.70f), 1);
	TestEqual(TEXT("HP ratio 0.66 is phase 2"), FBossPhaseFormula::GetBossPhase(0.66f), 2);
	TestEqual(TEXT("HP ratio 0.50 is phase 2"), FBossPhaseFormula::GetBossPhase(0.50f), 2);
	TestEqual(TEXT("HP ratio 0.33 is phase 3"), FBossPhaseFormula::GetBossPhase(0.33f), 3);
	TestEqual(TEXT("HP ratio 0.20 is phase 3"), FBossPhaseFormula::GetBossPhase(0.20f), 3);
	TestEqual(TEXT("HP ratio 1.00 is phase 1"), FBossPhaseFormula::GetBossPhase(1.00f), 1);
	TestEqual(TEXT("HP ratio 0.00 is phase 3"), FBossPhaseFormula::GetBossPhase(0.00f), 3);
	TestEqual(TEXT("HP ratio above one clamps to phase 1"), FBossPhaseFormula::GetBossPhase(2.00f), 1);
	TestEqual(TEXT("HP ratio below zero clamps to phase 3"), FBossPhaseFormula::GetBossPhase(-1.00f), 3);

	TestEqual(TEXT("Phase 1 attack multiplier"), FBossPhaseFormula::GetPhaseAtkMultiplier(1), 1.0f);
	TestEqual(TEXT("Phase 2 attack multiplier"), FBossPhaseFormula::GetPhaseAtkMultiplier(2), 1.25f);
	TestEqual(TEXT("Phase 3 attack multiplier"), FBossPhaseFormula::GetPhaseAtkMultiplier(3), 1.6f);
	TestEqual(TEXT("Unknown phase attack multiplier defaults neutral"), FBossPhaseFormula::GetPhaseAtkMultiplier(99), 1.0f);

	TestEqual(TEXT("Phase 1 attack speed multiplier"), FBossPhaseFormula::GetPhaseAtkSpeedMultiplier(1), 1.0f);
	TestEqual(TEXT("Phase 2 attack speed multiplier"), FBossPhaseFormula::GetPhaseAtkSpeedMultiplier(2), 1.15f);
	TestEqual(TEXT("Phase 3 attack speed multiplier"), FBossPhaseFormula::GetPhaseAtkSpeedMultiplier(3), 1.3f);
	TestEqual(TEXT("Unknown phase attack speed multiplier defaults neutral"), FBossPhaseFormula::GetPhaseAtkSpeedMultiplier(99), 1.0f);

	TestEqual(TEXT("Special attack interval"), FBossPhaseFormula::SpecialAttackIntervalSeconds, 6.0f);
	TestEqual(TEXT("Special attack damage multiplier"), FBossPhaseFormula::GetSpecialAttackDamageMultiplier(), 2.5f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCombatFormulasTest,
	"IdleProject.Combat.Formulas.ComputeDamage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCombatFormulasTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Atk 100 Def 20 damage"), FCombatFormulas::ComputeDamage(100.0f, 20.0f), 88.0f);
	TestEqual(TEXT("Minimum damage guarantee"), FCombatFormulas::ComputeDamage(10.0f, 100.0f), 0.5f);
	TestEqual(TEXT("Zero defense damage"), FCombatFormulas::ComputeDamage(50.0f, 0.0f), 50.0f);
	TestEqual(TEXT("Magic damage uses the same curve"), FCombatFormulas::ComputeMagicDamage(80.0f, 20.0f), 68.0f);
	TestEqual(TEXT("No element keeps neutral damage"), FCombatFormulas::ComputeElementMultiplier(ESkillElement::None, ESkillElement::Fire), 1.0f);
	TestEqual(TEXT("Matching monster weakness amplifies damage"), FCombatFormulas::ComputeElementMultiplier(ESkillElement::Fire, ESkillElement::Fire), 1.5f);
	TestEqual(TEXT("Opposed monster weakness resists damage"), FCombatFormulas::ComputeElementMultiplier(ESkillElement::Ice, ESkillElement::Fire), 0.5f);
	TestEqual(TEXT("Unrelated element keeps neutral damage"), FCombatFormulas::ComputeElementMultiplier(ESkillElement::Lightning, ESkillElement::Fire), 1.0f);
	TestEqual(TEXT("Holy attack exploits dark weakness"), FCombatFormulas::ComputeElementMultiplier(ESkillElement::Holy, ESkillElement::Dark), 1.5f);
	TestEqual(TEXT("Dark attack exploits holy weakness"), FCombatFormulas::ComputeElementMultiplier(ESkillElement::Dark, ESkillElement::Holy), 1.5f);
	TestEqual(TEXT("Dark attack stays neutral against no weakness"), FCombatFormulas::ComputeElementMultiplier(ESkillElement::Dark, ESkillElement::None), 1.0f);
	TestEqual(TEXT("Dark attack stays neutral against fire weakness"), FCombatFormulas::ComputeElementMultiplier(ESkillElement::Dark, ESkillElement::Fire), 1.0f);

	FRandomStream NeverCritStream(12345);
	FRandomStream AlwaysCritStream(12345);
	TestFalse(TEXT("Zero crit rate never crits"), FCombatFormulas::RollCrit(0.0f, NeverCritStream));
	TestTrue(TEXT("Full crit rate always crits"), FCombatFormulas::RollCrit(1.0f, AlwaysCritStream));
	TestEqual(TEXT("Non crit leaves base damage unchanged"), FCombatFormulas::ApplyCrit(34.0f, false, 1.8f), 34.0f);
	TestEqual(TEXT("Crit multiplies base damage"), FCombatFormulas::ApplyCrit(34.0f, true, 1.8f), 61.2f);
	TestEqual(TEXT("Crit damage below one cannot reduce damage"), FCombatFormulas::ApplyCrit(34.0f, true, 0.5f), 34.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCombatPowerFormulaTest,
	"IdleProject.Character.CombatPower.Formula",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCombatPowerFormulaTest::RunTest(const FString& Parameters)
{
	FDerivedStats Stats;
	Stats.Hp = 1234.0f;
	Stats.PhysAtk = 100.0f;
	Stats.MagicAtk = 50.0f;
	Stats.PhysDef = 30.0f;
	Stats.MagicDef = 20.0f;
	Stats.AtkSpeed = 1.5f;
	Stats.CritRate = 0.25f;
	Stats.CritDmg = 1.8f;

	TestEqual(TEXT("Weighted derived stats round to a stable int64 combat power"), FCombatPowerFormula::ComputeCombatPower(Stats), static_cast<int64>(978));

	Stats.Hp = 1234567.0f;
	Stats.PhysAtk = 10000000.0f;
	Stats.MagicAtk = 3000000.0f;
	Stats.PhysDef = 100000.0f;
	Stats.MagicDef = 50000.0f;
	Stats.AtkSpeed = 2.25f;
	Stats.CritRate = 0.333f;
	Stats.CritDmg = 2.75f;
	TestEqual(TEXT("Large derived stats use double precision parity anchor"), FCombatPowerFormula::ComputeCombatPower(Stats), static_cast<int64>(13424348));

	TestEqual(TEXT("Zero stats produce zero combat power"), FCombatPowerFormula::ComputeCombatPower(FDerivedStats()), static_cast<int64>(0));

	Stats.Hp = -100.0f;
	Stats.PhysAtk = -20.0f;
	Stats.MagicAtk = -10.0f;
	Stats.PhysDef = -5.0f;
	Stats.MagicDef = -5.0f;
	Stats.AtkSpeed = -1.0f;
	Stats.CritRate = -0.2f;
	Stats.CritDmg = -1.0f;
	TestEqual(TEXT("Negative weighted total clamps to zero combat power"), FCombatPowerFormula::ComputeCombatPower(Stats), static_cast<int64>(0));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCombatStatusDamageOverTimeTest,
	"IdleProject.Combat.Status.DamageOverTime",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCombatStatusDamageOverTimeTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UCombatComponent* Combat = NewObject<UCombatComponent>(Owner);
	Owner->AddInstanceComponent(Combat);
	Combat->InitializeCombat(100.0f, 10.0f, 0.0f, 1.0f);

	Combat->ApplyStatus(ESkillStatusEffect::Poison, 3.0f, 4.0f, 10.0f);
	Combat->TickStatuses(10.5f);
	TestEqual(TEXT("DoT waits until the first one-second tick"), Combat->CurrentHp, 100.0f);
	Combat->TickStatuses(11.0f);
	TestEqual(TEXT("Poison deals one tick of damage"), Combat->CurrentHp, 96.0f);
	Combat->TickStatuses(12.0f);
	TestEqual(TEXT("Poison deals a second tick of damage"), Combat->CurrentHp, 92.0f);
	Combat->TickStatuses(13.1f);
	TestEqual(TEXT("Expired poison stops ticking"), Combat->CurrentHp, 92.0f);
	TestFalse(TEXT("Expired poison is removed"), Combat->HasActiveStatus(ESkillStatusEffect::Poison));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCombatStatusDamageOverTimeCatchesUpTest,
	"IdleProject.Combat.Status.DamageOverTimeCatchesUp",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCombatStatusDamageOverTimeCatchesUpTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UCombatComponent* Combat = NewObject<UCombatComponent>(Owner);
	Owner->AddInstanceComponent(Combat);
	Combat->InitializeCombat(100.0f, 10.0f, 0.0f, 1.0f);

	Combat->ApplyStatus(ESkillStatusEffect::Burn, 4.0f, 5.0f, 10.0f);
	Combat->TickStatuses(12.6f);

	TestEqual(TEXT("Delayed status ticking catches up all elapsed one-second ticks"), Combat->CurrentHp, 90.0f);
	TestTrue(TEXT("Burn remains active until its end time"), Combat->HasActiveStatus(ESkillStatusEffect::Burn));

	Combat->TickStatuses(14.2f);

	TestEqual(TEXT("Catch-up never applies a tick at or beyond expiry"), Combat->CurrentHp, 85.0f);
	TestFalse(TEXT("Burn is removed after expiry"), Combat->HasActiveStatus(ESkillStatusEffect::Burn));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCombatStatusFreezeSlowTest,
	"IdleProject.Combat.Status.FreezeSlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCombatStatusFreezeSlowTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UCombatComponent* Combat = NewObject<UCombatComponent>(Owner);
	Owner->AddInstanceComponent(Combat);
	Combat->InitializeCombat(100.0f, 10.0f, 0.0f, 2.0f);

	Combat->ApplyStatus(ESkillStatusEffect::Freeze, 2.0f, 0.25f, 10.0f);
	Combat->TickStatuses(10.0f);
	TestEqual(TEXT("Freeze applies attack speed slow"), Combat->AtkSpeed, 1.5f);
	TestTrue(TEXT("Freeze is active before expiry"), Combat->HasActiveStatus(ESkillStatusEffect::Freeze));

	Combat->TickStatuses(12.1f);
	TestEqual(TEXT("Freeze slow is removed on expiry"), Combat->AtkSpeed, 2.0f);
	TestFalse(TEXT("Expired freeze is removed"), Combat->HasActiveStatus(ESkillStatusEffect::Freeze));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCombatStatusFreezeReapplyTest,
	"IdleProject.Combat.Status.FreezeReapplyRestoresAttackSpeed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCombatStatusFreezeReapplyTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UCombatComponent* Combat = NewObject<UCombatComponent>(Owner);
	Owner->AddInstanceComponent(Combat);
	Combat->InitializeCombat(100.0f, 10.0f, 0.0f, 2.0f);

	Combat->ApplyStatus(ESkillStatusEffect::Freeze, 2.0f, 0.25f, 10.0f);
	TestEqual(TEXT("Initial freeze slow applies once"), Combat->AtkSpeed, 1.5f);

	Combat->ApplyStatus(ESkillStatusEffect::Freeze, 3.0f, 0.50f, 11.0f);
	TestEqual(TEXT("Reapplied freeze restores previous slow before applying stronger slow"), Combat->AtkSpeed, 1.0f);

	Combat->TickStatuses(14.1f);
	TestEqual(TEXT("Reapplied freeze slow restores to base attack speed after expiry"), Combat->AtkSpeed, 2.0f);
	TestFalse(TEXT("Reapplied freeze is removed after expiry"), Combat->HasActiveStatus(ESkillStatusEffect::Freeze));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCombatStatusCurseAmplifyTest,
	"IdleProject.Combat.Status.CurseAmplifiesDamage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCombatStatusCurseAmplifyTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UCombatComponent* Combat = NewObject<UCombatComponent>(Owner);
	Owner->AddInstanceComponent(Combat);
	Combat->InitializeCombat(1000.0f, 10.0f, 0.0f, 1.0f);

	// 저주 활성: 받는 피해 +20% (100 -> 120)
	Combat->ApplyStatus(ESkillStatusEffect::Curse, 4.0f, 0.2f, 10.0f);
	TestTrue(TEXT("Curse is active after applying"), Combat->HasActiveStatus(ESkillStatusEffect::Curse));
	TestEqual(TEXT("Curse magnitude is reported"), Combat->GetActiveStatusMagnitude(ESkillStatusEffect::Curse), 0.2f);

	Combat->TakeDamageTyped(100.0f, Owner, false, EDamageKind::Physical);
	TestEqual(TEXT("Curse amplifies incoming damage by magnitude"), Combat->CurrentHp, 880.0f);

	// 만료 후 원복: 증폭 없는 100 피해
	Combat->TickStatuses(15.0f);
	TestFalse(TEXT("Expired curse is removed"), Combat->HasActiveStatus(ESkillStatusEffect::Curse));
	TestEqual(TEXT("Removed curse no longer reports magnitude"), Combat->GetActiveStatusMagnitude(ESkillStatusEffect::Curse), 0.0f);

	Combat->TakeDamageTyped(100.0f, Owner, false, EDamageKind::Physical);
	TestEqual(TEXT("Damage returns to base after curse expiry"), Combat->CurrentHp, 780.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCombatStatusCurseRegressionTest,
	"IdleProject.Combat.Status.CurseAbsentDamageUnchanged",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCombatStatusCurseRegressionTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UCombatComponent* Combat = NewObject<UCombatComponent>(Owner);
	Owner->AddInstanceComponent(Combat);
	Combat->InitializeCombat(1000.0f, 10.0f, 0.0f, 1.0f);

	// 저주가 없으면 피해 불변(회귀)
	TestEqual(TEXT("No curse reports zero magnitude"), Combat->GetActiveStatusMagnitude(ESkillStatusEffect::Curse), 0.0f);
	Combat->TakeDamageTyped(100.0f, Owner, false, EDamageKind::Physical);
	TestEqual(TEXT("Damage is unchanged without curse"), Combat->CurrentHp, 900.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCombatStatusCurseIndependentOfDoTTest,
	"IdleProject.Combat.Status.CurseIndependentOfOtherStatuses",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCombatStatusCurseIndependentOfDoTTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UCombatComponent* Combat = NewObject<UCombatComponent>(Owner);
	Owner->AddInstanceComponent(Combat);
	Combat->InitializeCombat(1000.0f, 10.0f, 0.0f, 2.0f);

	// 저주 + 중독 + 빙결 동시 적용 -> 각각 독립 동작
	Combat->ApplyStatus(ESkillStatusEffect::Curse, 5.0f, 0.5f, 10.0f);
	Combat->ApplyStatus(ESkillStatusEffect::Poison, 5.0f, 4.0f, 10.0f);
	Combat->ApplyStatus(ESkillStatusEffect::Freeze, 5.0f, 0.25f, 10.0f);

	TestTrue(TEXT("Curse coexists with poison"), Combat->HasActiveStatus(ESkillStatusEffect::Curse));
	TestTrue(TEXT("Poison coexists with curse"), Combat->HasActiveStatus(ESkillStatusEffect::Poison));
	TestTrue(TEXT("Freeze coexists with curse"), Combat->HasActiveStatus(ESkillStatusEffect::Freeze));

	// 빙결 슬로우는 저주와 독립으로 적용
	Combat->TickStatuses(10.0f);
	TestEqual(TEXT("Freeze slow applies independently of curse"), Combat->AtkSpeed, 1.5f);

	// 중독 DoT 1틱(4) 도 저주 50% 증폭을 받아 6 피해
	Combat->TickStatuses(11.0f);
	TestEqual(TEXT("Poison DoT tick is amplified by active curse"), Combat->CurrentHp, 994.0f);

	// 직접 피해도 저주 증폭(100 -> 150)
	Combat->TakeDamageTyped(100.0f, Owner, false, EDamageKind::Physical);
	TestEqual(TEXT("Direct damage amplified by curse alongside other statuses"), Combat->CurrentHp, 844.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillDarkCurseApplicationTest,
	"IdleProject.Combat.Skills.DarkSkillAppliesCurse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillDarkCurseApplicationTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UCombatComponent* OwnerCombat = NewObject<UCombatComponent>(Owner);
	USkillComponent* Skills = NewObject<USkillComponent>(Owner);
	Owner->AddInstanceComponent(OwnerCombat);
	Owner->AddInstanceComponent(Skills);

	AIdleMonster* Target = NewObject<AIdleMonster>();
	UCombatComponent* TargetCombat = Target->GetCombat();
	TestNotNull(TEXT("Monster combat component exists"), TargetCombat);
	if (!TargetCombat)
	{
		return false;
	}

	OwnerCombat->InitializeCombat(1000.0f, 100.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.5f);
	TargetCombat->InitializeCombat(1000.0f, 10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.5f);

	// Thief 의 Shadow Stab(Dark)이 적중 시 저주를 부여
	Skills->LoadDefaultThiefSkills();
	Skills->MarkSkillCast(TEXT("smoke_bomb"), 10.0f);
	Skills->MarkSkillCast(TEXT("backstab"), 10.0f);

	TArray<AActor*> AoeTargets;
	Skills->TickSkills(10.0f, Target, AoeTargets);

	TestTrue(TEXT("Dark skill applies curse status"), TargetCombat->HasActiveStatus(ESkillStatusEffect::Curse));
	TestEqual(TEXT("Curse magnitude matches Dark skill definition"), TargetCombat->GetActiveStatusMagnitude(ESkillStatusEffect::Curse), 0.2f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillElementStatusApplicationTest,
	"IdleProject.Combat.Skills.ElementStatusApplication",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillElementStatusApplicationTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UCombatComponent* OwnerCombat = NewObject<UCombatComponent>(Owner);
	USkillComponent* Skills = NewObject<USkillComponent>(Owner);
	Owner->AddInstanceComponent(OwnerCombat);
	Owner->AddInstanceComponent(Skills);

	AIdleMonster* Target = NewObject<AIdleMonster>();
	UCombatComponent* TargetCombat = Target->GetCombat();
	TestNotNull(TEXT("Monster combat component exists"), TargetCombat);
	if (!TargetCombat)
	{
		return false;
	}

	OwnerCombat->InitializeCombat(1000.0f, 10.0f, 0.0f, 1.0f, 100.0f, 0.0f, 0.0f, 1.5f);
	TargetCombat->InitializeCombat(1000.0f, 10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.5f);
	Target->SetWeakElement(ESkillElement::Fire);
	Skills->LoadDefaultMageSkills();
	Skills->MarkSkillCast(TEXT("chain_lightning"), 10.0f);
	Skills->MarkSkillCast(TEXT("mana_shield"), 10.0f);
	Skills->MarkSkillCast(TEXT("meteor"), 10.0f);

	TArray<AActor*> AoeTargets;
	Skills->TickSkills(10.0f, Target, AoeTargets);

	TestEqual(TEXT("Fire skill applies weakness multiplier to magic damage"), TargetCombat->CurrentHp, 640.0f);
	TestTrue(TEXT("Fire skill applies burn status"), TargetCombat->HasActiveStatus(ESkillStatusEffect::Burn));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCombatDamageReceivedEventTest,
	"IdleProject.Combat.Component.DamageReceivedEvent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCombatDamageReceivedEventTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UCombatComponent* Combat = NewObject<UCombatComponent>(Owner);
	Owner->AddInstanceComponent(Combat);
	Combat->InitializeCombat(100.0f, 10.0f, 0.0f, 1.0f);
	UDamageReceivedTestReceiver* Receiver = NewObject<UDamageReceivedTestReceiver>();
	Combat->OnDamageReceived.AddDynamic(Receiver, &UDamageReceivedTestReceiver::Capture);

	Combat->TakeDamageTyped(25.0f, Owner, true, EDamageKind::Magic);

	TestEqual(TEXT("Damage event amount matches final damage"), Receiver->LastAmount, 25.0f);
	TestTrue(TEXT("Damage event captures crit flag"), Receiver->bLastWasCrit);
	TestEqual(TEXT("Damage event captures damage kind"), static_cast<int32>(Receiver->LastKind), static_cast<int32>(EDamageKind::Magic));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCombatAnyDamageReceivedEventTest,
	"IdleProject.Combat.Component.AnyDamageReceivedEvent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCombatAnyDamageReceivedEventTest::RunTest(const FString& Parameters)
{
	AActor* FirstOwner = NewObject<AActor>();
	UCombatComponent* FirstCombat = NewObject<UCombatComponent>(FirstOwner);
	FirstOwner->AddInstanceComponent(FirstCombat);
	FirstCombat->InitializeCombat(100.0f, 10.0f, 0.0f, 1.0f);

	AActor* RespawnedOwner = NewObject<AActor>();
	UCombatComponent* RespawnedCombat = NewObject<UCombatComponent>(RespawnedOwner);
	RespawnedOwner->AddInstanceComponent(RespawnedCombat);
	RespawnedCombat->InitializeCombat(100.0f, 10.0f, 0.0f, 1.0f);

	AActor* LastDamagedActor = nullptr;
	float LastAmount = 0.0f;
	bool bLastWasCrit = false;
	EDamageKind LastKind = EDamageKind::Physical;
	int32 EventCount = 0;

	const FDelegateHandle Handle = UCombatComponent::OnAnyDamageReceived.AddLambda(
		[&LastDamagedActor, &LastAmount, &bLastWasCrit, &LastKind, &EventCount](
			AActor* DamagedActor,
			float Amount,
			bool bWasCrit,
			EDamageKind Kind)
		{
			LastDamagedActor = DamagedActor;
			LastAmount = Amount;
			bLastWasCrit = bWasCrit;
			LastKind = Kind;
			++EventCount;
		});

	FirstCombat->TakeDamageTyped(12.0f, FirstOwner, false, EDamageKind::Physical);
	RespawnedCombat->TakeDamageTyped(34.0f, FirstOwner, true, EDamageKind::Magic);

	UCombatComponent::OnAnyDamageReceived.Remove(Handle);

	TestEqual(TEXT("Global damage event captures both original and respawned targets"), EventCount, 2);
	TestEqual(TEXT("Global damage event reports latest damaged actor"), LastDamagedActor, RespawnedOwner);
	TestEqual(TEXT("Global damage event reports amount"), LastAmount, 34.0f);
	TestTrue(TEXT("Global damage event reports crit flag"), bLastWasCrit);
	TestEqual(TEXT("Global damage event reports damage kind"), static_cast<int32>(LastKind), static_cast<int32>(EDamageKind::Magic));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCombatClassDamageTest,
	"IdleProject.Combat.Formulas.ClassDamage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCombatClassDamageTest::RunTest(const FString& Parameters)
{
	FDerivedStats MageStats;
	MageStats.PhysAtk = 12.0f;
	MageStats.MagicAtk = 40.0f;
	TestEqual(TEXT("Mage damage uses magic attack and magic defense"), FCombatFormulas::ComputeDamage(MageStats, EClassId::Mage, 20.0f, 10.0f), 34.0f);

	FDerivedStats ArcherStats;
	ArcherStats.PhysAtk = 40.0f;
	ArcherStats.MagicAtk = 12.0f;
	ArcherStats.CritRate = 0.25f;
	ArcherStats.CritDmg = 1.8f;
	TestEqual(TEXT("Archer damage uses physical attack and physical defense"), FCombatFormulas::ComputeDamage(ArcherStats, EClassId::Archer, 10.0f, 80.0f), 34.0f);

	FDerivedStats ThiefStats;
	ThiefStats.PhysAtk = 40.0f;
	ThiefStats.MagicAtk = 12.0f;
	TestEqual(TEXT("Thief damage uses physical attack and physical defense"), FCombatFormulas::ComputeDamage(ThiefStats, EClassId::Thief, 10.0f, 80.0f), 34.0f);

	FDerivedStats ClericStats;
	ClericStats.PhysAtk = 12.0f;
	ClericStats.MagicAtk = 40.0f;
	TestEqual(TEXT("Cleric damage uses magic attack and magic defense"), FCombatFormulas::ComputeDamage(ClericStats, EClassId::Cleric, 80.0f, 10.0f), 34.0f);

	FDerivedStats SummonerStats;
	SummonerStats.PhysAtk = 12.0f;
	SummonerStats.MagicAtk = 40.0f;
	TestEqual(TEXT("Summoner damage uses magic attack and magic defense"), FCombatFormulas::ComputeDamage(SummonerStats, EClassId::Summoner, 80.0f, 10.0f), 34.0f);

	FDerivedStats WarriorStats;
	WarriorStats.PhysAtk = 40.0f;
	WarriorStats.MagicAtk = 80.0f;
	TestEqual(TEXT("Warrior damage keeps physical attack and physical defense"), FCombatFormulas::ComputeDamage(WarriorStats, EClassId::Warrior, 10.0f, 80.0f), 34.0f);

	FDerivedStats PaladinStats;
	PaladinStats.PhysAtk = 40.0f;
	PaladinStats.MagicAtk = 80.0f;
	TestEqual(TEXT("Paladin damage keeps physical attack and physical defense"), FCombatFormulas::ComputeDamage(PaladinStats, EClassId::Paladin, 10.0f, 80.0f), 34.0f);

	FDerivedStats BerserkerStats;
	BerserkerStats.PhysAtk = 40.0f;
	BerserkerStats.MagicAtk = 80.0f;
	TestEqual(TEXT("Berserker damage keeps physical attack and physical defense"), FCombatFormulas::ComputeDamage(BerserkerStats, EClassId::Berserker, 10.0f, 80.0f), 34.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCombatComponentExtendedStatsTest,
	"IdleProject.Combat.Component.ExtendedStats",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCombatComponentExtendedStatsTest::RunTest(const FString& Parameters)
{
	UCombatComponent* Combat = NewObject<UCombatComponent>();
	Combat->InitializeCombat(1000.0f, 50.0f, 20.0f, 1.0f, 80.0f, 30.0f, 0.25f, 1.8f);

	TestEqual(TEXT("Magic attack is initialized"), Combat->MagicAtk, 80.0f);
	TestEqual(TEXT("Magic defense is initialized"), Combat->MagicDef, 30.0f);
	TestEqual(TEXT("Crit rate is initialized"), Combat->CritRate, 0.25f);
	TestEqual(TEXT("Crit damage is initialized"), Combat->CritDmg, 1.8f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillMagicDamageTest,
	"IdleProject.Combat.Skills.MagicDamage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillMagicDamageTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UCombatComponent* OwnerCombat = NewObject<UCombatComponent>(Owner);
	USkillComponent* Skills = NewObject<USkillComponent>(Owner);
	Owner->AddInstanceComponent(OwnerCombat);
	Owner->AddInstanceComponent(Skills);

	AActor* Target = NewObject<AActor>();
	UCombatComponent* TargetCombat = NewObject<UCombatComponent>(Target);
	Target->AddInstanceComponent(TargetCombat);

	OwnerCombat->InitializeCombat(1000.0f, 10.0f, 0.0f, 1.0f, 80.0f, 0.0f, 0.0f, 1.5f);
	TargetCombat->InitializeCombat(1000.0f, 10.0f, 10.0f, 1.0f, 0.0f, 30.0f, 0.0f, 1.5f);
	UDamageReceivedTestReceiver* Receiver = NewObject<UDamageReceivedTestReceiver>();
	TargetCombat->OnDamageReceived.AddDynamic(Receiver, &UDamageReceivedTestReceiver::Capture);
	Skills->LoadDefaultMageSkills();

	Skills->MarkSkillCast(TEXT("chain_lightning"), 10.0f);
	Skills->MarkSkillCast(TEXT("mana_shield"), 10.0f);
	Skills->MarkSkillCast(TEXT("meteor"), 10.0f);
	Skills->TickSkills(12.0f, Target, TArray<AActor*>());

	TestEqual(TEXT("Arcane bolt uses MagicAtk vs MagicDef"), TargetCombat->CurrentHp, 1000.0f - 174.0f);
	TestEqual(TEXT("Magic skill broadcasts final damage amount"), Receiver->LastAmount, 174.0f);
	TestFalse(TEXT("Magic skill broadcasts non-crit flag"), Receiver->bLastWasCrit);
	TestEqual(TEXT("Magic skill broadcasts magic damage kind"), static_cast<int32>(Receiver->LastKind), static_cast<int32>(EDamageKind::Magic));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillCritDamageTest,
	"IdleProject.Combat.Skills.CritDamage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillCritDamageTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UCombatComponent* OwnerCombat = NewObject<UCombatComponent>(Owner);
	USkillComponent* Skills = NewObject<USkillComponent>(Owner);
	Owner->AddInstanceComponent(OwnerCombat);
	Owner->AddInstanceComponent(Skills);

	AActor* Target = NewObject<AActor>();
	UCombatComponent* TargetCombat = NewObject<UCombatComponent>(Target);
	Target->AddInstanceComponent(TargetCombat);

	OwnerCombat->InitializeCombat(1000.0f, 40.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 2.0f);
	TargetCombat->InitializeCombat(1000.0f, 10.0f, 10.0f, 1.0f);
	UDamageReceivedTestReceiver* Receiver = NewObject<UDamageReceivedTestReceiver>();
	TargetCombat->OnDamageReceived.AddDynamic(Receiver, &UDamageReceivedTestReceiver::Capture);
	Skills->LoadDefaultArcherSkills();

	Skills->MarkSkillCast(TEXT("arrow_rain"), 10.0f);
	Skills->MarkSkillCast(TEXT("focus"), 10.0f);
	Skills->MarkSkillCast(TEXT("piercing_arrow"), 10.0f);
	Skills->TickSkills(12.0f, Target, TArray<AActor*>());

	TestEqual(TEXT("Precision shot applies guaranteed crit after physical mitigation"), TargetCombat->CurrentHp, 1000.0f - 164.0f);
	TestEqual(TEXT("Physical skill broadcasts crit damage amount"), Receiver->LastAmount, 164.0f);
	TestTrue(TEXT("Physical skill broadcasts crit flag"), Receiver->bLastWasCrit);
	TestEqual(TEXT("Physical skill broadcasts physical damage kind"), static_cast<int32>(Receiver->LastKind), static_cast<int32>(EDamageKind::Physical));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBattleAIGroundChaseTest,
	"IdleProject.Combat.BattleAI.GroundChaseLocation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBattleAIGroundChaseTest::RunTest(const FString& Parameters)
{
	// FVector 성분은 double(FVector3d) 이므로 기대값·허용오차도 double 로 맞춘다 (오버로드 모호성 회피).
	constexpr double Tolerance = 0.01;

	// 몬스터(Z=-56)가 더 높은 캡슐 중심의 타깃(Z=-12, Y=50)을 추격해도
	// 자신의 Z·Y 는 유지하고 X 로만 전진해야 한다 (위로 떠오르지 않음).
	{
		const FVector Owner(0.0, 0.0, -56.0);
		const FVector Target(1000.0, 50.0, -12.0);
		const FVector Next = UBattleAIComponent::ComputeGroundChaseLocation(Owner, Target, 0.2f, 220.0f);

		TestEqual(TEXT("Z 는 추격 주체 값 유지 (위로 끌려가지 않음)"), Next.Z, -56.0, Tolerance);
		TestEqual(TEXT("Y 는 추격 주체 값 유지 (깊이 고정)"), Next.Y, 0.0, Tolerance);
		TestEqual(TEXT("X 는 속도×시간(=44)만큼 타깃 방향 전진"), Next.X, 44.0, Tolerance);
	}

	// 음(-)의 X 방향 타깃: X 만 음수로 전진, Z 유지.
	{
		const FVector Owner(0.0, 0.0, -56.0);
		const FVector Target(-1000.0, 0.0, 300.0);
		const FVector Next = UBattleAIComponent::ComputeGroundChaseLocation(Owner, Target, 0.2f, 220.0f);

		TestEqual(TEXT("음수 X 방향 전진"), Next.X, -44.0, Tolerance);
		TestEqual(TEXT("높은 타깃이어도 Z 유지"), Next.Z, -56.0, Tolerance);
	}

	// 한 스텝 내 도달 가능한 가까운 타깃: X 로 정확히 도달, Z 유지.
	{
		const FVector Owner(0.0, 0.0, -56.0);
		const FVector Target(10.0, 0.0, 500.0);
		const FVector Next = UBattleAIComponent::ComputeGroundChaseLocation(Owner, Target, 0.2f, 220.0f);

		TestEqual(TEXT("가까운 타깃 X 도달"), Next.X, 10.0, Tolerance);
		TestEqual(TEXT("도달 시에도 Z 유지"), Next.Z, -56.0, Tolerance);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillCooldownTest,
	"IdleProject.Combat.Skills.CooldownReadiness",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillCooldownTest::RunTest(const FString& Parameters)
{
	USkillComponent* Skills = NewObject<USkillComponent>();
	Skills->LoadDefaultWarriorSkills();

	const FName HeavyStrike(TEXT("heavy_strike"));
	TestTrue(TEXT("Heavy strike is ready before first cast"), Skills->IsReady(HeavyStrike, 10.0f));
	TestEqual(TEXT("Ready skill has zero cooldown remaining"), Skills->GetCooldownRemaining(HeavyStrike, 10.0f), 0.0f);
	TestEqual(TEXT("Ready skill has zero cooldown ratio"), Skills->GetCooldownRatio(HeavyStrike, 10.0f), 0.0f);

	TestTrue(TEXT("Cast starts cooldown"), Skills->MarkSkillCast(HeavyStrike, 10.0f));
	TestFalse(TEXT("Heavy strike is not ready during cooldown"), Skills->IsReady(HeavyStrike, 12.0f));
	TestEqual(TEXT("Cooldown remaining is seconds until ready"), Skills->GetCooldownRemaining(HeavyStrike, 12.0f), 2.0f);
	TestEqual(TEXT("Cooldown ratio reflects elapsed cooldown"), Skills->GetCooldownRatio(HeavyStrike, 12.0f), 0.5f);
	TestTrue(TEXT("Heavy strike is ready when cooldown expires"), Skills->IsReady(HeavyStrike, 14.0f));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRankPointsTest,
	"IdleProject.Combat.Skills.RankPoints",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRankPointsTest::RunTest(const FString& Parameters)
{
	USkillComponent* Skills = NewObject<USkillComponent>();
	Skills->LoadDefaultWarriorSkills();

	const FName HeavyStrike(TEXT("heavy_strike"));
	TestEqual(TEXT("Initial skill points are zero"), Skills->GetSkillPoints(), static_cast<int32>(0));
	TestEqual(TEXT("Initial skill rank is zero"), Skills->GetSkillRank(HeavyStrike), static_cast<int32>(0));
	TestFalse(TEXT("Unknown skill cannot rank up"), Skills->CanRankUp(TEXT("missing_skill")));

	Skills->GrantSkillPoint(2);
	Skills->GrantSkillPoint(-1);
	TestEqual(TEXT("Positive grants add points and negative grants are ignored"), Skills->GetSkillPoints(), static_cast<int32>(2));
	TestTrue(TEXT("Known skill can rank up when points are available"), Skills->CanRankUp(HeavyStrike));

	TestTrue(TEXT("First rank up succeeds"), Skills->RankUpSkill(HeavyStrike));
	TestTrue(TEXT("Second rank up succeeds"), Skills->RankUpSkill(HeavyStrike));
	TestEqual(TEXT("Rank up spends one point per rank"), Skills->GetSkillPoints(), static_cast<int32>(0));
	TestEqual(TEXT("Rank is stored per skill"), Skills->GetSkillRank(HeavyStrike), static_cast<int32>(2));
	TestFalse(TEXT("Cannot rank up without points"), Skills->RankUpSkill(HeavyStrike));

	Skills->GrantSkillPoint(60);
	for (int32 Index = 0; Index < 58; ++Index)
	{
		Skills->RankUpSkill(HeavyStrike);
	}
	TestEqual(TEXT("Rank is capped at max rank fifty"), Skills->GetSkillRank(HeavyStrike), static_cast<int32>(50));
	TestFalse(TEXT("Max-rank skill cannot rank up"), Skills->CanRankUp(HeavyStrike));
	TestFalse(TEXT("Rank up fails at max rank even with remaining points"), Skills->RankUpSkill(HeavyStrike));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRankEffectiveValuesTest,
	"IdleProject.Combat.Skills.RankEffectiveValues",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRankEffectiveValuesTest::RunTest(const FString& Parameters)
{
	USkillComponent* Skills = NewObject<USkillComponent>();
	Skills->LoadDefaultWarriorSkills();

	const FName HeavyStrike(TEXT("heavy_strike"));
	TestEqual(TEXT("Rank zero damage coeff uses base value"), Skills->GetEffectiveDamageCoeff(HeavyStrike), 2.5f);
	TestEqual(TEXT("Rank zero cooldown uses base value"), Skills->GetEffectiveCooldown(HeavyStrike), 4.0f);

	Skills->GrantSkillPoint(2);
	Skills->RankUpSkill(HeavyStrike);
	Skills->RankUpSkill(HeavyStrike);

	TestEqual(TEXT("Rank two damage coeff adds twenty percent"), Skills->GetEffectiveDamageCoeff(HeavyStrike), 3.0f);
	TestEqual(TEXT("Rank two cooldown reduces ten percent"), Skills->GetEffectiveCooldown(HeavyStrike), 3.6f);

	Skills->MarkSkillCast(HeavyStrike, 10.0f);
	TestEqual(TEXT("Cooldown remaining uses effective cooldown"), Skills->GetCooldownRemaining(HeavyStrike, 12.0f), 1.6f);
	TestEqual(TEXT("Cooldown ratio uses effective cooldown"), Skills->GetCooldownRatio(HeavyStrike, 12.0f), 1.6f / 3.6f);

	Skills->GrantSkillPoint(3);
	Skills->RankUpSkill(HeavyStrike);
	Skills->RankUpSkill(HeavyStrike);
	Skills->RankUpSkill(HeavyStrike);

	TestEqual(TEXT("Rank five damage coeff keeps legacy fifty percent gain"), Skills->GetEffectiveDamageCoeff(HeavyStrike), 3.75f);
	TestEqual(TEXT("Rank five cooldown keeps legacy twenty five percent reduction"), Skills->GetEffectiveCooldown(HeavyStrike), 3.0f);

	Skills->GrantSkillPoint(15);
	for (int32 Index = 0; Index < 15; ++Index)
	{
		Skills->RankUpSkill(HeavyStrike);
	}

	TestEqual(TEXT("Rank twenty cooldown floors at point one seconds"), Skills->GetEffectiveCooldown(HeavyStrike), 0.1f);
	TestEqual(TEXT("Rank twenty damage coeff triples base"), Skills->GetEffectiveDamageCoeff(HeavyStrike), 7.5f);

	Skills->GrantSkillPoint(30);
	for (int32 Index = 0; Index < 30; ++Index)
	{
		Skills->RankUpSkill(HeavyStrike);
	}

	TestEqual(TEXT("Rank fifty damage coeff reaches six times base"), Skills->GetEffectiveDamageCoeff(HeavyStrike), 15.0f);
	TestEqual(TEXT("Rank fifty cooldown remains at point one seconds"), Skills->GetEffectiveCooldown(HeavyStrike), 0.1f);
	TestEqual(TEXT("Zero-cooldown ultimate remains zero"), Skills->GetEffectiveCooldown(TEXT("berserkers_fury")), 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRankDamageApplicationTest,
	"IdleProject.Combat.Skills.RankDamageApplication",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRankDamageApplicationTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UCombatComponent* OwnerCombat = NewObject<UCombatComponent>(Owner);
	USkillComponent* Skills = NewObject<USkillComponent>(Owner);
	Owner->AddInstanceComponent(OwnerCombat);
	Owner->AddInstanceComponent(Skills);

	AActor* Target = NewObject<AActor>();
	UCombatComponent* TargetCombat = NewObject<UCombatComponent>(Target);
	Target->AddInstanceComponent(TargetCombat);

	OwnerCombat->InitializeCombat(1000.0f, 100.0f, 0.0f, 1.0f);
	TargetCombat->InitializeCombat(1000.0f, 10.0f, 0.0f, 1.0f);
	Skills->LoadDefaultWarriorSkills();
	Skills->GrantSkillPoint(1);
	Skills->RankUpSkill(TEXT("heavy_strike"));
	Skills->MarkSkillCast(TEXT("whirlwind"), 19.0f);
	Skills->MarkSkillCast(TEXT("shield_up"), 19.0f);
	Skills->MarkSkillCast(TEXT("charge"), 19.0f);

	Skills->TickSkills(20.0f, Target, TArray<AActor*>());

	TestEqual(TEXT("Ranked heavy strike uses effective damage coefficient"), TargetCombat->CurrentHp, 725.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleCharacterLevelUpSkillPointTest,
	"IdleProject.Character.LevelUp.GrantsSkillPoint",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleCharacterLevelUpSkillPointTest::RunTest(const FString& Parameters)
{
	AIdleCharacter* Character = NewObject<AIdleCharacter>();
	TestNotNull(TEXT("Idle character is created"), Character);
	if (!Character)
	{
		return false;
	}

	USkillComponent* Skills = Character->FindComponentByClass<USkillComponent>();
	TestNotNull(TEXT("Skill component exists"), Skills);
	if (!Skills)
	{
		return false;
	}

	const int32 BeforePoints = Skills->GetSkillPoints();
	Character->HandleLevelUp(2);

	TestEqual(TEXT("Level-up handler grants one skill point"), Skills->GetSkillPoints(), BeforePoints + 1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillClassDefaultsTest,
	"IdleProject.Combat.Skills.ClassDefaults",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillClassDefaultsTest::RunTest(const FString& Parameters)
{
	USkillComponent* Skills = NewObject<USkillComponent>();

	Skills->LoadDefaultWarriorSkills();
	TestEqual(TEXT("Warrior has eight skills"), Skills->Skills.Num(), 8);
	TestEqual(TEXT("Warrior has five active skills"), CountSkillsByType(*Skills, ESkillType::Active), 5);
	TestEqual(TEXT("Warrior has two passive skills"), CountSkillsByType(*Skills, ESkillType::Passive), 2);
	TestEqual(TEXT("Warrior has one ultimate skill"), CountSkillsByType(*Skills, ESkillType::Ultimate), 1);
	TestTrue(TEXT("Warrior loads heavy strike"), HasSkill(*Skills, TEXT("heavy_strike")));
	TestTrue(TEXT("Warrior loads berserker fury"), HasSkill(*Skills, TEXT("berserkers_fury")));
	TestTrue(TEXT("Warrior loads earthen_cleave"), HasSkill(*Skills, TEXT("earthen_cleave")));

	Skills->LoadDefaultMageSkills();
	TestEqual(TEXT("Mage has eight skills"), Skills->Skills.Num(), 8);
	TestEqual(TEXT("Mage has five active skills"), CountSkillsByType(*Skills, ESkillType::Active), 5);
	TestEqual(TEXT("Mage has two passive skills"), CountSkillsByType(*Skills, ESkillType::Passive), 2);
	TestEqual(TEXT("Mage has one ultimate skill"), CountSkillsByType(*Skills, ESkillType::Ultimate), 1);
	TestTrue(TEXT("Mage loads arcane bolt"), HasSkill(*Skills, TEXT("arcane_bolt")));
	TestTrue(TEXT("Mage loads meteor"), HasSkill(*Skills, TEXT("meteor")));
	TestTrue(TEXT("Mage loads flame_storm"), HasSkill(*Skills, TEXT("flame_storm")));

	Skills->LoadDefaultArcherSkills();
	TestEqual(TEXT("Archer has eight skills"), Skills->Skills.Num(), 8);
	TestEqual(TEXT("Archer has five active skills"), CountSkillsByType(*Skills, ESkillType::Active), 5);
	TestEqual(TEXT("Archer has two passive skills"), CountSkillsByType(*Skills, ESkillType::Passive), 2);
	TestEqual(TEXT("Archer has one ultimate skill"), CountSkillsByType(*Skills, ESkillType::Ultimate), 1);
	TestTrue(TEXT("Archer loads precision shot"), HasSkill(*Skills, TEXT("precision_shot")));
	TestTrue(TEXT("Archer loads arrow_rain"), HasSkill(*Skills, TEXT("arrow_rain")));
	TestTrue(TEXT("Archer loads multi_shot"), HasSkill(*Skills, TEXT("multi_shot")));

	Skills->LoadDefaultThiefSkills();
	TestEqual(TEXT("Thief has eight skills"), Skills->Skills.Num(), 8);
	TestEqual(TEXT("Thief has five active skills"), CountSkillsByType(*Skills, ESkillType::Active), 5);
	TestEqual(TEXT("Thief has two passive skills"), CountSkillsByType(*Skills, ESkillType::Passive), 2);
	TestEqual(TEXT("Thief has one ultimate skill"), CountSkillsByType(*Skills, ESkillType::Ultimate), 1);
	TestTrue(TEXT("Thief loads shadow stab"), HasSkill(*Skills, TEXT("shadow_stab")));
	TestTrue(TEXT("Thief loads smoke_bomb"), HasSkill(*Skills, TEXT("smoke_bomb")));
	TestTrue(TEXT("Thief loads shadow_strike"), HasSkill(*Skills, TEXT("shadow_strike")));

	Skills->LoadDefaultClericSkills();
	TestEqual(TEXT("Cleric has eight skills"), Skills->Skills.Num(), 8);
	TestEqual(TEXT("Cleric has five active skills"), CountSkillsByType(*Skills, ESkillType::Active), 5);
	TestEqual(TEXT("Cleric has two passive skills"), CountSkillsByType(*Skills, ESkillType::Passive), 2);
	TestEqual(TEXT("Cleric has one ultimate skill"), CountSkillsByType(*Skills, ESkillType::Ultimate), 1);
	TestTrue(TEXT("Cleric loads holy smite"), HasSkill(*Skills, TEXT("holy_smite")));
	TestTrue(TEXT("Cleric loads heal"), HasSkill(*Skills, TEXT("heal")));
	TestTrue(TEXT("Cleric loads divine_grace"), HasSkill(*Skills, TEXT("divine_grace")));

	Skills->LoadDefaultPaladinSkills();
	TestEqual(TEXT("Paladin has eight skills"), Skills->Skills.Num(), 8);
	TestEqual(TEXT("Paladin has five active skills"), CountSkillsByType(*Skills, ESkillType::Active), 5);
	TestEqual(TEXT("Paladin has two passive skills"), CountSkillsByType(*Skills, ESkillType::Passive), 2);
	TestEqual(TEXT("Paladin has one ultimate skill"), CountSkillsByType(*Skills, ESkillType::Ultimate), 1);
	TestTrue(TEXT("Paladin loads holy verdict"), HasSkill(*Skills, TEXT("holy_verdict")));
	TestTrue(TEXT("Paladin loads guardian_aegis"), HasSkill(*Skills, TEXT("guardian_aegis")));
	TestTrue(TEXT("Paladin loads judgment"), HasSkill(*Skills, TEXT("judgment")));

	Skills->LoadDefaultBerserkerSkills();
	TestEqual(TEXT("Berserker has eight skills"), Skills->Skills.Num(), 8);
	TestEqual(TEXT("Berserker has five active skills"), CountSkillsByType(*Skills, ESkillType::Active), 5);
	TestEqual(TEXT("Berserker has two passive skills"), CountSkillsByType(*Skills, ESkillType::Passive), 2);
	TestEqual(TEXT("Berserker has one ultimate skill"), CountSkillsByType(*Skills, ESkillType::Ultimate), 1);
	TestTrue(TEXT("Berserker loads rage_cleave"), HasSkill(*Skills, TEXT("rage_cleave")));
	TestTrue(TEXT("Berserker loads blood_frenzy"), HasSkill(*Skills, TEXT("blood_frenzy")));
	TestTrue(TEXT("Berserker loads blood_rage"), HasSkill(*Skills, TEXT("blood_rage")));

	Skills->LoadDefaultSummonerSkills();
	TestEqual(TEXT("Summoner has eight skills"), Skills->Skills.Num(), 8);
	TestEqual(TEXT("Summoner has five active skills"), CountSkillsByType(*Skills, ESkillType::Active), 5);
	TestEqual(TEXT("Summoner has two passive skills"), CountSkillsByType(*Skills, ESkillType::Passive), 2);
	TestEqual(TEXT("Summoner has one ultimate skill"), CountSkillsByType(*Skills, ESkillType::Ultimate), 1);
	TestTrue(TEXT("Summoner loads spirit_bolt"), HasSkill(*Skills, TEXT("spirit_bolt")));
	TestTrue(TEXT("Summoner loads grand_familiar"), HasSkill(*Skills, TEXT("grand_familiar")));
	TestTrue(TEXT("Summoner loads spirit_burst"), HasSkill(*Skills, TEXT("spirit_burst")));

	Skills->LoadSkillsForClass(EClassId::Warrior);
	TestTrue(TEXT("Class loader selects warrior skills"), HasSkill(*Skills, TEXT("heavy_strike")));
	Skills->LoadSkillsForClass(EClassId::Mage);
	TestTrue(TEXT("Class loader selects mage skills"), HasSkill(*Skills, TEXT("arcane_bolt")));
	Skills->LoadSkillsForClass(EClassId::Archer);
	TestTrue(TEXT("Class loader selects archer skills"), HasSkill(*Skills, TEXT("precision_shot")));
	Skills->LoadSkillsForClass(EClassId::Thief);
	TestTrue(TEXT("Class loader selects thief skills"), HasSkill(*Skills, TEXT("shadow_stab")));
	Skills->LoadSkillsForClass(EClassId::Cleric);
	TestTrue(TEXT("Class loader selects cleric skills"), HasSkill(*Skills, TEXT("holy_smite")));
	Skills->LoadSkillsForClass(EClassId::Paladin);
	TestTrue(TEXT("Class loader selects paladin skills"), HasSkill(*Skills, TEXT("holy_verdict")));
	Skills->LoadSkillsForClass(EClassId::Berserker);
	TestTrue(TEXT("Class loader selects berserker skills"), HasSkill(*Skills, TEXT("rage_cleave")));
	Skills->LoadSkillsForClass(EClassId::Summoner);
	TestTrue(TEXT("Class loader selects summoner skills"), HasSkill(*Skills, TEXT("spirit_bolt")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillDefinitionParityTest,
	"IdleProject.Combat.Skills.DefinitionParity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillDefinitionParityTest::RunTest(const FString& Parameters)
{
	USkillComponent* Skills = NewObject<USkillComponent>();
	int32 TotalCheckedSkills = 0;

	Skills->LoadDefaultWarriorSkills();
	const TArray<FExpectedSkillDefinition> WarriorSkills = {
		{TEXT("heavy_strike"), EClassId::Warrior, ESkillType::Active, ESkillEffectType::DamageSingle, 4.0f, 2.5f, 0.0f, 0.0f, 0.0f, 0.0f},
		{TEXT("whirlwind"), EClassId::Warrior, ESkillType::Active, ESkillEffectType::DamageAoe, 8.0f, 1.8f, 0.0f, 0.0f, 0.0f, 0.0f},
		{TEXT("shield_up"), EClassId::Warrior, ESkillType::Active, ESkillEffectType::SelfBuff, 12.0f, 0.0f, 0.5f, 4.0f, 0.0f, 0.0f},
		{TEXT("charge"), EClassId::Warrior, ESkillType::Active, ESkillEffectType::DashDamage, 10.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f},
		{TEXT("earthen_cleave"), EClassId::Warrior, ESkillType::Active, ESkillEffectType::DamageAoe, 6.0f, 3.2f, 0.0f, 0.0f, 0.0f, 0.0f},
		{TEXT("weapon_mastery"), EClassId::Warrior, ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.15f, 0.0f, 0.0f, 0.0f},
		{TEXT("toughness"), EClassId::Warrior, ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.2f, 0.0f, 0.0f, 0.0f},
		{TEXT("berserkers_fury"), EClassId::Warrior, ESkillType::Ultimate, ESkillEffectType::DamageSingle, 0.0f, 6.0f, 0.3f, 4.0f, 8.0f, 5.0f},
	};

	TestEqual(TEXT("Warrior DefinitionParity covers eight skills"), Skills->Skills.Num(), WarriorSkills.Num());
	TotalCheckedSkills += WarriorSkills.Num();
	for (const FExpectedSkillDefinition& Expected : WarriorSkills)
	{
		TestSkillDefinitionParity(*this, *Skills, Expected);
	}

	Skills->LoadDefaultMageSkills();
	const TArray<FExpectedSkillDefinition> MageSkills = {
		{TEXT("arcane_bolt"), EClassId::Mage, ESkillType::Active, ESkillEffectType::DamageSingle, 3.0f, 2.4f, 0.0f, 0.0f, 0.0f, 0.0f, ESkillStatusEffect::Burn, 3.0f, 4.0f, ESkillElement::Fire},
		{TEXT("chain_lightning"), EClassId::Mage, ESkillType::Active, ESkillEffectType::DamageAoe, 7.0f, 1.7f, 0.0f, 0.0f, 0.0f, 0.0f, ESkillStatusEffect::None, 0.0f, 0.0f, ESkillElement::Lightning},
		{TEXT("mana_shield"), EClassId::Mage, ESkillType::Active, ESkillEffectType::SelfBuff, 12.0f, 0.0f, 0.35f, 4.0f, 0.0f, 0.0f},
		{TEXT("meteor"), EClassId::Mage, ESkillType::Active, ESkillEffectType::DamageAoe, 14.0f, 2.8f, 0.0f, 0.0f, 0.0f, 0.0f, ESkillStatusEffect::Freeze, 2.0f, 0.25f, ESkillElement::Ice},
		{TEXT("flame_storm"), EClassId::Mage, ESkillType::Active, ESkillEffectType::DamageAoe, 7.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, ESkillStatusEffect::Burn, 3.0f, 3.5f, ESkillElement::Fire},
		{TEXT("spell_mastery"), EClassId::Mage, ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.15f, 0.0f, 0.0f, 0.0f},
		{TEXT("mana_flow"), EClassId::Mage, ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.2f, 0.0f, 0.0f, 0.0f},
		{TEXT("arcane_overload"), EClassId::Mage, ESkillType::Ultimate, ESkillEffectType::DamageAoe, 0.0f, 5.5f, 0.25f, 4.0f, 9.0f, 3.0f},
	};

	TestEqual(TEXT("Mage DefinitionParity covers eight skills"), Skills->Skills.Num(), MageSkills.Num());
	TotalCheckedSkills += MageSkills.Num();
	for (const FExpectedSkillDefinition& Expected : MageSkills)
	{
		TestSkillDefinitionParity(*this, *Skills, Expected);
	}

	Skills->LoadDefaultArcherSkills();
	const TArray<FExpectedSkillDefinition> ArcherSkills = {
		{TEXT("precision_shot"), EClassId::Archer, ESkillType::Active, ESkillEffectType::DamageSingle, 3.5f, 2.2f, 0.0f, 0.0f, 0.0f, 0.0f},
		{TEXT("arrow_rain"), EClassId::Archer, ESkillType::Active, ESkillEffectType::DamageAoe, 8.0f, 1.6f, 0.0f, 0.0f, 0.0f, 0.0f},
		{TEXT("focus"), EClassId::Archer, ESkillType::Active, ESkillEffectType::SelfBuff, 10.0f, 0.0f, 0.2f, 4.0f, 0.0f, 0.0f},
		{TEXT("piercing_arrow"), EClassId::Archer, ESkillType::Active, ESkillEffectType::DashDamage, 9.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f},
		{TEXT("multi_shot"), EClassId::Archer, ESkillType::Active, ESkillEffectType::DamageAoe, 5.5f, 3.2f, 0.0f, 0.0f, 0.0f, 0.0f},
		{TEXT("critical_eye"), EClassId::Archer, ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.05f, 0.0f, 0.0f, 0.0f},
		{TEXT("quick_draw"), EClassId::Archer, ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.1f, 0.0f, 0.0f, 0.0f},
		{TEXT("eagle_eye"), EClassId::Archer, ESkillType::Ultimate, ESkillEffectType::DamageSingle, 0.0f, 5.0f, 0.25f, 4.0f, 10.0f, 2.0f},
	};

	TestEqual(TEXT("Archer DefinitionParity covers eight skills"), Skills->Skills.Num(), ArcherSkills.Num());
	TotalCheckedSkills += ArcherSkills.Num();
	for (const FExpectedSkillDefinition& Expected : ArcherSkills)
	{
		TestSkillDefinitionParity(*this, *Skills, Expected);
	}

	Skills->LoadDefaultThiefSkills();
	const TArray<FExpectedSkillDefinition> ThiefSkills = {
		{TEXT("shadow_stab"), EClassId::Thief, ESkillType::Active, ESkillEffectType::DamageSingle, 3.0f, 2.3f, 0.0f, 0.0f, 0.0f, 0.0f, ESkillStatusEffect::Curse, 4.0f, 0.2f, ESkillElement::Dark},
		{TEXT("smoke_bomb"), EClassId::Thief, ESkillType::Active, ESkillEffectType::DamageAoe, 7.0f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, ESkillStatusEffect::Poison, 3.0f, 2.0f, ESkillElement::None},
		{TEXT("evasion_stance"), EClassId::Thief, ESkillType::Active, ESkillEffectType::SelfBuff, 10.0f, 0.0f, 0.2f, 4.0f, 0.0f, 0.0f},
		{TEXT("backstab"), EClassId::Thief, ESkillType::Active, ESkillEffectType::DashDamage, 9.0f, 2.1f, 0.0f, 0.0f, 0.0f, 0.0f},
		{TEXT("shadow_strike"), EClassId::Thief, ESkillType::Active, ESkillEffectType::DamageSingle, 6.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, ESkillStatusEffect::Curse, 4.0f, 0.2f, ESkillElement::Dark},
		{TEXT("nimble_hands"), EClassId::Thief, ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.05f, 0.0f, 0.0f, 0.0f},
		{TEXT("lucky_instinct"), EClassId::Thief, ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.05f, 0.0f, 0.0f, 0.0f},
		{TEXT("assassinate"), EClassId::Thief, ESkillType::Ultimate, ESkillEffectType::DamageSingle, 0.0f, 5.3f, 0.25f, 4.0f, 11.0f, 1.0f},
	};

	TestEqual(TEXT("Thief DefinitionParity covers eight skills"), Skills->Skills.Num(), ThiefSkills.Num());
	TotalCheckedSkills += ThiefSkills.Num();
	for (const FExpectedSkillDefinition& Expected : ThiefSkills)
	{
		TestSkillDefinitionParity(*this, *Skills, Expected);
	}

	Skills->LoadDefaultClericSkills();
	const TArray<FExpectedSkillDefinition> ClericSkills = {
		{TEXT("holy_smite"), EClassId::Cleric, ESkillType::Active, ESkillEffectType::DamageSingle, 3.2f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, ESkillStatusEffect::None, 0.0f, 0.0f, ESkillElement::Holy},
		{TEXT("heal"), EClassId::Cleric, ESkillType::Active, ESkillEffectType::Heal, 6.0f, 0.0f, 0.2f, 0.0f, 0.0f, 0.0f},
		{TEXT("blessing"), EClassId::Cleric, ESkillType::Active, ESkillEffectType::SelfBuff, 10.0f, 0.0f, 0.15f, 4.0f, 0.0f, 0.0f},
		{TEXT("purify"), EClassId::Cleric, ESkillType::Active, ESkillEffectType::SelfBuff, 12.0f, 0.0f, 0.25f, 4.0f, 0.0f, 0.0f},
		{TEXT("divine_grace"), EClassId::Cleric, ESkillType::Active, ESkillEffectType::SelfBuff, 12.0f, 0.0f, 0.3f, 5.0f, 0.0f, 0.0f, ESkillStatusEffect::None, 0.0f, 0.0f, ESkillElement::Holy},
		{TEXT("wisdom_training"), EClassId::Cleric, ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.1f, 0.0f, 0.0f, 0.0f},
		{TEXT("divine_vitality"), EClassId::Cleric, ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.2f, 0.0f, 0.0f, 0.0f},
		{TEXT("sanctuary"), EClassId::Cleric, ESkillType::Ultimate, ESkillEffectType::Heal, 0.0f, 0.0f, 0.4f, 0.0f, 6.0f, 6.0f},
	};

	TestEqual(TEXT("Cleric DefinitionParity covers eight skills"), Skills->Skills.Num(), ClericSkills.Num());
	TotalCheckedSkills += ClericSkills.Num();
	for (const FExpectedSkillDefinition& Expected : ClericSkills)
	{
		TestSkillDefinitionParity(*this, *Skills, Expected);
	}

	Skills->LoadDefaultPaladinSkills();
	const TArray<FExpectedSkillDefinition> PaladinSkills = {
		{TEXT("holy_verdict"), EClassId::Paladin, ESkillType::Active, ESkillEffectType::DamageSingle, 4.0f, 2.3f, 0.0f, 0.0f, 1.0f, 1.0f, ESkillStatusEffect::None, 0.0f, 0.0f, ESkillElement::Holy},
		{TEXT("radiant_sweep"), EClassId::Paladin, ESkillType::Active, ESkillEffectType::DamageAoe, 8.0f, 1.6f, 0.0f, 0.0f, 1.0f, 1.0f, ESkillStatusEffect::None, 0.0f, 0.0f, ESkillElement::Holy},
		{TEXT("guardian_aegis"), EClassId::Paladin, ESkillType::Active, ESkillEffectType::SelfBuff, 12.0f, 0.0f, 0.45f, 5.0f, 0.0f, 2.0f},
		{TEXT("lay_on_hands"), EClassId::Paladin, ESkillType::Active, ESkillEffectType::Heal, 10.0f, 0.0f, 0.18f, 0.0f, 0.0f, 0.0f},
		{TEXT("judgment"), EClassId::Paladin, ESkillType::Active, ESkillEffectType::DamageSingle, 8.0f, 2.4f, 0.0f, 0.0f, 1.0f, 1.0f, ESkillStatusEffect::None, 0.0f, 0.0f, ESkillElement::Holy},
		{TEXT("sacred_oath"), EClassId::Paladin, ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.15f, 0.0f, 0.0f, 0.0f},
		{TEXT("bulwark_training"), EClassId::Paladin, ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.15f, 0.0f, 0.0f, 0.0f},
		{TEXT("divine_bastion"), EClassId::Paladin, ESkillType::Ultimate, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.35f, 5.0f, 5.0f, 8.0f},
	};

	TestEqual(TEXT("Paladin DefinitionParity covers eight skills"), Skills->Skills.Num(), PaladinSkills.Num());
	TotalCheckedSkills += PaladinSkills.Num();
	for (const FExpectedSkillDefinition& Expected : PaladinSkills)
	{
		TestSkillDefinitionParity(*this, *Skills, Expected);
	}

	Skills->LoadDefaultBerserkerSkills();
	const TArray<FExpectedSkillDefinition> BerserkerSkills = {
		{TEXT("rage_cleave"), EClassId::Berserker, ESkillType::Active, ESkillEffectType::DamageSingle, 3.5f, 2.35f, 0.0f, 0.0f, 2.0f, 0.0f},
		{TEXT("blood_surge"), EClassId::Berserker, ESkillType::Active, ESkillEffectType::DamageAoe, 7.5f, 1.65f, 0.0f, 0.0f, 2.0f, 0.0f, ESkillStatusEffect::Burn, 2.0f, 3.0f, ESkillElement::Fire},
		{TEXT("frenzy_stance"), EClassId::Berserker, ESkillType::Active, ESkillEffectType::SelfBuff, 11.0f, 0.0f, 0.3f, 4.0f, 0.0f, 0.0f},
		{TEXT("savage_leap"), EClassId::Berserker, ESkillType::Active, ESkillEffectType::DashDamage, 9.0f, 2.05f, 0.0f, 0.0f, 2.0f, 0.0f},
		{TEXT("blood_rage"), EClassId::Berserker, ESkillType::Active, ESkillEffectType::DamageSingle, 6.0f, 3.0f, 0.0f, 0.0f, 2.0f, 0.0f},
		{TEXT("blood_frenzy"), EClassId::Berserker, ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.2f, 0.0f, 0.0f, 0.0f},
		{TEXT("pain_to_power"), EClassId::Berserker, ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.08f, 0.0f, 0.0f, 0.0f},
		{TEXT("berserk_apex"), EClassId::Berserker, ESkillType::Ultimate, ESkillEffectType::DamageSingle, 0.0f, 6.5f, 0.35f, 4.0f, 12.0f, 2.0f},
	};

	TestEqual(TEXT("Berserker DefinitionParity covers eight skills"), Skills->Skills.Num(), BerserkerSkills.Num());
	TotalCheckedSkills += BerserkerSkills.Num();
	for (const FExpectedSkillDefinition& Expected : BerserkerSkills)
	{
		TestSkillDefinitionParity(*this, *Skills, Expected);
	}

	Skills->LoadDefaultSummonerSkills();
	const TArray<FExpectedSkillDefinition> SummonerSkills = {
		{TEXT("spirit_bolt"), EClassId::Summoner, ESkillType::Active, ESkillEffectType::DamageSingle, 3.2f, 1.9f, 0.0f, 0.0f, 1.0f, 0.0f, ESkillStatusEffect::Poison, 3.0f, 2.5f, ESkillElement::None},
		{TEXT("familiar_swarm"), EClassId::Summoner, ESkillType::Active, ESkillEffectType::DamageAoe, 7.0f, 1.45f, 0.0f, 0.0f, 1.0f, 0.0f, ESkillStatusEffect::Poison, 4.0f, 2.0f, ESkillElement::None},
		{TEXT("arcane_binding"), EClassId::Summoner, ESkillType::Active, ESkillEffectType::SelfBuff, 10.0f, 0.0f, 0.22f, 4.0f, 0.0f, 0.0f},
		{TEXT("void_call"), EClassId::Summoner, ESkillType::Active, ESkillEffectType::DamageAoe, 12.0f, 2.0f, 0.0f, 0.0f, 1.5f, 0.0f, ESkillStatusEffect::Curse, 3.0f, 0.15f, ESkillElement::Dark},
		{TEXT("spirit_burst"), EClassId::Summoner, ESkillType::Active, ESkillEffectType::DamageAoe, 6.0f, 3.4f, 0.0f, 0.0f, 1.0f, 0.0f},
		{TEXT("pact_mastery"), EClassId::Summoner, ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.15f, 0.0f, 0.0f, 0.0f},
		{TEXT("spirit_reservoir"), EClassId::Summoner, ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.2f, 0.0f, 0.0f, 0.0f},
		{TEXT("grand_familiar"), EClassId::Summoner, ESkillType::Ultimate, ESkillEffectType::DamageAoe, 0.0f, 5.7f, 0.25f, 4.0f, 10.0f, 3.0f, ESkillStatusEffect::Poison, 5.0f, 4.0f, ESkillElement::Lightning},
	};

	TestEqual(TEXT("Summoner DefinitionParity covers eight skills"), Skills->Skills.Num(), SummonerSkills.Num());
	TotalCheckedSkills += SummonerSkills.Num();
	for (const FExpectedSkillDefinition& Expected : SummonerSkills)
	{
		TestSkillDefinitionParity(*this, *Skills, Expected);
	}

	TestEqual(TEXT("DefinitionParity covers eight classes times eight skills"), TotalCheckedSkills, 64);
	Skills->LoadDefaultClericSkills();
	TestTrue(TEXT("Cleric DefinitionParity includes Heal effect"), HasSkill(*Skills, TEXT("heal")));
	Skills->LoadDefaultPaladinSkills();
	TestTrue(TEXT("Paladin DefinitionParity includes Heal effect"), HasSkill(*Skills, TEXT("lay_on_hands")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillGaugeTest,
	"IdleProject.Combat.Skills.UltimateGauge",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillGaugeTest::RunTest(const FString& Parameters)
{
	USkillComponent* Skills = NewObject<USkillComponent>();
	Skills->LoadDefaultWarriorSkills();

	TestEqual(TEXT("Initial gauge is empty"), Skills->GetCurrentGauge(), 0.0f);
	Skills->AddGauge(60.0f);
	Skills->AddGauge(60.0f);
	TestEqual(TEXT("Gauge is clamped at 100"), Skills->GetCurrentGauge(), 100.0f);
	TestTrue(TEXT("Ultimate is ready at 100 gauge"), Skills->IsUltimateReady());

	TestTrue(TEXT("Ultimate cast consumes gauge"), Skills->TryConsumeUltimateGauge());
	TestEqual(TEXT("Gauge resets after ultimate"), Skills->GetCurrentGauge(), 0.0f);
	TestFalse(TEXT("Ultimate is no longer ready after reset"), Skills->IsUltimateReady());

	Skills->AddGauge(-20.0f);
	TestEqual(TEXT("Gauge cannot go below zero"), Skills->GetCurrentGauge(), 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillPassiveStatsTest,
	"IdleProject.Combat.Skills.PassiveStats",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillPassiveStatsTest::RunTest(const FString& Parameters)
{
	USkillComponent* Skills = NewObject<USkillComponent>();
	Skills->LoadDefaultWarriorSkills();

	FDerivedStats Stats;
	Stats.Hp = 1000.0f;
	Stats.PhysAtk = 200.0f;
	Stats.PhysDef = 50.0f;

	Skills->ApplyPassivesToStats(Stats);

	TestEqual(TEXT("Weapon mastery grants 15 percent physical attack"), Stats.PhysAtk, 230.0f);
	TestEqual(TEXT("Toughness grants 20 percent max HP"), Stats.Hp, 1200.0f);
	TestEqual(TEXT("Unrelated stats stay unchanged"), Stats.PhysDef, 50.0f);

	Skills->LoadDefaultMageSkills();
	Stats = FDerivedStats();
	Stats.MagicAtk = 200.0f;
	Stats.Mp = 1000.0f;
	Skills->ApplyPassivesToStats(Stats);
	TestEqual(TEXT("Mage spell mastery grants 15 percent magic attack"), Stats.MagicAtk, 230.0f);
	TestEqual(TEXT("Mage mana flow grants 20 percent max MP"), Stats.Mp, 1200.0f);

	Skills->LoadDefaultArcherSkills();
	Stats = FDerivedStats();
	Stats.CritRate = 0.10f;
	Stats.Dodge = 0.10f;
	Stats.AtkSpeed = 1.0f;
	Skills->ApplyPassivesToStats(Stats);
	TestEqual(TEXT("Archer critical eye grants five percentage points crit"), Stats.CritRate, 0.15f);
	TestEqual(TEXT("Archer quick draw grants 10 percent attack speed"), Stats.AtkSpeed, 1.1f);

	Skills->LoadDefaultThiefSkills();
	Stats = FDerivedStats();
	Stats.CritRate = 0.10f;
	Stats.Dodge = 0.10f;
	Skills->ApplyPassivesToStats(Stats);
	TestEqual(TEXT("Thief nimble hands grants five percentage points dodge"), Stats.Dodge, 0.15f);
	TestEqual(TEXT("Thief lucky instinct grants five percentage points crit"), Stats.CritRate, 0.15f);

	Skills->LoadDefaultClericSkills();
	Stats = FDerivedStats();
	Stats.Hp = 1000.0f;
	Stats.MagicAtk = 200.0f;
	Skills->ApplyPassivesToStats(Stats);
	TestEqual(TEXT("Cleric divine vitality grants 20 percent max HP"), Stats.Hp, 1200.0f);
	TestEqual(TEXT("Cleric wisdom training grants 10 percent magic attack"), Stats.MagicAtk, 220.0f);

	Skills->LoadDefaultPaladinSkills();
	Stats = FDerivedStats();
	Stats.Hp = 1000.0f;
	Stats.PhysDef = 200.0f;
	Skills->ApplyPassivesToStats(Stats);
	TestEqual(TEXT("Paladin sacred oath grants 15 percent max HP"), Stats.Hp, 1150.0f);
	TestEqual(TEXT("Paladin bulwark training grants 15 percent physical defense"), Stats.PhysDef, 230.0f);

	Skills->LoadDefaultBerserkerSkills();
	Stats = FDerivedStats();
	Stats.PhysAtk = 200.0f;
	Stats.CritRate = 0.10f;
	Skills->ApplyPassivesToStats(Stats);
	TestEqual(TEXT("Berserker blood frenzy grants 20 percent physical attack"), Stats.PhysAtk, 240.0f);
	TestEqual(TEXT("Berserker pain to power grants crit rate"), Stats.CritRate, 0.18f);

	Skills->LoadDefaultSummonerSkills();
	Stats = FDerivedStats();
	Stats.MagicAtk = 200.0f;
	Stats.Mp = 1000.0f;
	Skills->ApplyPassivesToStats(Stats);
	TestEqual(TEXT("Summoner pact mastery grants 15 percent magic attack"), Stats.MagicAtk, 230.0f);
	TestEqual(TEXT("Summoner spirit reservoir grants 20 percent max MP"), Stats.Mp, 1200.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillHealEffectTest,
	"IdleProject.Combat.Skills.HealEffect",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillHealEffectTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UCombatComponent* OwnerCombat = NewObject<UCombatComponent>(Owner);
	USkillComponent* Skills = NewObject<USkillComponent>(Owner);
	Owner->AddInstanceComponent(OwnerCombat);
	Owner->AddInstanceComponent(Skills);

	AActor* Target = NewObject<AActor>();
	UCombatComponent* TargetCombat = NewObject<UCombatComponent>(Target);
	Target->AddInstanceComponent(TargetCombat);

	OwnerCombat->InitializeCombat(1000.0f, 100.0f, 20.0f, 1.0f, 120.0f, 30.0f, 0.0f, 1.5f);
	TargetCombat->InitializeCombat(1000.0f, 10.0f, 0.0f, 1.0f);
	OwnerCombat->TakeDamage(500.0f, Target);
	Skills->LoadDefaultClericSkills();

	Skills->MarkSkillCast(TEXT("holy_smite"), 10.0f);
	Skills->MarkSkillCast(TEXT("blessing"), 10.0f);
	Skills->MarkSkillCast(TEXT("purify"), 10.0f);

	TArray<AActor*> AoeTargets;
	Skills->TickSkills(11.0f, Target, AoeTargets);

	TestEqual(TEXT("Heal restores 20 percent of MaxHp"), OwnerCombat->CurrentHp, 700.0f);
	TestEqual(TEXT("Heal does not damage the selected target"), TargetCombat->CurrentHp, TargetCombat->MaxHp);

	OwnerCombat->CurrentHp = 950.0f;
	Skills->MarkSkillCast(TEXT("heal"), 10.0f);
	Skills->TickSkills(17.0f, Target, AoeTargets);

	TestEqual(TEXT("Heal clamps to MaxHp"), OwnerCombat->CurrentHp, OwnerCombat->MaxHp);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillUltimateBuffTest,
	"IdleProject.Combat.Skills.UltimateBuff",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillUltimateBuffTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UCombatComponent* OwnerCombat = NewObject<UCombatComponent>(Owner);
	USkillComponent* Skills = NewObject<USkillComponent>(Owner);
	Owner->AddInstanceComponent(OwnerCombat);
	Owner->AddInstanceComponent(Skills);

	AActor* Target = NewObject<AActor>();
	UCombatComponent* TargetCombat = NewObject<UCombatComponent>(Target);
	Target->AddInstanceComponent(TargetCombat);

	OwnerCombat->InitializeCombat(1000.0f, 100.0f, 20.0f, 1.0f);
	TargetCombat->InitializeCombat(1000.0f, 10.0f, 0.0f, 1.0f);
	Skills->LoadDefaultWarriorSkills();
	Skills->AddGauge(100.0f);

	TArray<AActor*> AoeTargets;
	Skills->TickSkills(30.0f, Target, AoeTargets);

	TestEqual(TEXT("Ultimate resets gauge"), Skills->GetCurrentGauge(), 0.0f);
	TestEqual(TEXT("Ultimate grants 30 percent attack speed buff"), OwnerCombat->AtkSpeed, 1.3f);
	TestTrue(TEXT("Ultimate deals damage to target"), TargetCombat->CurrentHp < TargetCombat->MaxHp);

	Skills->TickSkills(35.0f, Target, AoeTargets);
	TestEqual(TEXT("Ultimate attack speed buff expires"), OwnerCombat->AtkSpeed, 1.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillHudDisplayModelTest,
	"IdleProject.UI.HUD.SkillDisplayModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillHudDisplayModelTest::RunTest(const FString& Parameters)
{
	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));

	USkillComponent* Skills = NewObject<USkillComponent>();
	Skills->LoadDefaultWarriorSkills();
	Skills->GrantSkillPoint(1);
	Skills->RankUpSkill(TEXT("heavy_strike"));
	Skills->GrantSkillPoint(1);

	constexpr float CastTime = 10.0f;
	constexpr float Now = 12.0f;
	Skills->MarkSkillCast(TEXT("heavy_strike"), CastTime);
	Skills->AddGauge(100.0f);

	const TArray<FIdleHUDSkillSlotViewModel> Slots = IdleProject::UI::BuildSkillSlotViewModels(*Skills, Now);

	TestEqual(TEXT("Only active skills are shown in HUD slots"), Slots.Num(), 4);
	TestEqual(TEXT("First active skill keeps localized display name"), Slots[0].DisplayName.ToString(), FString(TEXT("강타")));
	TestEqual(TEXT("Cooldown ratio mirrors skill component"), Slots[0].CooldownRatio, Skills->GetCooldownRatio(TEXT("heavy_strike"), Now));
	TestEqual(TEXT("Cooldown remaining mirrors skill component"), Slots[0].CooldownRemaining, Skills->GetCooldownRemaining(TEXT("heavy_strike"), Now));
	TestFalse(TEXT("Cooling skill is not ready"), Slots[0].bReady);
	TestEqual(TEXT("Skill point balance is exposed per slot"), Slots[0].AvailableSkillPoints, static_cast<int32>(1));
	TestEqual(TEXT("Current skill rank is exposed"), Slots[0].Rank, static_cast<int32>(1));
	TestEqual(TEXT("Max skill rank is exposed"), Slots[0].MaxRank, static_cast<int32>(50));
	TestTrue(TEXT("Rank-up availability is exposed"), Slots[0].bCanRankUp);
	TestEqual(TEXT("Unranked active skill still shows zero rank"), Slots[1].Rank, static_cast<int32>(0));

	const FIdleHUDUltimateViewModel Ultimate = IdleProject::UI::BuildUltimateViewModel(*Skills);
	TestEqual(TEXT("Gauge ratio is normalized"), Ultimate.GaugeRatio, 1.0f);
	TestTrue(TEXT("Ultimate ready flag is exposed"), Ultimate.bReady);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FClassSelectionHudDisplayModelTest,
	"IdleProject.UI.HUD.ClassSelectionDisplayModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FClassSelectionHudDisplayModelTest::RunTest(const FString& Parameters)
{
	const TArray<FIdleHUDClassSelectionOptionViewModel> Options = IdleProject::UI::BuildClassSelectionOptions(EClassId::Mage);

	TestEqual(TEXT("Class selector exposes eight classes"), Options.Num(), 8);
	TestEqual(TEXT("First class is warrior"), static_cast<int32>(Options[0].ClassId), static_cast<int32>(EClassId::Warrior));
	TestEqual(TEXT("Warrior summary highlights STR and CON"), Options[0].StatSummary.ToString(), FString(TEXT("STR/CON")));
	TestEqual(TEXT("Mage summary highlights INT and WIS"), Options[1].StatSummary.ToString(), FString(TEXT("INT/WIS")));
	TestEqual(TEXT("Archer summary highlights DEX and LUK"), Options[2].StatSummary.ToString(), FString(TEXT("DEX/LUK")));
	TestEqual(TEXT("Fourth class is thief"), static_cast<int32>(Options[3].ClassId), static_cast<int32>(EClassId::Thief));
	TestEqual(TEXT("Thief summary highlights DEX and LUK"), Options[3].StatSummary.ToString(), FString(TEXT("DEX/LUK")));
	TestEqual(TEXT("Fifth class is cleric"), static_cast<int32>(Options[4].ClassId), static_cast<int32>(EClassId::Cleric));
	TestEqual(TEXT("Cleric summary highlights WIS and INT"), Options[4].StatSummary.ToString(), FString(TEXT("WIS/INT")));
	TestEqual(TEXT("Sixth class is paladin"), static_cast<int32>(Options[5].ClassId), static_cast<int32>(EClassId::Paladin));
	TestEqual(TEXT("Paladin summary highlights CON and STR"), Options[5].StatSummary.ToString(), FString(TEXT("CON/STR")));
	TestEqual(TEXT("Seventh class is berserker"), static_cast<int32>(Options[6].ClassId), static_cast<int32>(EClassId::Berserker));
	TestEqual(TEXT("Berserker summary highlights STR and LUK"), Options[6].StatSummary.ToString(), FString(TEXT("STR/LUK")));
	TestEqual(TEXT("Eighth class is summoner"), static_cast<int32>(Options[7].ClassId), static_cast<int32>(EClassId::Summoner));
	TestEqual(TEXT("Summoner summary highlights INT and WIS"), Options[7].StatSummary.ToString(), FString(TEXT("INT/WIS")));
	TestTrue(TEXT("Current class is marked selected"), Options[1].bSelected);
	TestFalse(TEXT("Other classes are not selected"), Options[0].bSelected);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleCharacterClassSelectionTest,
	"IdleProject.Character.ClassSelection.AppliesSkills",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleCharacterClassSelectionTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("Transient test world is created"), World);
	if (!World)
	{
		return false;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AIdleCharacter* Character = World->SpawnActor<AIdleCharacter>(AIdleCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Idle character is spawned"), Character);
	if (!Character)
	{
		World->DestroyWorld(false);
		return false;
	}

	Character->SetClassId(EClassId::Summoner);
	USkillComponent* Skills = Character->FindComponentByClass<USkillComponent>();

	TestEqual(TEXT("ClassId stores selected class"), static_cast<int32>(Character->GetClassId()), static_cast<int32>(EClassId::Summoner));
	TestNotNull(TEXT("Skill component exists"), Skills);
	TestTrue(TEXT("Summoner skill set is loaded"), Skills && HasSkill(*Skills, TEXT("spirit_bolt")));
	TestFalse(TEXT("Warrior skill set is replaced"), Skills && HasSkill(*Skills, TEXT("heavy_strike")));

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleCharacterCurrentStatsAccessorsTest,
	"IdleProject.Character.Stats.CurrentStatsAccessors",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleCharacterCurrentStatsAccessorsTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("Transient test world is created"), World);
	if (!World)
	{
		return false;
	}

	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	TestNotNull(TEXT("Game instance is created"), GameInstance);
	if (!GameInstance)
	{
		World->DestroyWorld(false);
		return false;
	}
	World->SetGameInstance(GameInstance);

	GameInstance->AddExp(FLevelFormulas::CumulativeExp(100));
	GameInstance->MarkChapter1BossDefeated();
	TestTrue(TEXT("Test setup rebirths once"), GameInstance->Rebirth());
	GameInstance->LevelUp();
	GameInstance->LevelUp();
	GameInstance->GrantStatPoints(2);
	TestTrue(TEXT("Allocated STR setup succeeds"), GameInstance->AllocateStatPoint(EPrimaryStat::Str));
	TestTrue(TEXT("Allocated INT setup succeeds"), GameInstance->AllocateStatPoint(EPrimaryStat::Int));

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AIdleCharacter* Character = World->SpawnActor<AIdleCharacter>(AIdleCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Idle character is spawned"), Character);
	if (!Character)
	{
		World->DestroyWorld(false);
		return false;
	}

	Character->SetClassId(EClassId::Mage);

	FPrimaryStats ExpectedPrimary = FStatFormulas::DefaultPrimaryStats(EClassId::Mage, GameInstance->GetCharacterLevel());
	ExpectedPrimary.Str += 1.0f;
	ExpectedPrimary.Int_ += 1.0f;
	FDerivedStats ExpectedDerived = FStatFormulas::DeriveStats(
		ExpectedPrimary,
		GameInstance->GetCharacterLevel(),
		FDerivedStats(),
		GameInstance->GetRebirthBonusPoints());
	const USkillComponent* Skills = Character->FindComponentByClass<USkillComponent>();
	if (Skills)
	{
		Skills->ApplyPassivesToStats(ExpectedDerived);
	}
	const float AchievementMultiplier = GameInstance->GetAchievementStatMultiplier();
	ExpectedDerived.Hp *= AchievementMultiplier;
	ExpectedDerived.PhysAtk *= AchievementMultiplier;
	ExpectedDerived.MagicAtk *= AchievementMultiplier;
	ExpectedDerived.PhysDef *= AchievementMultiplier;
	ExpectedDerived.MagicDef *= AchievementMultiplier;
	const FPrimaryStats CurrentPrimary = Character->GetCurrentPrimaryStats();
	const FDerivedStats CurrentDerived = Character->GetCurrentDerivedStats();
	const UCombatComponent* Combat = Character->FindComponentByClass<UCombatComponent>();

	TestEqual(TEXT("Current level exposes game instance progression level"), Character->GetCurrentLevel(), GameInstance->GetCharacterLevel());
	TestNotNull(TEXT("Skill component exists"), Skills);
	TestEqual(TEXT("Current primary STR mirrors refreshed stats"), CurrentPrimary.Str, ExpectedPrimary.Str);
	TestEqual(TEXT("Current primary INT mirrors refreshed stats"), CurrentPrimary.Int_, ExpectedPrimary.Int_);
	TestEqual(TEXT("Current derived HP mirrors refreshed stats with achievement multiplier"), CurrentDerived.Hp, ExpectedDerived.Hp);
	TestEqual(TEXT("Current derived magic attack mirrors refreshed stats with achievement multiplier"), CurrentDerived.MagicAtk, ExpectedDerived.MagicAtk);
	TestNotNull(TEXT("Combat component exists"), Combat);
	TestEqual(TEXT("Combat max HP uses refreshed derived stats"), Combat ? Combat->MaxHp : 0.0f, CurrentDerived.Hp);
	TestEqual(TEXT("Combat magic attack uses refreshed derived stats"), Combat ? Combat->MagicAtk : 0.0f, CurrentDerived.MagicAtk);

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleCharacterCombatPowerAccessorsTest,
	"IdleProject.Character.CombatPower.Accessors",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleCharacterCombatPowerAccessorsTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("Transient test world is created"), World);
	if (!World)
	{
		return false;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AIdleCharacter* Character = World->SpawnActor<AIdleCharacter>(AIdleCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Idle character is spawned"), Character);
	if (!Character)
	{
		World->DestroyWorld(false);
		return false;
	}

	Character->SetClassId(EClassId::Warrior);

	const int64 ExpectedCombatPower = FCombatPowerFormula::ComputeCombatPower(Character->GetCurrentDerivedStats());
	TestEqual(TEXT("GetCombatPower mirrors current derived stats formula"), Character->GetCombatPower(), ExpectedCombatPower);

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleCharacterCombatPowerGrowthSourcesTest,
	"IdleProject.Character.CombatPower.GrowthSources",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleCharacterCombatPowerGrowthSourcesTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("Transient test world is created"), World);
	if (!World)
	{
		return false;
	}

	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	TestNotNull(TEXT("Game instance is created"), GameInstance);
	if (!GameInstance)
	{
		World->DestroyWorld(false);
		return false;
	}
	World->SetGameInstance(GameInstance);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AIdleCharacter* Character = World->SpawnActor<AIdleCharacter>(AIdleCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Idle character is spawned"), Character);
	if (!Character)
	{
		World->DestroyWorld(false);
		return false;
	}

	Character->SetClassId(EClassId::Warrior);
	const int64 BaseCombatPower = Character->GetCombatPower();
	TestCurrentCombatPowerParity(*this, TEXT("Base combat power mirrors current derived stats"), *Character);

	GameInstance->GrantStatPoints(2);
	TestTrue(TEXT("Allocated STR setup succeeds"), GameInstance->AllocateStatPoint(EPrimaryStat::Str));
	TestTrue(TEXT("Allocated DEX setup succeeds"), GameInstance->AllocateStatPoint(EPrimaryStat::Dex));
	Character->RefreshDerivedStats();
	const int64 AllocatedCombatPower = Character->GetCombatPower();
	TestTrue(TEXT("Stat allocation increases combat power through current derived stats"), AllocatedCombatPower > BaseCombatPower);
	TestCurrentCombatPowerParity(*this, TEXT("Allocated combat power mirrors current derived stats"), *Character);

	UInventoryComponent* Inventory = Character->FindComponentByClass<UInventoryComponent>();
	TestNotNull(TEXT("Inventory component exists"), Inventory);
	if (!Inventory)
	{
		World->DestroyWorld(false);
		return false;
	}

	TestTrue(TEXT("Weapon item is accepted"), Inventory->AddItem(MakeCombatPowerTestItem(EItemSlot::Weapon, EItemSet::Warrior, 80.0f, 0.0f, 0.0f)));
	TestTrue(TEXT("Helmet item is accepted"), Inventory->AddItem(MakeCombatPowerTestItem(EItemSlot::Helmet, EItemSet::Warrior, 0.0f, 20.0f, 100.0f)));
	TestTrue(TEXT("Top item is accepted"), Inventory->AddItem(MakeCombatPowerTestItem(EItemSlot::Top, EItemSet::Warrior, 0.0f, 20.0f, 100.0f)));
	TestTrue(TEXT("Gloves item is accepted"), Inventory->AddItem(MakeCombatPowerTestItem(EItemSlot::Gloves, EItemSet::Warrior, 40.0f, 0.0f, 0.0f)));
	Character->RefreshDerivedStats();
	const int64 EquippedCombatPower = Character->GetCombatPower();
	TestTrue(TEXT("Equipment affixes and set bonuses increase combat power"), EquippedCombatPower > AllocatedCombatPower);
	TestCurrentCombatPowerParity(*this, TEXT("Equipped combat power mirrors current derived stats"), *Character);

	TestTrue(TEXT("Enhancing equipped weapon succeeds"), Inventory->EnhanceEquippedItem(EItemSlot::Weapon));
	Character->RefreshDerivedStats();
	const int64 EnhancedCombatPower = Character->GetCombatPower();
	TestTrue(TEXT("Enhancement increases combat power"), EnhancedCombatPower > EquippedCombatPower);
	TestCurrentCombatPowerParity(*this, TEXT("Enhanced combat power mirrors current derived stats"), *Character);

	for (int32 Index = 0; Index < 5; ++Index)
	{
		GameInstance->AddExp(FLevelFormulas::CumulativeExp(100));
		GameInstance->MarkChapter1BossDefeated();
		TestTrue(TEXT("Test setup rebirth succeeds"), GameInstance->Rebirth());
	}
	Character->RefreshDerivedStats();
	const int64 RebirthCombatPower = Character->GetCombatPower();
	TestTrue(TEXT("Rebirth bonus points increase combat power versus original level one baseline"), RebirthCombatPower > BaseCombatPower);
	TestCurrentCombatPowerParity(*this, TEXT("Rebirth combat power mirrors current derived stats"), *Character);

	TestTrue(TEXT("Test setup transcend succeeds"), GameInstance->Transcend());
	Character->RefreshDerivedStats();
	const int64 TranscendedCombatPower = Character->GetCombatPower();
	const float TranscendMultiplier = GameInstance->GetTranscendStatMultiplier();
	FDerivedStats WithoutTranscend = Character->GetCurrentDerivedStats();
	WithoutTranscend.Hp /= TranscendMultiplier;
	WithoutTranscend.PhysAtk /= TranscendMultiplier;
	WithoutTranscend.MagicAtk /= TranscendMultiplier;
	WithoutTranscend.PhysDef /= TranscendMultiplier;
	WithoutTranscend.MagicDef /= TranscendMultiplier;
	TestTrue(TEXT("Transcend multiplier increases combat power over equivalent post-reset stats"), TranscendedCombatPower > FCombatPowerFormula::ComputeCombatPower(WithoutTranscend));
	TestEqual(TEXT("Combat power remains formula over current derived stats"), TranscendedCombatPower, FCombatPowerFormula::ComputeCombatPower(Character->GetCurrentDerivedStats()));
	TestCurrentCombatPowerParity(*this, TEXT("Transcended combat power mirrors current derived stats"), *Character);

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleCharacterTranscendNeutralStatsTest,
	"IdleProject.Character.Stats.TranscendNeutralMultiplier",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleCharacterTranscendNeutralStatsTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("Transient test world is created"), World);
	if (!World)
	{
		return false;
	}

	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	TestNotNull(TEXT("Game instance is created"), GameInstance);
	if (!GameInstance)
	{
		World->DestroyWorld(false);
		return false;
	}
	World->SetGameInstance(GameInstance);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AIdleCharacter* Character = World->SpawnActor<AIdleCharacter>(AIdleCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Idle character is spawned"), Character);
	if (!Character)
	{
		World->DestroyWorld(false);
		return false;
	}

	Character->SetClassId(EClassId::Warrior);

	const FPrimaryStats ExpectedPrimary = FStatFormulas::DefaultPrimaryStats(EClassId::Warrior, 1);
	FDerivedStats ExpectedDerived = FStatFormulas::DeriveStats(ExpectedPrimary, 1);
	const USkillComponent* Skills = Character->FindComponentByClass<USkillComponent>();
	if (Skills)
	{
		Skills->ApplyPassivesToStats(ExpectedDerived);
	}
	const FDerivedStats CurrentDerived = Character->GetCurrentDerivedStats();

	TestEqual(TEXT("Transcend count zero keeps HP unchanged"), CurrentDerived.Hp, ExpectedDerived.Hp);
	TestEqual(TEXT("Transcend count zero keeps physical attack unchanged"), CurrentDerived.PhysAtk, ExpectedDerived.PhysAtk);
	TestEqual(TEXT("Transcend count zero keeps magic attack unchanged"), CurrentDerived.MagicAtk, ExpectedDerived.MagicAtk);
	TestEqual(TEXT("Transcend count zero keeps physical defense unchanged"), CurrentDerived.PhysDef, ExpectedDerived.PhysDef);
	TestEqual(TEXT("Transcend count zero keeps magic defense unchanged"), CurrentDerived.MagicDef, ExpectedDerived.MagicDef);
	TestEqual(TEXT("Cached derived stats expose neutral multiplier result"), Character->GetCurrentDerivedStats().Hp, CurrentDerived.Hp);

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleCharacterTranscendDerivedStatsTest,
	"IdleProject.Character.Stats.TranscendMultiplier",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleCharacterTranscendDerivedStatsTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("Transient test world is created"), World);
	if (!World)
	{
		return false;
	}

	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	TestNotNull(TEXT("Game instance is created"), GameInstance);
	if (!GameInstance)
	{
		World->DestroyWorld(false);
		return false;
	}
	World->SetGameInstance(GameInstance);

	for (int32 Index = 0; Index < 5; ++Index)
	{
		GameInstance->AddExp(FLevelFormulas::CumulativeExp(100));
		GameInstance->MarkChapter1BossDefeated();
		TestTrue(TEXT("Test setup rebirth succeeds"), GameInstance->Rebirth());
	}
	TestTrue(TEXT("Test setup transcend succeeds"), GameInstance->Transcend());

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AIdleCharacter* Character = World->SpawnActor<AIdleCharacter>(AIdleCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Idle character is spawned"), Character);
	if (!Character)
	{
		World->DestroyWorld(false);
		return false;
	}

	Character->SetClassId(EClassId::Warrior);

	const FPrimaryStats ExpectedPrimary = FStatFormulas::DefaultPrimaryStats(EClassId::Warrior, 1);
	FDerivedStats BaseDerived = FStatFormulas::DeriveStats(ExpectedPrimary, 1);
	const USkillComponent* Skills = Character->FindComponentByClass<USkillComponent>();
	if (Skills)
	{
		Skills->ApplyPassivesToStats(BaseDerived);
	}
	const FDerivedStats CurrentDerived = Character->GetCurrentDerivedStats();
	const UCombatComponent* Combat = Character->FindComponentByClass<UCombatComponent>();

	const float CombinedProgressMultiplier = GameInstance->GetTranscendStatMultiplier() * GameInstance->GetAchievementStatMultiplier();
	TestEqual(TEXT("Transcend count one and achievements multiply HP"), CurrentDerived.Hp, BaseDerived.Hp * CombinedProgressMultiplier);
	TestEqual(TEXT("Transcend count one and achievements multiply physical attack"), CurrentDerived.PhysAtk, BaseDerived.PhysAtk * CombinedProgressMultiplier);
	TestEqual(TEXT("Transcend count one and achievements multiply magic attack"), CurrentDerived.MagicAtk, BaseDerived.MagicAtk * CombinedProgressMultiplier);
	TestEqual(TEXT("Transcend count one and achievements multiply physical defense"), CurrentDerived.PhysDef, BaseDerived.PhysDef * CombinedProgressMultiplier);
	TestEqual(TEXT("Transcend count one and achievements multiply magic defense"), CurrentDerived.MagicDef, BaseDerived.MagicDef * CombinedProgressMultiplier);
	TestEqual(TEXT("Transcend multiplier does not alter attack speed"), CurrentDerived.AtkSpeed, BaseDerived.AtkSpeed);
	TestEqual(TEXT("Transcend multiplier does not alter crit rate"), CurrentDerived.CritRate, BaseDerived.CritRate);
	TestNotNull(TEXT("Combat component exists"), Combat);
	TestEqual(TEXT("Combat max HP uses transcended derived stats"), Combat ? Combat->MaxHp : 0.0f, CurrentDerived.Hp);
	TestEqual(TEXT("Combat physical attack uses transcended derived stats"), Combat ? Combat->Atk : 0.0f, CurrentDerived.PhysAtk);

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleCharacterTowerMilestoneDerivedStatsTest,
	"IdleProject.Character.Stats.TowerMilestoneMultiplier",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleCharacterTowerMilestoneDerivedStatsTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("Transient test world is created"), World);
	if (!World)
	{
		return false;
	}

	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	TestNotNull(TEXT("Game instance is created"), GameInstance);
	if (!GameInstance)
	{
		World->DestroyWorld(false);
		return false;
	}
	World->SetGameInstance(GameInstance);
	GameInstance->InitializeTowerServiceForTests();
	UTowerService* Tower = GameInstance->GetTowerService();
	if (Tower)
	{
		Tower->TryClimbTower(FTowerFormula::GetFloorRequiredPower(10));
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AIdleCharacter* Character = World->SpawnActor<AIdleCharacter>(AIdleCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Idle character is spawned"), Character);
	if (!Character)
	{
		World->DestroyWorld(false);
		return false;
	}

	Character->SetClassId(EClassId::Warrior);

	const FPrimaryStats ExpectedPrimary = FStatFormulas::DefaultPrimaryStats(EClassId::Warrior, 1);
	FDerivedStats BaseDerived = FStatFormulas::DeriveStats(ExpectedPrimary, 1);
	const USkillComponent* Skills = Character->FindComponentByClass<USkillComponent>();
	if (Skills)
	{
		Skills->ApplyPassivesToStats(BaseDerived);
	}

	const float TowerMultiplier = FTowerMilestoneFormula::GetTowerMilestoneMultiplier(10);
	const FDerivedStats CurrentDerived = Character->GetCurrentDerivedStats();
	const UCombatComponent* Combat = Character->FindComponentByClass<UCombatComponent>();

	TestEqual(TEXT("Floor ten milestone multiplies HP"), CurrentDerived.Hp, BaseDerived.Hp * TowerMultiplier);
	TestEqual(TEXT("Floor ten milestone multiplies physical attack"), CurrentDerived.PhysAtk, BaseDerived.PhysAtk * TowerMultiplier);
	TestEqual(TEXT("Floor ten milestone multiplies magic attack"), CurrentDerived.MagicAtk, BaseDerived.MagicAtk * TowerMultiplier);
	TestEqual(TEXT("Floor ten milestone multiplies physical defense"), CurrentDerived.PhysDef, BaseDerived.PhysDef * TowerMultiplier);
	TestEqual(TEXT("Floor ten milestone multiplies magic defense"), CurrentDerived.MagicDef, BaseDerived.MagicDef * TowerMultiplier);
	TestEqual(TEXT("Tower milestone multiplier does not alter attack speed"), CurrentDerived.AtkSpeed, BaseDerived.AtkSpeed);
	TestEqual(TEXT("Tower milestone multiplier does not alter crit rate"), CurrentDerived.CritRate, BaseDerived.CritRate);
	TestEqual(TEXT("Cached derived stats expose tower multiplier result"), Character->GetCurrentDerivedStats().Hp, CurrentDerived.Hp);
	TestNotNull(TEXT("Combat component exists"), Combat);
	TestEqual(TEXT("Combat max HP uses tower milestone derived stats"), Combat ? Combat->MaxHp : 0.0f, CurrentDerived.Hp);
	TestEqual(TEXT("Combat physical attack uses tower milestone derived stats"), Combat ? Combat->Atk : 0.0f, CurrentDerived.PhysAtk);

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleCharacterPetStatMultiplierTest,
	"IdleProject.Character.Stats.PetStatMultiplier",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleCharacterPetStatMultiplierTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("Transient test world is created"), World);
	if (!World)
	{
		return false;
	}

	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	TestNotNull(TEXT("Game instance is created"), GameInstance);
	if (!GameInstance)
	{
		World->DestroyWorld(false);
		return false;
	}
	World->SetGameInstance(GameInstance);
	GameInstance->InitializePetSeasonServicesForTests();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AIdleCharacter* Character = World->SpawnActor<AIdleCharacter>(AIdleCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Idle character is spawned"), Character);
	if (!Character)
	{
		World->DestroyWorld(false);
		return false;
	}

	Character->SetClassId(EClassId::Warrior);
	FDerivedStats BaseDerived = Character->GetCurrentDerivedStats();

	UPetService* PetService = GameInstance->GetPetService();
	TestNotNull(TEXT("Pet service exists"), PetService);
	TestTrue(TEXT("Wolf unlock succeeds"), PetService ? PetService->TryUnlockPet(TEXT("wolf")) : false);
	TestTrue(TEXT("Wolf equip succeeds"), GameInstance->EquipPet(TEXT("wolf")));
	Character->RefreshDerivedStats();
	FDerivedStats WolfDerived = Character->GetCurrentDerivedStats();
	TestEqual(TEXT("Wolf multiplies physical attack by percent"), WolfDerived.PhysAtk, BaseDerived.PhysAtk * 1.10f);
	TestEqual(TEXT("Wolf does not add flat magic attack"), WolfDerived.MagicAtk, BaseDerived.MagicAtk);
	TestEqual(TEXT("Wolf does not add flat HP"), WolfDerived.Hp, BaseDerived.Hp);

	TestTrue(TEXT("Dragon unlock succeeds"), PetService ? PetService->TryUnlockPet(TEXT("dragon")) : false);
	TestTrue(TEXT("Dragon equip succeeds"), GameInstance->EquipPet(TEXT("dragon")));
	Character->RefreshDerivedStats();
	FDerivedStats DragonDerived = Character->GetCurrentDerivedStats();
	TestEqual(TEXT("Dragon multiplies physical attack by percent"), DragonDerived.PhysAtk, BaseDerived.PhysAtk * 1.08f);
	TestEqual(TEXT("Dragon multiplies magic attack by percent"), DragonDerived.MagicAtk, BaseDerived.MagicAtk * 1.08f);
	TestEqual(TEXT("Dragon multiplies physical defense by percent"), DragonDerived.PhysDef, BaseDerived.PhysDef * 1.08f);
	TestEqual(TEXT("Dragon multiplies magic defense by percent"), DragonDerived.MagicDef, BaseDerived.MagicDef * 1.08f);
	TestEqual(TEXT("Dragon does not multiply HP"), DragonDerived.Hp, BaseDerived.Hp);

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleCharacterTranscendAndTowerMilestoneDerivedStatsTest,
	"IdleProject.Character.Stats.TranscendAndTowerMilestoneMultiplier",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleCharacterTranscendAndTowerMilestoneDerivedStatsTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("Transient test world is created"), World);
	if (!World)
	{
		return false;
	}

	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	TestNotNull(TEXT("Game instance is created"), GameInstance);
	if (!GameInstance)
	{
		World->DestroyWorld(false);
		return false;
	}
	World->SetGameInstance(GameInstance);
	GameInstance->InitializeTowerServiceForTests();
	UTowerService* Tower = GameInstance->GetTowerService();
	if (Tower)
	{
		Tower->TryClimbTower(FTowerFormula::GetFloorRequiredPower(10));
	}

	for (int32 Index = 0; Index < 5; ++Index)
	{
		GameInstance->AddExp(FLevelFormulas::CumulativeExp(100));
		GameInstance->MarkChapter1BossDefeated();
		TestTrue(TEXT("Test setup rebirth succeeds"), GameInstance->Rebirth());
	}
	TestTrue(TEXT("Test setup transcend succeeds"), GameInstance->Transcend());

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AIdleCharacter* Character = World->SpawnActor<AIdleCharacter>(AIdleCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Idle character is spawned"), Character);
	if (!Character)
	{
		World->DestroyWorld(false);
		return false;
	}

	Character->SetClassId(EClassId::Warrior);

	const FPrimaryStats ExpectedPrimary = FStatFormulas::DefaultPrimaryStats(EClassId::Warrior, 1);
	FDerivedStats BaseDerived = FStatFormulas::DeriveStats(ExpectedPrimary, 1);
	const USkillComponent* Skills = Character->FindComponentByClass<USkillComponent>();
	if (Skills)
	{
		Skills->ApplyPassivesToStats(BaseDerived);
	}

	const float CombinedMultiplier = GameInstance->GetTranscendStatMultiplier() * FTowerMilestoneFormula::GetTowerMilestoneMultiplier(10) * GameInstance->GetAchievementStatMultiplier();
	const FDerivedStats CurrentDerived = Character->GetCurrentDerivedStats();

	TestEqual(TEXT("Transcend, tower milestone, and achievement multiply HP together"), CurrentDerived.Hp, BaseDerived.Hp * CombinedMultiplier);
	TestEqual(TEXT("Transcend, tower milestone, and achievement multiply physical attack together"), CurrentDerived.PhysAtk, BaseDerived.PhysAtk * CombinedMultiplier);
	TestEqual(TEXT("Transcend, tower milestone, and achievement multiply magic attack together"), CurrentDerived.MagicAtk, BaseDerived.MagicAtk * CombinedMultiplier);
	TestEqual(TEXT("Combined multipliers do not alter attack speed"), CurrentDerived.AtkSpeed, BaseDerived.AtkSpeed);
	TestEqual(TEXT("Combined multipliers do not alter crit rate"), CurrentDerived.CritRate, BaseDerived.CritRate);

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillAoeTargetsTest,
	"IdleProject.Combat.Skills.AoeTargets",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillAoeTargetsTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UCombatComponent* OwnerCombat = NewObject<UCombatComponent>(Owner);
	USkillComponent* Skills = NewObject<USkillComponent>(Owner);
	Owner->AddInstanceComponent(OwnerCombat);
	Owner->AddInstanceComponent(Skills);

	AActor* PrimaryTarget = NewObject<AActor>();
	UCombatComponent* PrimaryCombat = NewObject<UCombatComponent>(PrimaryTarget);
	PrimaryTarget->AddInstanceComponent(PrimaryCombat);

	AActor* SecondaryTarget = NewObject<AActor>();
	UCombatComponent* SecondaryCombat = NewObject<UCombatComponent>(SecondaryTarget);
	SecondaryTarget->AddInstanceComponent(SecondaryCombat);

	OwnerCombat->InitializeCombat(1000.0f, 100.0f, 0.0f, 1.0f);
	PrimaryCombat->InitializeCombat(1000.0f, 10.0f, 0.0f, 1.0f);
	SecondaryCombat->InitializeCombat(1000.0f, 10.0f, 0.0f, 1.0f);
	Skills->LoadDefaultWarriorSkills();

	Skills->MarkSkillCast(TEXT("heavy_strike"), 10.0f);
	Skills->MarkSkillCast(TEXT("shield_up"), 10.0f);
	Skills->MarkSkillCast(TEXT("charge"), 10.0f);

	TArray<AActor*> AoeTargets;
	AoeTargets.Add(PrimaryTarget);
	AoeTargets.Add(SecondaryTarget);
	Skills->TickSkills(12.0f, PrimaryTarget, AoeTargets);

	TestTrue(TEXT("Whirlwind damages primary target"), PrimaryCombat->CurrentHp < PrimaryCombat->MaxHp);
	TestTrue(TEXT("Whirlwind damages secondary target"), SecondaryCombat->CurrentHp < SecondaryCombat->MaxHp);
	TestEqual(TEXT("Whirlwind applies the same damage to every AoE target"), PrimaryCombat->CurrentHp, SecondaryCombat->CurrentHp);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBattleAIAoeTargetGatheringTest,
	"IdleProject.Combat.BattleAI.AoeTargetGathering",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBattleAIAoeTargetGatheringTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("Transient test world is created"), World);
	if (!World)
	{
		return false;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	auto SpawnPositionedActor = [&World, &SpawnParams](const FVector& Location)
	{
		AActor* Actor = World->SpawnActor<AActor>(AActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* Root = NewObject<USceneComponent>(Actor);
		Actor->SetRootComponent(Root);
		Root->RegisterComponent();
		Actor->SetActorLocation(Location);
		return Actor;
	};

	AActor* Owner = SpawnPositionedActor(FVector(0.0, 0.0, 0.0));
	UBattleAIComponent* BattleAI = NewObject<UBattleAIComponent>(Owner);
	Owner->AddInstanceComponent(BattleAI);
	BattleAI->RegisterComponent();
	BattleAI->TargetActorClass = AActor::StaticClass();

	AActor* NearTarget = SpawnPositionedActor(FVector(150.0, 0.0, 500.0));
	UCombatComponent* NearCombat = NewObject<UCombatComponent>(NearTarget);
	NearTarget->AddInstanceComponent(NearCombat);
	NearCombat->RegisterComponent();
	NearCombat->InitializeCombat(100.0f, 10.0f, 0.0f, 1.0f);

	AActor* EdgeTarget = SpawnPositionedActor(FVector(399.0, 0.0, -200.0));
	UCombatComponent* EdgeCombat = NewObject<UCombatComponent>(EdgeTarget);
	EdgeTarget->AddInstanceComponent(EdgeCombat);
	EdgeCombat->RegisterComponent();
	EdgeCombat->InitializeCombat(100.0f, 10.0f, 0.0f, 1.0f);

	AActor* FarTarget = SpawnPositionedActor(FVector(450.0, 0.0, 0.0));
	UCombatComponent* FarCombat = NewObject<UCombatComponent>(FarTarget);
	FarTarget->AddInstanceComponent(FarCombat);
	FarCombat->RegisterComponent();
	FarCombat->InitializeCombat(100.0f, 10.0f, 0.0f, 1.0f);

	AActor* DeadTarget = SpawnPositionedActor(FVector(100.0, 0.0, 0.0));
	UCombatComponent* DeadCombat = NewObject<UCombatComponent>(DeadTarget);
	DeadTarget->AddInstanceComponent(DeadCombat);
	DeadCombat->RegisterComponent();
	DeadCombat->InitializeCombat(100.0f, 10.0f, 0.0f, 1.0f);
	DeadCombat->TakeDamage(200.0f, Owner);

	const TArray<AActor*> AoeTargets = BattleAI->FindEnemiesInRange(400.0f);

	TestEqual(TEXT("Far target stays outside 400 units on X"), FarTarget->GetActorLocation().X, 450.0, 0.01);
	TestTrue(TEXT("AoE gathering includes a near target"), AoeTargets.Contains(NearTarget));
	TestTrue(TEXT("AoE gathering uses 2D distance and includes edge target"), AoeTargets.Contains(EdgeTarget));
	TestFalse(TEXT("AoE gathering excludes far target"), AoeTargets.Contains(FarTarget));
	TestFalse(TEXT("AoE gathering excludes dead target"), AoeTargets.Contains(DeadTarget));
	TestFalse(TEXT("AoE gathering excludes owner"), AoeTargets.Contains(Owner));

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBattleAIBossAttackPhaseScalingTest,
	"IdleProject.Combat.BattleAI.BossAttackPhaseScaling",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBattleAIBossAttackPhaseScalingTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("Transient test world is created"), World);
	if (!World)
	{
		return false;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AIdleMonster* Boss = World->SpawnActor<AIdleMonster>(AIdleMonster::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	AActor* Target = World->SpawnActor<AActor>(AActor::StaticClass(), FVector(100.0, 0.0, 0.0), FRotator::ZeroRotator, SpawnParams);
	UCombatComponent* TargetCombat = NewObject<UCombatComponent>(Target);
	Target->AddInstanceComponent(TargetCombat);
	TargetCombat->RegisterComponent();

	TestNotNull(TEXT("Boss is spawned"), Boss);
	TestNotNull(TEXT("Target is spawned"), Target);
	TestNotNull(TEXT("Target combat is created"), TargetCombat);
	if (!Boss || !Target || !TargetCombat)
	{
		World->DestroyWorld(false);
		return false;
	}

	Boss->SetBoss(true);
	UCombatComponent* BossCombat = Boss->GetCombat();
	UBattleAIComponent* BattleAI = Boss->FindComponentByClass<UBattleAIComponent>();
	TestNotNull(TEXT("Boss combat exists"), BossCombat);
	TestNotNull(TEXT("Boss battle AI exists"), BattleAI);
	if (!BossCombat || !BattleAI)
	{
		World->DestroyWorld(false);
		return false;
	}

	BossCombat->InitializeCombat(100.0f, 100.0f, 0.0f, 1.0f, 0.0f, 1.5f);
	BossCombat->CurrentHp = 20.0f;
	TargetCombat->InitializeCombat(1000.0f, 10.0f, 20.0f, 1.0f, 0.0f, 1.5f);

	UBossSpecialAttackTestReceiver* SpecialReceiver = NewObject<UBossSpecialAttackTestReceiver>();
	BattleAI->OnBossSpecialAttack.AddDynamic(SpecialReceiver, &UBossSpecialAttackTestReceiver::Capture);

	BattleAI->Attack(Target);

	TestEqual(TEXT("Boss opening attack uses phase scaling without special multiplier"), TargetCombat->CurrentHp, 1000.0f - 148.0f);
	TestEqual(TEXT("Boss opening attack does not broadcast special"), SpecialReceiver->Count, 0);

	World->TimeSeconds += FBossPhaseFormula::SpecialAttackIntervalSeconds + 0.1f;
	BattleAI->Attack(Target);

	BattleAI->OnBossSpecialAttack.RemoveDynamic(SpecialReceiver, &UBossSpecialAttackTestReceiver::Capture);

	TestEqual(TEXT("Boss periodic special multiplier affects damage after interval"), TargetCombat->CurrentHp, 1000.0f - 148.0f - 370.0f);
	TestEqual(TEXT("Boss base attack remains unchanged"), BossCombat->Atk, 100.0f);
	TestEqual(TEXT("Boss base attack speed remains unchanged"), BossCombat->AtkSpeed, 1.0f);
	TestEqual(TEXT("Boss special attack broadcasts once"), SpecialReceiver->Count, 1);
	TestTrue(TEXT("Boss special attack broadcasts owner"), SpecialReceiver->LastBoss.Get() == Boss);

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBattleAIBossPhaseAttackSpeedCooldownTest,
	"IdleProject.Combat.BattleAI.BossPhaseAttackSpeedCooldown",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBattleAIBossPhaseAttackSpeedCooldownTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("Transient test world is created"), World);
	if (!World)
	{
		return false;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AIdleMonster* Boss = World->SpawnActor<AIdleMonster>(AIdleMonster::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	AActor* Target = World->SpawnActor<AActor>(AActor::StaticClass(), FVector(100.0, 0.0, 0.0), FRotator::ZeroRotator, SpawnParams);
	UCombatComponent* TargetCombat = NewObject<UCombatComponent>(Target);
	Target->AddInstanceComponent(TargetCombat);
	TargetCombat->RegisterComponent();

	TestNotNull(TEXT("Boss is spawned"), Boss);
	TestNotNull(TEXT("Target is spawned"), Target);
	TestNotNull(TEXT("Target combat is created"), TargetCombat);
	if (!Boss || !Target || !TargetCombat)
	{
		World->DestroyWorld(false);
		return false;
	}

	Boss->SetBoss(true);
	UCombatComponent* BossCombat = Boss->GetCombat();
	UBattleAIComponent* BattleAI = Boss->FindComponentByClass<UBattleAIComponent>();
	TestNotNull(TEXT("Boss combat exists"), BossCombat);
	TestNotNull(TEXT("Boss battle AI exists"), BattleAI);
	if (!BossCombat || !BattleAI)
	{
		World->DestroyWorld(false);
		return false;
	}

	BossCombat->InitializeCombat(100.0f, 100.0f, 0.0f, 1.0f, 0.0f, 1.5f);
	BossCombat->CurrentHp = 20.0f;
	TargetCombat->InitializeCombat(1000.0f, 10.0f, 20.0f, 1.0f, 0.0f, 1.5f);

	BattleAI->Attack(Target);
	World->TimeSeconds += 0.8f;
	BattleAI->Attack(Target);

	TestEqual(TEXT("Phase 3 attack speed multiplier shortens cooldown for boss only"), TargetCombat->CurrentHp, 1000.0f - 148.0f - 148.0f);
	TestEqual(TEXT("Boss base attack speed remains unchanged after cooldown scaling"), BossCombat->AtkSpeed, 1.0f);

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBattleAINormalAttackUnchangedByBossPhaseTest,
	"IdleProject.Combat.BattleAI.NormalAttackUnchangedByBossPhase",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBattleAINormalAttackUnchangedByBossPhaseTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("Transient test world is created"), World);
	if (!World)
	{
		return false;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AIdleMonster* Monster = World->SpawnActor<AIdleMonster>(AIdleMonster::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	AActor* Target = World->SpawnActor<AActor>(AActor::StaticClass(), FVector(100.0, 0.0, 0.0), FRotator::ZeroRotator, SpawnParams);
	UCombatComponent* TargetCombat = NewObject<UCombatComponent>(Target);
	Target->AddInstanceComponent(TargetCombat);
	TargetCombat->RegisterComponent();

	TestNotNull(TEXT("Monster is spawned"), Monster);
	TestNotNull(TEXT("Target is spawned"), Target);
	TestNotNull(TEXT("Target combat is created"), TargetCombat);
	if (!Monster || !Target || !TargetCombat)
	{
		World->DestroyWorld(false);
		return false;
	}

	Monster->SetBoss(false);
	UCombatComponent* MonsterCombat = Monster->GetCombat();
	UBattleAIComponent* BattleAI = Monster->FindComponentByClass<UBattleAIComponent>();
	TestNotNull(TEXT("Monster combat exists"), MonsterCombat);
	TestNotNull(TEXT("Monster battle AI exists"), BattleAI);
	if (!MonsterCombat || !BattleAI)
	{
		World->DestroyWorld(false);
		return false;
	}

	MonsterCombat->InitializeCombat(100.0f, 100.0f, 0.0f, 1.0f, 0.0f, 1.5f);
	MonsterCombat->CurrentHp = 20.0f;
	TargetCombat->InitializeCombat(1000.0f, 10.0f, 20.0f, 1.0f, 0.0f, 1.5f);

	UBossSpecialAttackTestReceiver* SpecialReceiver = NewObject<UBossSpecialAttackTestReceiver>();
	BattleAI->OnBossSpecialAttack.AddDynamic(SpecialReceiver, &UBossSpecialAttackTestReceiver::Capture);

	BattleAI->Attack(Target);
	World->TimeSeconds += 0.8f;
	BattleAI->Attack(Target);

	BattleAI->OnBossSpecialAttack.RemoveDynamic(SpecialReceiver, &UBossSpecialAttackTestReceiver::Capture);

	TestEqual(TEXT("Normal monster keeps original damage and cooldown path"), TargetCombat->CurrentHp, 1000.0f - 88.0f);
	TestEqual(TEXT("Normal monster does not broadcast boss special"), SpecialReceiver->Count, 0);

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleMonsterBossConfigTest,
	"IdleProject.Combat.Monster.BossConfig",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleMonsterBossConfigTest::RunTest(const FString& Parameters)
{
	AIdleMonster* Monster = NewObject<AIdleMonster>();
	TestNotNull(TEXT("Monster is created"), Monster);
	if (!Monster)
	{
		return false;
	}

	TestFalse(TEXT("Default monster is not boss"), Monster->IsBoss());
	Monster->SetBoss(true);
	TestTrue(TEXT("Boss flag is stored"), Monster->IsBoss());
	TestTrue(TEXT("Boss max HP is stronger than normal monster HP"), Monster->GetConfiguredMaxHp() > 50.0f);
	TestTrue(TEXT("Boss attack is stronger than normal monster attack"), Monster->GetConfiguredAttack() > 8.0f);

	return true;
}
