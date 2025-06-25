#include "System/PlayerController/GS_CustomLobbyPC.h"
#include "System/GS_PlayerState.h"
#include "System/GameMode/GS_CustomLobbyGM.h"
#include "System/GameState/GS_InGameGS.h"
#include "UI/Screen/GS_CustomLobbyUI.h"
#include "System/GS_PlayerRole.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Kismet/GameplayStatics.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Engine/NetConnection.h"
#include "OnlineSubsystem.h"
#include "UI/Popup/GS_CharacterSelectList.h"
#include "RuneSystem/GS_ArcaneBoardManager.h"
#include "RuneSystem/GS_ArcaneBoardLPS.h"
#include "UI/RuneSystem/GS_ArcaneBoardWidget.h"
#include "System/GameState/GS_CustomLobbyGS.h"
#include "Character/Player/GS_LobbyDisplayActor.h"
#include "System/GS_SpawnSlot.h"
#include "Character/Player/GS_PawnMappingDataAsset.h"


AGS_CustomLobbyPC::AGS_CustomLobbyPC()
	: CachedPlayerState(nullptr)
	, CurrentModalWidget(nullptr)
	, PendingWork(EPendingWork::None)
{
}

void AGS_CustomLobbyPC::BeginPlay()
{
	Is_DEActive = false;
	
	Super::BeginPlay();

	if (IsLocalController())
	{
		TArray<AActor*> FoundCameras;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("LobbyCamera"), FoundCameras);

		if (FoundCameras.Num() > 0)
		{
			// 첫 번째로 찾은 카메라를 뷰 타겟으로 설정합니다.
			SetViewTargetWithBlend(FoundCameras[0]);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("LobbyCamera 태그를 가진 CameraActor를 찾을 수 없습니다."));
		}
		
		CollectAndCacheSpawnSlots();

		AGS_CustomLobbyGS* LGS = GetWorld()->GetGameState<AGS_CustomLobbyGS>();
		if (LGS)
		{
			LGS->OnPlayerListUpdated.AddDynamic(this, &AGS_CustomLobbyPC::OnLobbyPlayerListUpdated);
			OnLobbyPlayerListUpdated();
		}
	}
}

void AGS_CustomLobbyPC::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	UE_LOG(LogTemp, Warning, TEXT("AGS_CustomLobbyPC::OnRep_PlayerState CALLED on PC: %s. PlayerState Name: %s. IsLocalController: %s"),
		*GetNameSafe(this),
		PlayerState ? *PlayerState->GetPlayerName() : TEXT("NULL"),
		IsLocalController() ? TEXT("true") : TEXT("false")
	);

	if (IsLocalController() && PlayerState && !bHasInitializedUI)
	{
		ShowCustomLobbyUI();
		TryBindToPlayerStateDelegates();

		bHasInitializedUI = true;

		if (!bHasSetInitialRichPresence)
		{
			UE_LOG(LogTemp, Log, TEXT("AGS_CustomLobbyPC: PlayerState replicated. Setting initial Rich Presence."));
			UpdateRichPresenceForServerInvite();
			bHasSetInitialRichPresence = true;
		}
	}
	else if (IsLocalController() && PlayerState && bHasInitializedUI)
	{
		UE_LOG(LogTemp, Log, TEXT("AGS_CustomLobbyPC::OnRep_PlayerState - PlayerState changed after initial setup for PC: %s. Re-evaluating bindings/UI."), *GetNameSafe(this));
		TryBindToPlayerStateDelegates();
	}
}

