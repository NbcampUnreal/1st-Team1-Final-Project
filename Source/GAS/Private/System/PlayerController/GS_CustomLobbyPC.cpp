#include "System/PlayerController/GS_CustomLobbyPC.h"
#include "System/GS_PlayerState.h"
#include "Blueprint/UserWidget.h"
#include "System/GameMode/GS_CustomLobbyGM.h"

AGS_CustomLobbyPC::AGS_CustomLobbyPC()
	: CachedPlayerState(nullptr)
	, CurrentModalWidget(nullptr)
{
}

void AGS_CustomLobbyPC::BeginPlay()
{
	Super::BeginPlay();

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

void AGS_CustomLobbyPC::RequestToggleRole()
{
	AGS_PlayerState* PS = GetCachedPlayerState();
	if (PS)
	{
		if (!HasAuthority())
		{
			EPlayerRole NewRole = (PS->CurrentPlayerRole == EPlayerRole::PR_Seeker) ? EPlayerRole::PR_Guardian : EPlayerRole::PR_Seeker;
			PS->Server_SetPlayerRole(NewRole);
		}
		//else //이 부분은 리슨 서버 테스트용
		//{
		//	EPlayerRole NewRole = (PS->CurrentPlayerRole == EPlayerRole::PR_Seeker) ? EPlayerRole::PR_Guardian : EPlayerRole::PR_Seeker;
		//	PS->Server_SetPlayerRole_Implementation(NewRole);
		//}
	}
}

void AGS_CustomLobbyPC::RequestOpenJobSelectionPopup()
{
	AGS_PlayerState* PS = GetCachedPlayerState();
	if (PS || !JobSelectionWidgetClass) return;

	if (CurrentModalWidget && CurrentModalWidget->IsInViewport())
	{
		CurrentModalWidget->RemoveFromViewport();
		CurrentModalWidget = nullptr;
	}

	CurrentModalWidget = CreateWidget<UUserWidget>(this, JobSelectionWidgetClass);
	if (CurrentModalWidget)
	{
		CurrentModalWidget->AddToViewport();
		UE_LOG(LogTemp, Log, TEXT("Job Selection Popup Opened for role: %s"), *UEnum::GetValueAsString(PS->CurrentPlayerRole));
	}
}

void AGS_CustomLobbyPC::RequestOpenPerkOrDungeonPopup()
{
	AGS_PlayerState* PS = GetCachedPlayerState();
	if (!PS) return;

	if (CurrentModalWidget && CurrentModalWidget->IsInViewport())
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

	if (WidgetToOpen)
	{
		CurrentModalWidget = CreateWidget<UUserWidget>(this, WidgetToOpen);
		if (CurrentModalWidget)
		{
			CurrentModalWidget->AddToViewport();
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
		if (HasAuthority() == false)
		{
			PS->Server_SetSeekerJob(NewJob);
		}
		//else // 리슨서버 테스트용
		//{
		//	PS->Server_SetSeekerJob_Implementation(NewJob);
		//}
		
	}
}

void AGS_CustomLobbyPC::SelectGuardianJob(EGuardianJob NewJob)
{
	AGS_PlayerState* PS = GetCachedPlayerState();
	if (PS && PS->CurrentPlayerRole == EPlayerRole::PR_Guardian)
	{
		if (!HasAuthority())
		{
			PS->Server_SetGuardianJob(NewJob);
		}
		//else // 리슨서버 테스트용
		//{
		//	PS->Server_SetGuardianJob_Implementation(NewJob);
		//}
		
	}
}

void AGS_CustomLobbyPC::RequestToggleReadyStatus()
{
	AGS_PlayerState* PS = GetCachedPlayerState();
	if (PS)
	{
		if (!HasAuthority())
		{
			PS->Server_SetReadyStatus(!PS->bIsReady);
			UE_LOG(LogTemp, Log, TEXT("Client requested toggle ready status. Current status: %s, Requesting: %s"),
				PS->bIsReady ? TEXT("Ready") : TEXT("Not Ready"),
				!PS->bIsReady ? TEXT("Ready") : TEXT("Not Ready"));
		}
	}
}

void AGS_CustomLobbyPC::fdsa()
{
	if (IsLocalController())
	{

		GetWorldTimerManager().SetTimerForNextTick([this]() {
			AGS_PlayerState* PS = GetCachedPlayerState();
			if (PS)
			{
				UE_LOG(LogTemp, Warning, TEXT("AGS_CustomLobbyPC::BeginPlay (NextTick) - PlayerState is VALID (%s) for PC %s. Binding OnRoleChangedDelegate."), *PS->GetPlayerName(), *GetNameSafe(this));
				PS->OnRoleChangedDelegate.AddDynamic(this, &AGS_CustomLobbyPC::HandleRoleChanged);
				HandleRoleChanged(PS->CurrentPlayerRole); // 초기 UI 설정
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("AGS_CustomLobbyPC::BeginPlay (NextTick) - PlayerState is NULL for PC %s. Cannot bind OnRoleChangedDelegate."), *GetNameSafe(this));
			}
		});
	}
}

void AGS_CustomLobbyPC::Client_UpdateDynamicButtonUI_Implementation(EPlayerRole ForRole)
{
	//TODO: 버튼 텍스트 변경 로직 넣기

	UE_LOG(LogTemp, Log, TEXT("Client: Dynamic button UI should update for role: %s"), *UEnum::GetValueAsString(ForRole));
}
