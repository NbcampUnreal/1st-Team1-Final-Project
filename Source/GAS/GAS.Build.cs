// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GAS : ModuleRules
{
	public GAS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"GameLiftServerSDK",
			"SlateCore",
			"Slate",
			"UMG",
            "CoreOnline",
            "Slate",
			"SlateCore",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"OnlineSubsystemSteam",
			"AkAudio",
            "Niagara",
            "PhysicsCore",
            "CommonUI",
            "CommonInput",
            "Chooser",
            "PoseSearch",
            "GeometryCollectionEngine",
            "NavigationSystem",
            "MediaAssets"
        });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Linux) || (Target.Platform == UnrealTargetPlatform.Mac))
        {
            // 아래 모듈들이 필요합니다.
            PublicDependencyModuleNames.AddRange(new string[] { "SteamShared", "Steamworks", "OnlineSubsystemSteam" });

            // Steamworks 라이브러리를 링크합니다.
            AddEngineThirdPartyPrivateStaticDependencies(Target, "Steamworks");
        }

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[] {
                "GameLiftCore",
                "GameLiftPlugin"
            });
        }
    }
}