void AGS_CustomLobbyPC::TryBindToPlayerStateDelegates()
{
	// 이 함수는 IsLocalController() && PlayerState 가 유효할 때 호출된다고 가정
	AGS_PlayerState* PS = GetPlayerState<AGS_PlayerState>(); // 캐스팅된 PlayerState 가져오기
	if (PS)
	{
		UE_LOG(LogTemp, Warning, TEXT("AGS_CustomLobbyPC::TryBindToPlayerStateDelegates - AGS_PlayerState is VALID (%s) for PC %s. Binding delegates."), *PS->GetPlayerName(), *GetNameSafe(this));

		PS->OnRoleChangedDelegate.RemoveDynamic(this, &AGS_CustomLobbyPC::HandleRoleChanged);
		PS->OnRoleChangedDelegate.AddDynamic(this, &AGS_CustomLobbyPC::HandleRoleChanged);

		PS->OnReadyStatusChangedDelegate.RemoveDynamic(this, &AGS_CustomLobbyPC::HandleReadyStatusChanged);
		PS->OnReadyStatusChangedDelegate.AddDynamic(this, &AGS_CustomLobbyPC::HandleReadyStatusChanged);


		// 델리게이트 바인딩 후 현재 상태로 UI 즉시 업데이트
		HandleRoleChanged(PS->CurrentPlayerRole);
		HandleReadyStatusChanged(PS->bIsReady);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AGS_CustomLobbyPC::TryBindToPlayerStateDelegates - Cast to AGS_PlayerState FAILED for PC %s."), *GetNameSafe(this));
	}
}

void AGS_CustomLobbyPC::UpdateRichPresenceForServerInvite()
{
	if (!IsLocalController())
	{
		return;
	}

	UNetConnection* ServerConnection = GetNetConnection(); //서버의 IP와 Port가 같이 딸려옴

	if (ServerConnection && !ServerConnection->URL.Host.IsEmpty() && ServerConnection->URL.Port > 0)
	{
		FString ServerIP = ServerConnection->URL.Host;
		int32 ServerPort = ServerConnection->URL.Port;

		UE_LOG(LogTemp, Log, TEXT("AGS_CustomLobbyPC: Updating Rich Presence for server invite. Server IP: %s, Port: %d"), *ServerIP, ServerPort);

		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			IOnlinePresencePtr PresenceInterface = OnlineSub->GetPresenceInterface();
			IOnlineIdentityPtr IdentityInterface = OnlineSub->GetIdentityInterface();

			if (PresenceInterface.IsValid() && IdentityInterface.IsValid())
			{
				TSharedPtr<const FUniqueNetId> UserId = IdentityInterface->GetUniquePlayerId(0);
				if (UserId.IsValid())
				{
					FOnlineUserPresenceStatus PresenceStatusData;
					PresenceStatusData.State = EOnlinePresenceState::Online;
					PresenceStatusData.StatusStr = FString(TEXT("GridGames_Alpha"));

					// "connect" 명령을 위한 Rich Presence 속성 추가
					FString ConnectCmd = FString::Printf(TEXT("+connect %s:%d"), *ServerIP, ServerPort);
					PresenceStatusData.Properties.Add(TEXT("connect"), FVariantData(ConnectCmd));

					UE_LOG(LogTemp, Log, TEXT("AGS_CustomLobbyPC: Setting Rich Presence for user %s with connect string: %s"), *UserId->ToString(), *ConnectCmd);

					PresenceInterface->SetPresence(
						*UserId,
						PresenceStatusData,
						IOnlinePresence::FOnPresenceTaskCompleteDelegate::CreateLambda(
							[](const FUniqueNetId& InUserId, const bool bSuccess)
							{
								UE_LOG(LogTemp, Log, TEXT("%s Rich Presence %s"),
									*InUserId.ToString(),
									bSuccess ? TEXT("OK") : TEXT("FAILED"));
							}
						)
					);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("AGS_CustomLobbyPC: Cannot get UniquePlayerId for Rich Presence."));
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("AGS_CustomLobbyPC: Online Presence or Identity interface is not valid."));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("AGS_CustomLobbyPC: OnlineSubsystem not found."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AGS_CustomLobbyPC: Not connected to a server or connection details are invalid. Cannot set Rich Presence for invite."));
		if (ServerConnection)
		{
			UE_LOG(LogTemp, Warning, TEXT("AGS_CustomLobbyPC: ServerConnection URL Host: '%s', Port: %d"), *ServerConnection->URL.Host, ServerConnection->URL.Port);
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("AGS_CustomLobbyPC: ServerConnection is NULL."));
		}
	}
}

