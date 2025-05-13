// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "System/GS_PlayerRole.h"
#include "GS_CustomLobbyPC.generated.h"

class UUserWidget;
class AGS_PlayerState;

UCLASS()
class GAS_API AGS_CustomLobbyPC : public APlayerController
{
	GENERATED_BODY()

public:
	AGS_CustomLobbyPC();

protected:
	virtual void BeginPlay() override;
	//virtual void SetupInputComponent() override; // 키보드 기능. 나중에 esc 넣을때 주석 해제

	UPROPERTY()
	AGS_PlayerState* CachedPlayerState;

	AGS_PlayerState* GetCachedPlayerState();
	
	//ui 관련
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> JobSelectionWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> SeekerPerkWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> GuardianDungeonWidgetClass;

	UPROPERTY()
	UUserWidget* CurrentModalWidget;

	UFUNCTION()
	void HandleRoleChanged(EPlayerRole NewRole);

public:
	UFUNCTION(BlueprintCallable, Category = "Lobby Actions")
	void RequestToggleRole();
	UFUNCTION(BlueprintCallable, Category = "Lobby Actions")
	void RequestOpenJobSelectionPopup();
	UFUNCTION(BlueprintCallable, Category = "Lobby Actions")
	void RequestOpenPerkOrDungeonPopup();
	UFUNCTION(BlueprintCallable, Category = "Lobby Actions")
	void SelectSeekerJob(ESeekerJob NewJob);
	UFUNCTION(BlueprintCallable, Category = "Lobby Actions")
	void SelectGuardianJob(EGuardianJob NewJob);

	UFUNCTION(BlueprintCallable, Category = "Lobby Actions")
	void RequestToggleReadyStatus();

	UFUNCTION(Client, Reliable)
	void Client_UpdateDynamicButtonUI(EPlayerRole ForRole);


	UFUNCTION()
	void fdsa();
};
