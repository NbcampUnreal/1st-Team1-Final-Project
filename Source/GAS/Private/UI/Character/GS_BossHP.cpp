#include "UI/Character/GS_BossHP.h"

#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Player/Guardian/GS_GuardianController.h"
#include "Components/VerticalBox.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "System/GS_PlayerState.h"
#include "UI/Character/GS_DrakharFeverGauge.h"
#include "UI/Character/GS_HPWidget.h"

void UGS_BossHP::NativeConstruct()
{
	Super::NativeConstruct();
	
	FTimerHandle BossWidgetTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(BossWidgetTimerHandle, this, &UGS_BossHP::InitGuardianHPWidget, 3.f);
}

void UGS_BossHP::InitGuardianHPWidget()
{
	if (!IsValid(HPWidgetClass) || !IsValid(HPWidgetList))
	{
		return;
	}
	
	HPWidgetList->ClearChildren();

	if (FindBoss())
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("[CLIENT]------------Boss OK???")),true, true, FLinearColor::Red,8.f);
		HPWidgetInstance = CreateWidget<UGS_HPWidget>(this, HPWidgetClass);
		HPWidgetInstance->SetOwningActor(Guardian);
		HPWidgetInstance->InitializeHPWidget(Guardian->GetStatComp());
		HPWidgetList->AddChildToVerticalBox(HPWidgetInstance);
		SetVisibility(ESlateVisibility::Visible);
	}
}

bool UGS_BossHP::FindBoss()
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
					if (Player->IsA<AGS_Guardian>())
					{
						Guardian = Cast<AGS_Guardian>(Player);
						return true;
					}
				}
			}
		}
		return false;
	}
	return false;
}
