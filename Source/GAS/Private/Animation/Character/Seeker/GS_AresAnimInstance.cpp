// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Character/Seeker/GS_AresAnimInstance.h"
#include "Animation/Character/Seeker/GS_ChooserInputObj.h"
#include "Character/E_Character.h"

void UGS_AresAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	ChooserInputObj->CharacterType = ECharacterType::Merci;
}

void UGS_AresAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
}
