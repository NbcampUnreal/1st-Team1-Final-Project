// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GS_InGameGS.generated.h"

UCLASS()
class GAS_API AGS_InGameGS : public AGameState
{
	GENERATED_BODY()

public:
	AGS_InGameGS();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "Timer")
	FText GetFormattedTime() const;
	UFUNCTION(BlueprintPure, Category = "Timer")
	float GetRemainingTime() const;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Timer")
	float TotalGameTime;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentTime, BlueprintReadOnly, Category = "Timer")
	float CurrentTime;

	float LastServerTimeUpdate;

protected:
	virtual void BeginPlay() override;

	void UpdateGameTime();

	UFUNCTION()
	void OnRep_CurrentTime();

	FTimerHandle GameTimeHandle;
};
