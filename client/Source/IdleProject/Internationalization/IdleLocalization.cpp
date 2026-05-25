#include "Internationalization/IdleLocalization.h"

#include "HAL/FileManager.h"
#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/Csv/CsvParser.h"

namespace
{
struct FIdleLocalizationTable
{
	TMap<FString, FString> Rows;
};

TMap<FString, FIdleLocalizationTable> LoadedTables;

FString MakeCacheKey(const FString& Language, FName TableName)
{
	return FString::Printf(TEXT("%s:%s"), *IdleProject::Localization::NormalizeLanguage(Language), *TableName.ToString());
}

FString ResolveCsvPath(const FString& Language, FName TableName)
{
	return FPaths::Combine(
		FPaths::ProjectContentDir(),
		TEXT("Localization"),
		TEXT("Game"),
		IdleProject::Localization::NormalizeLanguage(Language),
		FString::Printf(TEXT("%s.csv"), *TableName.ToString()));
}

bool ParseLocalizationCsv(const FString& CsvText, TMap<FString, FString>& OutRows, TArray<FString>* OutErrors)
{
	FCsvParser Parser(CsvText);
	const FCsvParser::FRows& Rows = Parser.GetRows();
	if (Rows.Num() <= 1)
	{
		if (OutErrors)
		{
			OutErrors->Add(TEXT("CSV has no data rows."));
		}
		return false;
	}

	bool bOk = true;
	for (int32 RowIndex = 1; RowIndex < Rows.Num(); ++RowIndex)
	{
		const TArray<const TCHAR*>& Row = Rows[RowIndex];
		if (Row.Num() < 3)
		{
			bOk = false;
			if (OutErrors)
			{
				OutErrors->Add(FString::Printf(TEXT("Row %d has fewer than 3 columns."), RowIndex + 1));
			}
			continue;
		}

		const FString Key = Row[1] ? FString(Row[1]).TrimStartAndEnd() : FString();
		const FString SourceString = Row[2] ? FString(Row[2]).TrimStartAndEnd() : FString();
		if (Key.IsEmpty() || SourceString.IsEmpty())
		{
			bOk = false;
			if (OutErrors)
			{
				OutErrors->Add(FString::Printf(TEXT("Row %d has an empty key or translation."), RowIndex + 1));
			}
			continue;
		}

		if (OutRows.Contains(Key))
		{
			bOk = false;
			if (OutErrors)
			{
				OutErrors->Add(FString::Printf(TEXT("Duplicate key: %s"), *Key));
			}
			continue;
		}

		OutRows.Add(Key, SourceString);
	}

	return bOk;
}

const FIdleLocalizationTable* LoadTable(const FString& Language, FName TableName)
{
	const FString NormalizedLanguage = IdleProject::Localization::NormalizeLanguage(Language);
	const FString CacheKey = MakeCacheKey(NormalizedLanguage, TableName);
	if (const FIdleLocalizationTable* Cached = LoadedTables.Find(CacheKey))
	{
		return Cached;
	}

	FString CsvText;
	const FString CsvPath = ResolveCsvPath(NormalizedLanguage, TableName);
	if (!FFileHelper::LoadFileToString(CsvText, *CsvPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Localization] Failed to load %s"), *CsvPath);
		return nullptr;
	}

	FIdleLocalizationTable Table;
	TArray<FString> Errors;
	ParseLocalizationCsv(CsvText, Table.Rows, &Errors);
	for (const FString& Error : Errors)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Localization] %s: %s"), *CsvPath, *Error);
	}

	return &LoadedTables.Add(CacheKey, MoveTemp(Table));
}
}

namespace IdleProject::Localization
{
FString NormalizeLanguage(const FString& Language)
{
	FString Normalized = Language.Left(2).ToLower();
	return Normalized == TEXT("en") ? TEXT("en") : TEXT("ko");
}

bool SetCurrentLanguage(const FString& Language)
{
	const FString Normalized = NormalizeLanguage(Language);
	FInternationalization::Get().SetCurrentCulture(Normalized);
	return FInternationalization::Get().GetCurrentCulture()->GetTwoLetterISOLanguageName() == Normalized;
}

FString GetCurrentLanguage()
{
	return NormalizeLanguage(FInternationalization::Get().GetCurrentCulture()->GetTwoLetterISOLanguageName());
}

FText Text(FName TableName, const TCHAR* Key)
{
	const FString KeyString(Key);
	const FIdleLocalizationTable* Table = LoadTable(GetCurrentLanguage(), TableName);
	if (Table)
	{
		if (const FString* Value = Table->Rows.Find(KeyString))
		{
			return FText::FromString(*Value);
		}
	}

	const FIdleLocalizationTable* FallbackTable = LoadTable(TEXT("ko"), TableName);
	if (FallbackTable)
	{
		if (const FString* Value = FallbackTable->Rows.Find(KeyString))
		{
			return FText::FromString(*Value);
		}
	}

	return FText::FromString(KeyString);
}

FText Text(FName TableName, const TCHAR* Key, const FFormatNamedArguments& Args)
{
	return FText::Format(Text(TableName, Key), Args);
}

FText UI(const TCHAR* Key)
{
	return Text(TEXT("UI"), Key);
}

FText UI(const TCHAR* Key, const FFormatNamedArguments& Args)
{
	return Text(TEXT("UI"), Key, Args);
}

bool LoadTableKeysForTests(const FString& Language, FName TableName, TSet<FString>& OutKeys, TArray<FString>& OutErrors)
{
	OutKeys.Reset();
	OutErrors.Reset();

	FString CsvText;
	const FString CsvPath = ResolveCsvPath(Language, TableName);
	if (!FFileHelper::LoadFileToString(CsvText, *CsvPath))
	{
		OutErrors.Add(FString::Printf(TEXT("Failed to load %s"), *CsvPath));
		return false;
	}

	TMap<FString, FString> Rows;
	const bool bOk = ParseLocalizationCsv(CsvText, Rows, &OutErrors);
	Rows.GetKeys(OutKeys);
	return bOk;
}

void SetLanguageForTests(const FString& Language)
{
	SetCurrentLanguage(Language);
}

void ResetCacheForTests()
{
	LoadedTables.Reset();
}
}
