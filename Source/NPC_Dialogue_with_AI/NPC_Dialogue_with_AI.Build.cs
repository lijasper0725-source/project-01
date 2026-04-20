using UnrealBuildTool;

public class NPC_Dialogue_with_AI : ModuleRules
{
    public NPC_Dialogue_with_AI(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "UMG",
            "HTTP",
            "Json",
            "JsonUtilities",
            "Slate",
			"SlateCore"
        });

        PrivateDependencyModuleNames.AddRange(new string[] { });
    }
}