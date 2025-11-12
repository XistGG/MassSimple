// Copyright (c) 2025 Xist.GG LLC

using UnrealBuildTool;

public class XmsTarget : TargetRules
{
	public XmsTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("Xms");
	}
}
