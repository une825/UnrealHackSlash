// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MyHackSlash : ModuleRules
{
	public MyHackSlash(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateIncludePaths.AddRange(new string[] { "MyHackSlash" });

        PublicDependencyModuleNames.AddRange(new string[] { 
		"Core", 
		"CoreUObject", 
		"Engine", 
		"InputCore", 
		"NavigationSystem", 
		"AIModule", 
		"Niagara", 
		"EnhancedInput", 
		"GameplayAbilities", 
		"GameplayTags", 
		"GameplayTasks",
        "UMG",
		"Slate",
		"SlateCore"
		});
    }
}
