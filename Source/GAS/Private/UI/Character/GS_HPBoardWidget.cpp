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

	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("[CLIENT]--------------Start???")),true, true, FLinearColor::Red,5.f);

	FTimerHandle WidgetTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(WidgetTimerHandle, this, &UGS_HPBoardWidget::InitBoardWidget, 1.f);
}

void UGS_HPBoardWidget::InitBoardWidget()
{
	if (!IsValid(HPWidgetClass) || !IsValid(HPWidgetList))
	{
		return;
	}

	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("[CLIENT]--------------Init???")),true, true, FLinearColor::Red,8.f);

	HPWidgetList->ClearChildren();

	AGameStateBase* GS = UGameplayStatics::GetGameState(this);
	if (IsValid(GS))
	{
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("[CLIENT]--------------find GS%s???"),*GS->GetName()),true, true, FLinearColor::Red,8.f);

		TArray<APlayerState*> PSA = GS->PlayerArray; 
		
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("[CLIENT]--------------PlayerArray %d???"),GS->PlayerArray.Num()),true, true, FLinearColor::Red,8.f);

		for (APlayerState* PS : PSA)
		{
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("[CLIENT]--------------find PS???")),true, true, FLinearColor::Red,8.f);
			
			AGS_PlayerState* GSPS = Cast<AGS_PlayerState>(PS);
			if (IsValid(GSPS))
			{
				UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("[CLIENT]--------------find GSPS???")),true, true, FLinearColor::Red,8.f);

				UGS_HPWidget* HPWidget = CreateWidget<UGS_HPWidget>(this, HPWidgetClass);
				if (IsValid(HPWidget))
				{
					UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("[CLIENT]--------------find HPWidget???")),true, true, FLinearColor::Red,8.f);
					UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("[CLIENT]----%s--------find GSPS???"),*GSPS->GetName()),true, true, FLinearColor::Red,8.f);
					
					AGS_Character* Character = Cast<AGS_Character>(GSPS->GetPawn());
					if (IsValid(Character))
					{
						UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("[CLIENT]-----%s-------Find Character???"),*Character->GetName()),true, true, FLinearColor::Red,8.f);
						UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("[CLIENT]-------------Find Character???")),true, true, FLinearColor::Red,8.f);

						HPWidget->SetOwningActor(Character);
						HPWidget->InitializeHPWidget(Character->GetStatComp());
						HPWidgetList->AddChildToVerticalBox(HPWidget);
					}
				}
			}
		}
	}
	
}