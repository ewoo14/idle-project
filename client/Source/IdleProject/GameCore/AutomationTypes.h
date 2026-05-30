#pragma once

#include "CoreMinimal.h"
#include "AutomationTypes.generated.h"

// 자동화 기능 4종. 서버 automation.ts AUTOMATION_FEATURE 와 1:1(이름 동일).
// P1 에선 Progression 만 실사용, 나머지는 해금 게이트 정의만(P2~P4).
UENUM(BlueprintType)
enum class EAutomationFeature : uint8
{
	Progression,
	SkillTactics,
	AutoGear,
	AutoConsumable
};

// 자동 진행 모드. 서버 ProgressionMode 와 1:1.
UENUM(BlueprintType)
enum class EProgressionMode : uint8
{
	Advance,
	FarmLock,
	AutoRetreat
};

// 진행 결정 행동. 서버 ProgressionAction 과 1:1.
UENUM(BlueprintType)
enum class EProgressionAction : uint8
{
	Advance,
	Hold,
	Retreat
};

// 진행 결정 결과(행동 + 목표 글로벌 스테이지).
USTRUCT(BlueprintType)
struct FProgressionDecision
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Automation")
	EProgressionAction Action = EProgressionAction::Hold;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Automation")
	int32 TargetGlobalStage = 1;
};

// 스킬 자동 전술 조건. 서버 automation.ts SkillAutoCondition 와 1:1.
UENUM(BlueprintType)
enum class ESkillAutoCondition : uint8
{
	Always,
	BossEliteOnly,
	HpBelow,
	MaintainBuff
};

// 스킬별 자동 발동 규칙. 미설정 스킬은 Always(기존 동작).
USTRUCT(BlueprintType)
struct FSkillAutoRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Automation")
	FName SkillId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Automation")
	ESkillAutoCondition Condition = ESkillAutoCondition::Always;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Automation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HpThresholdPct = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Automation")
	int32 Priority = 0;
};
