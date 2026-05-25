using UnrealBuildTool;
using System.IO;

public class IdleProject : ModuleRules
{
	public IdleProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// 도메인 기반 디렉터리 구조 (CharacterSystem/, GameCore/, UI/, NetworkClient/, DataAssets/, Tests/)
		// 를 사용하므로 모듈 루트를 PublicIncludePaths 에 추가하여
		// '#include "CharacterSystem/StatFormulas.h"' 같은 모듈-루트 기준 경로를 허용한다.
		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"Paper2D",
			"EnhancedInput",
			"UMG",
			"Slate",
			"SlateCore",
			"HTTP",
			"Json",
			"JsonUtilities"
		});

		// VRM4U 타입을 직접 참조하지 않고 import된 UE 자산을 동적 로드하므로 런타임 모듈만 지연 로드합니다.
		DynamicallyLoadedModuleNames.AddRange(new string[]
		{
			"VRM4U"
		});
	}
}
