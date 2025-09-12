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
	
	// GameMode가 이 함수들을 호출하여 서버가 생성한 방 개수를 설정합니다.
	void SetDungeonData(int32 InTotalRoomCount);

protected:
	// 나중에 로딩 시스템의 기반이 될, 서버가 생성한 총 방의 개수입니다.
	UPROPERTY(Replicated)
	int32 TotalRoomCount;
	// 이 값이 true로 복제되면 클라이언트들이 방 생성이 완료되었는지 검사를 시작합니다.
	UPROPERTY(ReplicatedUsing = OnRep_DungeonDataReplicated)
	bool bDungeonDataReady;
	
	virtual void BeginPlay() override;

	void UpdateGameTime();

	UFUNCTION()
	void OnRep_CurrentTime();

	FTimerHandle GameTimeHandle;

	// bDungeonDataReady가 클라이언트에 복제될 때 호출될 함수입니다.
	UFUNCTION()
	void OnRep_DungeonDataReplicated();
	
	// 클라이언트 월드에 모든 방이 스폰되었는지 확인하는 함수입니다.
	void Client_VerifyRoomSpawning();
};
