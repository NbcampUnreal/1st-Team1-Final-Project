#include "System/Server/APITestHUD.h"
#include "System/Server/APITestOverlay.h"

void AAPITestHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = GetOwningPlayerController();
	if (IsValid(PlayerController) && APITestOverlayClass)
	{
		APITestOverlay = CreateWidget<UAPITestOverlay>(PlayerController, APITestOverlayClass);
	}
}
