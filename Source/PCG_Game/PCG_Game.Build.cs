// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PCG_Game : ModuleRules
{
	public PCG_Game(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine",
	        "RHI","RenderCore","Renderer","RHICore","InputCore", "NavigationSystem", "AIModule", "Niagara", "EnhancedInput" });
    }
}
