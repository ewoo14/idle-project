#pragma once

#include "CoreMinimal.h"
#include "CharacterSystem/StatFormulas.h"
#include "CombatSystem/CombatComponent.h"
#include "GameFramework/HUD.h"
#include "GameCore/DungeonTypes.h"
#include "GameCore/IdleGameInstance.h"
#include "GameCore/MasteryTypes.h"
#include "GameCore/OfflineRewardFormula.h"
#include "GameCore/PetService.h"
#include "GameCore/QuestService.h"
#include "GameCore/SeasonService.h"
#include "GameCore/StageService.h"
#include "ItemSystem/ItemTypes.h"
#include "RuneSystem/RuneCodexTypes.h"
#include "IdleHUD.generated.h"

class UIdleGameInstance;
class UBuffService;
class UDungeonService;
class ULeaderboardService;
class AIdleCharacter;
class AIdleMonster;
class UBattleAIComponent;
class UInventoryComponent;
class UMasteryService;
class URuneService;
class USkillComponent;
class UWeeklyBossService;
class UGuildService;
class SIdleHUDWidget;

struct IDLEPROJECT_API FIdleHUDSkillSlotViewModel
{
	FName SkillId;
	FText DisplayName;
	float CooldownRatio = 0.0f;
	float CooldownRemaining = 0.0f;
	int32 AvailableSkillPoints = 0;
	int32 Rank = 0;
	int32 MaxRank = 0;
	bool bReady = true;
	bool bCanRankUp = false;
};

struct IDLEPROJECT_API FIdleHUDUltimateViewModel
{
	float GaugeRatio = 0.0f;
	float GaugePercent = 0.0f;
	bool bReady = false;
};

struct IDLEPROJECT_API FIdleHUDOfflineRewardViewModel
{
	bool bVisible = false;
	FText Title;
	FText ElapsedLabel;
	FText GoldLabel;
	FText ExpLabel;
	FText ClaimLabel;
};

struct IDLEPROJECT_API FIdleHUDCloudSyncViewModel
{
	ECloudSyncState State = ECloudSyncState::Idle;
	FText Label;
	bool bVisible = false;
	bool bError = false;
	bool bTransient = false;
};

struct IDLEPROJECT_API FIdleHUDQuestLogRowViewModel
{
	FString QuestId;
	FText TypeLabel;
	FText Title;
	FText ProgressLabel;
	FText RewardLabel;
	FText ActionLabel;
	float ProgressRatio = 0.0f;
	bool bCanClaim = false;
	bool bClaimed = false;
};

struct IDLEPROJECT_API FIdleHUDQuestLogSectionViewModel
{
	EQuestType Type = EQuestType::Main;
	FText TypeLabel;
	TArray<FIdleHUDQuestLogRowViewModel> Rows;
};

struct IDLEPROJECT_API FIdleHUDQuestLogViewModel
{
	FText Title;
	FText ShortcutLabel;
	FText EmptyLabel;
	TArray<FIdleHUDQuestLogRowViewModel> Rows;
	TArray<FIdleHUDQuestLogSectionViewModel> Sections;
};

struct IDLEPROJECT_API FIdleHUDRebirthViewModel
{
	FText Title;
	FText StatusLabel;
	FText BossLabel;
	FText LevelLabel;
	FText CountLabel;
	FText BonusLabel;
	FText NextBonusLabel;
	FText ButtonLabel;
	bool bCanRebirth = false;
	bool bBossDefeated = false;
	bool bLevelReady = false;
};

struct IDLEPROJECT_API FIdleHUDTranscendViewModel
{
	FText Title;
	FText StatusLabel;
	FText RequirementLabel;
	FText CurrentMultiplierLabel;
	FText PreviewMultiplierLabel;
	FText CountLabel;
	FText ButtonLabel;
	bool bCanTranscend = false;
	bool bThresholdReady = false;
};

struct IDLEPROJECT_API FIdleHUDClassSelectionOptionViewModel
{
	EClassId ClassId = EClassId::Warrior;
	FText DisplayName;
	FText RoleLabel;
	FText StatSummary;
	bool bSelected = false;
};

struct IDLEPROJECT_API FIdleHUDPetRowViewModel
{
	FString PetId;
	FText Name;
	FText BonusLabel;
	FText LevelLabel;
	FText FeedCostLabel;
	FText ActionLabel;
	FText FeedActionLabel;
	FText StatusLabel;
	FText StarLabel;
	FText EvolveActionLabel;
	FText EvolveCostLabel;
	FText EvolveEffectLabel;
	bool bEquipped = false;
	bool bCanEquip = false;
	bool bCanFeed = false;
	bool bFeedDisabled = true;
	bool bMaxLevel = false;
	bool bCanEvolve = false;
};

struct IDLEPROJECT_API FIdleHUDPetPanelViewModel
{
	FText Title;
	FText EquippedLabel;
	FText GoldBonusLabel;
	FText DropBonusLabel;
	TArray<FIdleHUDPetRowViewModel> Rows;
};

struct IDLEPROJECT_API FIdleHUDSeasonTierRowViewModel
{
	int32 Tier = 0;
	FText TierLabel;
	FText RequirementLabel;
	FText RewardLabel;
	FText ActionLabel;
	float ProgressRatio = 0.0f;
	bool bReached = false;
	bool bClaimed = false;
	bool bCanClaim = false;
};

struct IDLEPROJECT_API FIdleHUDSeasonPassViewModel
{
	FText Title;
	FText TokenLabel;
	FText ProgressLabel;
	float ProgressRatio = 0.0f;
	TArray<FIdleHUDSeasonTierRowViewModel> Rows;
};

struct IDLEPROJECT_API FIdleHUDFloatingDamageEntry
{
	FVector WorldLocation = FVector::ZeroVector;
	float Amount = 0.0f;
	bool bWasCrit = false;
	EDamageKind Kind = EDamageKind::Physical;
	float StartTime = 0.0f;
};

struct IDLEPROJECT_API FIdleHUDFloatingDamageViewModel
{
	bool bVisible = false;
	FString Label;
	FVector2D ScreenPosition = FVector2D::ZeroVector;
	FLinearColor Color = FLinearColor::White;
	float TextScale = 1.0f;
};

struct IDLEPROJECT_API FIdleHUDStatusIndicatorViewModel
{
	ESkillStatusEffect Type = ESkillStatusEffect::None;
	FString Label;
	FLinearColor Color = FLinearColor::White;
	float RemainingSeconds = 0.0f;
	float Size = 0.0f;
};

struct IDLEPROJECT_API FIdleHUDStageViewModel
{
	FText TitleLabel;
	FText ChapterLabel;
	FText ProgressLabel;
	FText BossBadgeLabel;
	FText EliteBadgeLabel;
	FText WeaknessLabel;
	FText WeaknessIconLabel;
	FLinearColor BorderColor = FLinearColor::White;
	FLinearColor WeaknessColor = FLinearColor::White;
	float ProgressRatio = 0.0f;
	bool bBossStage = false;
	bool bEliteStage = false;
};

