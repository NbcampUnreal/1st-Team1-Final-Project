// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GS_BasePlayerController.generated.h"

class UInputAction;
struct FInputActionValue;
class UGS_InGameMenuUI;
class UGS_QuickManualUI;

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
	virtual void Client_StartGame_Implementation();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* MenuAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* KeyManualAction;

	void OpenMenuUI(const FInputActionValue& InputValue);
	void OpenKeyManual(const FInputActionValue& InputValue);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGS_InGameMenuUI> InGameMenuUIClass;

	UPROPERTY()
	UGS_InGameMenuUI* InGameMenuUI;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGS_QuickManualUI> QuickManualUIClass;

	UPROPERTY()
	UGS_QuickManualUI* QuickManualUI;

	UFUNCTION(Server, Reliable)
	void Server_NotifyPlayerIsReady();

	UFUNCTION(Client, Reliable)
	void Client_StartGame();

	UFUNCTION(Client, Reliable)
	void Client_ShowSeamlessLoadingCover();

	UFUNCTION(Client, Reliable)
	void Client_HideSeamlessLoadingCover();
};
