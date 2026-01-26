// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class BulletAnt : ModuleRules
{
	public BulletAnt(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput", 
			"AnimGraphRuntime",
			
			// GAS
			"GameplayAbilities", 
			"GameplayTags", 
			"GameplayTasks",
			
			// AI
			"StateTreeModule",
			"GameplayStateTreeModule",
			"NavigationSystem",
			"MassEntity", "MassCommon", "MassNavigation", "MassMovement", "MassSpawner", "MassActors",
			"MassAIBehavior", "MassRepresentation", "MassLOD", "MassSignals"
		});

		PublicIncludePaths.AddRange(new string[] {
            "BulletAnt" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
