// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Monster/GS_Monster.h"
#include "AI/GS_AIController.h"
#include "AI/RTS/GS_RTSController.h"
#include "Components/DecalComponent.h"

AGS_Monster::AGS_Monster()
{
	AIControllerClass = AGS_AIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	SelectionDecal = CreateDefaultSubobject<UDecalComponent>("SelectionDecal");
	SelectionDecal->SetupAttachment(RootComponent);
	SelectionDecal->SetVisibility(false);
}

void AGS_Monster::SetSelected(bool bIsSelected)
{
	if (SelectionDecal)
	{
		SelectionDecal->SetVisibility(bIsSelected);
	}
}

void AGS_Monster::NotifyActorOnClicked(FKey ButtonPressed)
{
	Super::NotifyActorOnClicked(ButtonPressed);
	
	if (ButtonPressed != EKeys::LeftMouseButton)
	{
		return;
	}
	
	if (AGS_RTSController* PC = Cast<AGS_RTSController>(GetWorld()->GetFirstPlayerController()))
	{
		if (SelectionDecal && SelectionDecal->IsVisible())
		{
			PC->RemoveUnitFromSelection(this);
		}
		else
		{
			PC->AddUnitToSelection(this);
		}
	}
}
