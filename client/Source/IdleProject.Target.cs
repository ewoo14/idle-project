using UnrealBuildTool;
using System.Collections.Generic;

public class IdleProjectTarget : TargetRules
{
	public IdleProjectTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("IdleProject");
	}
}
