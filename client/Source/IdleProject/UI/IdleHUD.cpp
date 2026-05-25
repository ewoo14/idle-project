#include "UI/IdleHUD.h"

#include "CombatSystem/CombatComponent.h"
#include "CombatSystem/SkillComponent.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/GameViewportClient.h"
#include "GameCore/IdleGameInstance.h"
#include "ItemSystem/InventoryComponent.h"
#include "UI/IdleHUDWidget.h"
#include "UI/UIThemeTokens.h"

namespace
{
const FName OfflineRewardClaimHitBoxName(TEXT("OfflineRewardClaim"));

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

FString FormatIntegerWithCommas(int64 Value)
{
	FString Digits = FString::Printf(TEXT("%lld"), FMath::Abs(Value));
	FString Formatted;
	int32 DigitCount = 0;
	for (int32 Index = Digits.Len() - 1; Index >= 0; --Index)
	{
		if (DigitCount > 0 && DigitCount % 3 == 0)
		{
			Formatted.InsertAt(0, TEXT(","));
		}
		Formatted.InsertAt(0, Digits[Index]);
		++DigitCount;
	}

	return Value < 0 ? FString::Printf(TEXT("-%s"), *Formatted) : Formatted;
}

FString FormatElapsedHoursMinutes(int64 CappedSeconds)
{
	const int64 ClampedSeconds = FMath::Max<int64>(0, CappedSeconds);
	const int64 Hours = ClampedSeconds / 3600;
	const int64 Minutes = (ClampedSeconds % 3600) / 60;
	return FString::Printf(TEXT("%lld:%02lld"), Hours, Minutes);
}
}

TArray<FIdleHUDSkillSlotViewModel> IdleProject::UI::BuildSkillSlotViewModels(const USkillComponent& SkillComponent, float Now)
{
	TArray<FIdleHUDSkillSlotViewModel> Slots;
	for (const FSkillDefinition& Skill : SkillComponent.Skills)
	{
		if (Skill.Type != ESkillType::Active)
		{
			continue;
		}

		FIdleHUDSkillSlotViewModel Slot;
		Slot.SkillId = Skill.SkillId;
		Slot.DisplayName = Skill.DisplayName;
		Slot.CooldownRatio = SkillComponent.GetCooldownRatio(Skill.SkillId, Now);
		Slot.CooldownRemaining = SkillComponent.GetCooldownRemaining(Skill.SkillId, Now);
		Slot.bReady = Slot.CooldownRemaining <= 0.0f;
		Slots.Add(Slot);
	}

	return Slots;
}

FIdleHUDUltimateViewModel IdleProject::UI::BuildUltimateViewModel(const USkillComponent& SkillComponent)
{
	FIdleHUDUltimateViewModel Ultimate;
	Ultimate.GaugePercent = FMath::Clamp(SkillComponent.GetCurrentGauge(), 0.0f, 100.0f);
	Ultimate.GaugeRatio = Ultimate.GaugePercent / 100.0f;
	Ultimate.bReady = SkillComponent.IsUltimateReady();
	return Ultimate;
}

