using UnrealBuildTool;
using System.Collections.Generic;

public class SSXPureTarget : TargetRules
{
	public SSXPureTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("SSXPure");
	}
}
