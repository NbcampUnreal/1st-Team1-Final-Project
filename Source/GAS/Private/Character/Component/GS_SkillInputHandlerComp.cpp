// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Component/GS_SkillInputHandlerComp.h"
#include "Character/GS_Character.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Character/Skill/GS_SkillComp.h"

// Sets default values for this component's properties
UGS_SkillInputHandlerComp::UGS_SkillInputHandlerComp()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGS_SkillInputHandlerComp::SetupEnhancedInput(UInputComponent* PlayerInputComponent)
{
	if (!OwnerCharacter) return;

	if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
	{
		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (SkillMappingContext)
				{
					Subsystem->AddMappingContext(SkillMappingContext, MappingPriority);
					UE_LOG(LogTemp, Warning, TEXT("Skill Mapping Context Added"));
				}
			}
		}
	}

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (IA_RightClick)
		{
			EnhancedInput->BindAction(IA_RightClick, ETriggerEvent::Started, this, &UGS_SkillInputHandlerComp::OnRightClick);
		}
		if (IA_CtrlLeftClick)
		{
			EnhancedInput->BindAction(IA_CtrlLeftClick, ETriggerEvent::Started, this, &UGS_SkillInputHandlerComp::OnCtrlLeftClick);
		}
		if (IA_CtrlRightClick)
		{
			EnhancedInput->BindAction(IA_CtrlRightClick, ETriggerEvent::Started, this, &UGS_SkillInputHandlerComp::OnCtrlRightClick);
		}
	}
}


void UGS_SkillInputHandlerComp::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<AGS_Character>(GetOwner());
	check(OwnerCharacter);
}

void UGS_SkillInputHandlerComp::OnRightClick(const FInputActionInstance& Instance)
{
	UE_LOG(LogTemp, Warning, TEXT("Right Click"));
	if (!OwnerCharacter || !OwnerCharacter->GetSkillComp())
	{
		return;
	}
}

void UGS_SkillInputHandlerComp::OnCtrlLeftClick(const FInputActionInstance& Instance)
{
	UE_LOG(LogTemp, Warning, TEXT("Ctrl+Left Click"));
	if (!OwnerCharacter || !OwnerCharacter->GetSkillComp())
	{
		return;
	}
}

void UGS_SkillInputHandlerComp::OnCtrlRightClick(const FInputActionInstance& Instance)
{
	UE_LOG(LogTemp, Warning, TEXT("Ctrl+Right Click"));
	if (!OwnerCharacter || !OwnerCharacter->GetSkillComp())
	{
		return;
	}
}

