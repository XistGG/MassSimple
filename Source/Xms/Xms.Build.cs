// Copyright (c) 2025 Xist.GG LLC

using UnrealBuildTool;

public class Xms : ModuleRules
{
	public Xms(ReadOnlyTargetRules Target) : base(Target)
	{
		// Add XMS DEBUG code in DebugGame
		int WithXmsDebug = Target.Configuration == UnrealTargetConfiguration.DebugGame ? 1 : 0;
		PublicDefinitions.Add("WITH_XMS_DEBUG="+WithXmsDebug);

		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[] { "Xms" });

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
		PrivateDependencyModuleNames.AddRange(new string[] { "AIModule", "Niagara" });
	}
}
