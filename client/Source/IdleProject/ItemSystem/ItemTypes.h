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
	Uncommon = 2 UMETA(DisplayName = "Uncommon"),
	Rare = 3 UMETA(DisplayName = "Rare"),
	Epic = 4 UMETA(DisplayName = "Epic"),
	Legendary = 5 UMETA(DisplayName = "Legendary"),
	Mythic = 6 UMETA(DisplayName = "Mythic")
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

/** 장비 1개의 런타임 인스턴스입니다. */
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
};

struct IDLEPROJECT_API FItemPowerScore
{
	static int32 Compute(const FItemInstance& Item)
	{
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
		const float RawScore = BaseScore
			* (1.0f + static_cast<float>(Item.EnhanceLevel) * 0.1f);
		return FMath::RoundToInt(RawScore);
	}
};
