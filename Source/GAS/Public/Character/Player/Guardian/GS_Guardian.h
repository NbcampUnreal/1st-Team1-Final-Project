#pragma once

#include "CoreMinimal.h"
#include "Character/Player/GS_Player.h"
#include "GS_Guardian.generated.h"

class UGS_DrakharAnimInstance;

UCLASS()
class GAS_API AGS_Guardian : public AGS_Player
{
	GENERATED_BODY()
	
public:
	AGS_Guardian();

	virtual void BeginPlay() override;
	virtual void PostInitializeComponents();

	//[input action binding function]
	UFUNCTION(NetMulticast,Unreliable)
	virtual void MulticastRPCComboAttack();

	UFUNCTION()
	virtual void Skill1();

	UFUNCTION()
	virtual void Skill2();

	UFUNCTION()
	virtual void UltimateSkill();

	//[test function]
	UFUNCTION()
	void MeleeAttackCheck();

	//[combo attack]
	//check montage end
	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	//set attack start state
	void AttackStartComboState();
	//set attack end state
	void AttackEndComboState();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = true))
	bool IsAttacking;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = true))
	bool CanNextCombo;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = true))
	bool IsComboInputOn;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = true))
	int32 CurrentCombo;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = true))
	int32 MaxCombo;

protected:



private:
	//for fever mode
	float FeverTime;
	float FeverGage;

	UPROPERTY()
	TObjectPtr<UGS_DrakharAnimInstance> GuardianAnim;
};
