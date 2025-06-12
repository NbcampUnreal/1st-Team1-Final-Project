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

	if (IsValid(GetOwningPlayer()->GetPawn()))
	{
		OwningCharacter = Cast<AGS_Character>(GetOwningPlayer()->GetPawn());
	}
	
	FTimerHandle WidgetTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(WidgetTimerHandle, this, &UGS_HPBoardWidget::InitBoardWidget, 3.f);
}

void UGS_HPBoardWidget::InitBoardWidget()
{
	if (!IsValid(HPWidgetClass) || !IsValid(HPWidgetList))
	{
		return;
	}
	
	HPWidgetList->ClearChildren();

	AGameStateBase* GS = UGameplayStatics::GetGameState(this);
	if (IsValid(GS))
	{
		TArray<APlayerState*> PSA = GS->PlayerArray; 
		for (APlayerState* PS : PSA)
		{
			AGS_PlayerState* GSPS = Cast<AGS_PlayerState>(PS);
			if (IsValid(GSPS))
			{
				UGS_HPWidget* HPWidget = CreateWidget<UGS_HPWidget>(this, HPWidgetClass);
				if (IsValid(HPWidget))
				{
					AGS_Character* Character = Cast<AGS_Character>(GSPS->GetPawn());

					//자기 자신 제외
					if (IsValid(Character) && Character != OwningCharacter)
					{
						//가디언 제외
						if (GSPS->CurrentPlayerRole == EPlayerRole::PR_Guardian)
						{
							continue;
						}
						
						//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("[CLIENT]-----%s-------Find Character???"),*Character->GetName()),true, true, FLinearColor::Red,8.f);
						HPWidget->SetOwningActor(Character);
						HPWidget->InitializeHPWidget(Character->GetStatComp());
						HPWidgetList->AddChildToVerticalBox(HPWidget);
					}
				}
			}
		}
	}
	
}