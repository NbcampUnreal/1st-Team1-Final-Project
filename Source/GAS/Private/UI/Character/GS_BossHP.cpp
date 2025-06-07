// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Character/GS_BossHP.h"

#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Player/Guardian/GS_GuardianController.h"
#include "Components/VerticalBox.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "System/GS_PlayerState.h"
#include "UI/Character/GS_HPWidget.h"

void UGS_BossHP::NativeConstruct()
{
	Super::NativeConstruct();
	
	FTimerHandle BossWidgetTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(BossWidgetTimerHandle, this, &UGS_BossHP::InitBossHPWidget, 3.f);
}

void UGS_BossHP::InitBossHPWidget()
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
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("[CLIENT]------------Boss %s Stat %f"),*Guardian->GetName(), Guardian->GetStatComp()->GetCurrentHealth()),true, true, FLinearColor::Red,8.f);
		HPWidgetInstance->SetOwningActor(Guardian);
		HPWidgetInstance->InitializeHPWidget(Guardian->GetStatComp());
		HPWidgetList->AddChildToVerticalBox(HPWidgetInstance);
		SetVisibility(ESlateVisibility::Visible);
		
		// if (WidgetOwner)
		// {
		// 	if (!WidgetOwner->ActorHasTag("Guardian"))
		// 	{
		// 
		// 		//SetVisibility(ESlateVisibility::);
		// 	}
		// }
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
						//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("[CLIENT]------------Find BOSS %s"),*Guardian->GetName()),true, true, FLinearColor::Red,8.f);
						return true;
					}
					//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("[CLIENT]------------Not BOSS")),true, true, FLinearColor::Red,8.f);

				}
			}
		}
		return false;
	}
	return false;
}
