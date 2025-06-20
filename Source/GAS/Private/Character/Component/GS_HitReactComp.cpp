// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Component/GS_HitReactComp.h"
#include "Character/Player/GS_Player.h"


// Sets default values for this component's properties
UGS_HitReactComp::UGS_HitReactComp()
{
	PrimaryComponentTick.bCanEverTick = false;

	AM_HitReacts.Init(nullptr, EHitReactType::TypeNum);
}

void UGS_HitReactComp::PlayHitReact(EHitReactType ReactType, FVector HitDirection)
{
	FName Section = CalculateHitDirection(HitDirection);
	
	if (AGS_Player* OwnerCharacter = Cast<AGS_Player>(GetOwner()))
	{
		OwnerCharacter->CanHitReact = false;
		OwnerCharacter->Multicast_PlaySkillMontage(AM_HitReacts[ReactType], Section);
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
		FVector Forward = OwnerCharacter->GetActorRotation().Vector();
		FVector Right = OwnerCharacter->GetActorRightVector();

		float ForwardDot = FVector::DotProduct(Forward, HitDirection);
		float RightDot = FVector::DotProduct(Right, HitDirection);
		UE_LOG(LogTemp, Warning, TEXT("Hit Direction : %s"), *HitDirection.ToString());
		UE_LOG(LogTemp, Warning, TEXT("Hit Forward Dot : %f / Right Dot : %f"), ForwardDot, RightDot);

		if (ForwardDot > 0.7f)
		{
			Section = FName("Forward");
		}
		else if (ForwardDot < -0.7f)
		{
			Section = FName("Right");
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

