// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GS_InGameGS.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimerUpdatedDelegate, const FText&, FormattedTime);
/**
 * 
 */
UCLASS()
class GAS_API AGS_InGameGS : public AGameState
{
	GENERATED_BODY()

public:
	AGS_InGameGS();
	
	UPROPERTY(EditAnywhere, Category="Timer")
	float TotalGameTime;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentTime, BlueprintReadOnly, Category="Timer")
	float CurrentTime;
	
	void UpdateGameTime();
	FText GetFormattedTime() const;

	UPROPERTY(BlueprintAssignable, Category="Timer")
	FOnTimerUpdatedDelegate OnTimerUpdated;
	
protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UFUNCTION()
	void OnRep_CurrentTime();

	FTimerHandle GameTimeHandle;


public:
	UPROPERTY(ReplicatedUsing = OnRep_Loaded)
	int32 NumClientsLoaded = 0;
	UPROPERTY(Replicated)                 
	int32 NumClientsExpected = 0;

	DECLARE_MULTICAST_DELEGATE(FAllClientsLoaded);
	FAllClientsLoaded OnAllClientsLoaded;

	UFUNCTION()
	void OnRep_Loaded();
	void AddLoadedClient();

};
