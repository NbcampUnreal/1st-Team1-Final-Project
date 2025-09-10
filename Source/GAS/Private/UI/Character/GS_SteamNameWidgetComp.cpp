#include "UI/Character/GS_SteamNameWidgetComp.h"

#include "UI/Character/GS_SteamNameWidget.h"


void UGS_SteamNameWidgetComp::BeginPlay()
{
	Super::BeginPlay();

	if (GetWidget())
	{
		if (GetWidget()->IsA<UGS_SteamNameWidget>())
		{
			UGS_SteamNameWidget* NameWidget = Cast<UGS_SteamNameWidget>(GetWidget());
			if (IsValid(NameWidget))
			{
				NameWidget->SetOwningActor(GetOwner());
			}
		}
	}
}
