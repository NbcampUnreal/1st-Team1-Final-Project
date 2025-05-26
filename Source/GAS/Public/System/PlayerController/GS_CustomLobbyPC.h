// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "System/GS_PlayerRole.h"
#include "GS_CustomLobbyPC.generated.h"

class UUserWidget;
class AGS_PlayerState;
class UUserWidget;
class UGS_CustomLobbyUI;

UCLASS()
class GAS_API AGS_CustomLobbyPC : public APlayerController
{
	GENERATED_BODY()

public:
	AGS_CustomLobbyPC();

protected:
	virtual void BeginPlay() override;
	//virtual void SetupInputComponent() override; // 키보드 기능. 나중에 esc 넣을때 주석 해제
	virtual void OnRep_PlayerState() override;

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
	UFUNCTION()
	void HandleReadyStatusChanged(bool bNewReadyStatus);

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
	UFUNCTION(Client, Reliable)
	void Client_UpdateReadyButtonUI(bool bIsReady);


	// 오버레이 초대
private:
	void UpdateRichPresenceForServerInvite();
	void ClearRichPresenceForServerInvite();

	bool bHasSetInitialRichPresence = false;

public:
	//addtoviewport
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowCustomLobbyUI();

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> CustomLobbyWidgetClass;
	UPROPERTY()
	UUserWidget* CustomLobbyWidgetInstance;
private:
	bool bHasInitializedUI = false;
	void TryBindToPlayerStateDelegates();
};
