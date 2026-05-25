#include "UI/IdleHUD.h"

#include "CombatSystem/CombatComponent.h"
#include "CombatSystem/SkillComponent.h"
#include "CharacterSystem/IdleCharacter.h"
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
const FName RebirthHitBoxName(TEXT("RebirthAction"));
const FString ClassSelectionHitBoxPrefix(TEXT("ClassSelect_"));
const FString QuestClaimHitBoxPrefix(TEXT("QuestClaim_"));
constexpr int32 RebirthRequiredLevel = 100;
constexpr int32 RebirthBonusPointsPerRun = 5;

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

FText QuestTypeToLabel(EQuestType Type)
{
	return Type == EQuestType::Main
		? FText::FromString(TEXT("메인"))
		: FText::FromString(TEXT("일일"));
}

FName MakeQuestClaimHitBoxName(const FString& QuestId)
{
	return FName(*(QuestClaimHitBoxPrefix + QuestId));
}

FName MakeClassSelectionHitBoxName(EClassId ClassId)
{
	return FName(*(ClassSelectionHitBoxPrefix + FString::FromInt(static_cast<int32>(ClassId))));
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

FIdleHUDQuestLogViewModel IdleProject::UI::BuildQuestLogViewModel(const TArray<FQuestState>& QuestStates)
{
	FIdleHUDQuestLogViewModel ViewModel;
	ViewModel.Title = FText::FromString(TEXT("퀘스트"));
	ViewModel.ShortcutLabel = FText::FromString(TEXT("Q 닫기"));
	ViewModel.EmptyLabel = FText::FromString(TEXT("진행 중인 퀘스트 없음"));

	for (const FQuestState& State : QuestStates)
	{
		FIdleHUDQuestLogRowViewModel Row;
		Row.QuestId = State.QuestId;
		Row.TypeLabel = QuestTypeToLabel(State.Type);
		Row.Title = State.Title;
		Row.ProgressRatio = State.TargetCount > 0
			? FMath::Clamp(static_cast<float>(State.Progress) / static_cast<float>(State.TargetCount), 0.0f, 1.0f)
			: 1.0f;
		Row.ProgressLabel = FText::FromString(FString::Printf(
			TEXT("진행 %d / %d"),
			State.Progress,
			State.TargetCount));
		Row.RewardLabel = FText::FromString(FString::Printf(
			TEXT("보상 골드 %lld / EXP %lld"),
			State.RewardGold,
			State.RewardExp));
		Row.bCanClaim = State.bCompleted && !State.bClaimed;
		Row.bClaimed = State.bClaimed;
		if (Row.bCanClaim)
		{
			Row.ActionLabel = FText::FromString(TEXT("수령"));
		}
		else if (Row.bClaimed)
		{
			Row.ActionLabel = FText::FromString(TEXT("완료"));
		}
		else
		{
			Row.ActionLabel = FText::FromString(TEXT("진행"));
		}
		ViewModel.Rows.Add(Row);
	}

	return ViewModel;
}

FIdleHUDRebirthViewModel IdleProject::UI::BuildRebirthViewModel(bool bCanRebirth, bool bBossDefeated, int32 CharacterLevel, int32 RebirthCount, int32 RebirthBonusPoints)
{
	FIdleHUDRebirthViewModel ViewModel;
	const int32 SafeLevel = FMath::Max(1, CharacterLevel);
	const int32 SafeRebirthCount = FMath::Max(0, RebirthCount);
	const int32 SafeBonusPoints = FMath::Max(0, RebirthBonusPoints);

	ViewModel.bCanRebirth = bCanRebirth;
	ViewModel.bBossDefeated = bBossDefeated;
	ViewModel.bLevelReady = SafeLevel >= RebirthRequiredLevel;
	ViewModel.Title = FText::FromString(TEXT("환생"));
	ViewModel.StatusLabel = FText::FromString(bCanRebirth ? TEXT("환생 가능") : TEXT("환생 조건 미달"));
	ViewModel.BossLabel = FText::FromString(bBossDefeated ? TEXT("보스 격파 완료") : TEXT("보스 격파 필요"));
	ViewModel.LevelLabel = ViewModel.bLevelReady
		? FText::FromString(TEXT("레벨 100 달성"))
		: FText::FromString(FString::Printf(TEXT("레벨 %d / 100"), SafeLevel));
	ViewModel.CountLabel = FText::FromString(FString::Printf(TEXT("환생 %d회"), SafeRebirthCount));
	ViewModel.BonusLabel = FText::FromString(FString::Printf(TEXT("영구 보너스 %d 포인트"), SafeBonusPoints));
	ViewModel.NextBonusLabel = FText::FromString(FString::Printf(TEXT("환생 진행 시 +%d 포인트"), RebirthBonusPointsPerRun));
	ViewModel.ButtonLabel = FText::FromString(TEXT("환생 진행"));
	return ViewModel;
}

TArray<FIdleHUDClassSelectionOptionViewModel> IdleProject::UI::BuildClassSelectionOptions(EClassId CurrentClassId)
{
	struct FClassOptionSeed
	{
		EClassId ClassId;
		const TCHAR* DisplayName;
		const TCHAR* RoleLabel;
		const TCHAR* StatSummary;
	};

	const FClassOptionSeed Seeds[] = {
		{ EClassId::Warrior, TEXT("전사"), TEXT("근접 방어형"), TEXT("STR/CON") },
		{ EClassId::Mage, TEXT("마법사"), TEXT("원거리 마법형"), TEXT("INT") },
		{ EClassId::Archer, TEXT("궁수"), TEXT("치명 원거리형"), TEXT("DEX") }
	};

	TArray<FIdleHUDClassSelectionOptionViewModel> Options;
	for (const FClassOptionSeed& Seed : Seeds)
	{
		FIdleHUDClassSelectionOptionViewModel Option;
		Option.ClassId = Seed.ClassId;
		Option.DisplayName = FText::FromString(Seed.DisplayName);
		Option.RoleLabel = FText::FromString(Seed.RoleLabel);
		Option.StatSummary = FText::FromString(Seed.StatSummary);
		Option.bSelected = Seed.ClassId == CurrentClassId;
		Options.Add(Option);
	}
	return Options;
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
	DrawRebirthPanel();
	DrawClassSelectionPanel();
	DrawQuestLog();
}

void AIdleHUD::NotifyHitBoxClick(FName BoxName)
{
	if (BoxName == OfflineRewardClaimHitBoxName)
	{
		ClaimOfflineRewardModal();
		return;
	}
	if (BoxName.ToString().StartsWith(QuestClaimHitBoxPrefix))
	{
		ClaimQuestFromHitBox(BoxName);
		return;
	}
	if (BoxName == RebirthHitBoxName)
	{
		TryRebirth();
		return;
	}
	if (BoxName.ToString().StartsWith(ClassSelectionHitBoxPrefix))
	{
		SelectClassFromHitBox(BoxName);
		return;
	}

	Super::NotifyHitBoxClick(BoxName);
}

void AIdleHUD::ToggleQuestLog()
{
	bQuestLogVisible = !bQuestLogVisible;
	if (PlayerOwner)
	{
		RefreshMouseInteraction();
	}
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
	const AIdleCharacter* Character = ResolvePlayerCharacter();
	return Character ? Character->FindComponentByClass<USkillComponent>() : nullptr;
}

AIdleCharacter* AIdleHUD::ResolvePlayerCharacter() const
{
	APawn* Pawn = PlayerOwner ? PlayerOwner->GetPawn() : nullptr;
	return Pawn ? Cast<AIdleCharacter>(Pawn) : nullptr;
}

void AIdleHUD::DrawClassSelectionPanel()
{
	using namespace IdleProject::UI;

	if (!Canvas)
	{
		return;
	}

	AIdleCharacter* IdleCharacter = ResolvePlayerCharacter();
	if (!IdleCharacter)
	{
		return;
	}

	const TArray<FIdleHUDClassSelectionOptionViewModel> Options = BuildClassSelectionOptions(IdleCharacter->GetClassId());
	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelWidth = 322.0f * Scale;
	const float HeaderHeight = 42.0f * Scale;
	const float OptionHeight = 48.0f * Scale;
	const float OptionGap = 8.0f * Scale;
	const float Padding = 14.0f * Scale;
	const float PanelHeight = HeaderHeight + Padding + Options.Num() * OptionHeight + FMath::Max(0, Options.Num() - 1) * OptionGap + Padding;
	const float X = 28.0f * Scale;
	const float Y = 92.0f * Scale;
	const float Border = 2.0f * Scale;

	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.91f), X, Y, PanelWidth, PanelHeight);
	DrawRect(Theme::AccentBlue, X, Y, PanelWidth, Border);
	DrawRect(Theme::AccentBlue, X, Y + PanelHeight - Border, PanelWidth, Border);
	DrawRect(Theme::AccentBlue, X, Y, Border, PanelHeight);
	DrawRect(Theme::AccentBlue, X + PanelWidth - Border, Y, Border, PanelHeight);

	DrawText(TEXT("시작 직업"), Theme::TextPrimary, X + Padding, Y + 12.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 0.92f * Scale);
	DrawText(TEXT("전사/마법사/궁수"), Theme::TextMuted, X + PanelWidth - 128.0f * Scale, Y + 16.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.78f * Scale);

	float OptionY = Y + HeaderHeight + Padding;
	for (const FIdleHUDClassSelectionOptionViewModel& Option : Options)
	{
		DrawClassSelectionOption(Option, X + Padding, OptionY, PanelWidth - Padding * 2.0f, OptionHeight);
		OptionY += OptionHeight + OptionGap;
	}

	RefreshMouseInteraction();
}

