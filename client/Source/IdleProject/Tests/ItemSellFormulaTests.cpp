#include "Misc/AutomationTest.h"
#include "ItemSystem/ItemSellFormula.h"
#include "ItemSystem/InventoryComponent.h"
#include "ItemSystem/ItemTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FItemSellParityTest,
	"IdleProject.Item.SellFormulaParity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FItemSellParityTest::RunTest(const FString& Parameters)
{
	using F = FItemSellFormula;
	// 매각가
	TestEqual(TEXT("common base"), F::ComputeSellValue(EItemRarity::Common, 0), (int64)100);
	TestEqual(TEXT("epic base"), F::ComputeSellValue(EItemRarity::Epic, 0), (int64)1500);
	TestEqual(TEXT("mythic base"), F::ComputeSellValue(EItemRarity::Mythic, 0), (int64)400000);
	TestEqual(TEXT("none zero"), F::ComputeSellValue(EItemRarity::None, 5), (int64)0);
	TestEqual(TEXT("common +5 enh"), F::ComputeSellValue(EItemRarity::Common, 5), (int64)200);
	TestEqual(TEXT("rare +10 enh"), F::ComputeSellValue(EItemRarity::Rare, 10), (int64)1200);
	TestEqual(TEXT("neg enh guard"), F::ComputeSellValue(EItemRarity::Common, -3), (int64)100);
	// 파워
	FItemInstance A; A.BonusAtk = 100.0f; A.BonusDef = 50.0f;
	TestEqual(TEXT("power sum"), F::ComputeItemPower(A), (int64)150);
	FItemInstance B; B.BonusHp = 1000.0f;
	TestEqual(TEXT("power hp/10"), F::ComputeItemPower(B), (int64)100);
	FItemInstance D; D.BonusCritRate = 0.1f;
	TestEqual(TEXT("power critrate*1000"), F::ComputeItemPower(D), (int64)100);
	FItemInstance E; E.BonusCritDmg = 0.5f;
	TestEqual(TEXT("power critdmg*100"), F::ComputeItemPower(E), (int64)50);
	FItemInstance C; C.BonusAtk = 100.0f; C.EnhanceLevel = 10;
	TestEqual(TEXT("power +10 enh"), F::ComputeItemPower(C), (int64)200);
	FItemInstance G; G.BonusAtk = 100.0f; G.EnhanceLevel = -5;
	TestEqual(TEXT("power neg enh guard"), F::ComputeItemPower(G), (int64)100);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventorySellEquipTest,
	"IdleProject.Item.InventorySellEquip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInventorySellEquipTest::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inv = NewObject<UInventoryComponent>();

	FItemInstance Weak; Weak.Slot = EItemSlot::Weapon; Weak.Rarity = EItemRarity::Common; Weak.BonusAtk = 10.0f;
	FItemInstance Strong; Strong.Slot = EItemSlot::Weapon; Strong.Rarity = EItemRarity::Rare; Strong.BonusAtk = 100.0f;
	Inv->AddItem(Weak);   // index 0 — 첫 장비라 자동 장착됨
	Inv->AddItem(Strong); // index 1 — 더 강해 AddItem 이 자동 장착(기존 동작)

	// 장착 해제 후 AutoEquipBestPerSlot 가 더 강한 Strong 을 다시 장착하는지 검증.
	Inv->UnequipSlot(EItemSlot::Weapon);
	const int32 Equipped = Inv->AutoEquipBestPerSlot();
	TestEqual(TEXT("auto-equipped one"), Equipped, 1);
	const FItemInstance* Now = Inv->GetEquippedItem(EItemSlot::Weapon);
	TestTrue(TEXT("strong equipped"), Now && Now->BonusAtk == 100.0f);

	// 장착 중인 Strong(index 1) 매각 거부.
	TestEqual(TEXT("equipped sell refused"), Inv->SellItem(1), (int64)0);

	// Weak(미장착, index 0) 매각 → Common base 100. 매각 후 Items=[Strong] (장착 인덱스 1→0 보정).
	const int64 Gold = Inv->SellItem(0);
	TestEqual(TEXT("weak sells for 100"), Gold, (int64)100);
	const FItemInstance* AfterSell = Inv->GetEquippedItem(EItemSlot::Weapon);
	TestTrue(TEXT("strong still equipped after shift"), AfterSell && AfterSell->BonusAtk == 100.0f);

	// 잠금 아이템 매각 거부(미장착 경로). Weapon 슬롯에 Strong 보다 약한 잠금 장비를 추가 →
	// AddItem 이 자동 장착하지 않으므로(기존 동작) 미장착 상태의 잠금 가드만 검증된다.
	FItemInstance Locked; Locked.Slot = EItemSlot::Weapon; Locked.Rarity = EItemRarity::Epic; Locked.BonusAtk = 5.0f; Locked.bLocked = true;
	Inv->AddItem(Locked); // Items=[Strong(0), Locked(1)] — Strong 이 더 강해 장착 유지
	const int32 LockedIdx = Inv->GetItemCount() - 1;
	TestTrue(TEXT("locked not equipped"), Inv->GetEquippedItem(EItemSlot::Weapon)->BonusAtk == 100.0f);
	TestEqual(TEXT("locked sell refused"), Inv->SellItem(LockedIdx), (int64)0);
	return true;
}

#endif
