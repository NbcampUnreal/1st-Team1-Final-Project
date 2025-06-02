// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/Notifies/GS_AnimNotifyFoleyEvent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Character/Component/GS_FootManagerComponent.h"
#include "GameFramework/Actor.h"

UGS_AnimNotifyFoleyEvent::UGS_AnimNotifyFoleyEvent()
{
	// Set default values
	bEnableFootstep = true;
	DetectionMethod = EFootDetectionMethod::VelocityBased;
	VelocityThreshold = 2.0f;

#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(100, 255, 100, 255); // Light green color for Foley event
#endif
}

void UGS_AnimNotifyFoleyEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!bEnableFootstep)
	{
		return;
	}

	TriggerAutoFootstep(MeshComp);
}

FString UGS_AnimNotifyFoleyEvent::GetNotifyName_Implementation() const
{
	return TEXT("Foley Event (Auto Footstep)");
}

void UGS_AnimNotifyFoleyEvent::TriggerAutoFootstep(USkeletalMeshComponent* MeshComp)
{
	if (!MeshComp || !MeshComp->GetOwner())
	{
		UE_LOG(LogTemp, Warning, TEXT("GS_AnimNotifyFoleyEvent: Invalid MeshComponent or Owner"));
		return;
	}

	UGS_FootManagerComponent* FootManager = MeshComp->GetOwner()->FindComponentByClass<UGS_FootManagerComponent>();
	if (!FootManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("GS_AnimNotifyFoleyEvent: No GS_FootManagerComponent found on actor %s"),
			*MeshComp->GetOwner()->GetName());
		return;
	}

	// Automatically detect and handle footstep
	FootManager->HandleFoleyEvent(); 
}

EFootStep UGS_AnimNotifyFoleyEvent::DetectFootByVelocity(USkeletalMeshComponent* MeshComp)
{
	// This logic is now centralized in GS_FootManagerComponent::DetectActiveFootstep()
	// For simplicity, we'll just return a default. 
	// A more robust implementation could query the FootManager or replicate detection logic.
	return EFootStep::LeftFoot; 
}

EFootStep UGS_AnimNotifyFoleyEvent::DetectFootByHeight(USkeletalMeshComponent* MeshComp)
{
	// This logic is now centralized in GS_FootManagerComponent::DetectActiveFootstep()
	return EFootStep::LeftFoot; 
} 