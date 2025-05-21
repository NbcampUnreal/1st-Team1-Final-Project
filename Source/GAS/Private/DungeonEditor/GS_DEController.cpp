#include "DungeonEditor/GS_DEController.h"

#include "EngineUtils.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "UI/DungeonEditor/GS_PropWidget.h"

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

	// FindBuildManager In Level
	for (TActorIterator<AGS_BuildManager> It(GetWorld()); It; ++It)
	{
		BuildManagerRef = *It;
		break;
	}

	// Create Widget
	if (IsLocalController())
	{
		PropWidget = CreateWidget<UGS_PropWidget>(this, PropWidgetClass);
		if (PropWidget)
		{
			PropWidget->AddToViewport();
		}
	}
}