void AGS_CustomLobbyPC::ClearRichPresenceForServerInvite()
{
	if (!IsLocalController())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("AGS_CustomLobbyPC: Clearing Rich Presence for server invite."));

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlinePresencePtr PresenceInterface = OnlineSub->GetPresenceInterface();
		IOnlineIdentityPtr IdentityInterface = OnlineSub->GetIdentityInterface();

		if (PresenceInterface.IsValid() && IdentityInterface.IsValid())
		{
			TSharedPtr<const FUniqueNetId> UserId = IdentityInterface->GetUniquePlayerId(0);
			if (UserId.IsValid())
			{
				FOnlineUserPresenceStatus PresenceStatusData;
				PresenceStatusData.State = EOnlinePresenceState::Online;
				PresenceStatusData.StatusStr = TEXT("온라인");      // 기본 메시지
				// connect 키를 **넣지 않으면** 기존 connect 값이 사라짐

				PresenceInterface->SetPresence(*UserId, PresenceStatusData);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Cannot get UniquePlayerId."));
			}
		}
	}
}

AGS_PlayerState* AGS_CustomLobbyPC::GetCachedPlayerState()
{
	if (!CachedPlayerState)
	{
		CachedPlayerState = GetPlayerState<AGS_PlayerState>();
	}
	return CachedPlayerState;
}

void AGS_CustomLobbyPC::HandleRoleChanged(EPlayerRole NewRole)
{
	if (IsLocalController())
	{
		UE_LOG(LogTemp, Warning, TEXT("LocalController: Role Changed to %s"), *UEnum::GetValueAsString(NewRole));
		Client_UpdateDynamicButtonUI(NewRole);

		if (CurrentModalWidget && CurrentModalWidget->IsInViewport())
		{
			CurrentModalWidget->RemoveFromParent();
			CurrentModalWidget = nullptr;
			//기존에 씨커 캐릭터 선택창 열려있다고 했을 때, 가디언으로 바꾸면 그 즉시 가디언 캐릭터 선택창 뜨게 하는 거 모르겠음
			//UX적으로 현재 구현 방식보다 설명한 방식이 좋긴 한데, 일단 이렇게 구현함
			//TODO: RemoveFromParent() 전에 저장하는 기능 넣어야 할듯
		}
	}
}

void AGS_CustomLobbyPC::HandleReadyStatusChanged(bool bNewReadyStatus)
{
	if (IsLocalController())
	{
		UE_LOG(LogTemp, Warning, TEXT("LocalController: Ready Status Changed to %s"), bNewReadyStatus ? TEXT("Ready") : TEXT("Not Ready"));
		Client_UpdateReadyButtonUI(bNewReadyStatus);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Server: Player %s is now %s"), *GetCachedPlayerState()->GetPlayerName(), bNewReadyStatus ? TEXT("Ready") : TEXT("Not Ready"));
	}
}

void AGS_CustomLobbyPC::RequestToggleRole()
{
	if (HasCurrentModalWidget())
	{
		if (CheckAndShowUnsavedChangesConfirm())
		{
			PendingWork = EPendingWork::ChangeRole;
			return;
		}
		else
		{
			ClearCurrentModalWidget();
		}
	}

	AGS_PlayerState* PS = GetCachedPlayerState();
	if (PS)
	{
		EPlayerRole NewRole = (PS->CurrentPlayerRole == EPlayerRole::PR_Seeker) ? EPlayerRole::PR_Guardian : EPlayerRole::PR_Seeker;
		PS->Server_SetPlayerRole(NewRole);
	}
}

void AGS_CustomLobbyPC::RequestOpenJobSelectionPopup()
{
	AGS_PlayerState* PS = GetCachedPlayerState();
	if (!PS || !JobSelectionWidgetClass)
	{
		if (!PS) UE_LOG(LogTemp, Warning, TEXT("AGS_CustomLobbyPC::RequestOpenJobSelectionPopup - PlayerState is NULL."));
		if (!JobSelectionWidgetClass) UE_LOG(LogTemp, Warning, TEXT("AGS_CustomLobbyPC::RequestOpenJobSelectionPopup - JobSelectionWidgetClass is NULL."));
		return;
	}
	
	if (CurrentModalWidget) //&& CurrentModalWidget->IsInViewport())
	{
		if (Cast<UGS_CharacterSelectList>(CurrentModalWidget))
		{
			ClearCurrentModalWidget();
			return;
		}

		if (CheckAndShowUnsavedChangesConfirm())
		{
			PendingWork = EPendingWork::JobSelection;
			return;
		}
		else
		{
			ClearCurrentModalWidget();
		}
	}

	CurrentModalWidget = CreateWidget<UUserWidget>(this, JobSelectionWidgetClass);
	if (CurrentModalWidget)
	{
		CurrentModalWidget->AddToViewport();
		CurrentModalWidget->SetPadding(FVector4(240.0, 100.0, 0.0, 0.0));
		if (UGS_CharacterSelectList* CharacterSelectList = Cast<UGS_CharacterSelectList>(CurrentModalWidget))
		{
			CharacterSelectList->CreateChildWidgets(PS->CurrentPlayerRole);
		}
		UE_LOG(LogTemp, Log, TEXT("Job Selection Popup Opened for role: %s"), *UEnum::GetValueAsString(PS->CurrentPlayerRole));
	}
}

