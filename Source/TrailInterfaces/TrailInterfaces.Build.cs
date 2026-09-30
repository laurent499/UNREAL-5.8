using UnrealBuildTool;

public class TrailInterfaces : ModuleRules
{
    public TrailInterfaces(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDefinitions.Add("NOMINMAX");
        PublicDefinitions.Add("WIN32_LEAN_AND_MEAN");
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "SharedTypes",
                "Engine"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            { }
        );
    }
}