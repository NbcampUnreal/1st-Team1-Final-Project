#include "System/GameMode/GS_InGameGM.h"
#include "System/GameState/GS_InGameGS.h"
#include "System/GS_PlayerState.h"
#include "System/GS_GameInstance.h"
#include "AI/RTS/GS_RTSController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/HUD.h"

AGS_InGameGM::AGS_InGameGM()
{
    DefaultPawnClass = nullptr;
	GameStateClass = AGS_InGameGS::StaticClass();
    PlayerStateClass = AGS_PlayerState::StaticClass();
    bUseSeamlessTravel = true;
}

TSubclassOf<APlayerController> AGS_InGameGM::GetPlayerControllerClassToSpawnForSeamlessTravel(APlayerController* PreviousPC)
{
    if (const auto* PS = PreviousPC ? PreviousPC->GetPlayerState<AGS_PlayerState>() : nullptr)
    {
        return (PS->CurrentPlayerRole == EPlayerRole::PR_Seeker) ? SeekerControllerClass : RTSControllerClass;
    }

    return Super::GetPlayerControllerClassToSpawnForSeamlessTravel(PreviousPC);
}

void AGS_InGameGM::HandleSeamlessTravelPlayer(AController*& C)
{
    Super::HandleSeamlessTravelPlayer(C);

    if (auto* PC = Cast<APlayerController>(C))
    {
        if (const auto* PS = PC->GetPlayerState<AGS_PlayerState>())
        {
            if (PS->CurrentPlayerRole == EPlayerRole::PR_Seeker)
            {
                PC->ClientSetHUD(SeekerHUDClass);
            }
            else
            {
                PC->ClientSetHUD(RTSHUDClass);
            }
        }
    }
}

UClass* AGS_InGameGM::GetDefaultPawnClassForController_Implementation(AController* InController)
{
    const AGS_PlayerState* PS = InController ? InController->GetPlayerState<AGS_PlayerState>() : nullptr;
    
    return (PS && PS->CurrentPlayerRole == EPlayerRole::PR_Seeker) ? *SeekerPawnClass : *RTSPawnClass;
}
