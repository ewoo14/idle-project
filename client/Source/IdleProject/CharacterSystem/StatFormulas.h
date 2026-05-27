#pragma once

#include "CoreMinimal.h"
#include "StatFormulas.generated.h"

/**
 * 서버 ClassId와 1:1로 대응되는 직업 ID입니다.
 * 0 (None) 은 UE5 reflection (UENUM) 요구사항에 따른 기본 sentinel 값으로,
 * 실제 게임 로직에서는 잘못된 ClassId 로 취급해야 합니다.
 */
UENUM(BlueprintType)
enum class EClassId : uint8
{
	None = 0 UMETA(Hidden),
	Warrior = 1 UMETA(DisplayName = "Warrior"),
	Mage = 2 UMETA(DisplayName = "Mage"),
	Archer = 3 UMETA(DisplayName = "Archer"),
	Thief = 4 UMETA(DisplayName = "Thief"),
	Cleric = 5 UMETA(DisplayName = "Cleric"),
	Paladin = 6 UMETA(DisplayName = "Paladin"),
	Berserker = 7 UMETA(DisplayName = "Berserker"),
	Summoner = 8 UMETA(DisplayName = "Summoner")
};

/** 서버 PrimaryStats를 결정적으로 미러링하는 1차 능력치입니다. */
USTRUCT(BlueprintType)
struct IDLEPROJECT_API FPrimaryStats
{
	GENERATED_BODY()

	FPrimaryStats() = default;

	FPrimaryStats(float InStr, float InDex, float InInt, float InWis, float InCon, float InLuk)
		: Str(InStr)
		, Dex(InDex)
		, Int_(InInt)
		, Wis(InWis)
		, Con(InCon)
		, Luk(InLuk)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Stats")
	float Str = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Stats")
	float Dex = 0.0f;

	/** C++ 키워드 충돌을 피하기 위한 INT 필드입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Stats")
	float Int_ = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Stats")
	float Wis = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Stats")
	float Con = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Stats")
	float Luk = 0.0f;
};

/** 서버 DerivedStats를 결정적으로 미러링하는 2차 능력치입니다. */
USTRUCT(BlueprintType)
struct IDLEPROJECT_API FDerivedStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Stats")
	float Hp = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Stats")
	float Mp = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Stats")
	float PhysAtk = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Stats")
	float MagicAtk = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Stats")
	float PhysDef = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Stats")
	float MagicDef = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Stats")
	float AtkSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Stats")
	float MoveSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Stats")
	float CritRate = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Stats")
	float CritDmg = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Stats")
	float Dodge = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Stats")
	float Accuracy = 0.0f;
};

/** 서버 stats.ts의 순수 수식만 C++로 옮긴 정적 유틸리티입니다. */
struct IDLEPROJECT_API FStatFormulas
{
	static FPrimaryStats DefaultPrimaryStats(EClassId ClassId, int32 Level);
	static FDerivedStats DeriveStats(const FPrimaryStats& Primary, int32 Level, const FDerivedStats& EquipmentBonus = FDerivedStats(), int32 RebirthBonusPoints = 0);
};
