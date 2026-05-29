#pragma once

#include "CoreMinimal.h"
#include "ConsumableTypes.generated.h"

UENUM(BlueprintType)
enum class EConsumableType : uint8
{
	AttackTonic = 0,
	GuardTonic = 1,
	AllStatElixir = 2,
	FortuneScroll = 3,
	GoldFeast = 4,
	WisdomBooster = 5
};

/** 소비 아이템 등급(소/중/대). Standard 가 #73 기본 효과이며, Lesser=×0.5 / Greater=×2.0 으로 효과 %를 차등합니다. */
UENUM(BlueprintType)
enum class EConsumableGrade : uint8
{
	Lesser = 0,
	Standard = 1,
	Greater = 2
};

USTRUCT(BlueprintType)
struct IDLEPROJECT_API FConsumableSaveEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Idle|Consumable")
	uint8 Type = 0;

	/** 소비 등급(0=소,1=중,2=대). v15 이하 세이브는 Grade 필드가 없어 기본값 Standard(1) 로 마이그레이션됩니다. */
	UPROPERTY(BlueprintReadWrite, Category = "Idle|Consumable")
	uint8 Grade = 1;

	UPROPERTY(BlueprintReadWrite, Category = "Idle|Consumable")
	int32 Count = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Idle|Consumable")
	int64 BuffEndUnixSec = 0;
};
