#pragma once

#include "CoreMinimal.h"
#include "Character/Player/GS_Player.h"
#include "CollisionShape.h"
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
	
	virtual void LeftMouse();
	virtual void Ctrl();
	virtual void CtrlStop();
	virtual void RightMouse();
	
	UFUNCTION()
	void OnRep_MoveSpeed();
	
	//[attck check function]
	UFUNCTION()
	void MeleeAttackCheck();
	
	//[REFACTORING]
	TSet<AGS_Character*> DetectPlayerInRange(const FVector& Start, float SkillRange, float Radius);
	void ApplyDamageToDetectedPlayer(const TSet<AGS_Character*>& DamagedCharacters, float PlusDamge);
	
	UFUNCTION()
	void OnRep_GuardianState();

	//[quit skill - server logic]
	UFUNCTION(BlueprintCallable)
	void QuitGuardianSkill();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCDrawDebugSphere(bool bIsOverlap, const FVector& Location, float CapsuleRadius);
	
protected:
	UPROPERTY()
	EGuardianState ClientGuardianState;
	
	float NormalMoveSpeed;
	float SpeedUpMoveSpeed;
};
