#include "Misc/AutomationTest.h"

#include "GameCore/IdleGameInstance.h"
#include "GameCore/IdleSaveGame.h"
#include "RuneSystem/ClassRuneFormula.h"
#include "RuneSystem/RuneFormula.h"
#include "RuneSystem/RuneSetFormula.h"
#include "RuneSystem/RuneService.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
FRuneInstance MakeSetRune(FName RuneId, ERuneType Type, ERuneSet RuneSet, EItemRarity Rarity = EItemRarity::Rare, int32 EnhanceLevel = 0)
{
	FRuneInstance Rune;
	Rune.RuneId = RuneId;
	Rune.RuneType = Type;
	Rune.RuneSet = RuneSet;
	Rune.Rarity = Rarity;
	Rune.EnhanceLevel = EnhanceLevel;
	return Rune;
}

FRuneInstance MakeClassSetRune(FName RuneId, ERuneSet RuneSet)
{
	FRuneInstance Rune;
	Rune.RuneId = RuneId;
	Rune.RuneType = ERuneType::ClassMastery;
	Rune.RuneSet = RuneSet;
	Rune.Rarity = EItemRarity::Rare;
	Rune.ClassRestriction = EClassId::Warrior;
	return Rune;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRuneSetServiceCoreBonusTest,
	"IdleProject.Rune.SetService.CoreBonus",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRuneSetServiceCoreBonusTest::RunTest(const FString& Parameters)
{
	URuneService* RuneService = NewObject<URuneService>();
	for (int32 Index = 0; Index < 6; ++Index)
	{
		RuneService->AddRune(MakeSetRune(*FString::Printf(TEXT("offense_%d"), Index), ERuneType::PhysAtk, ERuneSet::Offense));
		TestTrue(TEXT("Offense rune equips into regular slot"), RuneService->TryEquipRune(Index, Index));
	}

	const FRuneCoreMultipliers Core = RuneService->GetEquippedCoreMultipliers();
	const float RuneBonus = FRuneFormula::GetCoreRuneMultiplier(EItemRarity::Rare, 0);
	TestEqual(TEXT("Six Offense runes add tier-three physical attack set bonus"), Core.PhysAtk, 1.0f + 6.0f * RuneBonus + 0.25f);
	TestEqual(TEXT("Six Offense runes add tier-three magic attack set bonus"), Core.MagicAtk, 1.0f + 0.25f);
	TestEqual(TEXT("Unrelated core stat remains neutral"), Core.PhysDef, 1.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRuneSetServiceUtilBonusIgnoresCapTest,
	"IdleProject.Rune.SetService.UtilBonusIgnoresCap",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRuneSetServiceUtilBonusIgnoresCapTest::RunTest(const FString& Parameters)
{
	URuneService* RuneService = NewObject<URuneService>();
	for (int32 Index = 0; Index < 6; ++Index)
	{
		RuneService->AddRune(MakeSetRune(*FString::Printf(TEXT("fortune_%d"), Index), ERuneType::GoldFind, ERuneSet::Fortune, EItemRarity::Mythic, 10000));
		TestTrue(TEXT("Fortune rune equips into regular slot"), RuneService->TryEquipRune(Index, Index));
	}

	const FRuneUtilValues Util = RuneService->GetEquippedUtilValues();
	TestEqual(TEXT("Rune util value caps before set bonus is added"), Util.GoldFind, FRuneFormula::GetUtilCap(ERuneType::GoldFind) + 0.25f);
	TestEqual(TEXT("Fortune adds exp boost set bonus"), Util.ExpBoost, 0.25f);
	TestEqual(TEXT("Fortune adds crit damage set bonus"), Util.CritDamage, 0.25f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRuneSetServiceClassSlotExcludedTest,
	"IdleProject.Rune.SetService.ClassSlotExcluded",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRuneSetServiceClassSlotExcludedTest::RunTest(const FString& Parameters)
{
	URuneService* RuneService = NewObject<URuneService>();
	RuneService->SetOwnerClassId(EClassId::Warrior);
	RuneService->AddRune(MakeSetRune(TEXT("offense_regular"), ERuneType::PhysAtk, ERuneSet::Offense));
	RuneService->AddRune(MakeClassSetRune(TEXT("offense_class"), ERuneSet::Offense));

	TestTrue(TEXT("Regular rune equips into regular slot"), RuneService->TryEquipRune(0, 0));
	TestTrue(TEXT("Class rune equips into class slot"), RuneService->TryEquipRune(FClassRuneFormula::ClassRuneSlotIndex, 1));

	const FRuneCoreMultipliers Core = RuneService->GetEquippedCoreMultipliers();
	const float RegularBonus = FRuneFormula::GetCoreRuneMultiplier(EItemRarity::Rare, 0);
	const float ClassBonus = FRuneFormula::GetCoreRuneMultiplier(EItemRarity::Rare, 0);
	TestEqual(TEXT("Class slot RuneSet is excluded from set counting"), Core.PhysAtk, 1.0f + RegularBonus + ClassBonus);
	TestEqual(TEXT("Excluded class slot does not activate Offense magic attack"), Core.MagicAtk, 1.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRuneSetServiceSaveRoundTripTest,
	"IdleProject.Rune.SetService.SaveRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRuneSetServiceSaveRoundTripTest::RunTest(const FString& Parameters)
{
	URuneService* RuneService = NewObject<URuneService>();
	RuneService->AddRune(MakeSetRune(TEXT("vitality"), ERuneType::Hp, ERuneSet::Vitality));
	TestTrue(TEXT("Rune equips before save"), RuneService->TryEquipRune(0, 0));

	TArray<FRuneSaveEntry> SavedRunes;
	TArray<int32> SavedSlots;
	RuneService->CaptureState(SavedRunes, SavedSlots);
	TestEqual(TEXT("Captured rune set persists into save entry"), SavedRunes[0].RuneSet, ERuneSet::Vitality);

	URuneService* Restored = NewObject<URuneService>();
	Restored->RestoreState(SavedRunes, SavedSlots);
	TestEqual(TEXT("Restored rune keeps rune set"), Restored->GetOwnedRunes()[0].RuneSet, ERuneSet::Vitality);
	TestEqual(TEXT("Restored equipped slot points to rune"), Restored->GetEquippedOwnedIndex(0), 0);

	FRuneSaveEntry LegacyEntry = SavedRunes[0];
	LegacyEntry.RuneSet = ERuneSet::None;
	Restored->RestoreState({LegacyEntry}, SavedSlots);
	TestEqual(TEXT("Legacy v5-style entry without RuneSet contributes no set bonus"), Restored->GetEquippedCoreMultipliers().Hp, 1.0f + FRuneFormula::GetCoreRuneMultiplier(EItemRarity::Rare, 0));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRuneSetGameInstanceSaveVersionTest,
	"IdleProject.Rune.SetService.GameInstanceSaveVersion",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRuneSetGameInstanceSaveVersionTest::RunTest(const FString& Parameters)
{
	UIdleSaveGame* SaveGame = NewObject<UIdleSaveGame>();
	TestEqual(TEXT("Default save version is nine for rune sets"), SaveGame->SaveVersion, 9);

	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	GameInstance->InitializeRuneServiceForTests();
	GameInstance->AddRuneForTests(MakeSetRune(TEXT("saved_offense"), ERuneType::PhysAtk, ERuneSet::Offense));

	UIdleSaveGame* CapturedSave = NewObject<UIdleSaveGame>();
	TestTrue(TEXT("Capture writes save"), GameInstance->CaptureToSave(CapturedSave));
	TestEqual(TEXT("Captured save version is nine"), CapturedSave->SaveVersion, 9);
	TestEqual(TEXT("Captured rune includes set"), CapturedSave->Runes[0].RuneSet, ERuneSet::Offense);

	return true;
}

#endif
