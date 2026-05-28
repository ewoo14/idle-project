#pragma once

#include "CoreMinimal.h"
#include "ItemTypes.generated.h"

UENUM(BlueprintType)
enum class EItemSlot : uint8
{
	None = 0 UMETA(Hidden),
	Weapon = 1 UMETA(DisplayName = "Weapon"),
	Helmet = 2 UMETA(DisplayName = "Helmet"),
	Top = 3 UMETA(DisplayName = "Top"),
	Bottom = 4 UMETA(DisplayName = "Bottom"),
	Shoes = 5 UMETA(DisplayName = "Shoes"),
	Gloves = 6 UMETA(DisplayName = "Gloves"),
	Cloak = 7 UMETA(DisplayName = "Cloak"),
	Accessory = 8 UMETA(DisplayName = "Accessory")
};

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	None = 0 UMETA(Hidden),
	Common = 1 UMETA(DisplayName = "Common"),
	Rare = 2 UMETA(DisplayName = "Rare"),
	Epic = 3 UMETA(DisplayName = "Epic"),
	Unique = 4 UMETA(DisplayName = "Unique"),
	Legendary = 5 UMETA(DisplayName = "Legendary"),
	Transcendent = 6 UMETA(DisplayName = "Transcendent"),
	Mythic = 7 UMETA(DisplayName = "Mythic")
};

UENUM(BlueprintType)
enum class EItemSet : uint8
{
	None = 0 UMETA(Hidden),
	Warrior = 1 UMETA(DisplayName = "Warrior"),
	Guardian = 2 UMETA(DisplayName = "Guardian"),
	Arcane = 3 UMETA(DisplayName = "Arcane"),
	Assassin = 4 UMETA(DisplayName = "Assassin"),
	Hunter = 5 UMETA(DisplayName = "Hunter"),
	Holy = 6 UMETA(DisplayName = "Holy"),
	Berserker = 7 UMETA(DisplayName = "Berserker")
};

UENUM(BlueprintType)
enum class EUniqueTrait : uint8
{
	None = 0 UMETA(Hidden),
	AllStatSurge = 1 UMETA(DisplayName = "AllStatSurge"),
	CritDamageSurge = 2 UMETA(DisplayName = "CritDamageSurge"),
	CritRateSurge = 3 UMETA(DisplayName = "CritRateSurge"),
	LifeSurge = 4 UMETA(DisplayName = "LifeSurge"),
	SwiftSurge = 5 UMETA(DisplayName = "SwiftSurge"),
	PhysMastery = 6 UMETA(DisplayName = "PhysMastery"),
	MagicMastery = 7 UMETA(DisplayName = "MagicMastery"),
	GuardMastery = 8 UMETA(DisplayName = "GuardMastery")
};

/** 장비 1개의 런타임 인스턴스입니다. */
UENUM(BlueprintType)
enum class EPotentialGrade : uint8
{
	None = 0 UMETA(Hidden),
	Rare = 1 UMETA(DisplayName = "Rare"),
	Epic = 2 UMETA(DisplayName = "Epic"),
	Unique = 3 UMETA(DisplayName = "Unique"),
	Legendary = 4 UMETA(DisplayName = "Legendary")
};

UENUM(BlueprintType)
enum class EPotentialStat : uint8
{
	None = 0 UMETA(Hidden),
	PhysAtkPercent = 1 UMETA(DisplayName = "PhysAtkPercent"),
	MagicAtkPercent = 2 UMETA(DisplayName = "MagicAtkPercent"),
	HpPercent = 3 UMETA(DisplayName = "HpPercent"),
	PhysDefPercent = 4 UMETA(DisplayName = "PhysDefPercent"),
	MagicDefPercent = 5 UMETA(DisplayName = "MagicDefPercent"),
	CritRatePercent = 6 UMETA(DisplayName = "CritRatePercent"),
	AtkSpeedPercent = 7 UMETA(DisplayName = "AtkSpeedPercent"),
	CritDmgPercent = 8 UMETA(DisplayName = "CritDmgPercent")
};

UENUM(BlueprintType)
enum class EPotentialCubeType : uint8
{
	Reset = 0 UMETA(DisplayName = "Reset"),
	Rank = 1 UMETA(DisplayName = "Rank")
};

USTRUCT(BlueprintType)
struct IDLEPROJECT_API FPotentialLine
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	EPotentialStat Stat = EPotentialStat::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	float Value = 0.0f;
};

USTRUCT(BlueprintType)
struct IDLEPROJECT_API FItemInstance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	FName BaseItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	EItemSlot Slot = EItemSlot::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	EItemRarity Rarity = EItemRarity::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	EItemSet ItemSet = EItemSet::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	EUniqueTrait UniqueTrait1 = EUniqueTrait::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	EUniqueTrait UniqueTrait2 = EUniqueTrait::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	float BonusAtk = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	float BonusDef = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	float BonusHp = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	float BonusCritRate = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	float BonusAtkSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	float BonusMagicAtk = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	float BonusPhysDef = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	float BonusMagicDef = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	float BonusAffixHp = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	float BonusCritDmg = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item", meta = (ClampMin = "0", ClampMax = "50"))
	int32 EnhanceLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	EPotentialGrade PotentialGrade = EPotentialGrade::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	FPotentialLine PotentialLine1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	FPotentialLine PotentialLine2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	FPotentialLine PotentialLine3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	int32 EnhanceFailStreak = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	bool bLocked = false;
};

struct IDLEPROJECT_API FItemPowerScore
{
	static int32 Compute(const FItemInstance& Item)
	{
		const float EnhanceMultiplier = 1.0f + static_cast<float>(Item.EnhanceLevel) * 0.1f;
		const float BaseScore = Item.BonusAtk
			+ Item.BonusDef
			+ Item.BonusHp / 10.0f
			+ Item.BonusCritRate * 1000.0f
			+ Item.BonusAtkSpeed * 100.0f
			+ Item.BonusMagicAtk
			+ Item.BonusPhysDef
			+ Item.BonusMagicDef
			+ Item.BonusAffixHp / 10.0f
			+ Item.BonusCritDmg * 100.0f;
		auto PotentialBonus = [&Item, EnhanceMultiplier](const FPotentialLine& Line)
		{
			switch (Line.Stat)
			{
			case EPotentialStat::PhysAtkPercent:
				return Item.BonusAtk * EnhanceMultiplier * Line.Value;
			case EPotentialStat::MagicAtkPercent:
				return Item.BonusMagicAtk * EnhanceMultiplier * Line.Value;
			case EPotentialStat::HpPercent:
				return (Item.BonusHp + Item.BonusAffixHp) * EnhanceMultiplier * Line.Value / 10.0f;
			case EPotentialStat::PhysDefPercent:
				return (Item.BonusDef + Item.BonusPhysDef) * EnhanceMultiplier * Line.Value;
			case EPotentialStat::MagicDefPercent:
				return Item.BonusMagicDef * EnhanceMultiplier * Line.Value;
			case EPotentialStat::CritRatePercent:
				return Line.Value * 1000.0f;
			case EPotentialStat::AtkSpeedPercent:
				return Line.Value * 100.0f;
			case EPotentialStat::CritDmgPercent:
				return Line.Value * 100.0f;
			case EPotentialStat::None:
			default:
				return 0.0f;
			}
		};
		const float RawScore = BaseScore * EnhanceMultiplier
			+ PotentialBonus(Item.PotentialLine1)
			+ PotentialBonus(Item.PotentialLine2)
			+ PotentialBonus(Item.PotentialLine3);
		return FMath::RoundToInt(RawScore);
	}
};
