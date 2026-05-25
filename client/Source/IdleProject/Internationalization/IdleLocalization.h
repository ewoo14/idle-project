#pragma once

#include "CoreMinimal.h"

namespace IdleProject::Localization
{
IDLEPROJECT_API FString NormalizeLanguage(const FString& Language);
IDLEPROJECT_API bool SetCurrentLanguage(const FString& Language);
IDLEPROJECT_API FString GetCurrentLanguage();

IDLEPROJECT_API FText Text(FName TableName, const TCHAR* Key);
IDLEPROJECT_API FText Text(FName TableName, const TCHAR* Key, const FFormatNamedArguments& Args);
IDLEPROJECT_API FText UI(const TCHAR* Key);
IDLEPROJECT_API FText UI(const TCHAR* Key, const FFormatNamedArguments& Args);

IDLEPROJECT_API bool LoadTableKeysForTests(const FString& Language, FName TableName, TSet<FString>& OutKeys, TArray<FString>& OutErrors);
IDLEPROJECT_API void SetLanguageForTests(const FString& Language);
IDLEPROJECT_API void ResetCacheForTests();
}
