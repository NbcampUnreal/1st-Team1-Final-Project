#pragma once

#include "CoreMinimal.h"
#include "Character/Player/GS_Player.h"
#include "GS_Guardian.generated.h"

class UGS_DrakharAnimInstance;

UENUM(BlueprintType)
enum class EGuardianState : uint8
{
	None,
	Ready,
	Skill,
	ComboAttack,
	invincible
};

UCLASS()
class GAS_API AGS_Guardian : public AGS_Player
{
	GENERATED_BODY()
	
public:
	AGS_Guardian();

	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY()
	TObjectPtr<UGS_DrakharAnimInstance> GuardianAnim;

	UPROPERTY(ReplicatedUsing=OnRep_GuardianState)
	EGuardianState GuardianState;
	
	virtual void LeftMouse();
	virtual void Ctrl();
	virtual void CtrlStop();
	virtual void RightMouse();
	
	//[attck check function]
	UFUNCTION()
	void MeleeAttackCheck();

	//[skill check]
	UFUNCTION(Server, Reliable)
	void ServerRPCStartSkill();

	UFUNCTION(Server, Reliable)
	void ServerRPCStopSkill();
	
	UFUNCTION()
	void OnRep_GuardianState();
	
	// UFUNCTION(NetMulticast, Unreliable)
	// void MulticastRPCDrawDebugLine(const FVector& Start, float CapsuleRange, float Radius, const FVector& Forward, bool bIsHit);
protected:
	UPROPERTY()
	EGuardianState ClientGuardianState;
	
private:
	//for fever mode
	float FeverTime;
	float FeverGage;
};