void AGS_CustomLobbyPC::RequestOpenPerkOrDungeonPopup()
{
	AGS_PlayerState* PS = GetCachedPlayerState();
	if (!PS) return;

	if (CurrentModalWidget)// && CurrentModalWidget->GetParent())
	{
		if (Cast<UGS_CharacterSelectList>(CurrentModalWidget))
		{
			ClearCurrentModalWidget();
		}
		else
		{
			if (!CheckAndShowUnsavedChangesConfirm())
			{
				ClearCurrentModalWidget();
			}
			return;
		}
	}

	TSubclassOf<UUserWidget> WidgetToOpen = nullptr;
	FString LogMessage;

	if (PS->CurrentPlayerRole == EPlayerRole::PR_Seeker)
	{
		WidgetToOpen = SeekerPerkWidgetClass;
		LogMessage = TEXT("Seeker Perk UI Opened");

		UGS_CustomLobbyUI* LobbyUI = Cast<UGS_CustomLobbyUI>(CustomLobbyWidgetInstance);
		if (!LobbyUI) return;
		UOverlay* ModalOverlay = LobbyUI->GetModalOverlay();
		if (!ModalOverlay) return;

		if (WidgetToOpen)
		{
			CurrentModalWidget = CreateWidget<UUserWidget>(this, WidgetToOpen);
			if (CurrentModalWidget)
			{
				UOverlaySlot* OS = ModalOverlay->AddChildToOverlay(CurrentModalWidget);
				if (OS)
				{
					OS->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
					OS->SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
				}
				UE_LOG(LogTemp, Log, TEXT("%s"), *LogMessage);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to create Perk/Dungeon widget"));
		}
	}
	else
	{
		//// WidgetToOpen = GuardianDungeonWidgetClass;
		//CreateDEWidgets();
		//CustomLobbyWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
		//LogMessage = TEXT("Guardian Dungeon UI Opened");

		// 1. 던전 에디터 시작 위치를 찾습니다.
		// TArray<AActor*> FoundActors;
		// UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("DungeonEditorStart"), FoundActors);
		//
		// if (FoundActors.Num() > 0)
		// {
		// 	// 2. 에디터 모드 진입 함수를 호출합니다.
		// 	EnterEditorMode(FoundActors[0]);
		// }
		// else
		// {
		// 	UE_LOG(LogTemp, Error, TEXT("DungeonEditorStart 태그를 가진 액터를 찾을 수 없습니다."));
		// }

		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("DungeonEditorStart"), FoundActors);
		if (FoundActors.Num() > 0)
		{
			EnterEditorMode(FoundActors[0]);
		}
	}

	// UGS_CustomLobbyUI* LobbyUI = Cast<UGS_CustomLobbyUI>(CustomLobbyWidgetInstance);
	// if (!LobbyUI) return;
	// UOverlay* ModalOverlay = LobbyUI->GetModalOverlay();
	// if (!ModalOverlay) return;
	//
	// if (WidgetToOpen)
	// {
	// 	CurrentModalWidget = CreateWidget<UUserWidget>(this, WidgetToOpen);
	// 	if (CurrentModalWidget)
	// 	{
	// 		UOverlaySlot* OS = ModalOverlay->AddChildToOverlay(CurrentModalWidget);
	// 		if (OS)
	// 		{
	// 			OS->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
	// 			OS->SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
	// 		}
	// 		UE_LOG(LogTemp, Log, TEXT("%s"), *LogMessage);
	// 	}
	// }
	// else
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("Failed to create Perk/Dungeon widget"));
	// }
}

