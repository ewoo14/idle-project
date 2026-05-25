#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "IdleDataRows.generated.h"

/** 직업 CSV(ClassDB.csv) 임포트용 DataTable 행입니다. */
USTRUCT(BlueprintType)
struct IDLEPROJECT_API FClassRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Data")
	int32 ClassId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Data")
	FString NameKr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Data")
	FString NameEn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Data")
	FString PrimaryStat1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Data")
	FString PrimaryStat2;

	/** 1이면 MVP 기준 직업으로 취급합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Data")
	bool Mvp = false;
};

/** 레벨 곡선 CSV(LevelCurveDB.csv) 임포트용 DataTable 행입니다. */
USTRUCT(BlueprintType)
struct IDLEPROJECT_API FLevelCurveRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Data")
	int32 Level = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Data")
	int64 ExpToNext = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Data")
	int64 CumulativeExp = 0;
};
