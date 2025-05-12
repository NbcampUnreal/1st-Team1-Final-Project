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
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	//[input action binding function]
	virtual void ComboAttack();

	virtual void Skill1();

	virtual void Skill2();

	virtual void UltimateSkill();

	//[test function]
	UFUNCTION()
	void MeleeAttackCheck();

	//[combo attack]
	//check montage end
	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	//set attack start state
	UFUNCTION()
	void AttackStartComboState();

	//set attack end state
	void AttackEndComboState();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRPCComboAttack();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCComboAttack();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCJumpToAttackMontageSection(int32 ComboIndex);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRPCJumpToAttackMontageSection(int32 ComboIndex);


	UPROPERTY()
	bool IsAttacking;

	UPROPERTY()
	bool CanNextCombo;
	
	UPROPERTY()
	bool IsComboInputOn;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 CurrentCombo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 MaxCombo;

protected:
	UPROPERTY()
	TObjectPtr<UGS_DrakharAnimInstance> GuardianAnim;



private:
	//for fever mode
	float FeverTime;
	float FeverGage;

};
