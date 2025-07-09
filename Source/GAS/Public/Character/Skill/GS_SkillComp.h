// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GS_SkillSet.h"
#include "ESkill.h"
#include "GS_SkillComp.generated.h"


/*UENUM(BlueprintType)
enum class ESkillSlot : uint8
{
	Ready,
	Moving,
	Aiming,
	Ultimate,
	Rolling
};*/

USTRUCT()
struct FSkillCooldownState
{
	GENERATED_BODY()

	UPROPERTY()
	ESkillSlot Slot;

	UPROPERTY()
	float CooldownRemaining; 

	UPROPERTY() 
	bool bIsOnCooldown;

	FTimerHandle CooldownTimer;
	FTimerHandle UIUpdateTimer;
};

USTRUCT()
struct FSkillRuntimeState
{
	GENERATED_BODY()

	UPROPERTY()
	ESkillSlot Slot;

	UPROPERTY()
	bool bIsActive = false;
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSkillCooldownChanged, ESkillSlot, float);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAS_API UGS_SkillComp : public UActorComponent
{
	GENERATED_BODY()

public:
	UGS_SkillComp();
	
	FOnSkillCooldownChanged OnSkillCooldownChanged;

	UFUNCTION(Server, Reliable)
	void Server_TryActivateSkill(ESkillSlot Slot);

	UFUNCTION(Server, Reliable)
	void Server_TryDeactiveSkill(ESkillSlot Slot);
	
	UFUNCTION(Server, Reliable)
	void Server_TrySkillCommand(ESkillSlot Slot);
	
	void SetSkill(ESkillSlot Slot, const FSkillInfo& Info);
	void SetCanUseSkill(bool InCanUseSkill);

	void SetSkillActiveState(ESkillSlot Slot, bool InIsActive);
	bool IsSkillActive(ESkillSlot Slot) const;
	
	void StartCooldownForSkill(ESkillSlot Slot);

	void SkillsInterrupt();

	//for skill widget
	UFUNCTION()
	void InitializeSkillWidget(UGS_SkillWidget* InSkillWidget);

	UFUNCTION()
	UGS_SkillBase* GetSkillFromSkillMap(ESkillSlot Slot);

	// VFX 재생 RPC
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayCastVFX(ESkillSlot Slot, FVector Location, FRotator Rotation);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayRangeVFX(ESkillSlot Slot, FVector Location, float Radius);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayImpactVFX(ESkillSlot Slot, FVector Location);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayEndVFX(ESkillSlot Slot, FVector Location, FRotator Rotation);

	// 쿨타임 변경
	void ApplyCooldownModifier(ESkillSlot SkillSlot, float Modifier);
	
	void ResetCooldownModifier(ESkillSlot Slot);

	// 스킬 데이터 테이블 접근자
	FORCEINLINE UDataTable* GetSkillDataTable() const { return SkillDataTable; }

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// skill 
	UPROPERTY()
	TMap<ESkillSlot, UGS_SkillBase*> SkillMap;

	UPROPERTY()
	TMap<ESkillSlot, FSkillRuntimeState> SkillStates;
	
	UPROPERTY(ReplicatedUsing = OnRep_SkillStates)
	TArray<FSkillRuntimeState> ReplicatedSkillStates;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	UDataTable* SkillDataTable;

	UPROPERTY(Replicated)
	bool bCanUseSkill = true;

	// cooldown 
	UPROPERTY()
	TMap<ESkillSlot, FSkillCooldownState> CooldownStates;
    
	UPROPERTY(ReplicatedUsing = OnRep_CooldownStates)
	TArray<FSkillCooldownState> ReplicatedCooldownStates;
	
	

	

	UFUNCTION()
	void InitSkills();

private:
	UFUNCTION()
	void OnRep_SkillStates();
	
	UFUNCTION()
	void OnRep_CooldownStates();

	void HandleCooldownComplete(ESkillSlot Slot);
	void HandleCooldownProgress(ESkillSlot Slot);
	void UpdateReplicatedCooldownStates();
	
};

