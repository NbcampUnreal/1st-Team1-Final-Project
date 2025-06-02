// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/Debuff/EDebuffType.h"
#include "GS_DebuffComp.generated.h"

class UGS_DebuffBase;
struct FDebuffData;
class AGS_Character;

USTRUCT(BlueprintType)
struct FDebuffRepInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	EDebuffType Type;

	UPROPERTY(BlueprintReadOnly)
	float RemainingTime;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAS_API UGS_DebuffComp : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UGS_DebuffComp();

	// 디버프 적용
	void ApplyDebuff(EDebuffType Type, AGS_Character* Attacker);
	void RemoveDebuff(EDebuffType Type);

	// Type 디버프가 있는지 확인
	bool IsDebuffActive(EDebuffType Type);

	UPROPERTY(ReplicatedUsing = OnRep_DebuffList)
	TArray<FDebuffRepInfo> ReplicatedDebuffs;

	UFUNCTION()
	void OnRep_DebuffList();

	void ClearAllDebuffs();

protected:
	const FDebuffData* GetDebuffData(EDebuffType Type) const;
	UGS_DebuffBase* GetActiveDebuff(EDebuffType Type) const;

	void RefreshDebuffTimer(UGS_DebuffBase* Debuff, float Duration);
	void CreateAndApplyConcurrentDebuff(UGS_DebuffBase* Debuff);
	void AddDebuffToQueue(UGS_DebuffBase* Debuff);
	void ApplyNextDebuff();
	void UpdateReplicatedDebuffList();
	UFUNCTION(Server, Reliable)
	void Server_ClearAllDebuffs();

	UPROPERTY(EditDefaultsOnly)
	UDataTable* DebuffDataTable;

	UPROPERTY()
	UGS_DebuffBase* CurrentDebuff;

	UPROPERTY()
	TArray<UGS_DebuffBase*> DebuffQueue;
	
	UPROPERTY()
	TArray<UGS_DebuffBase*> ConcurrentDebuffs;

	UPROPERTY()
	TMap<UGS_DebuffBase*, FTimerHandle> DebuffTimers;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