FIdleHUDOfflineRewardViewModel IdleProject::UI::BuildOfflineRewardViewModel(const FOfflineRewardResult& Reward)
{
	FIdleHUDOfflineRewardViewModel ViewModel;
	ViewModel.bVisible = Reward.Gold > 0 || Reward.Exp > 0;
	ViewModel.Title = FText::FromString(TEXT("오프라인 보상"));
	ViewModel.ElapsedLabel = FText::FromString(FString::Printf(
		TEXT("경과 시간 %s"),
		*FormatElapsedHoursMinutes(Reward.CappedSeconds)));
	ViewModel.GoldLabel = FText::FromString(FString::Printf(
		TEXT("골드 +%s"),
		*FormatIntegerWithCommas(Reward.Gold)));
	ViewModel.ExpLabel = FText::FromString(FString::Printf(
		TEXT("EXP +%s"),
		*FormatIntegerWithCommas(Reward.Exp)));
	ViewModel.ClaimLabel = FText::FromString(TEXT("수령"));
	return ViewModel;
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
	PreviewOfflineRewardModal();
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

void AIdleHUD::DrawHUD()
{
	Super::DrawHUD();

	const UWorld* World = GetWorld();
	const USkillComponent* PlayerSkills = ResolvePlayerSkills();
	if (World && PlayerSkills)
	{
		DrawSkillHud(*PlayerSkills, World->GetTimeSeconds());
	}

	DrawOfflineRewardModal();
}

void AIdleHUD::NotifyHitBoxClick(FName BoxName)
{
	if (BoxName == OfflineRewardClaimHitBoxName)
	{
		ClaimOfflineRewardModal();
		return;
	}

	Super::NotifyHitBoxClick(BoxName);
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

USkillComponent* AIdleHUD::ResolvePlayerSkills() const
{
	const APawn* Pawn = PlayerOwner ? PlayerOwner->GetPawn() : nullptr;
	return Pawn ? Pawn->FindComponentByClass<USkillComponent>() : nullptr;
}

void AIdleHUD::DrawSkillHud(const USkillComponent& SkillComponent, float Now)
{
	if (!Canvas)
	{
		return;
	}

	const TArray<FIdleHUDSkillSlotViewModel> Slots = IdleProject::UI::BuildSkillSlotViewModels(SkillComponent, Now);
	if (Slots.Num() <= 0)
	{
		return;
	}

	const float HudScale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float SlotWidth = 148.0f * HudScale;
	const float SlotHeight = 48.0f * HudScale;
	const float SlotGap = 10.0f * HudScale;
	const float TotalWidth = Slots.Num() * SlotWidth + (Slots.Num() - 1) * SlotGap;
	const float StartX = FMath::Max(24.0f * HudScale, (Canvas->SizeX - TotalWidth) * 0.5f);
	const float SlotY = FMath::Max(96.0f * HudScale, Canvas->SizeY - 116.0f * HudScale);

	for (int32 Index = 0; Index < Slots.Num(); ++Index)
	{
		DrawSkillSlot(Slots[Index], Index, StartX + Index * (SlotWidth + SlotGap), SlotY, SlotWidth, SlotHeight);
	}

	DrawUltimateGauge(
		IdleProject::UI::BuildUltimateViewModel(SkillComponent),
		StartX,
		SlotY - 34.0f * HudScale,
		TotalWidth,
		22.0f * HudScale);
}

void AIdleHUD::DrawSkillSlot(const FIdleHUDSkillSlotViewModel& Slot, int32 SlotIndex, float X, float Y, float Width, float Height)
{
	using namespace IdleProject::UI;

	const float Scale = Height / 48.0f;
	const FLinearColor PanelColor = Theme::BgPanel.CopyWithNewOpacity(0.86f);
	const FLinearColor BorderColor = Slot.bReady ? Theme::AccentGold : Theme::TextMuted.CopyWithNewOpacity(0.70f);
	const FLinearColor FillColor = Slot.bReady ? Theme::AccentGold : Theme::AccentBlue;
	const FLinearColor CooldownOverlay = FLinearColor(0.0f, 0.0f, 0.0f, 0.34f);

	DrawRect(PanelColor, X, Y, Width, Height);
	DrawRect(BorderColor, X, Y, Width, 2.0f * Scale);
	DrawRect(BorderColor, X, Y + Height - 2.0f * Scale, Width, 2.0f * Scale);
	DrawRect(BorderColor, X, Y, 2.0f * Scale, Height);
	DrawRect(BorderColor, X + Width - 2.0f * Scale, Y, 2.0f * Scale, Height);

	const float ReadyRatio = 1.0f - Slot.CooldownRatio;
	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.92f), X + 8.0f * Scale, Y + Height - 12.0f * Scale, Width - 16.0f * Scale, 5.0f * Scale);
	DrawRect(FillColor, X + 8.0f * Scale, Y + Height - 12.0f * Scale, (Width - 16.0f * Scale) * ReadyRatio, 5.0f * Scale);

	if (!Slot.bReady)
	{
		DrawRect(CooldownOverlay, X + Width * ReadyRatio, Y, Width * Slot.CooldownRatio, Height);
	}

	const FString KeyLabel = FString::Printf(TEXT("%d"), SlotIndex + 1);
	DrawText(KeyLabel, Theme::TextMuted, X + 8.0f * Scale, Y + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.90f * Scale);
	DrawText(Slot.DisplayName.ToString(), Theme::TextPrimary, X + 28.0f * Scale, Y + 5.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.90f * Scale);

	const FString StatusLabel = Slot.bReady
		? TEXT("READY")
		: FString::Printf(TEXT("%.1fs"), Slot.CooldownRemaining);
	DrawText(StatusLabel, Slot.bReady ? Theme::AccentGold : Theme::TextMuted, X + 10.0f * Scale, Y + 24.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.86f * Scale);
}