void AGS_CustomLobbyPC::SelectSeekerJob(ESeekerJob NewJob)
{
	AGS_PlayerState* PS = GetCachedPlayerState();
	if (PS && PS->CurrentPlayerRole == EPlayerRole::PR_Seeker)
	{
		PS->Server_SetSeekerJob(NewJob);
	}
}

void AGS_CustomLobbyPC::SelectGuardianJob(EGuardianJob NewJob)
{
	AGS_PlayerState* PS = GetCachedPlayerState();
	if (PS && PS->CurrentPlayerRole == EPlayerRole::PR_Guardian)
	{
		PS->Server_SetGuardianJob(NewJob);
	}
}

void AGS_CustomLobbyPC::RequestToggleReadyStatus()
{
	AGS_PlayerState* PS = GetCachedPlayerState();
	if (PS)
	{
		PS->Server_SetReadyStatus(!PS->bIsReady);
		UE_LOG(LogTemp, Log, TEXT("Client requested toggle ready status. Current status: %s, Requesting: %s"),
			PS->bIsReady ? TEXT("Ready") : TEXT("Not Ready"),
			!PS->bIsReady ? TEXT("Ready") : TEXT("Not Ready"));
	}
}

void AGS_CustomLobbyPC::ShowCustomLobbyUI()
{
	if (!IsLocalController()) return;

	if (CustomLobbyWidgetClass)
	{
		if (CustomLobbyWidgetInstance && CustomLobbyWidgetInstance->IsInViewport())
		{
			UE_LOG(LogTemp, Log, TEXT("AGS_CustomLobbyPC: CustomLobbyUI is already visible."));
			return;
		}
		if (CustomLobbyWidgetInstance) {
			CustomLobbyWidgetInstance->RemoveFromParent();
			CustomLobbyWidgetInstance = nullptr;
		}

		CustomLobbyWidgetInstance = CreateWidget<UUserWidget>(this, CustomLobbyWidgetClass);

		if (CustomLobbyWidgetInstance)
		{
			CustomLobbyWidgetInstance->AddToViewport();
			UE_LOG(LogTemp, Log, TEXT("AGS_CustomLobbyPC: CustomLobbyUI created and added to viewport."));

			FInputModeUIOnly InputModeData;
			//InputModeData.SetWidgetToFocus(CustomLobbyWidgetInstance->TakeWidget()); // 키보드 포커스 설정
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); // 마우스 락 해제
			SetInputMode(InputModeData);
			SetShowMouseCursor(true);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AGS_CustomLobbyPC: Failed to create CustomLobbyWidget instance."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AGS_CustomLobbyPC: CustomLobbyWidgetClass is not set in PlayerController's properties."));
	}
}

void AGS_CustomLobbyPC::Client_UpdateDynamicButtonUI_Implementation(EPlayerRole ForRole)
{
	UE_LOG(LogTemp, Log, TEXT("Client: Dynamic button UI should update for role: %s"), *UEnum::GetValueAsString(ForRole));

	if (CustomLobbyWidgetInstance)
	{
		UGS_CustomLobbyUI* LobbyUI = Cast<UGS_CustomLobbyUI>(CustomLobbyWidgetInstance);
		if (LobbyUI)
		{
			LobbyUI->UpdateRoleSpecificText(ForRole);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("AGS_CustomLobbyPC: CustomLobbyWidgetInstance is not of type UGS_CustomLobbyUI!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AGS_CustomLobbyPC: CustomLobbyWidgetInstance is NULL. Cannot update UI."));
	}
}