struct IDLEPROJECT_API FIdleHUDBossViewModel
{
	bool bVisible = false;
	FText TitleLabel;
	FText HpLabel;
	FText PhaseLabel;
	FLinearColor PhaseColor = FLinearColor::White;
	float HpRatio = 0.0f;
	float HpPercent = 0.0f;
	int32 Phase = 1;
};

struct IDLEPROJECT_API FIdleHUDEnhanceSlotViewModel
{
	EItemSlot Slot = EItemSlot::None;
	FText SlotLabel;
	FText ItemName;
	FText RarityLabel;
	FLinearColor RarityColor = FLinearColor::White;
	FText LevelLabel;
	FText CostLabel;
	FText SuccessRateLabel;
	FText RiskLabel;
	FText FailStreakLabel;
	FText ProtectionButtonLabel;
	FText StatusLabel;
	FText ButtonLabel;
	int32 EnhanceLevel = INDEX_NONE;
	int32 FailStreak = 0;
	int64 Cost = 0;
	float SuccessRate = 0.0f;
	bool bEquipped = false;
	bool bCanEnhance = false;
	bool bCanUseProtection = false;
	bool bRiskLevel = false;
	bool bMaxLevel = false;
	bool bGoldEnough = false;
};

struct IDLEPROJECT_API FIdleHUDEnhancePanelViewModel
{
	FText Title;
	FText GoldLabel;
	FText ProtectionLabel;
	FText FeedbackLabel;
	bool bFeedbackSuccess = false;
	TArray<FIdleHUDEnhanceSlotViewModel> Rows;
};

struct IDLEPROJECT_API FIdleHUDPotentialSlotViewModel
{
	EItemSlot Slot = EItemSlot::None;
	FText SlotLabel;
	FText ItemName;
	FText GradeLabel;
	FText LineSummaryLabel;
	FText ResetActionLabel;
	FText RankActionLabel;
	FText LockActionLabel;
	FText StatusLabel;
	FName ResetHitBoxName;
	FName RankHitBoxName;
	FName LockHitBoxName;
	FLinearColor GradeColor = FLinearColor::White;
	EPotentialGrade Grade = EPotentialGrade::None;
	EPotentialGrade MaxGrade = EPotentialGrade::None;
	bool bEquipped = false;
	bool bLocked = false;
	bool bHasPotential = false;
	bool bCanResetPotential = false;
	bool bCanRankPotential = false;
};

struct IDLEPROJECT_API FIdleHUDPotentialPanelViewModel
{
	FText Title;
	FText ResetCubeLabel;
	FText RankCubeLabel;
	TArray<FIdleHUDPotentialSlotViewModel> Rows;
};

struct IDLEPROJECT_API FIdleHUDShopPanelViewModel
{
	FText Title;
	FText GoldLabel;
	FText CostLabel;
	FText ProtectionScrollCostLabel;
	FText ResetCubeCostLabel;
	FText RankCubeCostLabel;
	FText ButtonLabel;
	FText ProtectionScrollButtonLabel;
	FText ResetCubeButtonLabel;
	FText RankCubeButtonLabel;
	FText StatusLabel;
	FText LastResultLabel;
	FName GearRollHitBoxName;
	FName ProtectionScrollHitBoxName;
	FName ResetCubeHitBoxName;
	FName RankCubeHitBoxName;
	EItemRarity LastResultRarity = EItemRarity::None;
	int64 GearRollCost = 0;
	int64 ProtectionScrollCost = 0;
	int64 ResetCubeCost = 0;
	int64 RankCubeCost = 0;
	int64 Gold = 0;
	bool bCanBuyGearRoll = false;
	bool bCanBuyProtectionScroll = false;
	bool bCanBuyResetCube = false;
	bool bCanBuyRankCube = false;
	bool bHasLastResult = false;
	bool bLastResultError = false;
};

struct IDLEPROJECT_API FIdleHUDConsumableRowViewModel
{
	EConsumableType Type = EConsumableType::AttackTonic;
	EConsumableGrade Grade = EConsumableGrade::Standard;
	FText NameLabel;
	FText GradeLabel;
	FText EffectLabel;
	FText CountLabel;
	FText ActionLabel;
	FText RemainingLabel;
	FName UseHitBoxName;
	int32 Count = 0;
	int64 RemainingSec = 0;
	bool bCanUse = false;
	bool bActive = false;
};

struct IDLEPROJECT_API FIdleHUDConsumablePanelViewModel
{
	FText Title;
	FText ActiveBuffTitle;
	FText EmptyActiveBuffLabel;
	TArray<FIdleHUDConsumableRowViewModel> Rows;
	TArray<FIdleHUDConsumableRowViewModel> ActiveBuffRows;
};

struct IDLEPROJECT_API FIdleHUDLeaderboardRowViewModel
{
	FString CharacterId;
	FText CharacterLabel;
	FText RankLabel;
	FText ScoreLabel;
	bool bSelf = false;
};

struct IDLEPROJECT_API FIdleHUDLeaderboardPanelViewModel
{
	FText Title;
	FText SeasonLabel;
	FText PowerTabLabel;
	FText RebirthTabLabel;
	FText WeeklyTabLabel;
	FText WeekLabel;
	FText MyRankTitle;
	FText EmptyLabel;
	FText OfflineLabel;
	FText LoadingLabel;
	FText RefreshLabel;
	FIdleHUDLeaderboardRowViewModel MyEntry;
	TArray<FIdleHUDLeaderboardRowViewModel> Rows;
	ELeaderboardKind ActiveKind = ELeaderboardKind::Power;
	bool bLoading = false;
	bool bOffline = false;
};

struct IDLEPROJECT_API FIdleHUDWeeklyBossMilestoneRowViewModel
{
	int32 Milestone = 0;
	FText MilestoneLabel;
	FText ThresholdLabel;
	FText RewardLabel;
	FText StatusLabel;
	FText ActionLabel;
	FName ClaimHitBoxName;
	int64 Threshold = 0;
	int64 GoldReward = 0;
	int64 EssenceReward = 0;
	bool bReached = false;
	bool bClaimed = false;
	bool bCanClaim = false;
};

struct IDLEPROJECT_API FIdleHUDWeeklyBossPanelViewModel
{
	FText Title;
	FText WeekLabel;
	FText DamageLabel;
	FText RemainingLabel;
	FText MilestoneSummaryLabel;
	FText ResetLabel;
	FText ChallengeLabel;
	FName ChallengeHitBoxName;
	int64 Damage = 0;
	int64 NextMilestoneThreshold = 0;
	int32 RemainingChallenges = 0;
	int32 ReachedMilestones = 0;
	int32 ClaimedMilestones = 0;
	float ProgressRatio = 0.0f;
	bool bCanChallenge = false;
	TArray<FIdleHUDWeeklyBossMilestoneRowViewModel> Rows;
};

// ── 길드 패널 뷰모델(PR-G1) ──────────────────────────────────────────────────
/** 무소속 화면의 길드 목록 1행. */
struct IDLEPROJECT_API FIdleHUDGuildListRowViewModel
{
	FString GuildId;
	FText NameLabel;
	FText InfoLabel;     // Lv n · m/30
	FText JoinLabel;     // 가입/신청(가입 모드별)
	FName JoinHitBoxName;
	bool bApproval = false;
};

