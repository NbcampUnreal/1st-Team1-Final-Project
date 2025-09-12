// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GS_BasePlayerController.generated.h"

class UInputAction;
struct FInputActionValue;
class UGS_InGameMenuUI;

/**
 * 
 */
UCLASS()
class GAS_API AGS_BasePlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* MenuAction;

	void OpenMenuUI(const FInputActionValue& InputValue);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGS_InGameMenuUI> InGameMenuUIClass;

	UPROPERTY()
	UGS_InGameMenuUI* InGameMenuUI;
};
