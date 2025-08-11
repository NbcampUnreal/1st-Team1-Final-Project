#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "System/GS_PlayerRole.h"
#include "DungeonEditor/Data/GS_DungeonEditorTypes.h"
#include "ResourceSystem/Aether/GS_AetherComp.h"
#include "GS_PlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoleChangedSignature, EPlayerRole, NewRole);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJobChangedSignature, EPlayerRole, CurrentRole);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReadyStatusChangedSignature, bool, bNewReadyStatus);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPlayerAliveStatusChangedSignature, AGS_PlayerState*, bool);

class UGS_StatComp;

UCLASS()
class GAS_API AGS_PlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
    AGS_PlayerState();

    void InitializeDefaults();
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void CopyProperties(APlayerState* OtherPlayerState) override;
    virtual void SeamlessTravelTo(APlayerState* NewPlayerState) override;

	// Steam Avatar
    UFUNCTION(BlueprintCallable, Category = "Avatar")
    void FetchMySteamAvatar();

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Avatar")
    UTexture2D* MySteamAvatar;

    // 역할
    UPROPERTY(ReplicatedUsing = OnRep_PlayerRole, BlueprintReadOnly, Category = "Lobby")
    EPlayerRole CurrentPlayerRole;
    UFUNCTION()
    void OnRep_PlayerRole();
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_SetPlayerRole(EPlayerRole NewRole);

    // Seeker
    UPROPERTY(ReplicatedUsing = OnRep_SeekerJob, BlueprintReadOnly, Category = "Lobby")
    ESeekerJob CurrentSeekerJob;
    UFUNCTION()
    void OnRep_SeekerJob();
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_SetSeekerJob(ESeekerJob NewJob);

    // Guardian
    UPROPERTY(ReplicatedUsing = OnRep_GuardianJob, BlueprintReadOnly, Category = "Lobby")
    EGuardianJob CurrentGuardianJob;
    UFUNCTION()
    void OnRep_GuardianJob();
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_SetGuardianJob(EGuardianJob NewJob);

    UPROPERTY(Replicated)
    TArray<FDESaveData> ObjectData;
    UFUNCTION(Server, Reliable)
	void Server_SetObjectData(const TArray<FDESaveData>& InObjectData);
    FString CurrentSaveSlotName = TEXT("Preset_0");

    //AetherComp 추가 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    TObjectPtr<UGS_AetherComp> AetherComp;

    UGS_AetherComp* GetAetherComp() const;


    // 준비
    UPROPERTY(ReplicatedUsing = OnRep_IsReady, BlueprintReadOnly, Category = "Lobby")
    bool bIsReady;
    UFUNCTION()
    void OnRep_IsReady();
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_SetReadyStatus(bool bNewReadyStatus);

    // 델리것
    UPROPERTY(BlueprintAssignable, Category = "Lobby|Events")
    FOnRoleChangedSignature OnRoleChangedDelegate;
    UPROPERTY(BlueprintAssignable, Category = "Lobby|Events")
    FOnJobChangedSignature OnJobChangedDelegate;
    UPROPERTY(BlueprintAssignable, Category = "Lobby|Events")
    FOnReadyStatusChangedSignature OnReadyStatusChangedDelegate;

    FOnPlayerAliveStatusChangedSignature OnPlayerAliveStatusChangedDelegate;

    //상태
    float CurrentHealth;
    UPROPERTY(ReplicatedUsing = OnRep_IsAlive, EditAnywhere, BlueprintReadWrite, Category = "PlayerStatus")
    bool bIsAlive = true;
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game Result")
    EGameResult CurrentGameResult;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UGS_StatComp> BoundStatComp;

    UFUNCTION()
    void OnRep_IsAlive();
    void SetIsAlive(bool bNewIsAlive);
    void SetupStatCompBinding(UGS_StatComp* InStatComp);
    void OnPawnStatInitialized();

    UFUNCTION(BlueprintCallable, Category = "PlayerState")
    void SetPlayerRole(EPlayerRole NewRole);

    UFUNCTION()
    void HandleCurrentHPChanged(UGS_StatComp* StatComp);

};