using UnrealBuildTool;
using System.Collections.Generic;

public class NPC_Dialogue_with_AITarget : TargetRules
{
    public NPC_Dialogue_with_AITarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("NPC_Dialogue_with_AI");
    }
}