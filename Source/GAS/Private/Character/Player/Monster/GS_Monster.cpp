// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Monster/GS_Monster.h"
#include "AI/GS_AIController.h"
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
