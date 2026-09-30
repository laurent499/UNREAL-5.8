using UnrealBuildTool;

public class TrailUtils : ModuleRules
{
    public TrailUtils(ReadOnlyTargetRules target) : base(target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "SharedTypes",
                "TrailInterfaces"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Engine"
            }
        );
    }
}