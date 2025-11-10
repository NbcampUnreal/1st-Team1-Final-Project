#include "System/GS_PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "System/GS_PlayerRole.h"
#include "System/GameMode/GS_CustomLobbyGM.h"
#include "Character/Player/GS_Player.h"
#include "Character/Component/GS_StatComp.h"
#include "System/SteamAvatarHelper.h"
#include "DungeonEditor/Data/GS_DungeonEditorSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "DungeonEditor/Data/GS_DungeonEditorTypes.h"
#include "System/GameMode/GS_BossLevelGM.h"
#include "System/GameMode/GS_InGameGM.h"

AGS_PlayerState::AGS_PlayerState()
    : CurrentPlayerRole(EPlayerRole::PR_Seeker)
    , CurrentSeekerJob(ESeekerJob::Merci)
    , CurrentGuardianJob(EGuardianJob::Drakhar)
    , CurrentGameResult(EGameResult::GR_InProgress)
    , bIsReady(false)
	, CurrentHealth(99999.f)
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
    DOREPLIFETIME(AGS_PlayerState, MySteamAvatar);
    /*DOREPLIFETIME(AGS_PlayerState, ObjectData);*/
}

void AGS_PlayerState::BeginPlay()
{
    Super::BeginPlay();

    APlayerController* PC = GetPlayerController();
    if (PC && PC->IsLocalController())
    {
        FetchMySteamAvatar();
    }    
}
//
// void AGS_PlayerState::CopyProperties(APlayerState* NewPlayerState)
// {
//     Super::CopyProperties(NewPlayerState);
//
//     if (AGS_PlayerState* NewPS = Cast<AGS_PlayerState>(NewPlayerState))
//     {
//         // 다음 맵으로 넘길 애들 추가 까먹지 말기!!!!!!!!
//         NewPS->CurrentPlayerRole = CurrentPlayerRole;
//         NewPS->CurrentSeekerJob = CurrentSeekerJob;
//         NewPS->CurrentGuardianJob = CurrentGuardianJob;
//         NewPS->CurrentGameResult = CurrentGameResult;
//         NewPS->CurrentHealth = CurrentHealth;
//         NewPS->bIsAlive = bIsAlive;
//         NewPS->BoundStatComp = BoundStatComp;
//         NewPS->MySteamAvatar = MySteamAvatar;
//         if (GetWorld()->GetAuthGameMode<AGS_CustomLobbyGM>()
//             || GetWorld()->GetAuthGameMode<AGS_InGameGM>())
//         {
//             NewPS->ObjectData = ObjectData;
//         }
//     }
// }

void AGS_PlayerState::SeamlessTravelTo(APlayerState* NewPlayerState)
{
    Super::SeamlessTravelTo(NewPlayerState);

    if (AGS_PlayerState* NewPS = Cast<AGS_PlayerState>(NewPlayerState))
    {
        // 다음 맵으로 넘길 애들 추가 까먹지 말기!!!!!!!!
        NewPS->bHasInitializedStats  = bHasInitializedStats;
        NewPS->CurrentPlayerRole = CurrentPlayerRole;
        UE_LOG(LogTemp,Warning,TEXT("[PS 이동] 1. %s : NewRole = %d, CurRole = %d"), *GetPlayerName(), NewPS->CurrentPlayerRole, CurrentPlayerRole);
        NewPS->CurrentSeekerJob = CurrentSeekerJob;
        UE_LOG(LogTemp,Warning,TEXT("[PS 이동] 2. %s : NewSeekerJob = %d, CurSeekerJob = %d"), *GetPlayerName(), NewPS->CurrentSeekerJob, CurrentSeekerJob);
        NewPS->CurrentGuardianJob = CurrentGuardianJob;
        UE_LOG(LogTemp,Warning,TEXT("[PS 이동] 3. %s : NewGuardianJob = %d, CurGuardianJob = %d"), *GetPlayerName(), NewPS->CurrentGuardianJob, CurrentGuardianJob);
        NewPS->CurrentGameResult = CurrentGameResult;
        UE_LOG(LogTemp,Warning,TEXT("[PS 이동] 4. %s : NewGameResult = %d, CurGameResult = %d"), *GetPlayerName(), NewPS->CurrentGameResult, CurrentGameResult);
        NewPS->CurrentHealth = CurrentHealth;
        UE_LOG(LogTemp,Warning,TEXT("[PS 이동] 5. %s : NewCurrentHealth = %f, CurCurrentHealth = %f"), *GetPlayerName(), NewPS->CurrentHealth, CurrentHealth);
        NewPS->bIsAlive = bIsAlive;
        UE_LOG(LogTemp,Warning,TEXT("[PS 이동] 6. %s : NewIsAlive = %d, CurIsAlive = %d"), *GetPlayerName(), NewPS->bIsAlive, bIsAlive);
        NewPS->MySteamAvatar = MySteamAvatar;
        UE_LOG(LogTemp,Warning,TEXT("[PS 이동] 7. %s : MySteamAvatar복사 완료"), *GetPlayerName());
        if (GetWorld()->GetAuthGameMode<AGS_CustomLobbyGM>())
        {
            NewPS->ObjectData = ObjectData;
            UE_LOG(LogTemp,Warning,TEXT("[PS 이동] 8. %s : ObjectData복사 완료"), *GetPlayerName());
        }
    }
}

