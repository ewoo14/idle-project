#pragma once

#include "CoreMinimal.h"
#include "GameCore/RebirthPerkTypes.h"
#include "UObject/Object.h"
#include "RebirthPerkService.generated.h"

/**
 * 환생 특성(Rebirth Perks) 서비스. 환생 횟수로 적립된 특성 포인트를 6종 특성에
 * 자유 분배하며, 특성별 보너스 비율(PerkStep)을 산출한다. 적립/분배는 클라 로컬
 * 세이브 권위이며, 서버 rebirthPerk.ts(PerkStep/Total)를 1:1 미러링한다(f32 곱셈 —
 * 서버 Math.fround 와 동일, double 금지). 총 포인트(TotalPoints)는 GameInstance 가
 * 환생 횟수로 SetTotalPoints 주입하고, 보너스 적용은 GameInstance 단일 합산 지점이
 * 담당한다(이중 적용 금지, #72). 기존 환생 포인트(RebirthBonusPoints, HP/공격)와는
 * 완전히 별도 풀이며 그 동작은 건드리지 않는다(비파괴).
 */
UCLASS(BlueprintType)
class IDLEPROJECT_API URebirthPerkService : public UObject
{
	GENERATED_BODY()

public:
	// 특성 레벨당 % 보너스 비율(서버 PERK_STEP parity). 비율 값(0.02 = +2%/레벨). f32.
	// None 은 0. 서버 rebirthPerk.ts PERK_STEP 와 1:1.
	static float GetPerkStep(ERebirthPerk Perk);

	// 환생 횟수로 적립되는 총 환생 특성 포인트(서버 getTotalRebirthPerkPoints 1:1).
	// 1/환생, 음수는 0 가드. RebirthCount 가 음수일 일은 없으나 회귀 안전.
	static int32 GetTotalRebirthPerkPoints(int32 RebirthCount);

	// perk 를 Level 만큼 분배했을 때의 보너스 비율(서버 getPerkBonus 1:1).
	// fround(PerkStep * max(0, Level)). 음수 Level 은 0 가드. f32 곱셈으로 서버 parity.
	static float GetPerkBonusForLevel(ERebirthPerk Perk, int32 Level);

	// 현재 perk 분배 레벨의 보너스 비율(GetPerkBonusForLevel(perk, GetPerkLevel(perk))).
	UFUNCTION(BlueprintPure, Category = "Idle|RebirthPerk")
	float GetPerkBonus(ERebirthPerk Perk) const;

	// perk 의 현재 분배 레벨(없으면 0).
	UFUNCTION(BlueprintPure, Category = "Idle|RebirthPerk")
	int32 GetPerkLevel(ERebirthPerk Perk) const;

	// 분배에 사용한 총 포인트(Σ Allocations).
	UFUNCTION(BlueprintPure, Category = "Idle|RebirthPerk")
	int32 GetSpent() const;

	// 총 포인트(GameInstance 가 환생 횟수로 주입).
	UFUNCTION(BlueprintPure, Category = "Idle|RebirthPerk")
	int32 GetTotalPoints() const { return TotalPoints; }

	// 가용 포인트(Total - Spent, 0 미만 클램프).
	UFUNCTION(BlueprintPure, Category = "Idle|RebirthPerk")
	int32 GetAvailable() const;

	// perk 분배(+1). perk!=None && 가용>0 이면 Allocations[perk]++ 후 true.
	UFUNCTION(BlueprintCallable, Category = "Idle|RebirthPerk")
	bool AllocatePerk(ERebirthPerk Perk);

	// perk 분배 해제(-1). perk!=None && 현재 레벨>0 이면 -- 후 true.
	UFUNCTION(BlueprintCallable, Category = "Idle|RebirthPerk")
	bool DeallocatePerk(ERebirthPerk Perk);

	// 전체 분배 초기화(가용 포인트 회수).
	UFUNCTION(BlueprintCallable, Category = "Idle|RebirthPerk")
	void ResetPerks();

	// 총 포인트 설정(GameInstance 가 환생 횟수 변경 시 주입). 음수는 0 클램프.
	// 줄어들어 Spent > Total 이 되어도 분배는 유지(가용만 0)하여 비파괴.
	void SetTotalPoints(int32 InTotalPoints);

	const TMap<ERebirthPerk, int32>& GetAllocations() const { return Allocations; }

	// 세이브 복원. None/음수 레벨 항목은 무시하며, 복원 후 SetTotalPoints 는 호출 측 책임.
	void RestoreState(const TMap<ERebirthPerk, int32>& InAllocations);

private:
	// perk → 분배 레벨(>0). 레벨 0 은 항목 미보유로 표현.
	UPROPERTY()
	TMap<ERebirthPerk, int32> Allocations;

	UPROPERTY()
	int32 TotalPoints = 0;
};