/** 내 길드 멤버 1행(닉네임 + 계급 배지 + 승급/강등 액션). */
struct IDLEPROJECT_API FIdleHUDGuildMemberRowViewModel
{
	FString CharacterId;
	FText NicknameLabel;
	FText RankBadgeLabel;
	EGuildRank Rank = EGuildRank::Member;
	bool bSelf = false;
	// 길드장 관리(승급/강등) — 미해금/정원 초과 시 비활성.
	bool bShowPromote = false;
	bool bCanPromote = false;
	FText PromoteLabel;        // 승급 / 인원 부족 / 정원 초과
	EGuildRank PromoteTargetRank = EGuildRank::Member;
	FName PromoteHitBoxName;
	bool bShowDemote = false;
	FName DemoteHitBoxName;
};

/** 길드 상점 1행(이름·가격·구매 버튼, PR-G2). 포인트 부족 시 비활성. */
struct IDLEPROJECT_API FIdleHUDGuildShopRowViewModel
{
	FString ItemId;
	FText NameLabel;
	FText PriceLabel;        // {Price} P
	bool bCanBuy = false;    // 포인트 충분 + 액션 가능
	FText BuyLabel;          // 구매 / 포인트 부족
	FName BuyHitBoxName;
};

/** 길드 가입 신청 1행(길드장/부 승인 큐). */
struct IDLEPROJECT_API FIdleHUDGuildRequestRowViewModel
{
	FString CharacterId;
	FText CharacterLabel;
	FName ApproveHitBoxName;
	FName RejectHitBoxName;
};

/** 주간 길드 랭킹 1행(PR-G3, 서버 GET /rankings). bSelf 면 내 길드 강조. */
struct IDLEPROJECT_API FIdleHUDGuildRankingRowViewModel
{
	FString GuildId;
	FText RankLabel;          // #{Rank}
	FText NameLabel;          // 길드명
	FText InfoLabel;          // Lv {Level}
	FText ContributionLabel;  // 주간 기여 {Weekly}
	bool bSelf = false;       // 내 길드 여부(강조)
};

/** 길드 패널 전체 뷰모델 — bHasGuild 로 무소속/내 길드 화면 분기. */
struct IDLEPROJECT_API FIdleHUDGuildPanelViewModel
{
	// 공통
	FText Title;
	FText RefreshLabel;
	FText StateLabel;        // 로딩/오프라인(비면 표시 안 함)
	bool bOffline = false;
	bool bLoading = false;
	bool bHasGuild = false;

	// 무소속 화면
	FText NoneTitle;
	FText ListEmptyLabel;
	TArray<FIdleHUDGuildListRowViewModel> ListRows;
	FText CreateTitle;
	FText CreateNameLabel;     // 이름: {Name}
	FText CreateNameCycleLabel;
	FText CreateLabel;
	FName CreateNameCycleHitBoxName;
	FName CreateHitBoxName;

	// 내 길드 화면
	FText MyTitle;
	FText GuildNameLabel;
	FText SummaryLabel;        // Lv n · m/30 · 가입모드
	FText MyRankBadgeLabel;
	FText LeaveLabel;
	FName LeaveHitBoxName;

	// 길드 레벨/EXP/버프/기여(PR-G2)
	FText LevelLabel;          // 길드 Lv {Level}
	FText ExpLabel;            // EXP {Into}/{Span} (다음 레벨까지)
	float ExpProgressRatio = 0.0f;
	FText BuffLabel;           // 공격 +X% · 골드 +Y%
	FText ContributionLabel;   // 내 기여 {Points} P
	FText WeeklyContributionLabel; // 주간 기여 {Weekly}

	// 출석/헌납 액션(PR-G2)
	FText AttendLabel;         // 출석 / 출석 완료
	bool bCanAttend = false;
	FName AttendHitBoxName;
	FText DonateLabel;         // 헌납 {Amount}
	bool bCanDonate = false;
	FName DonateHitBoxName;
	FText DonateCycleLabel;    // 금액 변경
	FName DonateCycleHitBoxName;

	// 길드 상점(PR-G2)
	FText ShopTitle;
	FText ShopEmptyLabel;
	TArray<FIdleHUDGuildShopRowViewModel> ShopRows;

	// ── 길드 보스(PR-G3, 공유 HP 풀 — 서버 권위) ──
	FText BossTitle;
	FText BossHpLabel;             // 누적 {Accum} / {Hp}
	float BossHpRatio = 0.0f;      // accum / hp (0~1)
	FText BossDefeatedLabel;       // 격파 {Count}회
	FText BossChallengeLabel;      // 도전 (잔여 {Remaining})
	bool bCanChallengeBoss = false;
	FText BossClaimLabel;          // 보상 수령 {Count}
	bool bCanClaimBoss = false;

	// ── 주간 길드 랭킹 탭(PR-G3) ──
	bool bRankingsView = false;    // 랭킹 탭 활성 여부(내 길드 화면 한정)
	FText MyTabLabel;              // 내 길드
	FText RankingsTabLabel;        // 주간 랭킹
	FText RankingsTitle;
	FText RankingsEmptyLabel;
	FText RankingsLoadingLabel;
	FText MyRankingTitle;          // 내 길드 순위
	FText MyRankingLabel;          // #{Rank} · 주간 {Weekly} (순위권 밖이면 -)
	TArray<FIdleHUDGuildRankingRowViewModel> RankingRows;

	FText MemberListTitle;
	TArray<FIdleHUDGuildMemberRowViewModel> MemberRows;

	// 길드장 관리(내 rank=Master 일 때만 노출)
	bool bShowManage = false;
	FText ManageTitle;
	FText ToggleJoinModeLabel;
	FName ToggleJoinModeHitBoxName;
	FText RequestsTitle;
	FText RequestsEmptyLabel;
	TArray<FIdleHUDGuildRequestRowViewModel> RequestRows;
};

struct IDLEPROJECT_API FIdleHUDRuneSlotViewModel
{
	int32 SlotIndex = INDEX_NONE;
	int32 OwnedIndex = INDEX_NONE;
	FText SlotLabel;
	FText TypeLabel;
	FText RarityLabel;
	FText LevelLabel;
	FText ValueLabel;
	FText StatusLabel;
	FText ActionLabel;
	FLinearColor RarityColor = FLinearColor::White;
	FName ActionHitBoxName;
	bool bEquipped = false;
	bool bCanEquipSelected = false;
	bool bCanUnequip = false;
};

