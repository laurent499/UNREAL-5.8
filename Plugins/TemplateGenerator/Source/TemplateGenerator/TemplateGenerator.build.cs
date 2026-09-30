using UnrealBuildTool;

public class TemplateGenerator : ModuleRules
{
	public TemplateGenerator(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"UMG",
				"EditorScriptingUtilities",
				"Slate",
				"SlateCore",
				"Blutility",
				"UMGEditor",
				"UnrealEd",
				"Kismet",
				"AssetRegistry",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				
				
			}
		);
	}
}