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

	virtual void Ctrl();

	virtual void RightMouse();

	//[test function]
	UFUNCTION()
	void MeleeAttackCheck();

protected:
	UPROPERTY()
	TObjectPtr<UGS_DrakharAnimInstance> GuardianAnim;

private:
	//for fever mode
	float FeverTime;
	float FeverGage;
};
