// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PluginsStudy : ModuleRules
{
	public PluginsStudy(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput" });
	}
}
