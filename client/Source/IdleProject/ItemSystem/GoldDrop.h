#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GoldDrop.generated.h"

class UStaticMeshComponent;

/** 몬스터 사망 지점에서 생성되어 가장 가까운 캐릭터에게 자동 흡수되는 골드입니다. */
UCLASS()
class IDLEPROJECT_API AGoldDrop : public AActor
{
	GENERATED_BODY()

public:
	AGoldDrop();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Gold")
	int64 Amount = 10;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Idle|Gold")
	TObjectPtr<UStaticMeshComponent> Mesh;

private:
	void EnablePickup();
	AActor* FindClosestCharacter() const;

	FTimerHandle PickupDelayHandle;
	bool bPickupEnabled = false;
};
