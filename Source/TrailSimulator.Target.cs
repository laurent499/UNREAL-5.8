// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class TrailSimulatorTarget : TargetRules
{
	public TrailSimulatorTarget(TargetInfo target) : base(target)
	{
		Type = TargetType.Game;
		// BuildEnvironment = TargetBuildEnvironment.Unique;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.Add("TrailSimulator");
		
		bUseUnityBuild = false;
		bUseAdaptiveUnityBuild = false;   // important : sinon UBT peut rester "semi-unity"
		
		
		RegisterModulesCreatedByRider();
	}

	private void RegisterModulesCreatedByRider()
	{
		ExtraModuleNames.AddRange(new string[] { "Subsystems", "SharedTypes", "TrailInterfaces", "TrailUtils" });
	}
	
}
