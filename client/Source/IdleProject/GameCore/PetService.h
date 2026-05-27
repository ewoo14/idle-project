#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PetService.generated.h"

UENUM(BlueprintType)
enum class EPetBonusType : uint8
{
	None = 0 UMETA(Hidden),
	Gold = 1,
	Drop = 2
};

USTRUCT(BlueprintType)
struct FPetDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle|Pet")
	FString PetId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle|Pet")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle|Pet")
	EPetBonusType BonusType = EPetBonusType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle|Pet")
	float BonusPercent = 0.0f;
};

UCLASS(BlueprintType)
class IDLEPROJECT_API UPetService : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Idle|Pet")
	void InitializeDefaultPets();

	UFUNCTION(BlueprintCallable, Category = "Idle|Pet")
	bool EquipPet(const FString& PetId);

	UFUNCTION(BlueprintPure, Category = "Idle|Pet")
	const TArray<FPetDefinition>& GetPetDefinitions() const { return Definitions; }

	UFUNCTION(BlueprintPure, Category = "Idle|Pet")
	const FString& GetEquippedPetId() const { return EquippedPetId; }

	UFUNCTION(BlueprintPure, Category = "Idle|Pet")
	int32 GetPetLevel(const FString& PetId) const;

	const TMap<FString, int32>& GetPetLevels() const { return PetLevels; }

	void RestoreState(const FString& PetId, const TMap<FString, int32>& Levels);

	UFUNCTION(BlueprintCallable, Category = "Idle|Pet")
	bool FeedPet(const FString& PetId);

	UFUNCTION(BlueprintPure, Category = "Idle|Pet")
	float GetEquippedPetGoldBonusPercent() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Pet")
	float GetEquippedPetDropBonusPercent() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Pet")
	int64 ApplyGoldBonus(int64 BaseAmount) const;

	UFUNCTION(BlueprintPure, Category = "Idle|Pet")
	float ApplyDropBonusChance(float BaseChance) const;

private:
	TArray<FPetDefinition> Definitions;
	TMap<FString, FPetDefinition> DefinitionById;
	TSet<FString> OwnedPetIds;
	TMap<FString, int32> PetLevels;
	FString EquippedPetId;

	void BuildDefaultDefinitions();
	const FPetDefinition* GetEquippedPetDefinition() const;
};
