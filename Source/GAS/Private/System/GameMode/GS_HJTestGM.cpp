#include "System/GameMode/GS_HJTestGM.h"

#include "AI/RTS/GS_RTSController.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

APlayerController* AGS_HJTestGM::SpawnPlayerController(ENetRole InRemoteRole, const FString& Options)
{
    const int32 NumPlayersSoFar = GameState->PlayerArray.Num();

    if (NumPlayersSoFar == 0)
    {
        PlayerControllerClass = GuardianControllerClass;
    }
    else
    {
        PlayerControllerClass = SeekerControllerClass;
    }

    UE_LOG(LogTemp, Error, TEXT("######################## number %d"), NumPlayersSoFar);

    return Super::SpawnPlayerController(InRemoteRole, Options);
}

APawn* AGS_HJTestGM::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
    const int32 Id = NewPlayer->PlayerState ? NewPlayer->PlayerState->GetPlayerId() : 0;

    TSubclassOf<APawn> DesiredPawnClass = (Id == 256) ? GuardianPawnClass : SeekerPawnClass;
    
    UE_LOG(LogTemp, Error, TEXT("######################## ID %d"), Id);

    if (NewPlayer->IsA(AGS_RTSController::StaticClass()))
    {
        return nullptr;
    }    

    return DesiredPawnClass
        ? GetWorld()->SpawnActor<APawn>(DesiredPawnClass, StartSpot->GetActorLocation(),
            StartSpot->GetActorRotation())
        : Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
}