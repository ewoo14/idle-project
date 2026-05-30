#pragma once

#include "CoreMinimal.h"
#include "RebirthPerkTypes.generated.h"

// 환생 특성(Rebirth Perks) 6종. 서버 rebirthPerk.ts RebirthPerk 문자열과 1:1(None 제외).
// None 은 무효/미선택 가드용(분배 대상 아님). enum 이름 문자열이 서버 perk 키와 동일하다.
// GoldPct: 골드 획득 / DropPct: 드롭률 / CritDmgPct: 크리티컬 데미지 /
// AllStatPct: 전체 스탯 / ExpPct: 경험치 / OfflineEffPct: 오프라인 효율.
UENUM(BlueprintType)
enum class ERebirthPerk : uint8
{
	None,
	GoldPct,
	DropPct,
	CritDmgPct,
	AllStatPct,
	ExpPct,
	OfflineEffPct
};
