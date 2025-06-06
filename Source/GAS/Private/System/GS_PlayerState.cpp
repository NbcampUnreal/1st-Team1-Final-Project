#include "System/GS_PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "System/GS_PlayerRole.h"
#include "System/GameMode/GS_CustomLobbyGM.h"
#include "Character/Player/GS_Player.h"
#include "Character/Component/GS_StatComp.h"

AGS_PlayerState::AGS_PlayerState()
    : CurrentPlayerRole(EPlayerRole::PR_None)
    , CurrentSeekerJob(ESeekerJob::Merci)
    , CurrentGuardianJob(EGuardianJob::Drakhar)
    , CurrentGameResult(EGameResult::GR_InProgress)
    , bIsReady(false)
	, CurrentHealth(99999.f)
	, bIsAlive(true)
	, BoundStatComp(nullptr)
{
    bReplicates = true;
}

void AGS_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGS_PlayerState, CurrentPlayerRole);
	DOREPLIFETIME(AGS_PlayerState, CurrentSeekerJob);
	DOREPLIFETIME(AGS_PlayerState, CurrentGuardianJob);
    DOREPLIFETIME(AGS_PlayerState, CurrentGameResult);
    DOREPLIFETIME(AGS_PlayerState, bIsReady);
    DOREPLIFETIME(AGS_PlayerState, bIsAlive);
}

void AGS_PlayerState::BeginPlay()
{
    Super::BeginPlay();
}

void AGS_PlayerState::CopyProperties(APlayerState* NewPlayerState)
{
    Super::CopyProperties(NewPlayerState);

    // this  == Old PlayerState
    // NewPlayerState == Newly-created PlayerState
    if (AGS_PlayerState* NewPS = Cast<AGS_PlayerState>(NewPlayerState))
    {
        // 다음 맵으로 넘길 애들 추가 까먹지 말기!!!!!!!!
        NewPS->CurrentPlayerRole = CurrentPlayerRole;
        NewPS->CurrentSeekerJob = CurrentSeekerJob;
        NewPS->CurrentGuardianJob = CurrentGuardianJob;
        NewPS->CurrentGameResult = CurrentGameResult;
        NewPS->CurrentHealth = CurrentHealth;
        NewPS->bIsAlive = bIsAlive;
        NewPS->BoundStatComp = BoundStatComp;
    }
}

void AGS_PlayerState::SeamlessTravelTo(APlayerState* NewPlayerState)
{
    Super::SeamlessTravelTo(NewPlayerState);

    // 여기서는 this == Old, NewPlayerState == New 와 동일.
    if (AGS_PlayerState* NewPS = Cast<AGS_PlayerState>(NewPlayerState))
    {
        // 다음 맵으로 넘길 애들 추가 까먹지 말기!!!!!!!!
        NewPS->CurrentPlayerRole = CurrentPlayerRole;
        NewPS->CurrentSeekerJob = CurrentSeekerJob;
        NewPS->CurrentGuardianJob = CurrentGuardianJob;
        NewPS->CurrentGameResult = CurrentGameResult;
        NewPS->CurrentHealth = CurrentHealth;
        NewPS->bIsAlive = bIsAlive;
        NewPS->BoundStatComp = BoundStatComp;
    }
}

void AGS_PlayerState::InitializeDefaults()
{
    CurrentPlayerRole = EPlayerRole::PR_Seeker;
    CurrentSeekerJob = ESeekerJob::Merci;
	CurrentGuardianJob = EGuardianJob::Drakhar;
	CurrentGameResult = EGameResult::GR_InProgress;
    bIsReady = false;
    bIsAlive = true;

    if (GetNetMode() != NM_Client)
    {
		OnRep_PlayerRole();
		OnRep_SeekerJob();
		OnRep_GuardianJob();
        OnRep_IsReady();
		OnRep_IsAlive();
    }
}

void AGS_PlayerState::OnRep_PlayerRole()
{
    UE_LOG(LogTemp, Warning, TEXT("AGS_PlayerState::OnRep_PlayerRole CALLED on %s. New Role: %s. NetMode: %d. IsLocalPlayerController: %s"),
        *GetPlayerName(),
        *UEnum::GetValueAsString(CurrentPlayerRole),
        (int32)GetNetMode(),
        (GetOwner() && GetOwner()->IsA<APlayerController>() && Cast<APlayerController>(GetOwner())->IsLocalController()) ? TEXT("true") : TEXT("false")
    );
    OnRoleChangedDelegate.Broadcast(CurrentPlayerRole);
	OnJobChangedDelegate.Broadcast(CurrentPlayerRole);
}

