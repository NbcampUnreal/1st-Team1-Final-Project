#include "UI/Character/GS_HPTextWidgetComp.h"
#include "AI/RTS/GS_RTSController.h"
#include "UI/Character/GS_HPText.h"

void UGS_HPTextWidgetComp::InitWidget()
{
	Super::InitWidget();

	
	//Character(monster) -> HPText
	//Player -> Steam Name
	if (GetWidget())
	{
		if (GetWidget()->IsA<UGS_HPText>())
		{
			UGS_HPText* HPText = Cast<UGS_HPText>(GetWidget());
			if (IsValid(HPText))
			{
				HPText->SetOwningActor(GetOwner());

				APlayerController* PC = GetWorld()->GetFirstPlayerController();
				
				bool bIsEnemy = true;
				if (PC && PC->IsA<AGS_RTSController>())
				{
					bIsEnemy = false;
				}
				
				if (HPText->GetOwningActor()->ActorHasTag(("Monster")))
				{
					HPText->SetHPBarColor(bIsEnemy);
				}
				//return;
			}
		}
		// if (GetWidget()->IsA<UGS_SteamNameWidget>())
		// {
		// 	UGS_SteamNameWidget* NameWidget = Cast<UGS_SteamNameWidget>(GetWidget());
		// 	if (IsValid(NameWidget))
		// 	{
		// 		NameWidget->SetOwningActor(GetOwner());
		// 	}
		// }
	}
}