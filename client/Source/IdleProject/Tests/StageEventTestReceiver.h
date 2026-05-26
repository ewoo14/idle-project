#pragma once

#include "CoreMinimal.h"
#include "StageEventTestReceiver.generated.h"

UCLASS()
class UStageEventTestReceiver : public UObject
{
	GENERATED_BODY()

public:
	int32 Count = 0;
	int32 LastClearedChapter = 0;

	UFUNCTION()
	void CaptureChapterBossDefeated(int32 ClearedChapter)
	{
		++Count;
		LastClearedChapter = ClearedChapter;
	}
};
