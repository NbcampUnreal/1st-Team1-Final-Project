// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Character/GS_CrossHairImage.h"
#include "Character/GS_Character.h"

void UGS_CrossHairImage::NativeConstruct()
{
	Super::NativeConstruct();
	OwningCharacter = Cast<AGS_Character>(GetOwningPlayer()->GetPawn());
}

void UGS_CrossHairImage::SetOwingActor(AGS_Character* InOwningCharacter)
{
	OwningCharacter = InOwningCharacter;
}