struct IDLEPROJECT_API FIdleHUDRuneOwnedRowViewModel
{
	int32 OwnedIndex = INDEX_NONE;
	ERuneType RuneType = ERuneType::None;
	EItemRarity Rarity = EItemRarity::None;
	FText TypeLabel;
	FText RarityLabel;
	FText LevelLabel;
	FText ValueLabel;
	FText EnhanceCostLabel;
	FText EnhancePreviewLabel;
	FText DisenchantLabel;
	FText SelectActionLabel;
	FText EnhanceActionLabel;
	FText DisenchantActionLabel;
	FLinearColor RarityColor = FLinearColor::White;
	FName SelectHitBoxName;
	FName EnhanceHitBoxName;
	FName DisenchantHitBoxName;
	int64 EnhanceEssenceCost = 0;
	int64 EnhanceGoldCost = 0;
	int64 DisenchantEssence = 0;
	bool bEquipped = false;
	bool bSelected = false;
	bool bCanEnhance = false;
	bool bCanDisenchant = false;
};

struct IDLEPROJECT_API FIdleHUDRuneActionViewModel
{
	int32 SourceOwnedIndex = INDEX_NONE;
	int32 TransferTargetOwnedIndex = INDEX_NONE;
	bool bHasSelection = false;
	bool bIsMythic = false;

	FText TitleLabel;
	FText CurrentSetLabel;
	FText RerollCostLabel;
	FText RerollActionLabel;
	FText UpgradeInfoLabel;
	FText UpgradeActionLabel;
	FText TransferTargetLabel;
	FText TransferCostLabel;
	FText TransferActionLabel;
	FText TransferCycleLabel;
	FText EmptyLabel;

	FName RerollHitBoxName;
	FName UpgradeHitBoxName;
	FName TransferHitBoxName;
	FName TransferCycleHitBoxName;

	int64 RerollEssenceCost = 0;
	int64 UpgradeEssenceCost = 0;
	int64 UpgradeGoldCost = 0;
	int64 TransferEssenceCost = 0;
	float UpgradeChance = 0.0f;

	bool bCanReroll = false;
	bool bCanUpgrade = false;
	bool bCanTransfer = false;
	bool bCanCycleTarget = false;
};

struct IDLEPROJECT_API FIdleHUDRuneSetRowViewModel
{
	ERuneSet RuneSet = ERuneSet::None;
	FText SetLabel;
	FText CountLabel;
	FText TierLabel;
	FText BonusLabel;
	FText NextTierLabel;
	int32 Count = 0;
	bool bActive = false;
	bool bTwoSetActive = false;
	bool bFourSetActive = false;
	bool bSixSetActive = false;
};

struct IDLEPROJECT_API FIdleHUDRuneViewModel
{
	FText Title;
	FText EssenceLabel;
	FText GoldLabel;
	FText ShopCostLabel;
	FText ShopButtonLabel;
	FText ShopStatusLabel;
	FText EmptyOwnedLabel;
	FText ClassCraftCostLabel;
	FText ClassCraftButtonLabel;
	FName ShopHitBoxName;
	FName ClassCraftHitBoxName;
	int32 SelectedOwnedIndex = INDEX_NONE;
	int64 RuneEssence = 0;
	int64 Gold = 0;
	int64 ShopCost = 0;
	int64 ClassCraftCost = 0;
	bool bCanBuyRuneRoll = false;
	bool bCanCraftClassRune = false;
	TArray<FIdleHUDRuneSlotViewModel> Slots;
	TArray<FIdleHUDRuneOwnedRowViewModel> OwnedRows;
	FText SetTitle;
	TArray<FIdleHUDRuneSetRowViewModel> SetRows;
	FIdleHUDRuneActionViewModel Action;
};

struct IDLEPROJECT_API FIdleHUDRuneCodexCellViewModel
{
	ERuneType RuneType = ERuneType::None;
	EItemRarity Rarity = EItemRarity::None;
	FText TypeLabel;
	FText RarityLabel;
	FText StatusLabel;
	FLinearColor AccentColor = FLinearColor::White;
	int32 RowIndex = INDEX_NONE;
	int32 ColumnIndex = INDEX_NONE;
	bool bUnlocked = false;
	bool bCoreType = false;
};

struct IDLEPROJECT_API FIdleHUDRuneCodexRowViewModel
{
	EItemRarity Rarity = EItemRarity::None;
	FText RarityLabel;
	FText BonusLabel;
	FLinearColor AccentColor = FLinearColor::White;
	int32 RowIndex = INDEX_NONE;
	int32 UnlockedCount = 0;
	bool bComplete = false;
};

struct IDLEPROJECT_API FIdleHUDRuneCodexViewModel
{
	FText Title;
	FText ProgressLabel;
	FText CoreCategoryLabel;
	FText UtilCategoryLabel;
	FText CoreBonusLabel;
	FText UtilCapLabel;
	int32 UnlockedCells = 0;
	int32 TotalCells = 63;
	int32 CoreUnlockedCells = 0;
	int32 UtilUnlockedCells = 0;
	float ProgressRatio = 0.0f;
	bool bCoreCategoryComplete = false;
	bool bUtilCategoryComplete = false;
	TArray<FText> ColumnLabels;
	TArray<FIdleHUDRuneCodexRowViewModel> Rows;
	TArray<FIdleHUDRuneCodexCellViewModel> Cells;
};

struct IDLEPROJECT_API FIdleHUDStatRowViewModel
{
	EPrimaryStat Stat = EPrimaryStat::Str;
	FText StatLabel;
	FText ValueLabel;
	FName AllocationHitBoxName;
	int32 BaseValue = 0;
	int32 AllocatedValue = 0;
	int32 TotalValue = 0;
	bool bCanAllocate = false;
};

struct IDLEPROJECT_API FIdleHUDStatPanelViewModel
{
	FText Title;
	FText AvailableLabel;
	FText ResetLabel;
	FName ResetHitBoxName;
	int32 AvailablePoints = 0;
	bool bCanReset = false;
	TArray<FIdleHUDStatRowViewModel> Rows;
};

struct IDLEPROJECT_API FIdleHUDStatInfoRowViewModel
{
	FText StatLabel;
	FText ValueLabel;
};

struct IDLEPROJECT_API FIdleHUDStatInfoViewModel
{
	FText Title;
	FText HeaderLabel;
	FText CombatPowerLabel;
	FText ToggleLabel;
	FName ToggleHitBoxName;
	TArray<FIdleHUDStatInfoRowViewModel> PrimaryRows;
	TArray<FIdleHUDStatInfoRowViewModel> DerivedRows;
};

struct IDLEPROJECT_API FIdleHUDSetSummaryRowViewModel
{
	EItemSet ItemSet = EItemSet::None;
	FText SetLabel;
	FText TierLabel;
	FText SummaryLabel;
	FText BonusLabel;
	FText NextTierLabel;
	int32 PieceCount = 0;
	bool bTwoPieceActive = false;
	bool bFourPieceActive = false;
};

struct IDLEPROJECT_API FIdleHUDSetSummaryViewModel
{
	TArray<FIdleHUDSetSummaryRowViewModel> Rows;
};

struct IDLEPROJECT_API FIdleHUDTowerViewModel
{
	FText Title;
	FText HighestFloorLabel;
	FText NextRequiredPowerLabel;
	FText CombatPowerLabel;
	FText MilestoneMultiplierLabel;
	FText NextMilestoneLabel;
	FText StatusLabel;
	FText ButtonLabel;
	FName ClimbHitBoxName;
	int32 HighestFloor = 0;
	int32 NextMilestoneFloor = 0;
	int64 NextRequiredPower = 0;
	int64 CombatPower = 0;
	float MilestoneMultiplier = 1.0f;
	bool bCanClimb = false;
};

