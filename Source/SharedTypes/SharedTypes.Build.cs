using UnrealBuildTool;

public class SharedTypes : ModuleRules
{
	public SharedTypes(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDefinitions.Add("NOMINMAX");
		PublicDefinitions.Add("WIN32_LEAN_AND_MEAN");
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Slate",
				"SlateCore",
				"UMG"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// Rien pour l’instant
			}
		);
	}
}