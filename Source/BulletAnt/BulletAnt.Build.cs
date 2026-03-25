// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
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
			"GeometryFramework", "GeometryCore",
			"Niagara",

			//Character
			"AnimGraphRuntime",
			"MotionWarping",
			
			// GAS
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			
			// AI
			"AIModule", "StateTreeModule",
			"GameplayStateTreeModule",
			"NavigationSystem",
			"MassEntity", "MassCommon", "MassNavigation", "MassMovement", "MassSpawner", "MassActors",
			"MassAIBehavior", "MassRepresentation", "MassLOD", "MassSignals",

			// ChaosDestruction
			"GeometryCollectionEngine",

			// UI
			"UMG",
			"Slate",
			"SlateCore",

			// EOS
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"OnlineSubsystemEOS",
			"EOSSDK",
			"OnlineSubsystemSteam",
			"Steamworks",

			// EOS - VoiceChat
			"EOSVoiceChat",
			"VoiceChat"
		});

		//string SteamVersion = "v161";
		//string SteamDir = Path.Combine(Target.UEThirdPartySourceDirectory, "Steamworks", SteamVersion, "sdk");

		PublicIncludePaths.AddRange(new string[]
		{
			"BulletAnt",
			//Path.Combine(SteamDir, "public")
		});

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
