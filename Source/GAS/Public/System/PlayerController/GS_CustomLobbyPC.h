// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DungeonEditor/GS_DEController.h"
#include "GameFramework/PlayerController.h"
#include "System/GS_PlayerRole.h"
#include "System/GameState/GS_CustomLobbyGS.h"
#include "GS_CustomLobbyPC.generated.h"

class ADirectionalLight;
class UUserWidget;
class AGS_PlayerState;
class UGS_CustomLobbyUI;
class UGS_ArcaneBoardWidget;
class AGS_LobbyDisplayActor;
class AGS_SpawnSlot;
class UGS_PawnMappingDataAsset;

enum class EPendingWork : uint8
{
	None,
	JobSelection,
	ChangeRole
};

UCLASS()
class GAS_API AGS_CustomLobbyPC : public AGS_DEController
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

	UPROPERTY(VisibleAnywhere, Category = "Actors")
	TObjectPtr<ADirectionalLight> LobbyDirectionalLight;
	
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
	void RequestDungeonEditorToLobby();
	
	UFUNCTION(BlueprintCallable, Category = "Lobby Actions")
	void RequestToggleReadyStatus();

	UFUNCTION(Client, Reliable)
	void Client_UpdateDynamicButtonUI(EPlayerRole ForRole);
	UFUNCTION(Client, Reliable)
	void Client_UpdateReadyButtonUI(bool bIsReady);


	bool HasCurrentModalWidget();
	void ClearCurrentModalWidget();

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

public:
	//아케인보드 저장확인 관련
	void OnPerkSaveYes();
	void OnPerkSaveNo();
	bool CheckAndShowUnsavedChangesConfirm();
private:
	EPendingWork PendingWork;
	void ShowPerkSaveConfirmPopup();

protected:
	// 던전 에디터 관련
	// virtual void EnterEditorMode(AActor* SpawnPoint) override;
	// virtual void ExitEditorMode() override;

	// 서버가 클라이언트에게 모드 변경이 완료되었음을 알리는 RPC를 오버라이드
	virtual void Client_OnEnteredEditorMode_Implementation() override;
	virtual void Client_OnExitedEditorMode_Implementation() override;
	
	// 플레이어 스폰
private:
	UPROPERTY()
	TMap<TWeakObjectPtr<APlayerState>, TObjectPtr<AGS_LobbyDisplayActor>> SpawnedLobbyActors;

	// 스폰 슬롯 액터들을 캐싱하기 위한 배열
	UPROPERTY()
	TArray<TObjectPtr<AGS_SpawnSlot>> CachedGuardianSlots;

	UPROPERTY()
	TArray<TObjectPtr<AGS_SpawnSlot>> CachedSeekerSlots;

	UPROPERTY(EditDefaultsOnly, Category = "Lobby")
	TObjectPtr<UGS_PawnMappingDataAsset> PawnMappingData;

	// GameState의 플레이어 목록이 업데이트 될 때 호출될 함수
	UFUNCTION()
	void OnLobbyPlayerListUpdated();

	void SpawnOrUpdateLobbyActor(const FLobbyPlayerInfo& PlayerInfo);
	void CollectAndCacheSpawnSlots();

	// 던전 정보 넘기기
public:
	UFUNCTION(Client, Reliable)
	void Client_RequestLoadAndSendData();

protected:
	// 청크 크기 (예: 60KB, 네트워크 환경에 맞게 조절 필요)
	const int32 ChunkSize = 60 * 1024;

	/**
	* @brief 서버에서 여러 RPC 호출에 걸쳐 수신된 데이터 청크를 재조립하기 위한 임시 버퍼입니다.
	* 한 번에 한 명의 플레이어 데이터만 처리하므로 TMap이 아닌 단일 배열로 충분합니다.
	*/
	TArray<uint8> ReassembledDungeonData;
	
	UFUNCTION(Server, Reliable)
	void Server_ReceiveDungeonDataChunk(const TArray<uint8>& Chunk, bool bIsLast);

	// 클라이언트에서 데이터를 청크로 나눠 보내는 함수
	void SendDataInChunks(const TArray<uint8>& FullData);
};
