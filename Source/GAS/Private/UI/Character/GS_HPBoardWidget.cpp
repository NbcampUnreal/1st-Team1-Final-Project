#include "UI/Character/GS_HPBoardWidget.h"

#include "Character/GS_Character.h"
#include "Components/VerticalBox.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "System/GS_PlayerState.h"
#include "UI/Character/GS_HPWidget.h"

void UGS_HPBoardWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("[CLIENT]--------------Start???")));

	InitBoardWidget();
}

void UGS_HPBoardWidget::InitBoardWidget()
{
	if (!IsValid(HPWidgetClass) || !IsValid(HPWidgetList))
	{
		return;
	}

	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("[CLIENT]--------------Init???")));

	HPWidgetList->ClearChildren();

	AGameStateBase* GS = UGameplayStatics::GetGameState(this);
	if (IsValid(GS))
	{
		TArray<APlayerState*> PSA = GS->PlayerArray;

		for (APlayerState* PS : PSA)
		{
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("[CLIENT]--------------find PS???")));
			
			AGS_PlayerState* GSPS = Cast<AGS_PlayerState>(PS);
			if (IsValid(GSPS))
			{
				UGS_HPWidget* HPWidget = CreateWidget<UGS_HPWidget>(this, HPWidgetClass);
				if (IsValid(HPWidget))
				{
					UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("[CLIENT]--------------find HPWidget???")));
					UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("[CLIENT]----%s--------find GSPS???"),*GSPS->GetName()));
					
					AGS_Character* Character = Cast<AGS_Character>(GSPS->GetPawn());
					if (IsValid(Character))
					{
						UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("[CLIENT]-----%s-------Find Character???"),*Character->GetName()));
						UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("[CLIENT]-------------Find Character???")));

						HPWidget->SetOwningActor(Character);
						HPWidget->InitializeHPWidget(Character->GetStatComp());
						HPWidgetList->AddChildToVerticalBox(HPWidget);
					}
				}
			}
		}
	}
	
}