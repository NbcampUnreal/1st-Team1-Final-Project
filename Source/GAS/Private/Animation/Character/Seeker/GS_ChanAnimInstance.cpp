// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Character/Seeker/GS_ChanAnimInstance.h"
#include "Character/Player/Seeker/GS_Chan.h"
#include "Animation/Character/Seeker/GS_ChooserInputObj.h"
#include "Character/E_Character.h"

void UGS_ChanAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	ChooserInputObj->CharacterType = ECharacterType::Chan;
}

void UGS_ChanAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
}

/*void UGS_ChanAnimInstance::AnimNotify_ComboInput()
{
	if (APawn* OwnerPawn = TryGetPawnOwner())
	{
		if (AGS_Chan* Chan = Cast<AGS_Chan>(OwnerPawn))
		{
			Chan->ComboInputOpen();
		}
	}
}*/