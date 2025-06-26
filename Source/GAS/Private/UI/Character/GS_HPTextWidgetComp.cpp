#include "UI/Character/GS_HPTextWidgetComp.h"

#include "Kismet/KismetSystemLibrary.h"
#include "UI/Character/GS_HPText.h"
#include "UI/Character/GS_SteamNameWidget.h"

void UGS_HPTextWidgetComp::InitWidget()
{
	Super::InitWidget();

	if (GetWidget())
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%s@@@@@@@@@@@@@@"), *GetWidget()->GetName()), true, true, FLinearColor::Red, 5.f);
		if (GetWidget()->IsA<UGS_HPText>())
		{
			UGS_HPText* HPText = Cast<UGS_HPText>(GetWidget());
			if (IsValid(HPText))
			{
				HPText->SetOwningActor(GetOwner());
				return;
			}
		}
		if (GetWidget()->IsA<UGS_SteamNameWidget>())
		{
			UGS_SteamNameWidget* NameWidget = Cast<UGS_SteamNameWidget>(GetWidget());
			if (IsValid(NameWidget))
			{
				UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%s@@@@@@@@@@@@@@"), *NameWidget->GetName()), true, true, FLinearColor::Red, 5.f);
				NameWidget->SetOwningActor(GetOwner());
			}
		}
		// UGS_HPText* HPText = Cast<UGS_HPText>(GetWidget());
		// if (IsValid(HPText))
		// {
		// 	HPText->SetOwningActor(GetOwner());
		// 	return;
		// }
		//
		// UGS_SteamNameWidget* SteamNameWidget = Cast<UGS_SteamNameWidget>(GetWidget());
		// if (IsValid(SteamNameWidget))
		// {
		// 	SteamNameWidget->SetOwningActor(GetOwner());
		// }
	}
	
}

//Character -> HPText
//Player -> Steam Name