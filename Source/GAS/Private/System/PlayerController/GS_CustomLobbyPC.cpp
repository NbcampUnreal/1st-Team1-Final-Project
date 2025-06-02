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


AGS_CustomLobbyPC::AGS_CustomLobbyPC()
	: CachedPlayerState(nullptr)
	, CurrentModalWidget(nullptr)
{
}

void AGS_CustomLobbyPC::BeginPlay()
{
	Super::BeginPlay();
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
	
	if (CurrentModalWidget && CurrentModalWidget->IsInViewport())
	{
		if (Cast<UGS_CharacterSelectList>(CurrentModalWidget))
		{
			// 나중에 하나로 기능을 묶는게 나을 것 같음.
			CurrentModalWidget->RemoveFromParent();
			CurrentModalWidget = nullptr;
			return;
		}
		CurrentModalWidget->RemoveFromParent();
		CurrentModalWidget = nullptr;
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

	if (CurrentModalWidget && CurrentModalWidget->GetParent())
	{
		CurrentModalWidget->RemoveFromParent();
		CurrentModalWidget = nullptr;
	}

	TSubclassOf<UUserWidget> WidgetToOpen = nullptr;
	FString LogMessage;

	if (PS->CurrentPlayerRole == EPlayerRole::PR_Seeker)
	{
		WidgetToOpen = SeekerPerkWidgetClass;
		LogMessage = TEXT("Seeker Perk UI Opened");
	}
	else
	{
		WidgetToOpen = GuardianDungeonWidgetClass;
		LogMessage = TEXT("Guardian Dungeon UI Opened");
	}

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