void AIdleHUD::DrawClassSelectionOption(const FIdleHUDClassSelectionOptionViewModel& Option, float X, float Y, float Width, float Height)
{
	using namespace IdleProject::UI;

	const float Scale = Height / 48.0f;
	const FLinearColor StateColor = Option.bSelected ? Theme::AccentGold : Theme::TextMuted.CopyWithNewOpacity(0.56f);
	DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.90f), X, Y, Width, Height);
	DrawRect(StateColor, X, Y, 4.0f * Scale, Height);

	DrawText(Option.DisplayName.ToString(), Option.bSelected ? Theme::AccentGold : Theme::TextPrimary, X + 14.0f * Scale, Y + 7.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.96f * Scale);
	DrawText(Option.RoleLabel.ToString(), Theme::TextMuted, X + 82.0f * Scale, Y + 8.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.82f * Scale);
	DrawText(Option.StatSummary.ToString(), Theme::AccentBlue, X + 14.0f * Scale, Y + 27.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.82f * Scale);

	const FString StateLabel = Option.bSelected ? TEXT("SELECTED") : TEXT("SELECT");
	DrawText(StateLabel, Option.bSelected ? Theme::AccentGold : Theme::TextMuted, X + Width - 74.0f * Scale, Y + 18.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.78f * Scale);
	AddHitBox(FVector2D(X, Y), FVector2D(Width, Height), MakeClassSelectionHitBoxName(Option.ClassId), true, 70);
}

