#include "System/PlayerController/GS_ResultPC.h"
#include "Blueprint/UserWidget.h"
#include "UI/Screen/Result/GS_GameResultUI.h"
#include "System/GameMode/GS_ResultGM.h"

void AGS_ResultPC::Client_ShowResultUI_Implementation()
{
	if (!ResultUIClass) return;

	if (IsLocalController())
	{
		UGS_GameResultUI* ResultWidget = CreateWidget<UGS_GameResultUI>(this, ResultUIClass);
		if (ResultWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("AGS_ResultPC::Client_ShowResultUI_Implementation() - Trying to Add to Viewport"));
			ResultWidget->AddToViewport();
			FInputModeGameAndUI InputModeData;
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputModeData);
			bShowMouseCursor = true;
		}
	}
}

void AGS_ResultPC::Server_RequestTravelToLobby_Implementation()
{
	AGS_ResultGM* GM = GetWorld()->GetAuthGameMode<AGS_ResultGM>();

	if (GM)
	{
		const FString LobbyMapURL = TEXT("/Game/Maps/CustomLobbyLevel");
		GetWorld()->ServerTravel(LobbyMapURL);
	}
}
