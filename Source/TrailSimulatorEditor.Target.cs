// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class TrailSimulatorEditorTarget : TargetRules
{
	public TrailSimulatorEditorTarget( TargetInfo target) : base(target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.Add("TrailSimulator");
		
		
		
		
		RegisterModulesCreatedByRider();
	}

	private void RegisterModulesCreatedByRider()
	{
		ExtraModuleNames.AddRange(new string[] { "Subsystems", "SharedTypes", "TrailInterfaces", "TrailUtils" });
	}
}
