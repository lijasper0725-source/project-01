// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class NPC_Dialogue_with_AITarget : TargetRules
{
	public NPC_Dialogue_with_AITarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V6;

		ExtraModuleNames.AddRange( new string[] { "NPC_Dialogue_with_AI" } );
	}
}
