#pragma once

#include "CoreMinimal.h"
#include "ItemSystem/ItemTypes.h"

/** 아이템 매각가/파워 — 서버 itemSell.ts 1:1 parity. 라우트/DB 없음. */
class IDLEPROJECT_API FItemSellFormula
{
public:
	// 등급 base × (1 + 0.2 × max(0,enhance)). None=0.
	static int64 ComputeSellValue(EItemRarity Rarity, int32 EnhanceLevel);

	// 같은 슬롯 내 우열 스칼라(보너스 가중합 × 강화 배수). CombatPower 와 별개.
	static int64 ComputeItemPower(const FItemInstance& Item);
};
