using UnrealBuildTool;

public class Subsystems : ModuleRules
{
    public Subsystems(ReadOnlyTargetRules target) : base(target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDefinitions.Add("NOMINMAX");
        PublicDefinitions.Add("WIN32_LEAN_AND_MEAN");
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "HTTP",
                "HTTPServer",
                "Json",
                "JsonUtilities",
                "SharedTypes",
                "TrailUtils",
                "CoreUObject",
                "TrailInterfaces", 
                "CesiumRuntime",
                "SlateNotifications"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Engine","CinematicCamera", "DeveloperSettings", "Slate", "SlateCore",
                "RenderCore", "RHI", "MeshDescription", "StaticMeshDescription", "OWLCamera"
            }
        );

        if (target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
        }
    }
}