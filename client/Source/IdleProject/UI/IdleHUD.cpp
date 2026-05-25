#include "UI/IdleHUD.h"

#include "CombatSystem/CombatComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "GameCore/IdleGameInstance.h"
#include "ItemSystem/InventoryComponent.h"
#include "UI/IdleHUDWidget.h"

namespace
{
FString RarityToString(EItemRarity Rarity)
{
	switch (Rarity)
	{
	case EItemRarity::Uncommon:
		return TEXT("Uncommon");
	case EItemRarity::Rare:
		return TEXT("Rare");
	case EItemRarity::Common:
		return TEXT("Common");
	case EItemRarity::None:
	default:
		return TEXT("None");
	}
}
}

void AIdleHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	RootWidget = SNew(SIdleHUDWidget);
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->AddViewportWidgetContent(RootWidget.ToSharedRef());
	}

	IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	if (!IdleGameInstance)
	{
		return;
	}

	IdleGameInstance->OnGoldChanged.AddDynamic(this, &AIdleHUD::HandleGoldChanged);
	IdleGameInstance->OnExpChanged.AddDynamic(this, &AIdleHUD::HandleExpChanged);
	IdleGameInstance->OnLevelUp.AddDynamic(this, &AIdleHUD::HandleLevelUp);

	RootWidget->UpdateGold(IdleGameInstance->GetGold());
	RootWidget->UpdateExp(IdleGameInstance->GetCurrentExp(), IdleGameInstance->GetNextExp());
	RootWidget->UpdateLevel(IdleGameInstance->GetCharacterLevel());

	BindPlayerCombat();
	BindPlayerInventory();
}

void AIdleHUD::BeginPlay()
{
	Super::BeginPlay();
	BindPlayerCombat();
	BindPlayerInventory();
}

void AIdleHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GEngine && GEngine->GameViewport && RootWidget)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(RootWidget.ToSharedRef());
	}

	RootWidget.Reset();
	PlayerCombat = nullptr;
	PlayerInventory = nullptr;

	Super::EndPlay(EndPlayReason);
}

void AIdleHUD::HandleGoldChanged(int64 NewGold)
{
	UE_LOG(LogTemp, Display, TEXT("[IdleHUD] Gold=%lld"), NewGold);
	if (RootWidget)
	{
		RootWidget->UpdateGold(NewGold);
	}
}

void AIdleHUD::HandleExpChanged(int64 CurrentExp, int64 NextExp)
{
	UE_LOG(LogTemp, Display, TEXT("[IdleHUD] EXP=%lld/%lld"), CurrentExp, NextExp);
	if (RootWidget)
	{
		RootWidget->UpdateExp(CurrentExp, NextExp);
	}
}

void AIdleHUD::HandleLevelUp(int32 NewLevel)
{
	UE_LOG(LogTemp, Display, TEXT("[IdleHUD] Level=%d"), NewLevel);
	if (RootWidget)
	{
		RootWidget->UpdateLevel(NewLevel);
	}
}

void AIdleHUD::HandleHpChanged(float NewHp)
{
	if (RootWidget && PlayerCombat)
	{
		RootWidget->UpdateHp(NewHp, PlayerCombat->MaxHp);
	}
}

void AIdleHUD::HandleEquippedChanged(EItemSlot Slot)
{
	RefreshEquipmentSummary();
}

void AIdleHUD::BindPlayerCombat()
{
	if (PlayerCombat)
	{
		return;
	}

	APawn* Pawn = PlayerOwner ? PlayerOwner->GetPawn() : nullptr;
	if (!Pawn)
	{
		return;
	}

	PlayerCombat = Pawn->FindComponentByClass<UCombatComponent>();
	if (!PlayerCombat)
	{
		return;
	}

	PlayerCombat->OnHpChanged.AddDynamic(this, &AIdleHUD::HandleHpChanged);
	if (RootWidget)
	{
		RootWidget->UpdateHp(PlayerCombat->CurrentHp, PlayerCombat->MaxHp);
	}
}

void AIdleHUD::BindPlayerInventory()
{
	if (PlayerInventory)
	{
		return;
	}

	APawn* Pawn = PlayerOwner ? PlayerOwner->GetPawn() : nullptr;
	if (!Pawn)
	{
		return;
	}

	PlayerInventory = Pawn->FindComponentByClass<UInventoryComponent>();
	if (!PlayerInventory)
	{
		return;
	}

	PlayerInventory->OnEquippedChanged.AddDynamic(this, &AIdleHUD::HandleEquippedChanged);
	RefreshEquipmentSummary();
}

void AIdleHUD::RefreshEquipmentSummary()
{
	if (!RootWidget || !PlayerInventory)
	{
		return;
	}

	FText WeaponLine = FText::FromString(TEXT("⚔ 무기 없음"));
	if (const FItemInstance* Weapon = PlayerInventory->GetEquippedItem(EItemSlot::Weapon))
	{
		WeaponLine = FText::FromString(FString::Printf(
			TEXT("⚔ %s (%s, ATK+%.0f)"),
			*Weapon->DisplayName.ToString(),
			*RarityToString(Weapon->Rarity),
			Weapon->BonusAtk));
	}

	const EItemSlot ArmorSlots[] = {
		EItemSlot::Helmet,
		EItemSlot::Top,
		EItemSlot::Bottom,
		EItemSlot::Shoes,
		EItemSlot::Gloves,
		EItemSlot::Cloak,
		EItemSlot::Accessory
	};

	int32 EquippedArmorCount = 0;
	float BonusDef = 0.0f;
	float BonusHp = 0.0f;

	for (EItemSlot Slot : ArmorSlots)
	{
		if (const FItemInstance* Item = PlayerInventory->GetEquippedItem(Slot))
		{
			++EquippedArmorCount;
			BonusDef += Item->BonusDef;
			BonusHp += Item->BonusHp;
		}
	}

	const FText ArmorLine = FText::FromString(FString::Printf(
		TEXT("🛡 방어구 %d/7 슬롯 (DEF+%.0f, HP+%.0f)"),
		EquippedArmorCount,
		BonusDef,
		BonusHp));

	RootWidget->UpdateEquipment(WeaponLine, ArmorLine);
}