void AGS_CustomLobbyPC::Client_UpdateReadyButtonUI_Implementation(bool bIsReady)
{
	AGS_PlayerState* PS = GetCachedPlayerState();
	if (PS)
	{
		UE_LOG(LogTemp, Log, TEXT("Client: ReadyButton Text should update for Status: %s"), bIsReady ? TEXT("Ready") : TEXT("Not Ready"));
		if (CustomLobbyWidgetInstance)
		{
			UGS_CustomLobbyUI* LobbyUI = Cast<UGS_CustomLobbyUI>(CustomLobbyWidgetInstance);
			if (LobbyUI)
			{
				LobbyUI->UpdateReadyButtonText(bIsReady);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("AGS_CustomLobbyPC: CustomLobbyWidgetInstance is not of type UGS_CustomLobbyUI!"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("AGS_CustomLobbyPC: CustomLobbyWidgetInstance is NULL. Cannot update UI."));

		}
	}
	
}

bool AGS_CustomLobbyPC::HasCurrentModalWidget()
{
	if (CurrentModalWidget)
		return true;
	else
		return false;
}

void AGS_CustomLobbyPC::ClearCurrentModalWidget()
{
	if (CurrentModalWidget)
	{
		CurrentModalWidget->RemoveFromParent();
	}
	CurrentModalWidget = nullptr;
}

void AGS_CustomLobbyPC::ShowPerkSaveConfirmPopup()
{
	if (UGS_CustomLobbyUI* LobbyUI = Cast<UGS_CustomLobbyUI>(CustomLobbyWidgetInstance))
	{
		LobbyUI->ShowPerkSaveConfirmPopup();
	}
}

// void AGS_CustomLobbyPC::EnterEditorMode(AActor* SpawnPoint)
// {
// 	Super::EnterEditorMode(SpawnPoint);
//
// 	if (CustomLobbyWidgetInstance)
// 	{
// 		CustomLobbyWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
// 	}
// }
//
// void AGS_CustomLobbyPC::ExitEditorMode()
// {
// 	Super::ExitEditorMode();
//
// 	// 2. 로비 카메라를 다시 뷰 타겟으로 설정합니다.
// 	TArray<AActor*> FoundCameras;
// 	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("LobbyCamera"), FoundCameras);
// 	if (FoundCameras.Num() > 0)
// 	{
// 		SetViewTargetWithBlend(FoundCameras[0]);
// 	}
//
// 	// 3. 로비 UI를 다시 보여줍니다.
// 	if (CustomLobbyWidgetInstance)
// 	{
// 		CustomLobbyWidgetInstance->SetVisibility(ESlateVisibility::Visible);
// 		// 로비에 맞는 입력 모드로 다시 설정합니다.
// 		FInputModeUIOnly InputModeData;
// 		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
// 		SetInputMode(InputModeData);
// 		SetShowMouseCursor(true);
// 	}
// }

void AGS_CustomLobbyPC::Client_OnEnteredEditorMode_Implementation()
{
	// 부모의 클라이언트 로직 실행 (입력, 에디터 UI 생성 등)
	Super::Client_OnEnteredEditorMode_Implementation();

	// 로비 UI 숨기기
	if (CustomLobbyWidgetInstance)
	{
		CustomLobbyWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AGS_CustomLobbyPC::Client_OnExitedEditorMode_Implementation()
{
	// 부모의 클라이언트 로직 실행 (입력 초기화, 에디터 UI 제거 등)
	Super::Client_OnExitedEditorMode_Implementation();

	// 로비 카메라로 뷰 타겟 변경
	TArray<AActor*> FoundCameras;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("LobbyCamera"), FoundCameras);
	if (FoundCameras.Num() > 0)
	{
		SetViewTargetWithBlend(FoundCameras[0]);
	}

	// 로비 UI 보이기
	if (CustomLobbyWidgetInstance)
	{
		CustomLobbyWidgetInstance->SetVisibility(ESlateVisibility::Visible);
        
		// 로비에 맞는 입력 모드로 복귀
		FInputModeUIOnly InputModeData;
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputModeData);
		SetShowMouseCursor(true);
	}
}

void AGS_CustomLobbyPC::RequestDungeonEditorToLobby()
{
	if (IsLocalController())
	{
		ExitEditorMode();
		
		TArray<AActor*> FoundCameras;
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("LobbyCamera"), FoundCameras);

		if (FoundCameras.Num() > 0)
		{
			// 첫 번째로 찾은 카메라를 뷰 타겟으로 설정합니다.
			SetViewTargetWithBlend(FoundCameras[0]);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("LobbyCamera 태그를 가진 CameraActor를 찾을 수 없습니다."));
		}
		
		ShowCustomLobbyUI();
	}
}

