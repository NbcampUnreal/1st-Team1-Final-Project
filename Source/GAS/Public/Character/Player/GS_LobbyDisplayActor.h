// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GS_LobbyDisplayActor.generated.h"

class USkeletalMeshComponent;
class UAnimInstance;

UCLASS()
class GAS_API AGS_LobbyDisplayActor : public AActor
{
	GENERATED_BODY()
	
public:
    AGS_LobbyDisplayActor();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

public:
    UFUNCTION(NetMulticast, Reliable)
    void SetupDisplay(USkeletalMesh* NewMesh, TSubclassOf<UAnimInstance> NewAnimClass, const TArray<FWeaponMeshPair>& WeaponMeshList);

    UFUNCTION(NetMulticast, Reliable)
    void SetReadyState(bool bIsReady);
	
};
