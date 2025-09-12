// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Character/GS_CompassIndicatorComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "System/GS_PlayerState.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "UObject/UObjectGlobals.h"

UGS_CompassIndicatorComponent::UGS_CompassIndicatorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bShowOnCompass = true;
	CustomIcon = nullptr;
	IconColor = FLinearColor::White;
	MaxDisplayDistance = 10000.0f;
	bCheckPlayerStatus = true;
	bIsManuallyHidden = false;
}

FVector UGS_CompassIndicatorComponent::GetWorldLocation() const
{
	if (AActor* Owner = GetOwner())
	{
		return Owner->GetActorLocation();
	}
	return FVector::ZeroVector;
}

bool UGS_CompassIndicatorComponent::IsValidForCompass() const
{
	if (!bShowOnCompass || bIsManuallyHidden)
	{
		return false;
	}

	// If owner is a player pawn, check their status from PlayerState.
	if (bCheckPlayerStatus)
	{
		if (const APawn* OwnerPawn = Cast<APawn>(GetOwner()))
		{
			// If PlayerState is available, its bIsAlive status is the definitive answer.
			if (const AGS_PlayerState* PS = OwnerPawn->GetPlayerState<AGS_PlayerState>())
			{
				return PS->bIsAlive;
			}
			// If PlayerState is not yet replicated/available, assume the player is alive temporarily.
			// This prevents the icon from disappearing during network transitions.
			return true;
		}
	}

	// For non-player actors (like monsters), or if player status check is disabled,
	// visibility is determined by bShowOnCompass and bIsManuallyHidden only.
	return true;
}

ESeekerJob UGS_CompassIndicatorComponent::GetSeekerJob() const
{
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		if (AGS_PlayerState* PS = OwnerPawn->GetPlayerState<AGS_PlayerState>())
		{
			return PS->CurrentSeekerJob;
		}
	}
	return ESeekerJob::Ares; // Default fallback
}

FString UGS_CompassIndicatorComponent::GetPlayerName() const
{
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		if (APlayerState* PS = OwnerPawn->GetPlayerState())
		{
			return PS->GetPlayerName();
		}
	}
	return TEXT("Unknown");
}

bool UGS_CompassIndicatorComponent::IsPlayerAlive() const
{
	if (const APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		if (const AGS_PlayerState* PS = OwnerPawn->GetPlayerState<AGS_PlayerState>())
		{
			return PS->bIsAlive;
		}
		// Fallback for when PlayerState is not yet available
		return true;
	}
	// Not a pawn, so the concept of being "alive" in a player sense doesn't apply.
	return true;
}

TArray<UGS_CompassIndicatorComponent*> UGS_CompassIndicatorComponent::GetAllCompassIndicators(const UObject* WorldContext)
{
	TArray<UGS_CompassIndicatorComponent*> FoundComponents;
	
	if (!WorldContext)
	{
		return FoundComponents;
	}

	UWorld* World = WorldContext->GetWorld();
	if (!World)
	{
		return FoundComponents;
	}

	// Iterate through all actors in the world
	for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
	{
		AActor* Actor = *ActorIterator;
		if (IsValid(Actor))
		{
			// Skip actors that are not replicated to this client in multiplayer
			if (World->GetNetMode() != NM_Standalone)
			{
				// In multiplayer, only show actors that are properly replicated
				if (!Actor->GetIsReplicated() && Actor->GetRemoteRole() == ROLE_None)
				{
					continue;
				}
			}

			// Find compass indicator component
			if (UGS_CompassIndicatorComponent* CompassComponent = Actor->FindComponentByClass<UGS_CompassIndicatorComponent>())
			{
				// Additional multiplayer safety checks
				if (IsValid(CompassComponent) && CompassComponent->IsValidForCompass())
				{
					FoundComponents.Add(CompassComponent);
				}
			}
		}
	}
    
	return FoundComponents;
} 