struct IDLEPROJECT_API FIdleHUDDungeonRowViewModel
{
	EDungeonType Type = EDungeonType::None;
	FText NameLabel;
	FText EntriesLabel;
	FText TierLabel;
	FText RequiredPowerLabel;
	FText NextTierLabel;
	FText RewardLabel;
	FText StatusLabel;
	FText ActionLabel;
	FName EnterHitBoxName;
	int32 SelectedTier = 1;
	int32 MaxAccessibleTier = 0;
	int32 RemainingEntries = 0;
	int32 EntryLimit = 0;
	int64 RequiredPower = 0;
	int64 NextTierRequirement = 0;
	int64 CombatPower = 0;
	bool bCanEnter = false;
	bool bSoldOut = false;
	bool bNeedsPower = false;
};

struct IDLEPROJECT_API FIdleHUDDungeonPanelViewModel
{
	FText Title;
	TArray<FIdleHUDDungeonRowViewModel> Rows;
};

struct IDLEPROJECT_API FIdleHUDAchievementRowViewModel
{
	EAchievementCategory Category = EAchievementCategory::Misc;
	FText CategoryLabel;
	FText TierLabel;
	FText PointsLabel;
	FText ValueLabel;
	FText NextThresholdLabel;
	int32 Tier = 0;
	int32 Points = 0;
	int64 CurrentValue = 0;
	int64 NextThreshold = 0;
	float ProgressRatio = 0.0f;
};

struct IDLEPROJECT_API FIdleHUDAchievementViewModel
{
	FText Title;
	FText TotalPointsLabel;
	FText StatMultiplierLabel;
	int32 TotalPoints = 0;
	float StatMultiplier = 1.0f;
	TArray<FIdleHUDAchievementRowViewModel> Rows;
};

struct IDLEPROJECT_API FIdleHUDMasteryTrackRowViewModel
{
	EMasteryTrack Track = EMasteryTrack::Combat;
	FText TrackLabel;
	FText LevelLabel;
	FText XpLabel;
	FText ProgressLabel;
	FText LocalBonusLabel;
	FText LocalBonus2Label;
	FText TooltipLabel;
	float ProgressRatio = 0.0f;
	int32 LocalBonusPercent = 0;
	// 2종 보너스 표시값. 심연은 던전 입장 +N(정수), 그 외 트랙은 % 정수.
	int32 LocalBonus2Percent = 0;
	int32 LocalBonus2AbyssEntries = 0;
};

struct IDLEPROJECT_API FIdleHUDMasteryPanelViewModel
{
	FText Title;
	FText WorldPowerLabel;
	TArray<FIdleHUDMasteryTrackRowViewModel> Rows;
};

// 칭호 패널 행 뷰모델. 해금 칭호는 장착/해제 버튼, 미해금은 해금 조건/진행을 표시한다.
struct IDLEPROJECT_API FIdleHUDTitleRowViewModel
{
	FString TitleId;
	FText Name;
	FText BonusLabel;
	FText StatusLabel;
	FText ConditionLabel;
	FText ProgressLabel;
	FText ActionLabel;
	float ProgressRatio = 0.0f;
	bool bUnlocked = false;
	bool bEquipped = false;
	// 장착 가능(해금 && 미장착) — 장착 버튼 활성.
	bool bCanEquip = false;
};

struct IDLEPROJECT_API FIdleHUDTitlePanelViewModel
{
	FText Title;
	FText EquippedLabel;
	TArray<FIdleHUDTitleRowViewModel> Rows;
};

// 미션 패널 행 뷰모델. 미션별 목표 설명·진행 바·보상·수령 버튼 상태를 담는다.
struct IDLEPROJECT_API FIdleHUDMissionRowViewModel
{
	FString MissionId;
	FText ObjectiveLabel;   // 메트릭 표시명 + 목표치(예: "몬스터 처치 100")
	FText ProgressLabel;    // 현재/목표(예: "40 / 100")
	FText RewardLabel;      // 보상 타입 + 값(예: "골드 +5,000")
	FText ActionLabel;      // 수령/수령완료
	float ProgressRatio = 0.0f;
	bool bCompleted = false;
	bool bClaimed = false;
	// 수령 가능(완료 && 미수령) — 수령 버튼 활성.
	bool bCanClaim = false;
};

// 미션 패널 뷰모델. 선택된 기간(일일/주간) 탭에 해당하는 미션 행만 담는다.
struct IDLEPROJECT_API FIdleHUDMissionPanelViewModel
{
	FText Title;
	FText DailyTabLabel;
	FText WeeklyTabLabel;
	FText EmptyLabel;
	EMissionPeriod ActivePeriod = EMissionPeriod::Daily;
	TArray<FIdleHUDMissionRowViewModel> Rows;
};

