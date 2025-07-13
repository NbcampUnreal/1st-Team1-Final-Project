// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "System/GS_BaseGM.h"
#include "GS_HJTestGM.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API AGS_HJTestGM : public AGS_BaseGM
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APlayerController> SeekerControllerClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APlayerController> GuardianControllerClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APlayerController> RTSControllerClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APawn> SeekerPawnClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APawn> GuardianPawnClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APawn> RTSPawnClass;

protected:
	UFUNCTION()
	virtual APlayerController* SpawnPlayerController(ENetRole InRemoteRole, const FString& Options) override;

	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;


};
