#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "System/GS_PlayerRole.h"
#include "GS_CustomLobbyGS.generated.h"

class AGS_PlayerState;

USTRUCT(BlueprintType)
struct FLobbyPlayerInfo
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<AGS_PlayerState> PlayerState;
	UPROPERTY()
	FString PlayerName;
	UPROPERTY()
	EPlayerRole PlayerRole = EPlayerRole::PR_Seeker;
	UPROPERTY()
	ESeekerJob SeekerJob = ESeekerJob::Merci;
	UPROPERTY()
	EGuardianJob GuardianJob = EGuardianJob::Drakhar;
	UPROPERTY()
	int32 SlotIndex = -1;
	UPROPERTY()
	bool bIsReady = false;

};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerListUpdatedSignature);

UCLASS()
class GAS_API AGS_CustomLobbyGS : public AGameState
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable, Category = "Lobby")
	FOnPlayerListUpdatedSignature OnPlayerListUpdated;

	UPROPERTY(ReplicatedUsing=OnRep_PlayerList)
	TArray<FLobbyPlayerInfo> PlayerList;

	void AddPlayerToList(AGS_PlayerState* PlayerState, int32 SlotIndex);
	void RemovePlayerFromList(AGS_PlayerState* PlayerState);
	void UpdatePlayerInList(AGS_PlayerState* PlayerState, int32 NewSlotIndex = -1);

protected:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	UFUNCTION()
	void OnRep_PlayerList();
};
