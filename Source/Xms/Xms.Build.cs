// Copyright (c) 2025 Xist.GG LLC

using UnrealBuildTool;

public class Xms : ModuleRules
{
	public Xms(ReadOnlyTargetRules Target) : base(Target)
	{
		// Add XMS DEBUG code in DebugGame
		int withXmsDebug = Target.Configuration == UnrealTargetConfiguration.DebugGame ? 1 : 0;
		PublicDefinitions.Add("WITH_XMS_DEBUG="+withXmsDebug);

		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[] { "Xms" });

		// This stuff is required just for the base Simple game with nothing in it!
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
		PrivateDependencyModuleNames.AddRange(new string[] { "AIModule", "Niagara" });

		// This stuff is required specifically by Xms code
		PrivateDependencyModuleNames.AddRange(new string[] { "MassEntity" });

		// This stuff is required specifically for Mass debugging in Editor
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[] { "MassEntityEditor" });
		}
	}
}