void AGS_PlayerState::OnPawnStatInitialized()
{
    APawn* MyPawn = GetPawn();

    if (MyPawn)
    {
        UE_LOG(LogTemp, Log, TEXT("AGS_PlayerState (%s): OnPawnStatInitialized - Found Pawn: %s (Class: %s)"),
            *GetName(), *MyPawn->GetName(), *MyPawn->GetClass()->GetName());

        UGS_StatComp* StatComp = MyPawn->FindComponentByClass<UGS_StatComp>();

        if (StatComp)
        {
            UE_LOG(LogTemp, Log, TEXT("AGS_PlayerState (%s): OnPawnStatInitialized - Found StatComp: %s on Pawn %s"),
                *GetName(), *StatComp->GetName(), *MyPawn->GetName());

            SetupStatCompBinding(StatComp);
            UE_LOG(LogTemp, Log, TEXT("AGS_PlayerState (%s): StatComp binding successful! from OnPawnStatInitialized"), *GetName()); // 성공 로그
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AGS_PlayerState (%s): OnPawnStatInitialized - StatComp NOT FOUND on Pawn: %s. This should not happen if Character called this."),
                *GetName(), *MyPawn->GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AGS_PlayerState (%s): OnPawnStatInitialized - MyPawn is NULL. This should not happen."), *GetName());
    }
}

void AGS_PlayerState::SetupStatCompBinding(UGS_StatComp* InStatComp)
{
    if (InStatComp && InStatComp != BoundStatComp)
    {
        if (BoundStatComp)
        {
            BoundStatComp->OnCurrentHPChanged.RemoveAll(this);
        }
        InStatComp->OnCurrentHPChanged.AddUObject(this, &AGS_PlayerState::HandleCurrentHPChanged);
        BoundStatComp = InStatComp;

        HandleCurrentHPChanged(BoundStatComp);
    }
}

void AGS_PlayerState::HandleCurrentHPChanged(UGS_StatComp* StatComp)
{
    if (StatComp)
    {
        float HealthFromStatComp = StatComp->GetCurrentHealth();
        UE_LOG(LogTemp, Warning, TEXT("AGS_PlayerState (%s) in HandleCurrentHPChanged: Value from StatComp->GetCurrentHealth() is: %f"),
            *GetName(), HealthFromStatComp);

        CurrentHealth = HealthFromStatComp;
        UE_LOG(LogTemp, Log, TEXT("AGS_PlayerState (%s) HP updated to %f (this is PlayerState.CurrentHealth)"), *GetName(), CurrentHealth);

        if (HasAuthority())
        {
            if (CurrentHealth <= 0.f && bIsAlive)
            {
                SetIsAlive(false);
            }
        }
    }
}

void AGS_PlayerState::OnRep_IsAlive()
{
    OnPlayerAliveStatusChangedDelegate.Broadcast(this, bIsAlive);
    UE_LOG(LogTemp, Log, TEXT("AGS_PlayerState (%s) OnRep_IsAlive called. bIsAlive = %s"),
        *GetName(),
        bIsAlive ? TEXT("True") : TEXT("False"));
}

void AGS_PlayerState::SetIsAlive(bool bNewIsAlive)
{
    if (HasAuthority() && bIsAlive != bNewIsAlive)
    {
        bIsAlive = bNewIsAlive;

        // 서버에서 델리게이트 브로드캐스트
        OnPlayerAliveStatusChangedDelegate.Broadcast(this, bIsAlive);

        // 서버에서도 OnRep_IsAlive를 호출하여 서버 측 로직을 동일하게 처리할 수 있음 (선택 사항)
        // OnRep_IsAlive(); 

        UE_LOG(LogTemp, Warning, TEXT("AGS_PlayerState (%s) SetIsAlive called on Server. New State: %s"),
            *GetName(),
            bIsAlive ? TEXT("True") : TEXT("False"));
    }
}

bool AGS_PlayerState::Server_SetPlayerRole_Validate(EPlayerRole NewRole) { return true; }
void AGS_PlayerState::Server_SetPlayerRole_Implementation(EPlayerRole NewRole)
{
    if (CurrentPlayerRole != NewRole)
    {
		UE_LOG(LogTemp, Warning, TEXT("Server: Player %s changed role from %s to %s"), *GetPlayerName(), *UEnum::GetValueAsString(CurrentPlayerRole), *UEnum::GetValueAsString(NewRole));
        CurrentPlayerRole = NewRole;
        OnRep_PlayerRole();

        if (AGS_CustomLobbyGM* GM = GetWorld()->GetAuthGameMode<AGS_CustomLobbyGM>())
        {
            GM->HandlePlayerStateUpdated(this);
        }

        if (bIsReady)
        {
            Server_SetReadyStatus(false);
        }
    }
}

void AGS_PlayerState::OnRep_SeekerJob()
{
    if (CurrentPlayerRole == EPlayerRole::PR_Seeker)
    {
        OnJobChangedDelegate.Broadcast(CurrentPlayerRole);
    }
}

bool AGS_PlayerState::Server_SetSeekerJob_Validate(ESeekerJob NewJob) { return true; }
void AGS_PlayerState::Server_SetSeekerJob_Implementation(ESeekerJob NewJob)
{
    if (CurrentPlayerRole == EPlayerRole::PR_Seeker && CurrentSeekerJob != NewJob)
    {
		UE_LOG(LogTemp, Warning, TEXT("Server: Player %s changed Seeker job from %s to %s"), *GetPlayerName(), *UEnum::GetValueAsString(CurrentSeekerJob), *UEnum::GetValueAsString(NewJob));
		CurrentSeekerJob = NewJob;
		OnRep_SeekerJob();

        if (AGS_CustomLobbyGM* GM = GetWorld()->GetAuthGameMode<AGS_CustomLobbyGM>())
        {
            GM->HandlePlayerStateUpdated(this);
        }

        if (bIsReady)
        {
            Server_SetReadyStatus(false);
        }
    }
}

void AGS_PlayerState::OnRep_GuardianJob()
{
    if (CurrentPlayerRole == EPlayerRole::PR_Guardian)
    {
		OnJobChangedDelegate.Broadcast(CurrentPlayerRole);
    }
}

bool AGS_PlayerState::Server_SetGuardianJob_Validate(EGuardianJob NewJob) { return true; }
void AGS_PlayerState::Server_SetGuardianJob_Implementation(EGuardianJob NewJob)
{
	if (CurrentPlayerRole == EPlayerRole::PR_Guardian && CurrentGuardianJob != NewJob)
	{
		UE_LOG(LogTemp, Warning, TEXT("Server: Player %s changed Guardian job from %s to %s"), *GetPlayerName(), *UEnum::GetValueAsString(CurrentGuardianJob), *UEnum::GetValueAsString(NewJob));
		CurrentGuardianJob = NewJob;
		OnRep_GuardianJob();

        if (AGS_CustomLobbyGM* GM = GetWorld()->GetAuthGameMode<AGS_CustomLobbyGM>())
        {
            GM->HandlePlayerStateUpdated(this);
        }

        if (bIsReady)
        {
            Server_SetReadyStatus(false);
        }
	}
}

void AGS_PlayerState::OnRep_IsReady()
{
    OnReadyStatusChangedDelegate.Broadcast(bIsReady);
}

bool AGS_PlayerState::Server_SetReadyStatus_Validate(bool bNewReadyStatus) { return true; }
void AGS_PlayerState::Server_SetReadyStatus_Implementation(bool bNewReadyStatus)
{
    if (bIsReady != bNewReadyStatus)
    {
		bIsReady = bNewReadyStatus;
        UE_LOG(LogTemp, Warning, TEXT("Server: Player %s set ready status to: %s"), *GetPlayerName(), bIsReady ? TEXT("Ready") : TEXT("Not Ready"));
        OnRep_IsReady();

		AGS_CustomLobbyGM* GM = GetWorld()->GetAuthGameMode<AGS_CustomLobbyGM>();
        if (GM)
        {
            GM->UpdatePlayerReadyStatus(this, bIsReady);
        }
    }
}