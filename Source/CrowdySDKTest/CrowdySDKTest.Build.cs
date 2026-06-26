// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CrowdySDKTest : ModuleRules
{
	public CrowdySDKTest(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });

		// The sample calls into several CrowdySDK plugin modules:
		//   CrowdySDK - the game-instance subsystem (login, voice)
		//   CrowdyReplication - entity component, executor, RPC macro, actor tracker, settings
		//   CrowdyServices - utilities, teams, persistence
		//   CrowdyNet - the routing enums (recipient, decay, distance)
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CrowdySDK",
			"CrowdyReplication",
			"CrowdyServices",
			"CrowdyNet",
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
