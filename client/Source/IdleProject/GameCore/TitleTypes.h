#pragma once

#include "CoreMinimal.h"
#include "GameCore/AchievementFormula.h"
#include "TitleTypes.generated.h"

// 칭호 장착 보너스 차원. None 을 제외하고 서버 title.ts 의 TitleBonusType 와 1:1.
// bonusValue 는 비율(0.03 = +3%)로 저장한다(퍼센트 정수 아님).
UENUM(BlueprintType)
enum class ETitleBonus : uint8
{
	None = 0 UMETA(Hidden),
	AllStatPct = 1,
	GoldPct = 2,
	ExpPct = 3,
	CritDmgPct = 4
};

// 칭호 정의. 서버 카탈로그(TITLE_CATALOG)를 1:1 이식한다.
// Metric 이 Threshold 이상이면 영구 해금, 장착 시 BonusType/BonusValue 패시브 적용.
USTRUCT(BlueprintType)
struct IDLEPROJECT_API FTitleDefinition
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Title")
	FString TitleId;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Title")
	FText Name;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Title")
	EAchievementMetric Metric = EAchievementMetric::MonstersKilled;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Title")
	int64 Threshold = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Title")
	ETitleBonus BonusType = ETitleBonus::None;

	// 보너스 비율(0.03 = +3%). 퍼센트 정수 아님.
	UPROPERTY(BlueprintReadOnly, Category = "Idle|Title")
	float BonusValue = 0.0f;
};

// 장착 중인 칭호 1개의 해소된 보너스. 미장착/미해금이면 None/0.
USTRUCT(BlueprintType)
struct IDLEPROJECT_API FTitleBonus
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Title")
	ETitleBonus Type = ETitleBonus::None;

	// 비율(0.03 = +3%).
	UPROPERTY(BlueprintReadOnly, Category = "Idle|Title")
	float Value = 0.0f;
};
