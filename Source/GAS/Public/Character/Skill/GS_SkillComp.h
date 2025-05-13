// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GS_SkillComp.generated.h"

class UGS_SkillBase;

UENUM(BlueprintType)
enum class ESkillSlot : uint8
{
	Moving,
	Aiming,
	Ultimate
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAS_API UGS_SkillComp : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UGS_SkillComp();

	UFUNCTION()
	void TryActivateSkill(ESkillSlot Slot);

	UFUNCTION()
	void TrySkillCommand(ESkillSlot Slot);

	void SetSkill(ESkillSlot Slot, TSubclassOf<UGS_SkillBase> SkillClass);

	void SetCanUseSkill(bool InCanUseSkill);

	bool IsSkillActive(ESkillSlot Slot) const;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY()
	TMap<ESkillSlot, UGS_SkillBase*> SkillMap;

	UFUNCTION(Server, Reliable)
	void Server_TryActiveSkill(ESkillSlot Slot);

	UFUNCTION(Server, Reliable)
	void Server_TrySkillCommand(ESkillSlot Slot);

	void InitSkills();

	UPROPERTY()
	bool bCanUseSkill = true;
};