void AGS_PlayerState::FetchMySteamAvatar()
{
    FUniqueNetIdRepl MySteamId = USteamAvatarHelper::GetLocalSteamID();

    if (MySteamId.IsValid())
    {
        MySteamAvatar = USteamAvatarHelper::GetSteamAvatar(MySteamId, ESteamAvatarSize::SteamAvatar_Large);
    }
}

void AGS_PlayerState::InitializeDefaults()
{
	CurrentGameResult = EGameResult::GR_InProgress;
	CurrentHealth = 99999.f;
    bIsReady = false;
    bIsAlive = true;
    bHasInitializedStats = false;

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
    if (!HasAuthority())
    {
        return; // 체력 소스는 서버 기준
    }
    
    APawn* MyPawn = GetPawn();
    if (!MyPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnPawnStatInitialized: No Pawn for PS %s"), *GetPlayerName());
        return;
    }

    UGS_StatComp* StatComp = MyPawn->FindComponentByClass<UGS_StatComp>();
    if (!StatComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnPawnStatInitialized: No StatComp found on Pawn %s"), *MyPawn->GetName());
        return;
    }

    StatComp->SetCurrentHealth(CurrentHealth, /*bIsHealing=*/true);
    SetupStatCompBinding(StatComp);

    UE_LOG(LogTemp, Warning, TEXT("OnPawnStatInitialized: PS(%s) -> StatComp HP Sync Done. HP=%f"),
        *GetPlayerName(), CurrentHealth);
}

void AGS_PlayerState::SetupStatCompBinding(UGS_StatComp* InStatComp)
{
    if (InStatComp && InStatComp != BoundStatComp)
    {
        if (BoundStatComp)
        {
            BoundStatComp->OnCurrentHPChanged.RemoveAll(this);
        }
        
        BoundStatComp = InStatComp;
        BoundStatComp->OnCurrentHPChanged.AddUObject(this, &AGS_PlayerState::HandleCurrentHPChanged);
    }
}

void AGS_PlayerState::HandleCurrentHPChanged(UGS_StatComp* StatComp)
{
    if (!StatComp || StatComp != BoundStatComp)
        return;

    const float MaxHP = StatComp->GetMaxHealth();
    const float NewHP = StatComp->GetCurrentHealth();

    // 1) StatComp가 아직 제대로 안 올라온 상태는 무시
    if (MaxHP <= 0.f)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("HandleCurrentHPChanged: Ignoring HP update from uninitialized StatComp (MaxHP=0, HP=%f)"),
            NewHP);
        return;
    }

    // 2) 이미 살아있는 애가 맵 전환 직후 첫 이벤트로 0 찍히면 초기화 버그라고 보고 무시
    if (bHasInitializedStats && CurrentHealth > 0.f && NewHP <= 0.f)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("HandleCurrentHPChanged: Ignoring suspicious zero HP right after travel."));
        return;
    }

    CurrentHealth = FMath::Clamp(NewHP, 0.f, MaxHP);

    const bool bWasAlive = bIsAlive;
    bIsAlive = (CurrentHealth > 0.f);

    if (bWasAlive && !bIsAlive)
    {
        SetIsAlive(false);
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

        if (AGameMode* GM = GetWorld()->GetAuthGameMode<AGameMode>())
        {
            if (AGS_BossLevelGM* BGM = Cast<AGS_BossLevelGM>(GM))
            {
                BGM->HandlePlayerAliveStatusChanged(this, bIsAlive);
            }
            else if (AGS_InGameGM* IGM = Cast<AGS_InGameGM>(GM))
            {
                IGM->HandlePlayerAliveStatusChanged(this, bIsAlive);
            }
        }

        OnPlayerAliveStatusChangedDelegate.Broadcast(this, bIsAlive);

        UE_LOG(LogTemp, Warning, TEXT("AGS_PlayerState (%s) SetIsAlive called on Server. New State: %s"), *GetName(), bIsAlive ? TEXT("True") : TEXT("False"));
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

void AGS_PlayerState::Server_SetObjectData_Implementation(const TArray<FDESaveData>& InObjectData)
{
    this->ObjectData = InObjectData; 
    UE_LOG(LogTemp, Warning, TEXT("Server: Received and stored %d dungeon data objects for player %s."), ObjectData.Num(), *GetPlayerName());
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