namespace IdleProject::UI
{
IDLEPROJECT_API FText RarityToLabel(EItemRarity Rarity);
IDLEPROJECT_API FLinearColor RarityToColor(EItemRarity Rarity);
IDLEPROJECT_API FText BuildAffixSummary(const FItemInstance& Item);
IDLEPROJECT_API FText BuildUniqueTraitSummary(const FItemInstance& Item);
IDLEPROJECT_API FIdleHUDSetSummaryViewModel BuildSetSummaryViewModel(const TArray<FItemInstance>& EquippedItems);
IDLEPROJECT_API TArray<FIdleHUDSkillSlotViewModel> BuildSkillSlotViewModels(const USkillComponent& SkillComponent, float Now);
IDLEPROJECT_API FIdleHUDUltimateViewModel BuildUltimateViewModel(const USkillComponent& SkillComponent);
IDLEPROJECT_API FIdleHUDStageViewModel BuildStageViewModel(const FStageInfo& StageInfo);
IDLEPROJECT_API FText BuildChapterEntryFeedbackLabel(int32 Chapter);
IDLEPROJECT_API FText BuildChapterClearFeedbackLabel(int32 Chapter);
IDLEPROJECT_API FIdleHUDOfflineRewardViewModel BuildOfflineRewardViewModel(const FOfflineRewardResult& Reward);
IDLEPROJECT_API FIdleHUDQuestLogViewModel BuildQuestLogViewModel(const TArray<FQuestState>& QuestStates);
IDLEPROJECT_API FIdleHUDRebirthViewModel BuildRebirthViewModel(bool bCanRebirth, bool bBossDefeated, int32 CharacterLevel, int32 RebirthCount, int32 RebirthBonusPoints, int32 PreviewRebirthReward);
IDLEPROJECT_API FIdleHUDTranscendViewModel BuildTranscendViewModel(bool bCanTranscend, int32 RebirthCount, int32 Threshold, int32 TranscendCount, float CurrentMultiplier, float PreviewMultiplier);
IDLEPROJECT_API TArray<FIdleHUDClassSelectionOptionViewModel> BuildClassSelectionOptions(EClassId CurrentClassId);
IDLEPROJECT_API FIdleHUDPetPanelViewModel BuildPetPanelViewModel(const TArray<FPetDefinition>& PetDefinitions, const FString& EquippedPetId, float GoldBonusPercent, float DropBonusPercent, int64 Gold, TFunctionRef<int32(const FString&)> GetPetLevel, TFunctionRef<bool(const FString&)> IsPetOwned, TFunctionRef<int32(const FString&)> GetPetStar);
IDLEPROJECT_API FIdleHUDSeasonPassViewModel BuildSeasonPassViewModel(const TArray<FSeasonTierDefinition>& Tiers, int32 SeasonTokens, int32 ReachedTier, TFunctionRef<bool(int32)> IsTierClaimed);
IDLEPROJECT_API FIdleHUDBossViewModel BuildBossViewModel(float CurrentHp, float MaxHp);
IDLEPROJECT_API FIdleHUDFloatingDamageViewModel BuildFloatingDamageViewModel(const FIdleHUDFloatingDamageEntry& Entry, float Now, FVector2D ProjectedScreenPosition, float HudScale);
IDLEPROJECT_API TArray<FIdleHUDStatusIndicatorViewModel> BuildStatusIndicatorViewModels(const TArray<FActiveSkillStatus>& Statuses, float Now, float HudScale);
IDLEPROJECT_API FIdleHUDEnhancePanelViewModel BuildEnhancePanelViewModel(const UInventoryComponent& Inventory, int64 Gold, FText FeedbackLabel, bool bFeedbackSuccess);
IDLEPROJECT_API FIdleHUDEnhancePanelViewModel BuildEnhancePanelViewModel(const UInventoryComponent& Inventory, int64 Gold, int64 ProtectionScrolls, FText FeedbackLabel, bool bFeedbackSuccess);
IDLEPROJECT_API FIdleHUDPotentialPanelViewModel BuildPotentialPanelViewModel(const UInventoryComponent& Inventory, int64 ResetCubes, int64 RankCubes);
IDLEPROJECT_API FIdleHUDShopPanelViewModel BuildShopPanelViewModel(int64 GearRollCost, int64 ProtectionScrollCost, int64 ResetCubeCost, int64 RankCubeCost, int64 Gold, const FShopPurchaseResult& LastResult);
IDLEPROJECT_API FIdleHUDConsumablePanelViewModel BuildConsumablePanelViewModel(const UBuffService& BuffService, int64 NowUnixSec);
IDLEPROJECT_API FIdleHUDLeaderboardPanelViewModel BuildLeaderboardPanelViewModel(const ULeaderboardService& LeaderboardService, ELeaderboardKind Kind, int32 SeasonId, const FString& WeekId, bool bLoading, bool bOffline);
IDLEPROJECT_API FIdleHUDWeeklyBossPanelViewModel BuildWeeklyBossPanelViewModel(const UWeeklyBossService& WeeklyBossService);
IDLEPROJECT_API FIdleHUDGuildPanelViewModel BuildGuildPanelViewModel(const UGuildService& GuildService, const TArray<FGuildSummary>& BrowseList, const FString& PendingCreateName, bool bLoading, bool bOffline, int64 PlayerGold, int64 DonateAmount, const TArray<FGuildShopItemInfo>& ShopItems, bool bRankingsView, bool bRankingsLoading, const TArray<FGuildRankingEntry>& Rankings, const FGuildRankingEntry& MyRanking);
IDLEPROJECT_API FIdleHUDRuneViewModel BuildRuneViewModel(const URuneService& RuneService, int64 RuneEssence, int64 Gold, int32 ProgressIndex, int32 SelectedOwnedIndex, int32 TransferTargetOwnedIndex);
IDLEPROJECT_API FIdleHUDRuneCodexViewModel BuildRuneCodexViewModel(const URuneService& RuneService);
IDLEPROJECT_API FIdleHUDStatPanelViewModel BuildStatPanelViewModel(const FPrimaryStats& BaseStats, const FPrimaryStats& AllocatedStats, int32 AvailablePoints);
IDLEPROJECT_API FIdleHUDStatInfoViewModel BuildStatInfoViewModel(const FPrimaryStats& PrimaryStats, const FDerivedStats& DerivedStats, int32 Level, EClassId ClassId, int32 RebirthCount, int64 CombatPower);
IDLEPROJECT_API FIdleHUDTowerViewModel BuildTowerViewModel(int32 HighestFloor, int64 NextRequiredPower, int64 CombatPower, float MilestoneMultiplier = -1.0f);
IDLEPROJECT_API FText BuildTowerClimbFeedbackLabel(int32 NewHighestFloor, int64 TotalReward);
IDLEPROJECT_API FIdleHUDDungeonPanelViewModel BuildDungeonPanelViewModel(const UDungeonService& DungeonService, int64 CombatPower, const FString& TodayUtc);
IDLEPROJECT_API FIdleHUDAchievementViewModel BuildAchievementViewModel(const UAchievementService& AchievementService);
IDLEPROJECT_API FIdleHUDMasteryPanelViewModel BuildMasteryPanelViewModel(const UMasteryService& MasteryService);
IDLEPROJECT_API FIdleHUDTitlePanelViewModel BuildTitlePanelViewModel(const UTitleService& TitleService, const UAchievementService& AchievementService);
IDLEPROJECT_API FIdleHUDMissionPanelViewModel BuildMissionPanelViewModel(const UMissionService& MissionService, EMissionPeriod ActivePeriod);
IDLEPROJECT_API FText BuildAchievementUnlockedFeedbackLabel(const FString& AchievementId, int32 Tier);
IDLEPROJECT_API FText BuildProgressSavedFeedbackLabel();
IDLEPROJECT_API FIdleHUDCloudSyncViewModel BuildCloudSyncViewModel(ECloudSyncState State);
}

/** Slate HUD 구현을 붙이기 위한 최소 AHUD 베이스입니다. */
UCLASS()
class IDLEPROJECT_API AIdleHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void DrawHUD() override;
	virtual void NotifyHitBoxClick(FName BoxName) override;

	void ToggleQuestLog();

protected:
	UFUNCTION()
	void HandleGoldChanged(int64 NewGold);

	UFUNCTION()
	void HandleExpChanged(int64 CurrentExp, int64 NextExp);

	UFUNCTION()
	void HandleLevelUp(int32 NewLevel);

	UFUNCTION()
	void HandleHpChanged(float NewHp);

	void HandleDamageReceived(AActor* DamagedActor, float Amount, bool bWasCrit, EDamageKind Kind);

	UFUNCTION()
	void HandleEquippedChanged(EItemSlot Slot);

	UFUNCTION()
	void HandleEnhanceResult(const FEnhanceAttemptResult& Result);

	UFUNCTION()
	void HandleShopPurchase(const FShopPurchaseResult& Result);

	UFUNCTION()
	void HandlePetFed(const FPetFeedResult& Result);

	UFUNCTION()
	void HandleStatPointsChanged();

	UFUNCTION()
	void HandleTranscend();

	UFUNCTION()
	void HandleBossSpecialAttack(AActor* Boss);

	UFUNCTION()
	void HandleStageChanged(FStageInfo NewStageInfo);

	UFUNCTION()
	void HandleChapterBossDefeated(int32 ClearedChapter);

	UFUNCTION()
	void HandleTowerClimbed(int32 NewHighestFloor, int64 TotalReward);

	UFUNCTION()
	void HandleProgressSaved();

	UFUNCTION()
	void HandleCloudSyncStateChanged(ECloudSyncState NewState);

	UFUNCTION()
	void HandleAchievementUnlocked(const FString& AchievementId, int32 Tier);

