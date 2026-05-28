#pragma once

#include "CoreMinimal.h"
#include "CharacterSystem/StatFormulas.h"
#include "ItemSystem/ItemTypes.h"
#include "RuneTypes.generated.h"

UENUM(BlueprintType)
enum class ERuneType : uint8
{
	None = 0 UMETA(Hidden),
	PhysAtk = 1 UMETA(DisplayName = "PhysAtk"),
	MagicAtk = 2 UMETA(DisplayName = "MagicAtk"),
	PhysDef = 3 UMETA(DisplayName = "PhysDef"),
	MagicDef = 4 UMETA(DisplayName = "MagicDef"),
	Hp = 5 UMETA(DisplayName = "Hp"),
	CritDamage = 6 UMETA(DisplayName = "CritDamage"),
	GoldFind = 7 UMETA(DisplayName = "GoldFind"),
	ExpBoost = 8 UMETA(DisplayName = "ExpBoost"),
	OfflineEff = 9 UMETA(DisplayName = "OfflineEff"),
	ClassMastery = 10 UMETA(DisplayName = "ClassMastery")
};

USTRUCT(BlueprintType)
struct IDLEPROJECT_API FRuneCoreMultipliers
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Rune")
	float PhysAtk = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Rune")
	float MagicAtk = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Rune")
	float PhysDef = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Rune")
	float MagicDef = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Rune")
	float Hp = 1.0f;
};

USTRUCT(BlueprintType)
struct IDLEPROJECT_API FRuneUtilValues
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Rune")
	float CritDamage = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Rune")
	float GoldFind = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Rune")
	float ExpBoost = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Rune")
	float OfflineEff = 0.0f;
};

USTRUCT(BlueprintType)
struct IDLEPROJECT_API FRuneInstance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Rune")
	FName RuneId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Rune")
	ERuneType RuneType = ERuneType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Rune")
	EItemRarity Rarity = EItemRarity::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Rune", meta = (ClampMin = "0"))
	int32 EnhanceLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Rune")
	EClassId ClassRestriction = EClassId::None;
};

USTRUCT(BlueprintType)
struct IDLEPROJECT_API FRuneSaveEntry
{
	GENERATED_BODY()

	UPROPERTY()
	ERuneType RuneType = ERuneType::None;

	UPROPERTY()
	EItemRarity Rarity = EItemRarity::None;

	UPROPERTY()
	int32 EnhanceLevel = 0;

	UPROPERTY()
	FName RuneId = NAME_None;

	UPROPERTY()
	EClassId ClassRestriction = EClassId::None;
};
