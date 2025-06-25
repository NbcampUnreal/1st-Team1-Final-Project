#include "System/GS_BaseGM.h"

#include "System/GS_GameInstance.h"

void AGS_BaseGM::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (UGS_GameInstance* GI = GetGameInstance<UGS_GameInstance>())
    {
        GI->OnPlayerCountChanged.Broadcast();
    }
}

void AGS_BaseGM::Logout(AController* Exiting)
{
    if (UGS_GameInstance* GI = GetGameInstance<UGS_GameInstance>())
    {
        GI->OnPlayerCountChanged.Broadcast();
    }

    Super::Logout(Exiting);
}
