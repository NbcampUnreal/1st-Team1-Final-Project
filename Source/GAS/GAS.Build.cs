// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
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
            "MediaAssets",
            "HTTP",
            "Json",
            "GameplayTags"
        });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Linux) || (Target.Platform == UnrealTargetPlatform.Mac))
        {
            // 아래 모듈들이 필요합니다.
            PublicDependencyModuleNames.AddRange(new string[] { "SteamShared", "Steamworks", "OnlineSubsystemSteam" });

            // Steamworks 라이브러리를 링크합니다.
            AddEngineThirdPartyPrivateStaticDependencies(Target, "Steamworks");
        }
        
        bool bUseGameLiftAutomation = true; // EC2 버전을 빌드할 때는 true, 내부망 버전을 빌드할 때는 false로 변경
        if (bUseGameLiftAutomation)
        {
	        PrivateDefinitions.Add("WITH_GAMELIFT_AUTOMATION=1");
        }
        else
        {
	        PrivateDefinitions.Add("WITH_GAMELIFT_AUTOMATION=0");
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
