using UnrealBuildTool;

public class IdleProject : ModuleRules
{
	public IdleProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"Paper2D",
			"EnhancedInput",
			"UMG",
			"HTTP",
			"Json",
			"JsonUtilities"
		});
	}
}
