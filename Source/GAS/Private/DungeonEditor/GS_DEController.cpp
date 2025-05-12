#include "DungeonEditor/GS_DEController.h"
#include "EnhancedInputSubsystems.h"

AGS_DEController::AGS_DEController()
{
	ZoomSpeed = 300.0f;
}

void AGS_DEController::BeginPlay()
{
	Super::BeginPlay();

	EditorPawn = GetPawn();
	SetShowMouseCursor(true);

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* SubSystem =
			LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (nullptr != InputMappingContext)
			{
				SubSystem->AddMappingContext(InputMappingContext, 0);
			}
		}
	}
}
