#include "UI/Character/GS_SteamNameWidget.h"

#include "Character/Player/GS_Player.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/KismetSystemLibrary.h"

void UGS_SteamNameWidget::NativeConstruct()
{
	Super::NativeConstruct();

	FTimerHandle WidgetTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(WidgetTimerHandle, this, &UGS_SteamNameWidget::InitializeSteamNameWidget, 3.f, false);
}

void UGS_SteamNameWidget::InitializeSteamNameWidget()
{
	if (OwningActor)
	{
		AGS_Player* OwningPlayer = Cast<AGS_Player>(OwningActor);
		
		if (IsValid(OwningPlayer))
		{
			if (OwningPlayer->GetPlayerState())
			{
				SteamNameText->SetText(FText::FromString(OwningPlayer->GetPlayerState()->GetPlayerName()));
				UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("$@@@@@@@@@@@@@@@@ %s"),*OwningPlayer->GetPlayerState()->GetPlayerName()), true, true, FLinearColor::Red, 5.f);
			}
		}
	}
}
