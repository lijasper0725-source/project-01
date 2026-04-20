using UnrealBuildTool;
using System.Collections.Generic;

public class NPC_Dialogue_with_AIEditorTarget : TargetRules
{
    public NPC_Dialogue_with_AIEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("NPC_Dialogue_with_AI");
    }
}