void AIdleHUD::SelectClassFromHitBox(FName BoxName)
{
	FString RawClassId = BoxName.ToString();
	RawClassId.RightChopInline(ClassSelectionHitBoxPrefix.Len());
	const int32 ClassIdValue = FCString::Atoi(*RawClassId);
	if (ClassIdValue < static_cast<int32>(EClassId::Warrior) || ClassIdValue > static_cast<int32>(EClassId::Archer))
	{
		return;
	}

	AIdleCharacter* IdleCharacter = ResolvePlayerCharacter();
	if (!IdleCharacter)
	{
		return;
	}

	IdleCharacter->SetClassId(static_cast<EClassId>(ClassIdValue));
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
		RefreshMouseInteraction();
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
	RefreshMouseInteraction();
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

void AIdleHUD::DrawQuestLog()
{
	using namespace IdleProject::UI;

	if (!Canvas || !bQuestLogVisible)
	{
		return;
	}
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance)
	{
		return;
	}

	const FIdleHUDQuestLogViewModel ViewModel = BuildQuestLogViewModel(IdleGameInstance->GetActiveQuestStates());
	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelWidth = FMath::Clamp(Canvas->SizeX * 0.36f, 420.0f * Scale, 620.0f * Scale);
	const float RowHeight = 74.0f * Scale;
	const float HeaderHeight = 54.0f * Scale;
	const float Padding = 18.0f * Scale;
	const float PanelHeight = HeaderHeight + Padding + FMath::Max(1, ViewModel.Rows.Num()) * (RowHeight + 10.0f * Scale) + Padding;
	const float X = Canvas->SizeX - PanelWidth - 28.0f * Scale;
	const float Y = 92.0f * Scale;
	const float Border = 2.0f * Scale;

	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.94f), X, Y, PanelWidth, PanelHeight);
	DrawRect(Theme::AccentBlue, X, Y, PanelWidth, Border);
	DrawRect(Theme::AccentBlue, X, Y + PanelHeight - Border, PanelWidth, Border);
	DrawRect(Theme::AccentBlue, X, Y, Border, PanelHeight);
	DrawRect(Theme::AccentBlue, X + PanelWidth - Border, Y, Border, PanelHeight);

	DrawText(ViewModel.Title.ToString(), Theme::TextPrimary, X + Padding, Y + 14.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 1.0f * Scale);
	DrawText(ViewModel.ShortcutLabel.ToString(), Theme::TextMuted, X + PanelWidth - 72.0f * Scale, Y + 18.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.82f * Scale);

	float RowY = Y + HeaderHeight;
	if (ViewModel.Rows.IsEmpty())
	{
		DrawText(ViewModel.EmptyLabel.ToString(), Theme::TextMuted, X + Padding, RowY + 20.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.95f * Scale);
		return;
	}

	for (const FIdleHUDQuestLogRowViewModel& Row : ViewModel.Rows)
	{
		const float RowX = X + Padding;
		const float InnerWidth = PanelWidth - Padding * 2.0f;
		DrawRect(Theme::BgPrimary.CopyWithNewOpacity(0.90f), RowX, RowY, InnerWidth, RowHeight);
		DrawRect(Row.bCanClaim ? Theme::AccentGold : Theme::TextMuted.CopyWithNewOpacity(0.42f), RowX, RowY, 4.0f * Scale, RowHeight);

		DrawText(Row.TypeLabel.ToString(), Row.bCanClaim ? Theme::AccentGold : Theme::AccentBlue, RowX + 12.0f * Scale, RowY + 8.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.82f * Scale);
		DrawText(Row.Title.ToString(), Theme::TextPrimary, RowX + 58.0f * Scale, RowY + 8.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.92f * Scale);
		DrawText(Row.ProgressLabel.ToString(), Theme::TextMuted, RowX + 12.0f * Scale, RowY + 33.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.82f * Scale);
		DrawText(Row.RewardLabel.ToString(), Theme::TextMuted, RowX + 132.0f * Scale, RowY + 33.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.82f * Scale);

		const float BarX = RowX + 12.0f * Scale;
		const float BarY = RowY + RowHeight - 12.0f * Scale;
		const float BarWidth = InnerWidth - 104.0f * Scale;
		DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.96f), BarX, BarY, BarWidth, 5.0f * Scale);
		DrawRect(Row.bCanClaim ? Theme::AccentGold : Theme::AccentBlue, BarX, BarY, BarWidth * Row.ProgressRatio, 5.0f * Scale);

		const float ButtonWidth = 72.0f * Scale;
		const float ButtonHeight = 30.0f * Scale;
		const float ButtonX = RowX + InnerWidth - ButtonWidth - 12.0f * Scale;
		const float ButtonY = RowY + 22.0f * Scale;
		DrawRect(Row.bCanClaim ? Theme::AccentGold : Theme::BgPanel, ButtonX, ButtonY, ButtonWidth, ButtonHeight);
		DrawText(Row.ActionLabel.ToString(), Row.bCanClaim ? Theme::BgPrimary : Theme::TextMuted, ButtonX + 19.0f * Scale, ButtonY + 7.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.86f * Scale);
		if (Row.bCanClaim)
		{
			AddHitBox(FVector2D(ButtonX, ButtonY), FVector2D(ButtonWidth, ButtonHeight), MakeQuestClaimHitBoxName(Row.QuestId), true, 90);
		}

		RowY += RowHeight + 10.0f * Scale;
	}
}

