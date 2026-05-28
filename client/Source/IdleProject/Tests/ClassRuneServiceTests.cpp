#include "Misc/AutomationTest.h"

#include "RuneSystem/ClassRuneFormula.h"
#include "RuneSystem/RuneFormula.h"
#include "RuneSystem/RuneService.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
FRuneInstance MakeClassRune(FName RuneId, EClassId ClassId)
{
	FRuneInstance Rune;
	Rune.RuneId = RuneId;
	Rune.RuneType = ERuneType::ClassMastery;
	Rune.Rarity = EItemRarity::Rare;
	Rune.ClassRestriction = ClassId;
	return Rune;
}

FRuneInstance MakeRegularRune(FName RuneId, ERuneType Type)
{
	FRuneInstance Rune;
	Rune.RuneId = RuneId;
	Rune.RuneType = Type;
	Rune.Rarity = EItemRarity::Rare;
	return Rune;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FClassRuneServiceEquipRulesTest,
	"IdleProject.Rune.ClassService.EquipRules",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FClassRuneServiceEquipRulesTest::RunTest(const FString& Parameters)
{
	URuneService* RuneService = NewObject<URuneService>();
	RuneService->SetOwnerClassId(EClassId::Warrior);
	RuneService->AddRune(MakeClassRune(TEXT("warrior_class"), EClassId::Warrior));
	RuneService->AddRune(MakeClassRune(TEXT("mage_class"), EClassId::Mage));
	RuneService->AddRune(MakeRegularRune(TEXT("phys"), ERuneType::PhysAtk));

	TestTrue(TEXT("Matching class mastery rune equips into class slot"), RuneService->TryEquipRune(FClassRuneFormula::ClassRuneSlotIndex, 0));
	TestFalse(TEXT("Mismatched class mastery rune is rejected from class slot"), RuneService->TryEquipRune(FClassRuneFormula::ClassRuneSlotIndex, 1));
	TestFalse(TEXT("Regular rune is rejected from class slot"), RuneService->TryEquipRune(FClassRuneFormula::ClassRuneSlotIndex, 2));
	TestFalse(TEXT("Class mastery rune is rejected from regular slots"), RuneService->TryEquipRune(0, 0));

	const FRuneCoreMultipliers Multipliers = RuneService->GetEquippedCoreMultipliers();
	const float Unit = FRuneFormula::GetCoreRuneMultiplier(EItemRarity::Rare, 0);
	TestEqual(TEXT("Equipped warrior class rune adds physical attack bonus"), Multipliers.PhysAtk, 1.0f + Unit);
	TestEqual(TEXT("Equipped warrior class rune adds physical defense bonus"), Multipliers.PhysDef, 1.0f + Unit);
	TestEqual(TEXT("Unmapped core stat stays neutral"), Multipliers.MagicAtk, 1.0f);

	RuneService->SetOwnerClassId(EClassId::Mage);
	TestEqual(TEXT("Changing owner class clears mismatched class slot"), RuneService->GetEquippedOwnedIndex(FClassRuneFormula::ClassRuneSlotIndex), INDEX_NONE);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FClassRuneServiceSaveRoundTripTest,
	"IdleProject.Rune.ClassService.SaveRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FClassRuneServiceSaveRoundTripTest::RunTest(const FString& Parameters)
{
	URuneService* RuneService = NewObject<URuneService>();
	RuneService->SetOwnerClassId(EClassId::Cleric);
	RuneService->AddRune(MakeClassRune(TEXT("cleric_class"), EClassId::Cleric));
	TestTrue(TEXT("Class rune equips before save"), RuneService->TryEquipRune(FClassRuneFormula::ClassRuneSlotIndex, 0));

	TArray<FRuneSaveEntry> SavedRunes;
	TArray<int32> SavedSlots;
	RuneService->CaptureState(SavedRunes, SavedSlots);

	TestEqual(TEXT("Captured slots include class slot"), SavedSlots.Num(), FRuneFormula::RuneSlotCount);
	TestEqual(TEXT("Captured class restriction round trips into save entry"), SavedRunes[0].ClassRestriction, EClassId::Cleric);

	URuneService* Restored = NewObject<URuneService>();
	Restored->SetOwnerClassId(EClassId::Cleric);
	Restored->RestoreState(SavedRunes, SavedSlots);

	TestEqual(TEXT("Restored class rune keeps class restriction"), Restored->GetOwnedRunes()[0].ClassRestriction, EClassId::Cleric);
	TestEqual(TEXT("Restored class slot points to restored rune"), Restored->GetEquippedOwnedIndex(FClassRuneFormula::ClassRuneSlotIndex), 0);

	return true;
}

#endif