void AIdleHUD::DrawUltimateGauge(const FIdleHUDUltimateViewModel& Ultimate, float X, float Y, float Width, float Height)
{
	using namespace IdleProject::UI;

	const float Scale = Height / 22.0f;
	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.86f), X, Y, Width, Height);
	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.92f), X + 4.0f * Scale, Y + 4.0f * Scale, Width - 8.0f * Scale, Height - 8.0f * Scale);
	DrawRect(Theme::Auth, X + 4.0f * Scale, Y + 4.0f * Scale, (Width - 8.0f * Scale) * Ultimate.GaugeRatio, Height - 8.0f * Scale);

	const FString GaugeLabel = Ultimate.bReady
		? TEXT("ULTIMATE READY")
		: FString::Printf(TEXT("ULTIMATE %.0f%%"), Ultimate.GaugePercent);
	DrawText(GaugeLabel, Ultimate.bReady ? Theme::AccentGold : Theme::TextPrimary, X + 10.0f * Scale, Y + 2.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.86f * Scale);
}

void AIdleHUD::PreviewOfflineRewardModal()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance)
	{
		return;
	}

	const int64 NowUnixSec = FDateTime::UtcNow().ToUnixTimestamp();
	OfflineRewardModal = IdleProject::UI::BuildOfflineRewardViewModel(IdleGameInstance->PreviewOfflineRewards(NowUnixSec));
	if (OfflineRewardModal.bVisible && PlayerOwner)
	{
		PlayerOwner->bShowMouseCursor = true;
		PlayerOwner->bEnableClickEvents = true;
	}
}

void AIdleHUD::ClaimOfflineRewardModal()
{
	if (!OfflineRewardModal.bVisible || !IdleGameInstance)
	{
		return;
	}

	IdleGameInstance->ClaimOfflineRewards();
	OfflineRewardModal.bVisible = false;
}

void AIdleHUD::DrawOfflineRewardModal()
{
	using namespace IdleProject::UI;

	if (!Canvas || !OfflineRewardModal.bVisible)
	{
		return;
	}

	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float ModalWidth = FMath::Clamp(Canvas->SizeX * 0.42f, 420.0f * Scale, 640.0f * Scale);
	const float ModalHeight = 284.0f * Scale;
	const float X = (Canvas->SizeX - ModalWidth) * 0.5f;
	const float Y = (Canvas->SizeY - ModalHeight) * 0.5f;
	const float Padding = 28.0f * Scale;
	const float Border = 2.0f * Scale;

	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.56f), 0.0f, 0.0f, Canvas->SizeX, Canvas->SizeY);
	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.96f), X, Y, ModalWidth, ModalHeight);
	DrawRect(Theme::AccentGold, X, Y, ModalWidth, Border);
	DrawRect(Theme::AccentGold, X, Y + ModalHeight - Border, ModalWidth, Border);
	DrawRect(Theme::AccentGold, X, Y, Border, ModalHeight);
	DrawRect(Theme::AccentGold, X + ModalWidth - Border, Y, Border, ModalHeight);

	DrawText(OfflineRewardModal.Title.ToString(), Theme::TextPrimary, X + Padding, Y + 22.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 1.05f * Scale);
	DrawText(OfflineRewardModal.ElapsedLabel.ToString(), Theme::TextMuted, X + Padding, Y + 72.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 1.0f * Scale);
	DrawText(OfflineRewardModal.GoldLabel.ToString(), Theme::AccentGold, X + Padding, Y + 120.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 1.0f * Scale);
	DrawText(OfflineRewardModal.ExpLabel.ToString(), Theme::AccentBlue, X + Padding, Y + 160.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 1.0f * Scale);

	const float ButtonWidth = 156.0f * Scale;
	const float ButtonHeight = 44.0f * Scale;
	const float ButtonX = X + ModalWidth - Padding - ButtonWidth;
	const float ButtonY = Y + ModalHeight - Padding - ButtonHeight;
	DrawRect(Theme::AccentGold, ButtonX, ButtonY, ButtonWidth, ButtonHeight);
	DrawText(OfflineRewardModal.ClaimLabel.ToString(), Theme::BgPrimary, ButtonX + 52.0f * Scale, ButtonY + 10.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 1.0f * Scale);
	AddHitBox(FVector2D(ButtonX, ButtonY), FVector2D(ButtonWidth, ButtonHeight), OfflineRewardClaimHitBoxName, true, 100);
}