void AIdleHUD::ClaimQuestFromHitBox(FName BoxName)
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance)
	{
		return;
	}

	FString QuestId = BoxName.ToString();
	QuestId.RightChopInline(QuestClaimHitBoxPrefix.Len());
	if (QuestId.IsEmpty())
	{
		return;
	}

	const FQuestClaimResult Claim = IdleGameInstance->ClaimQuest(QuestId);
	if (Claim.bSuccess)
	{
		UE_LOG(LogTemp, Display, TEXT("[QuestLog] ClaimQuest success questId=%s gold=%lld exp=%lld"), *QuestId, Claim.RewardGold, Claim.RewardExp);
	}
}

void AIdleHUD::DrawRebirthPanel()
{
	using namespace IdleProject::UI;

	if (!Canvas)
	{
		return;
	}
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance)
	{
		return;
	}

	const FIdleHUDRebirthViewModel ViewModel = BuildRebirthViewModel(
		IdleGameInstance->CanRebirth(),
		IdleGameInstance->HasDefeatedChapter1Boss(),
		IdleGameInstance->GetCharacterLevel(),
		IdleGameInstance->GetRebirthCount(),
		IdleGameInstance->GetRebirthBonusPoints());

	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelWidth = FMath::Clamp(Canvas->SizeX * 0.24f, 320.0f * Scale, 440.0f * Scale);
	const float PanelHeight = 206.0f * Scale;
	const float X = Canvas->SizeX - PanelWidth - 28.0f * Scale;
	const float Y = 304.0f * Scale;
	const float Padding = 16.0f * Scale;
	const float Border = 2.0f * Scale;
	const FLinearColor StateColor = ViewModel.bCanRebirth ? Theme::AccentGold : Theme::TextMuted.CopyWithNewOpacity(0.72f);
	const FLinearColor BossColor = ViewModel.bBossDefeated ? Theme::AccentGold : Theme::AccentRed;
	const FLinearColor LevelColor = ViewModel.bLevelReady ? Theme::AccentBlue : Theme::TextMuted;

	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.91f), X, Y, PanelWidth, PanelHeight);
	DrawRect(StateColor, X, Y, PanelWidth, Border);
	DrawRect(StateColor, X, Y + PanelHeight - Border, PanelWidth, Border);
	DrawRect(StateColor, X, Y, Border, PanelHeight);
	DrawRect(StateColor, X + PanelWidth - Border, Y, Border, PanelHeight);

	DrawText(ViewModel.Title.ToString(), Theme::TextPrimary, X + Padding, Y + 12.0f * Scale, GEngine ? GEngine->GetMediumFont() : nullptr, 0.95f * Scale);
	DrawText(ViewModel.StatusLabel.ToString(), StateColor, X + PanelWidth - 112.0f * Scale, Y + 17.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.80f * Scale);
	DrawText(ViewModel.BossLabel.ToString(), BossColor, X + Padding, Y + 52.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.88f * Scale);
	DrawText(ViewModel.LevelLabel.ToString(), LevelColor, X + 142.0f * Scale, Y + 52.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.88f * Scale);
	DrawText(ViewModel.CountLabel.ToString(), Theme::TextPrimary, X + Padding, Y + 86.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.90f * Scale);
	DrawText(ViewModel.BonusLabel.ToString(), Theme::AccentGold, X + Padding, Y + 116.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.90f * Scale);
	DrawText(ViewModel.NextBonusLabel.ToString(), Theme::TextMuted, X + Padding, Y + 144.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.84f * Scale);

	const float ButtonWidth = 112.0f * Scale;
	const float ButtonHeight = 34.0f * Scale;
	const float ButtonX = X + PanelWidth - Padding - ButtonWidth;
	const float ButtonY = Y + PanelHeight - Padding - ButtonHeight;
	DrawRect(ViewModel.bCanRebirth ? Theme::AccentGold : Theme::BgPrimary.CopyWithNewOpacity(0.94f), ButtonX, ButtonY, ButtonWidth, ButtonHeight);
	DrawText(ViewModel.ButtonLabel.ToString(), ViewModel.bCanRebirth ? Theme::BgPrimary : Theme::TextMuted, ButtonX + 18.0f * Scale, ButtonY + 8.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.84f * Scale);
	if (ViewModel.bCanRebirth)
	{
		AddHitBox(FVector2D(ButtonX, ButtonY), FVector2D(ButtonWidth, ButtonHeight), RebirthHitBoxName, true, 80);
	}
	RefreshMouseInteraction();
}

void AIdleHUD::TryRebirth()
{
	if (!IdleGameInstance)
	{
		IdleGameInstance = GetGameInstance<UIdleGameInstance>();
	}
	if (!IdleGameInstance || !IdleGameInstance->Rebirth())
	{
		return;
	}

	if (AIdleCharacter* IdleCharacter = PlayerOwner ? Cast<AIdleCharacter>(PlayerOwner->GetPawn()) : nullptr)
	{
		IdleCharacter->RefreshDerivedStats();
	}
	RefreshMouseInteraction();
}

void AIdleHUD::RefreshMouseInteraction()
{
	if (!PlayerOwner)
	{
		return;
	}

	const bool bRebirthReady = IdleGameInstance && IdleGameInstance->CanRebirth();
	const bool bNeedsPointer = ResolvePlayerCharacter() || bQuestLogVisible || OfflineRewardModal.bVisible || bRebirthReady;
	PlayerOwner->bShowMouseCursor = bNeedsPointer;
	PlayerOwner->bEnableClickEvents = bNeedsPointer;
}
