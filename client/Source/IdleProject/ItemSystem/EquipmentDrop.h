#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemSystem/ItemTypes.h"
#include "EquipmentDrop.generated.h"

class UStaticMeshComponent;

/** 몬스터 사망 지점에서 생성되어 가장 가까운 캐릭터에게 흡수되는 장비 드롭입니다. */
UCLASS()
class IDLEPROJECT_API AEquipmentDrop : public AActor
{
	GENERATED_BODY()

public:
	AEquipmentDrop();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	FItemInstance Payload;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Idle|Item")
	TObjectPtr<UStaticMeshComponent> Mesh;

private:
	void EnablePickup();
	AActor* FindClosestCharacter() const;
	FLinearColor GetRarityColor() const;

	FTimerHandle PickupDelayHandle;
	bool bPickupEnabled = false;
};
