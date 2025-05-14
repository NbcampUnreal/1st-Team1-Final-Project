#include "System/GS_PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "System/GS_PlayerRole.h"
#include "System/GameMode/GS_CustomLobbyGM.h"

AGS_PlayerState::AGS_PlayerState()
    : CurrentPlayerRole(EPlayerRole::PR_None)
    , CurrentSeekerJob(ESeekerJob::SJ_Job1)
    , CurrentGuardianJob(EGuardianJob::GJ_Job1)
    , bIsReady(false)
{
    bReplicates = true;
}

void AGS_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGS_PlayerState, CurrentPlayerRole);
	DOREPLIFETIME(AGS_PlayerState, CurrentSeekerJob);
	DOREPLIFETIME(AGS_PlayerState, CurrentGuardianJob);
    DOREPLIFETIME(AGS_PlayerState, bIsReady);
}

void AGS_PlayerState::InitializeDefaults()
{
    CurrentPlayerRole = EPlayerRole::PR_Seeker;
    CurrentSeekerJob = ESeekerJob::SJ_Job1;
	CurrentGuardianJob = EGuardianJob::GJ_Job1;
    bIsReady = false;

    if (GetNetMode() != NM_Client)
    {
		OnRep_PlayerRole();
		OnRep_SeekerJob();
		OnRep_GuardianJob();
        OnRep_IsReady();
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

bool AGS_PlayerState::Server_SetPlayerRole_Validate(EPlayerRole NewRole) { return true; }
void AGS_PlayerState::Server_SetPlayerRole_Implementation(EPlayerRole NewRole)
{
    if (CurrentPlayerRole != NewRole)
    {
		UE_LOG(LogTemp, Warning, TEXT("Server: Player %s changed role from %s to %s"), *GetPlayerName(), *UEnum::GetValueAsString(CurrentPlayerRole), *UEnum::GetValueAsString(NewRole));
        CurrentPlayerRole = NewRole;
        OnRep_PlayerRole();

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