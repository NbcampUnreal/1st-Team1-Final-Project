// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Character/Player/GS_PawnMappingDataAsset.h"
#include "GS_LobbyDisplayActor.generated.h"

class USkeletalMeshComponent;
class UAnimInstance;
class UWidgetComponent;
class AGS_PlayerState;

UCLASS()
class GAS_API AGS_LobbyDisplayActor : public AActor
{
	GENERATED_BODY()
	
public:
    AGS_LobbyDisplayActor();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UWidgetComponent> UserInfoWidgetComponent;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> UserInfoWidgetClass;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
public:
	UPROPERTY(ReplicatedUsing = OnRep_SetupDisplay)
	TObjectPtr<USkeletalMesh> CurrentSkeletalMesh;

	UPROPERTY(ReplicatedUsing = OnRep_SetupDisplay)
	TSubclassOf<UAnimInstance> CurrentAnimClass;
    
	UPROPERTY(ReplicatedUsing = OnRep_SetupDisplay)
	TArray<FWeaponMeshPair> CurrentWeaponMeshList;

	UPROPERTY(ReplicatedUsing = OnRep_SetupDisplay)
	TArray<USkeletalMesh*> CurrentSubMeshList;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerState)
	TObjectPtr<AGS_PlayerState> AssociatedPlayerState;

	UPROPERTY(ReplicatedUsing = OnRep_ReadyState)
	bool bIsReady = false;

	// OnRep functions to apply replicated data on clients
	UFUNCTION()
	void OnRep_SetupDisplay();
    
	UFUNCTION()
	void OnRep_ReadyState();
	
	UFUNCTION()
	void OnRep_PlayerState();

private:
	FTimerHandle InitWidgetTimerHandle;

	void UpdateWidgetInfo();
};
