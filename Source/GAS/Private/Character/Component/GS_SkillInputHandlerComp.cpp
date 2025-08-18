// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Component/GS_SkillInputHandlerComp.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Character/Player/GS_Player.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Character/Skill/GS_SkillComp.h"

// Sets default values for this component's properties
UGS_SkillInputHandlerComp::UGS_SkillInputHandlerComp()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGS_SkillInputHandlerComp::SetupEnhancedInput(UInputComponent* PlayerInputComponent)
{
	if (!OwnerCharacter) OwnerCharacter = Cast<AGS_Player>(GetOwner());

	if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
	{
		if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (SkillMappingContext)
				{
					Subsystem->AddMappingContext(SkillMappingContext, MappingPriority);
					//UE_LOG(LogTemp, Warning, TEXT("Skill Mapping Context Added"));
				}
			}
		}
	}

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (IA_RightClick)
		{
			EnhancedInput->BindAction(IA_RightClick, ETriggerEvent::Started, this, &UGS_SkillInputHandlerComp::OnRightClick);
			EnhancedInput->BindAction(IA_RightClick, ETriggerEvent::Completed, this, &UGS_SkillInputHandlerComp::OnRightClickRelease);
		}
		if (IA_ModifierCtrl)
		{
			EnhancedInput->BindAction(IA_ModifierCtrl, ETriggerEvent::Started, this, &UGS_SkillInputHandlerComp::OnCtrlModifierStarted);
			EnhancedInput->BindAction(IA_ModifierCtrl, ETriggerEvent::Completed, this, &UGS_SkillInputHandlerComp::OnCtrlModifierEnded);
		}
		if (IA_LeftClick)
		{
			EnhancedInput->BindAction(IA_LeftClick, ETriggerEvent::Started, this, &UGS_SkillInputHandlerComp::OnLeftClick);
			EnhancedInput->BindAction(IA_LeftClick, ETriggerEvent::Completed, this, &UGS_SkillInputHandlerComp::OnLeftClickRelease);
		}
		if (IA_Scroll)
		{
			EnhancedInput->BindAction(IA_Scroll, ETriggerEvent::Triggered, this, &UGS_SkillInputHandlerComp::OnScroll);
		}
		if (IA_Roll)
		{
			EnhancedInput->BindAction(IA_Roll, ETriggerEvent::Started, this, &UGS_SkillInputHandlerComp::OnRoll);
		}
		if (IA_KeyReset)
		{
			EnhancedInput->BindAction(IA_KeyReset, ETriggerEvent::Started, this, &UGS_SkillInputHandlerComp::OnKeyReset);
		}
	}
}


void UGS_SkillInputHandlerComp::BeginPlay()
{
	Super::BeginPlay();
	if (!OwnerCharacter)
	{
		OwnerCharacter = Cast<AGS_Player>(GetOwner());
	}
	
	check(OwnerCharacter);
}

void UGS_SkillInputHandlerComp::OnRightClick(const FInputActionInstance& Instance)
{
	bWasCtrlHeldWhenLeftClicked = bCtrlHeld;

	if (OwnerCharacter->IsDead())
	{
		return;
	}
	
	if (!OwnerCharacter || !OwnerCharacter->GetSkillComp())
	{
		return;
	}

	if (bCtrlHeld)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Ctrl+Right Click"));
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("Right Click"));
	}
}

void UGS_SkillInputHandlerComp::OnLeftClick(const FInputActionInstance& Instance)
{
	bWasCtrlHeldWhenLeftClicked = bCtrlHeld;
	if (OwnerCharacter->IsDead())
	{
		return;
	}

	if (!OwnerCharacter || !OwnerCharacter->GetSkillComp())
	{
		return;
	}

	if (bCtrlHeld)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Ctrl+Left Click"));
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("Left Click"));
	}
}

void UGS_SkillInputHandlerComp::OnCtrlModifierStarted()
{
	bCtrlHeld = true;
	//UE_LOG(LogTemp, Warning, TEXT("Ctrl Click Start"));
}

void UGS_SkillInputHandlerComp::OnCtrlModifierEnded()
{
	bCtrlHeld = false;
	//UE_LOG(LogTemp, Warning, TEXT("Ctrl Click End"));
}

void UGS_SkillInputHandlerComp::OnRightClickRelease(const FInputActionInstance& Instance)
{
	if (OwnerCharacter->IsDead())
	{
		return;
	}
	//UE_LOG(LogTemp, Warning, TEXT("Right Click Release"));
}

void UGS_SkillInputHandlerComp::OnLeftClickRelease(const FInputActionInstance& Instance)
{
	if (OwnerCharacter->IsDead())
	{
		return;
	}
	// UE_LOG(LogTemp, Warning, TEXT("Left Click Release"));
}

void UGS_SkillInputHandlerComp::OnScroll(const FInputActionInstance& Instance)
{
	if (OwnerCharacter->IsDead())
	{
		return;
	}
	//UE_LOG(LogTemp, Warning, TEXT("Scroll Mouse"));
}

void UGS_SkillInputHandlerComp::OnRoll(const struct FInputActionInstance& Instance)
{
	return;
}

void UGS_SkillInputHandlerComp::OnKeyReset(const struct FInputActionInstance& Instance)
{
	UE_LOG(LogTemp, Warning, TEXT("OnKeyReset")); // SJE
	AGS_Seeker* Seeker = Cast<AGS_Seeker>(OwnerCharacter);
	Seeker->Server_RestKey();
	
	/*OwnerCharacter->SetSkillInputControl(true, true, true, true);
	OwnerCharacter->Server_RestKey();
	OwnerCharacter->Server_SetCanHitReact(true);*/

	
	
}

