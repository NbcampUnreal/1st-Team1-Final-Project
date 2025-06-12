#pragma once

#include "CoreMinimal.h"
#include "Character/Player/GS_Player.h"
#include "GS_Guardian.generated.h"

class UGS_DrakharAnimInstance;

UENUM(BlueprintType)
enum class EGuardianState : uint8
{
	None,
	CtrlUp,
	CtrlSkillEnd,
	ForceLanded,
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

	UPROPERTY(ReplicatedUsing=OnRep_MoveSpeed)
	float MoveSpeed;

	UFUNCTION()
	void OnRep_MoveSpeed();
	
	virtual void LeftMouse();
	virtual void Ctrl();
	virtual void CtrlStop();
	virtual void RightMouse();
	
	//[attck check function]
	UFUNCTION()
	void MeleeAttackCheck();

	UFUNCTION()
	void OnRep_GuardianState();

	//[quit skill - server logic]
	UFUNCTION(BlueprintCallable)
	void QuitGuardianSkill();
	
	
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCDrawDebugLine(const FVector& Start, const FVector& End, float CapsuleRange, float Radius, const FVector& Forward, bool bIsHit);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCDrawDebug(const FVector& Start, float Radius, bool bHit);
protected:
	UPROPERTY()
	EGuardianState ClientGuardianState;
	
	float NormalMoveSpeed;
	float SpeedUpMoveSpeed;

private:
	//for fever mode
	float FeverTime;
	float FeverGage;
	
};
