#include "Misc/AutomationTest.h"

#include "RuneSystem/RuneCodexFormula.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRuneCodexFormulaEmptyTest,
	"IdleProject.Rune.CodexFormula.Empty",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRuneCodexFormulaEmptyTest::RunTest(const FString& Parameters)
{
	FRuneCodexCompletion Completion;
	const FRuneCodexBonus Bonus = FRuneCodexFormula::ComputeBonus(Completion);

	TestEqual(TEXT("Codex has sixty three total cells"), FRuneCodexFormula::TotalCells, 63);
	TestEqual(TEXT("Core category has thirty five cells"), FRuneCodexFormula::CoreCategoryCells, 35);
	TestEqual(TEXT("Util category has twenty eight cells"), FRuneCodexFormula::UtilCategoryCells, 28);
	TestEqual(TEXT("Empty codex has zero core bonus"), Bonus.CoreStatAdd, 0.0f);
	TestEqual(TEXT("Empty codex has zero util cap extension"), Bonus.UtilCapExtension, 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRuneCodexFormulaRowsAndCategoriesTest,
	"IdleProject.Rune.CodexFormula.RowsAndCategories",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRuneCodexFormulaRowsAndCategoriesTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Common row completion bonus"), FRuneCodexFormula::GetRowCompletionBonus(EItemRarity::Common), 0.01f);
	TestEqual(TEXT("Rare row completion bonus"), FRuneCodexFormula::GetRowCompletionBonus(EItemRarity::Rare), 0.02f);
	TestEqual(TEXT("Epic row completion bonus"), FRuneCodexFormula::GetRowCompletionBonus(EItemRarity::Epic), 0.03f);
	TestEqual(TEXT("Unique row completion bonus"), FRuneCodexFormula::GetRowCompletionBonus(EItemRarity::Unique), 0.05f);
	TestEqual(TEXT("Legendary row completion bonus"), FRuneCodexFormula::GetRowCompletionBonus(EItemRarity::Legendary), 0.08f);
	TestEqual(TEXT("Transcendent row completion bonus"), FRuneCodexFormula::GetRowCompletionBonus(EItemRarity::Transcendent), 0.10f);
	TestEqual(TEXT("Mythic row completion bonus"), FRuneCodexFormula::GetRowCompletionBonus(EItemRarity::Mythic), 0.12f);
	TestEqual(TEXT("Invalid row completion bonus"), FRuneCodexFormula::GetRowCompletionBonus(EItemRarity::None), 0.0f);

	FRuneCodexCompletion Completion;
	Completion.UnlockedCells = 63;
	Completion.RowComplete = {true, true, true, true, true, true, true};
	Completion.bCoreCategoryComplete = true;
	Completion.bUtilCategoryComplete = true;

	const FRuneCodexBonus Bonus = FRuneCodexFormula::ComputeBonus(Completion);
	TestEqual(TEXT("Full codex core bonus includes cells, rows, and core category"), Bonus.CoreStatAdd, 0.712f);
	TestEqual(TEXT("Full codex util category extends util caps"), Bonus.UtilCapExtension, 0.10f);

	return true;
}

#endif
