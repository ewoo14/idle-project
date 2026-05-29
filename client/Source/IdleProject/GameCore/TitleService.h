#pragma once

#include "CoreMinimal.h"
#include "GameCore/TitleTypes.h"
#include "UObject/Object.h"
#include "TitleService.generated.h"

class UAchievementService;

/**
 * 칭호(Title) 서비스. 업적 메트릭 달성으로 칭호를 영구 해금하고, 1개를 장착해
 * 패시브 보너스(AllStat/Gold/Exp/CritDmg)를 제공한다. 해금/장착은 클라 로컬 세이브 권위.
 * 카탈로그는 서버 title.ts(TITLE_CATALOG) 18종을 1:1 이식한다(임계/보너스 비율 동일).
 */
UCLASS(BlueprintType)
class IDLEPROJECT_API UTitleService : public UObject
{
	GENERATED_BODY()

public:
	// 서버 TITLE_CATALOG 18종을 1:1 로 빌드한다(중복 호출 시 재구성하지 않음).
	UFUNCTION(BlueprintCallable, Category = "Idle|Title")
	void InitializeDefaultTitles();

	// 각 칭호의 Metric 값(Ach->GetMetricValue)이 Threshold 이상이면 영구 해금한다(추가 only, 해제 없음).
	void RecomputeUnlocked(const UAchievementService* Achievements);

	// 해금된 칭호만 장착 가능. 성공 시 true. 미해금/미정의는 거부(false).
	UFUNCTION(BlueprintCallable, Category = "Idle|Title")
	bool EquipTitle(const FString& TitleId);

	UFUNCTION(BlueprintCallable, Category = "Idle|Title")
	void UnequipTitle();

	// 장착된 칭호 1개의 보너스. 미장착/미해금이면 None/0.
	UFUNCTION(BlueprintPure, Category = "Idle|Title")
	FTitleBonus GetEquippedTitleBonus() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Title")
	const TArray<FTitleDefinition>& GetDefinitions() const { return Definitions; }

	UFUNCTION(BlueprintPure, Category = "Idle|Title")
	bool IsUnlocked(const FString& TitleId) const { return UnlockedTitleIds.Contains(TitleId); }

	UFUNCTION(BlueprintPure, Category = "Idle|Title")
	const FString& GetEquippedTitleId() const { return EquippedTitleId; }

	const TSet<FString>& GetUnlockedTitleIds() const { return UnlockedTitleIds; }

	// 세이브 복원. 정의에 없는 id 는 무시하며, 장착은 해금된 경우에만 유지된다.
	void RestoreState(const TSet<FString>& Unlocked, const FString& Equipped);

private:
	TArray<FTitleDefinition> Definitions;
	TMap<FString, FTitleDefinition> DefinitionById;
	TSet<FString> UnlockedTitleIds;
	FString EquippedTitleId;

	void BuildDefaultDefinitions();
	const FTitleDefinition* GetEquippedDefinition() const;
};
