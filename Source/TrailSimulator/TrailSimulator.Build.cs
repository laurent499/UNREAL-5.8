// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;


public class TrailSimulator : ModuleRules
{
	public TrailSimulator(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDefinitions.Add("NOMINMAX");
		PublicDefinitions.Add("WIN32_LEAN_AND_MEAN");
		PublicDependencyModuleNames.AddRange(new string[] 
		{
			"Core",		// contient UE::Tasks
			"CoreUObject",
			"SharedTypes",
			"InputCore",
			"EnhancedInput",
			"HTTP",
			"HTTPServer",
			"CesiumRuntime",
			"Slate",
			"SlateCore",
			"UMG",
			"TrailUtils",
			"SlateNotifications",
			"CinematicCamera"
			
			
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Json",
			"JsonUtilities",
			"Subsystems",
			"TrailInterfaces",
			"Engine",
			"ImageWrapper",
			"OWLCamera",
			"OWLMedia"
			
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