void AGS_CustomLobbyPC::OnLobbyPlayerListUpdated()
{
	if (!IsLocalController()) return;

	AGS_CustomLobbyGS* LGS = GetWorld()->GetGameState<AGS_CustomLobbyGS>();
	if (!LGS)
	{
		UE_LOG(LogTemp, Error, TEXT("PC::OnLobbyPlayerListUpdated - GameState가 유효하지 않습니다."));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("PC::OnLobbyPlayerListUpdated - 호출됨. PlayerList 수: %d"), LGS->PlayerList.Num());

	TSet<TWeakObjectPtr<APlayerState>> PlayersInLobby;

	// GameState의 최신 목록을 기반으로 액터 스폰 또는 업데이트
	for (const FLobbyPlayerInfo& PlayerInfo : LGS->PlayerList)
	{
		if (PlayerInfo.PlayerState.IsValid())
		{
			PlayersInLobby.Add(PlayerInfo.PlayerState);
			SpawnOrUpdateLobbyActor(PlayerInfo);
		}
	}

	// 로컬에만 존재하고 GameState 목록에는 없는 액터(나간 플레이어) 정리
	TArray<TWeakObjectPtr<APlayerState>> ActorsToRemove;
	for (auto& Pair : SpawnedLobbyActors)
	{
		if (!PlayersInLobby.Contains(Pair.Key))
		{
			if (Pair.Value)
			{
				Pair.Value->Destroy();
			}
			ActorsToRemove.Add(Pair.Key);
		}
	}

	for (TWeakObjectPtr<APlayerState> Key : ActorsToRemove)
	{
		SpawnedLobbyActors.Remove(Key);
	}
}

void AGS_CustomLobbyPC::SpawnOrUpdateLobbyActor(const FLobbyPlayerInfo& PlayerInfo)
{
	if (!PlayerInfo.PlayerState.IsValid() || !PawnMappingData) return;
	UE_LOG(LogTemp, Log, TEXT("====== SpawnOrUpdateLobbyActor 호출 시작: %s ======"), *PlayerInfo.PlayerName);

	// TWeakObjectPtr를 키로 사용하므로 바로 찾기
	TObjectPtr<AGS_LobbyDisplayActor>* FoundActorPtr = SpawnedLobbyActors.Find(PlayerInfo.PlayerState);

	// 이미 스폰된 액터가 있는지 확인
	if (FoundActorPtr && *FoundActorPtr)
	{
		(*FoundActorPtr)->Destroy();
		SpawnedLobbyActors.Remove(PlayerInfo.PlayerState);
		UE_LOG(LogTemp, Warning, TEXT("--> %s의 기존 액터를 파괴하고 새로 스폰합니다."), *PlayerInfo.PlayerName);
	}

	// 스폰할 위치 찾기
	TArray<TObjectPtr<AGS_SpawnSlot>>& SlotsToSearch = (PlayerInfo.PlayerRole == EPlayerRole::PR_Guardian) ? CachedGuardianSlots : CachedSeekerSlots;
	AGS_SpawnSlot* SpawnSlot = nullptr;
	UE_LOG(LogTemp, Log, TEXT("--> 역할(%s)에 맞는 스폰 슬롯을 찾습니다. (요구 인덱스: %d)"), (PlayerInfo.PlayerRole == EPlayerRole::PR_Guardian) ? TEXT("가디언") : TEXT("시커"), PlayerInfo.SlotIndex);

	for (AGS_SpawnSlot* Slot : SlotsToSearch)
	{
		if (Slot && Slot->GetSlotIndex() == PlayerInfo.SlotIndex)
		{
			SpawnSlot = Slot;
			break;
		}
	}

	if (!SpawnSlot)
	{
		UE_LOG(LogTemp, Error, TEXT("--> SpawnOrUpdateLobbyActor 실패 (%s): 인덱스 %d에 해당하는 스폰 슬롯을 찾지 못했습니다."), *PlayerInfo.PlayerName, PlayerInfo.SlotIndex);
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("--> 스폰 슬롯 찾음: %s (위치: %s)"), *SpawnSlot->GetName(), *SpawnSlot->GetActorLocation().ToString());

	// 스폰할 에셋 정보 찾기
	const FAssetToSpawn* SpawnAssetInfo = nullptr;
	if (PlayerInfo.PlayerRole == EPlayerRole::PR_Guardian)
	{
		SpawnAssetInfo = PawnMappingData->GuardianPawnClasses.Find(PlayerInfo.GuardianJob);
	}
	else
	{
		SpawnAssetInfo = PawnMappingData->SeekerPawnClasses.Find(PlayerInfo.SeekerJob);
	}

	if (!SpawnAssetInfo || !SpawnAssetInfo->DisplayActorClass) return;
	UE_LOG(LogTemp, Log, TEXT("--> 에셋 정보 찾음. 스폰할 DisplayActor 클래스: %s"), *SpawnAssetInfo->DisplayActorClass->GetName());

	// 액터 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	FTransform SpawnTransform = SpawnSlot->GetActorTransform();

	UE_LOG(LogTemp, Log, TEXT("--> LobbyDisplayActor 스폰 시도..."));
	AGS_LobbyDisplayActor* NewDisplayActor = GetWorld()->SpawnActor<AGS_LobbyDisplayActor>(SpawnAssetInfo->DisplayActorClass, SpawnTransform, SpawnParams);

	if (NewDisplayActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("--> 스폰 성공 (%s)! 외형 설정을 진행합니다."), *PlayerInfo.PlayerName);
		NewDisplayActor->SetupDisplay(SpawnAssetInfo->SkeletalMeshClass, SpawnAssetInfo->Lobby_AnimBlueprintClass, SpawnAssetInfo->WeaponMeshList);
		NewDisplayActor->SetReadyState(PlayerInfo.bIsReady);
		SpawnedLobbyActors.Add(PlayerInfo.PlayerState, NewDisplayActor);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("--> 스폰 실패 (%s)!"), *PlayerInfo.PlayerName);
	}
}

