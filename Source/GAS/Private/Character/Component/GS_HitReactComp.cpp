// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Component/GS_HitReactComp.h"

#include "Animation/Character/GS_SeekerAnimInstance.h"
#include "Character/Player/GS_Player.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Character/Skill/GS_SkillBase.h"


// Sets default values for this component's properties
UGS_HitReactComp::UGS_HitReactComp()
{
	PrimaryComponentTick.bCanEverTick = false;

	AM_HitReacts.Init(nullptr, static_cast<int>(EHitReactType::TypeNum));
}

void UGS_HitReactComp::PlayHitReact(EHitReactType ReactType, FVector HitDirection)
{
	FName Section = CalculateHitDirection(HitDirection);

	if (AGS_Player* OwnerCharacter = Cast<AGS_Player>(GetOwner()))
	{
		if (ReactType == EHitReactType::Interrupt)
		{
			if (AGS_Seeker* OwnerSeeker = Cast<AGS_Seeker>(OwnerCharacter))
			{
				OwnerSeeker->GetSkillComp()->SkillsInterrupt();

				OwnerSeeker->Multicast_SetIsFullBodySlot(true);
				OwnerSeeker->Multicast_SetIsUpperBodySlot(false);

				OwnerSeeker->SetMoveControlValue(false, false);
				OwnerSeeker->SetSkillInputControl(false, false, false);

				OwnerCharacter->Multicast_PlaySkillMontage(AM_HitReacts[static_cast<int>(ReactType)], Section);
			}

			OwnerCharacter->Multicast_SetCanHitReact(false);
		}
		else if (ReactType == EHitReactType::Additive)
		{

			OwnerCharacter->Multicast_SetCanHitReact(false);
		}

	}
}


void UGS_HitReactComp::StopHitReact(UAnimMontage* TargetMontage)
{
	if (AGS_Player* OwnerCharacter = Cast<AGS_Player>(GetOwner()))
	{
		OwnerCharacter->Multicast_StopSkillMontage(TargetMontage);
	}
}

FName UGS_HitReactComp::CalculateHitDirection(FVector HitDirection)
{
	FName Section = NAME_None;
	
	if (AGS_Player* OwnerCharacter = Cast<AGS_Player>(GetOwner()))
	{
		FVector Front = OwnerCharacter->GetActorRotation().Vector();
		FVector Right = OwnerCharacter->GetActorRightVector();

		float FrontDot = FVector::DotProduct(Front, HitDirection);
		float RightDot = FVector::DotProduct(Right, HitDirection);

		if (FrontDot > 0.7f)
		{
			Section = FName("Front");
		}
		else if (FrontDot < -0.7f)
		{
			Section = FName("Back");
		}
		else if (RightDot > 0.0f)
		{
			Section = FName("Right");
		}
		else
		{
			Section = FName("Left");
		}
	}
	
	return Section;
}

// Called when the game starts
void UGS_HitReactComp::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