private:
	void BindStageService();
	void UnbindStageService();
	void BindTowerService();
	void UnbindTowerService();
	void BindAchievementService();
	void UnbindAchievementService();
	void BindPlayerCombat();
	void UnbindPlayerCombat();
	void BindPlayerInventory();
	void BindBossSpecialAttack(AIdleMonster* Boss);
	void UnbindBossSpecialAttack();
	void RefreshEquipmentSummary();
	AIdleCharacter* ResolvePlayerCharacter() const;
	AIdleMonster* ResolveVisibleBoss() const;
	USkillComponent* ResolvePlayerSkills() const;
	void DrawClassSelectionPanel();
	void DrawClassSelectionOption(const FIdleHUDClassSelectionOptionViewModel& Option, float X, float Y, float Width, float Height);
	void SelectClassFromHitBox(FName BoxName);
	void DrawSkillHud(const USkillComponent& SkillComponent, float Now);
	void DrawSkillSlot(const FIdleHUDSkillSlotViewModel& Slot, int32 SlotIndex, float X, float Y, float Width, float Height);
	void DrawUltimateGauge(const FIdleHUDUltimateViewModel& Ultimate, float X, float Y, float Width, float Height);
	void DrawStageIndicator();
	void DrawBossBar();
	void DrawBossSpecialWarning(float Now);
	void DrawShopPanel();
	void TryBuyGearRoll();
	void TryBuyProtectionScroll();
	void TryBuyResetCube();
	void TryBuyRankCube();
	void DrawConsumablePanel();
	void DrawConsumableRow(const FIdleHUDConsumableRowViewModel& Row, float X, float Y, float Width, float Height);
	void TryUseConsumableFromHitBox(FName BoxName);
	void DrawLeaderboardPanel();
	void DrawLeaderboardRow(const FIdleHUDLeaderboardRowViewModel& Row, float X, float Y, float Width, float Height);
	void SelectLeaderboardKind(ELeaderboardKind Kind);
	void RefreshSelectedLeaderboard();
	void DrawWeeklyBossPanel();
	void DrawWeeklyBossMilestoneRow(const FIdleHUDWeeklyBossMilestoneRowViewModel& Row, float X, float Y, float Width, float Height);
	void TryChallengeWeeklyBoss();
	void ClaimWeeklyBossFromHitBox(FName BoxName);
	void DrawGuildPanel();
	void DrawGuildListRow(const FIdleHUDGuildListRowViewModel& Row, float X, float Y, float Width, float Height);
	void DrawGuildMemberRow(const FIdleHUDGuildMemberRowViewModel& Row, float X, float Y, float Width, float Height);
	void DrawGuildRequestRow(const FIdleHUDGuildRequestRowViewModel& Row, float X, float Y, float Width, float Height);
	void DrawGuildShopRow(const FIdleHUDGuildShopRowViewModel& Row, float X, float Y, float Width, float Height);
	void RefreshGuildBrowseList();
	void RefreshGuildShop();
	void CycleGuildCreateName();
	void TryCreateGuild();
	void JoinGuildFromHitBox(FName BoxName);
	void TryLeaveGuild();
	void ToggleGuildJoinMode();
	void ApproveGuildRequestFromHitBox(FName BoxName);
	void RejectGuildRequestFromHitBox(FName BoxName);
	void SetGuildMemberRankFromHitBox(FName BoxName, bool bPromote);
	void TryGuildAttendance();
	void CycleGuildDonateAmount();
	void TryGuildDonate();
	void BuyGuildShopItemFromHitBox(FName BoxName);
	void DrawGuildRankingRow(const FIdleHUDGuildRankingRowViewModel& Row, float X, float Y, float Width, float Height);
	void TryChallengeGuildBoss();
	void TryClaimGuildBossReward();
	void RefreshGuildRankings();
	void SetGuildRankingsView(bool bRankings);
	void SetGuildFeedback(const TCHAR* Key, bool bSuccess);
	void DrawRunePanel();
	void DrawRuneCodexPanel();
	void DrawRuneCodexCell(const FIdleHUDRuneCodexCellViewModel& Cell, float X, float Y, float Size);
	void DrawRuneSetRow(const FIdleHUDRuneSetRowViewModel& Row, float X, float Y, float Width, float Height);
	void DrawRuneSlot(const FIdleHUDRuneSlotViewModel& Slot, float X, float Y, float Width, float Height);
	void DrawRuneOwnedRow(const FIdleHUDRuneOwnedRowViewModel& Row, float X, float Y, float Width, float Height);
	void DrawRuneActionSection(const FIdleHUDRuneActionViewModel& Action, float X, float Y, float Width, float Height);
	void SelectRuneFromHitBox(FName BoxName);
	void EquipSelectedRuneFromHitBox(FName BoxName);
	void UnequipRuneFromHitBox(FName BoxName);
	void TryEnhanceRuneFromHitBox(FName BoxName);
	void TryDisenchantRuneFromHitBox(FName BoxName);
	void TryRerollRuneSetAction();
	void TryUpgradeRuneRarityAction();
	void TryTransferRuneEnhancementAction();
	void CycleRuneTransferTarget();
	int32 ResolveRuneTransferTargetIndex() const;
	void SetRuneActionFeedback(const TCHAR* Key, bool bSuccess);
	void TryBuyRuneRoll();
	void TryCraftClassRune();
	void DrawEnhancePanel();
	void DrawEnhanceSlotRow(const FIdleHUDEnhanceSlotViewModel& Row, float X, float Y, float Width, float Height);
	void TryEnhanceFromHitBox(FName BoxName);
	void DrawPotentialPanel();
	void DrawPotentialSlotRow(const FIdleHUDPotentialSlotViewModel& Row, float X, float Y, float Width, float Height);
	void TryRerollPotentialFromHitBox(FName BoxName, EPotentialCubeType CubeType);
	void ToggleItemLockFromHitBox(FName BoxName);
	void DrawStatAllocationPanel();
	void DrawStatAllocationRow(const FIdleHUDStatRowViewModel& Row, float X, float Y, float Width, float Height);
	void AllocateStatFromHitBox(FName BoxName);
	void ResetStatAllocation();
	void DrawStatInfoPanel();
	void DrawStatInfoToggle(const FIdleHUDStatInfoViewModel& ViewModel, float X, float Y, float Scale);
	void DrawStatInfoRow(const FIdleHUDStatInfoRowViewModel& Row, float X, float Y, float Width, float Height, const FLinearColor& AccentColor);
	void ToggleStatInfoPanel();
	void RankUpSkillFromHitBox(FName BoxName);
	void PreviewOfflineRewardModal();
	void ClaimOfflineRewardModal();
	void DrawOfflineRewardModal();
	void DrawQuestLog();
	void ClaimQuestFromHitBox(FName BoxName);
	void DrawRebirthPanel();
	void TryRebirth();
	void DrawTranscendPanel();
	void TryTranscend();
	void DrawTowerPanel();
	void TryClimbTower();
	void DrawDungeonPanel();
	void DrawDungeonRow(const FIdleHUDDungeonRowViewModel& Row, float X, float Y, float Width, float Height);
	void TryRunDungeonFromHitBox(FName BoxName);
	void DrawAchievementPanel();
	void DrawMasteryPanel();
	void DrawPetPanel();
	void DrawPetRow(const FIdleHUDPetRowViewModel& Row, float X, float Y, float Width, float Height);
	void EquipPetFromHitBox(FName BoxName);
	void TryFeedPetFromHitBox(FName BoxName);
	void TryEvolvePetFromHitBox(FName BoxName);
	void DrawTitlePanel();
	void DrawTitleRow(const FIdleHUDTitleRowViewModel& Row, float X, float Y, float Width, float Height);
	void EquipTitleFromHitBox(FName BoxName);
	void UnequipTitleAction();
	void DrawMissionPanel();
	void DrawMissionRow(const FIdleHUDMissionRowViewModel& Row, float X, float Y, float Width, float Height);
	void SelectMissionPeriod(EMissionPeriod Period);
	void ClaimMissionFromHitBox(FName BoxName);
	void DrawSeasonPassPanel();
	void DrawSeasonTierRow(const FIdleHUDSeasonTierRowViewModel& Row, float X, float Y, float Width, float Height);
	void ClaimSeasonTierFromHitBox(FName BoxName);
	void DrawFloatingDamageTexts(float Now);
	void DrawStatusIndicators(float Now);
	void DrawProgressSavedIndicator(float Now);
	void DrawCloudSyncIndicator(float Now);
	void RefreshMouseInteraction();

	UPROPERTY(Transient)
	TObjectPtr<UIdleGameInstance> IdleGameInstance;

	UPROPERTY(Transient)
	TObjectPtr<UCombatComponent> PlayerCombat;

	UPROPERTY(Transient)
	TObjectPtr<APawn> PlayerCombatPawn;

	UPROPERTY(Transient)
	TObjectPtr<UInventoryComponent> PlayerInventory;

	TSharedPtr<SIdleHUDWidget> RootWidget;
	FIdleHUDOfflineRewardViewModel OfflineRewardModal;
	FText EnhanceFeedbackLabel;
	FText PetFeedbackLabel;
	FText TranscendFeedbackLabel;
	bool bEnhanceFeedbackSuccess = false;
	bool bPetFeedbackSuccess = false;
	float PetFeedbackStartTime = -1000.0f;
	float TranscendFeedbackStartTime = -1000.0f;
	FShopPurchaseResult LastShopPurchaseResult;
	TArray<FIdleHUDFloatingDamageEntry> FloatingDamageEntries;
	FDelegateHandle AnyDamageReceivedHandle;
	TWeakObjectPtr<UBattleAIComponent> BoundBossBattleAI;
	TWeakObjectPtr<AActor> BossSpecialAttackActor;
	TWeakObjectPtr<UStageService> BoundStageService;
	TWeakObjectPtr<UTowerService> BoundTowerService;
	TWeakObjectPtr<UAchievementService> BoundAchievementService;
	FText StageFeedbackLabel;
	FText TowerFeedbackLabel;
	FText DungeonFeedbackLabel;
	FText AchievementFeedbackLabel;
	FText WeeklyBossFeedbackLabel;
	FText ProgressSavedFeedbackLabel;
	FIdleHUDCloudSyncViewModel CloudSyncViewModel;
	ELeaderboardKind SelectedLeaderboardKind = ELeaderboardKind::Power;
	bool bLeaderboardLoading = false;
	// 미션 패널 선택 탭(일일/주간) — 표시 전용 로컬 상태.
	EMissionPeriod SelectedMissionPeriod = EMissionPeriod::Daily;
	int32 SelectedRuneOwnedIndex = INDEX_NONE;
	int32 RuneTransferTargetOwnedIndex = INDEX_NONE;
	FText RuneActionFeedbackLabel;
	bool bRuneActionFeedbackSuccess = false;
	float RuneActionFeedbackStartTime = -1000.0f;
	float BossSpecialAttackStartTime = -1000.0f;
	float StageFeedbackStartTime = -1000.0f;
	float TowerFeedbackStartTime = -1000.0f;
	float DungeonFeedbackStartTime = -1000.0f;
	float AchievementFeedbackStartTime = -1000.0f;
	float WeeklyBossFeedbackStartTime = -1000.0f;
	float ProgressSavedFeedbackStartTime = -1000.0f;
	float CloudSyncFeedbackStartTime = -1000.0f;
	bool bQuestLogVisible = false;
	bool bStatInfoVisible = false;

	// ── 길드 패널 UI 상태(PR-G1, 서버 권위 — 캐시/표시 전용) ──────────────────
	TArray<FGuildSummary> GuildBrowseList;     // 무소속 화면 목록 캐시(서버 ListGuilds)
	bool bGuildBrowseLoading = false;          // 목록 비동기 로딩 표시
	bool bGuildActionPending = false;          // 생성/가입/탈퇴/관리 액션 in-flight
	int32 GuildCreateNamePresetIndex = 0;      // 생성 이름 프리셋 인덱스(이름 변경 버튼 순환)
	// ── 길드 기여/상점 UI 상태(PR-G2) ─────────────────────────────────────────
	TArray<FGuildShopItemInfo> GuildShopItems; // 길드 상점 카탈로그 캐시(별도 fetch)
	bool bGuildShopLoading = false;            // 상점 비동기 로딩 표시
	FString GuildShopLoadedForId;              // 상점을 로드한 길드 id(전환/탈퇴 시 무효화)
	int32 GuildDonatePresetIndex = 0;          // 헌납 금액 프리셋 인덱스(금액 변경 버튼 순환)
	// ── 길드 보스/주간 랭킹 UI 상태(PR-G3, 서버 권위 — 캐시/표시 전용) ──────────────
	bool bGuildRankingsView = false;           // 내 길드 화면에서 주간 랭킹 탭 활성 여부
	TArray<FGuildRankingEntry> GuildRankings;  // 주간 랭킹 캐시(서버 GET /rankings)
	FGuildRankingEntry GuildMyRanking;         // 내 길드 순위(Rank=0 이면 순위권 밖)
	bool bGuildRankingsLoading = false;        // 랭킹 비동기 로딩 표시
	bool bGuildRankingsLoaded = false;         // 진입 시 1회 auto-fetch 가드
	FText GuildFeedbackLabel;
	bool bGuildFeedbackSuccess = false;
	float GuildFeedbackStartTime = -1000.0f;
};
