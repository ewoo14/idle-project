#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "GenerateToonOutlineCommandlet.generated.h"

/**
 * 툰(만화) 스타일 깊이-에지 아웃라인 포스트프로세스 머티리얼(/Game/Render/M_ToonOutline)을
 * 절차적으로 생성·저장하는 에디터 전용 커맨드릿. MD_PostProcess 도메인이며,
 * SceneDepth 의 중심/4-이웃 샘플 깊이 차이를 임계값과 비교해 에지 마스크를 만들고
 * 씬 컬러(PostProcessInput0)와 어두운 아웃라인 색을 Lerp 해 EmissiveColor 로 출력한다.
 * 실행: UnrealEditor-Cmd.exe "<IdleProject.uproject>" -run=GenerateToonOutline
 *       -unattended -nopause -nosplash
 */
UCLASS()
class UGenerateToonOutlineCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	virtual int32 Main(const FString& Params) override;
};
