// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GS_SkillSet.h"
#include "GS_SkillComp.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSkill1CoolTimeChangedDelegate, float);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSkill2CoolTimeChangedDelegate, float);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSkill3CoolTimeChangedDelegate, float);


UENUM(BlueprintType)
enum class ESkillSlot : uint8
{
	Ready,
	Moving,
	Aiming,
	Ultimate
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

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAS_API UGS_SkillComp : public UActorComponent
{
	GENERATED_BODY()

public:
	//skill cooltime delegates
	FOnSkill1CoolTimeChangedDelegate Skill1CoolTimeChanged;
	FOnSkill2CoolTimeChangedDelegate Skill2CoolTimeChanged;
	FOnSkill3CoolTimeChangedDelegate Skill3CoolTimeChanged;

	UPROPERTY(ReplicatedUsing=OnRep_Skill1)
	float Skill1LeftCoolTime = 0.0f;
	
	UPROPERTY(ReplicatedUsing=OnRep_Skill2)
	float Skill2LeftCoolTime = 0.0f;
	
	UPROPERTY(ReplicatedUsing=OnRep_Skill3)
	float Skill3LeftCoolTime = 0.0f;

	UFUNCTION()
	void OnRep_Skill1();
	UFUNCTION()
	void OnRep_Skill2();
	UFUNCTION()
	void OnRep_Skill3();
	
	// Sets default values for this component's properties
	UGS_SkillComp();

	UFUNCTION()
	void TryActivateSkill(ESkillSlot Slot);

	UFUNCTION(Server, Reliable)
	void Server_TryDeactiveSkill(ESkillSlot Slot);

	UFUNCTION()
	void TrySkillCommand(ESkillSlot Slot);

	void SetSkill(ESkillSlot Slot, const FSkillInfo& Info);

	void SetCanUseSkill(bool InCanUseSkill);

	void SetSkillActiveState(ESkillSlot Slot, bool InIsActive);

	bool IsSkillActive(ESkillSlot Slot) const;

	//for skill widget
	UFUNCTION()
	void InitializeSkillWidget(UGS_SkillWidget* InSkillWidget);

	UFUNCTION()
	UGS_SkillBase* GetSkillFromSkillMap(ESkillSlot Slot);
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY()
	TMap<ESkillSlot, UGS_SkillBase*> SkillMap;
	
	UPROPERTY(ReplicatedUsing = OnRep_SkillStates)
	TArray<FSkillRuntimeState> ReplicatedSkillStates;

	TMap<ESkillSlot, FSkillRuntimeState> SkillStates;
	
	UFUNCTION()
	void OnRep_SkillStates();

	UFUNCTION(Server, Reliable)
	void Server_TryActiveSkill(ESkillSlot Slot);

	UFUNCTION(Server, Reliable)
	void Server_TrySkillCommand(ESkillSlot Slot);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	UDataTable* SkillDataTable;

	UFUNCTION()
	void InitSkills();

	UPROPERTY(Replicated)
	bool bCanUseSkill = true;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
