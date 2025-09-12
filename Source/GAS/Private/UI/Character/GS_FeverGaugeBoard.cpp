#include "UI/Character/GS_FeverGaugeBoard.h"

#include "Character/Player/GS_Player.h"
#include "Components/VerticalBox.h"
#include "Character/Player/Guardian/GS_Drakhar.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "System/GS_PlayerState.h"
#include "UI/Character/GS_DrakharFeverGauge.h"


void UGS_FeverGaugeBoard::NativeConstruct()
{
	Super::NativeConstruct();

	FTimerHandle BossWidgetTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(BossWidgetTimerHandle, this, &UGS_FeverGaugeBoard::InitDrakharFeverWidget, 3.f);
}

void UGS_FeverGaugeBoard::InitDrakharFeverWidget()
{
	if (!IsValid(DrakharFeverWidgetClass) || !IsValid(FeverWidgetList))
	{
		return;
	}

	FeverWidgetList->ClearChildren();
	
	if (FindDrakhar())
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("[CLIENT]------------Drakhar OK???")),true, true, FLinearColor::Red,8.f);
		DrakharFeverWidgetInstance = CreateWidget<UGS_DrakharFeverGauge>(this,  DrakharFeverWidgetClass);
		DrakharFeverWidgetInstance->SetOwningActor(Drakhar);
		DrakharFeverWidgetInstance->InitializeGauge(Drakhar->GetCurrentFeverGauge());
		FeverWidgetList->AddChildToVerticalBox(DrakharFeverWidgetInstance);
	}
}

bool UGS_FeverGaugeBoard::FindDrakhar()
{
	AGameStateBase* GS = UGameplayStatics::GetGameState(this);
	if (IsValid(GS))
	{
		TArray<APlayerState*> PSA = GS->PlayerArray; 
		for (APlayerState* PS : PSA)
		{
			AGS_PlayerState* GSPS = Cast<AGS_PlayerState>(PS);
			if (IsValid(GSPS))
			{
				AGS_Player* Player = Cast<AGS_Player>(GSPS->GetPawn());
				if (IsValid(Player))
				{
					if (Player->IsA<AGS_Drakhar>())
					{
						Drakhar = Cast<AGS_Drakhar>(Player);
						return true;
					}
				}
			}
		}
		return false;
	}
	return false;
}