void AGS_CustomLobbyPC::CollectAndCacheSpawnSlots()
{
	CachedGuardianSlots.Empty();
    CachedSeekerSlots.Empty();
    
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGS_SpawnSlot::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        AGS_SpawnSlot* Slot = Cast<AGS_SpawnSlot>(Actor);
        if (Slot)
        {
            if (Slot->GetRole() == EPlayerRole::PR_Guardian)
            {
                CachedGuardianSlots.Add(Slot);
            }
            else if (Slot->GetRole() == EPlayerRole::PR_Seeker)
            {
                CachedSeekerSlots.Add(Slot);
            }
        }
    }

    // 인덱스 순으로 정렬
    auto SortSlots = [](const TObjectPtr<AGS_SpawnSlot>& A, const TObjectPtr<AGS_SpawnSlot>& B) {
        if (!A || !B) return false;
        return A->GetSlotIndex() < B->GetSlotIndex();
    };
    CachedGuardianSlots.Sort(SortSlots);
    CachedSeekerSlots.Sort(SortSlots);
}

bool AGS_CustomLobbyPC::CheckAndShowUnsavedChangesConfirm()
{
	if (CurrentModalWidget)
	{
		if (Cast<UGS_ArcaneBoardWidget>(CurrentModalWidget))
		{
			if (UGS_ArcaneBoardLPS* ArcaneBoardLPS = GetLocalPlayer()->GetSubsystem<UGS_ArcaneBoardLPS>())
			{
				if (ArcaneBoardLPS->HasUnsavedChanges())
				{
					ShowPerkSaveConfirmPopup();
					return true;
				}
			}
		}

		// 던전에디터 체크 (나중에 추가)
		
	}
	return false;
}

void AGS_CustomLobbyPC::OnPerkSaveYes()
{
	if (UGS_ArcaneBoardLPS* ArcaneBoardLPS = GetLocalPlayer()->GetSubsystem<UGS_ArcaneBoardLPS>())
	{
		ArcaneBoardLPS->ApplyBoardChanges();
	}

	ClearCurrentModalWidget();

	switch (PendingWork)
	{
	case EPendingWork::JobSelection:
		RequestOpenJobSelectionPopup();
		break;
	case EPendingWork::ChangeRole:
		RequestToggleRole();
		break;
	default:
		break;
	}

	PendingWork = EPendingWork::None;
}

void AGS_CustomLobbyPC::OnPerkSaveNo()
{
	ClearCurrentModalWidget();

	switch (PendingWork)
	{
	case EPendingWork::JobSelection:
		RequestOpenJobSelectionPopup();
		break;
	case EPendingWork::ChangeRole:
		RequestToggleRole();
		break;
	default:
		break;
	}

	PendingWork = EPendingWork::None;
}
