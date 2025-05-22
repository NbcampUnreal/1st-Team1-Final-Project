#pragma once

#include "CoreMinimal.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "GS_Drakhar.generated.h"

class AGS_DrakharProjectile;

UCLASS()
class GAS_API AGS_Drakhar : public AGS_Guardian
{
	GENERATED_BODY()
	
public:
	AGS_Drakhar();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	//[combo attack variables]
	UPROPERTY(ReplicatedUsing = OnRep_IsComboAttacking, VisibleAnywhere, BlueprintReadOnly)
	bool bIsComboAttacking;

	UPROPERTY(ReplicatedUsing = OnRep_CanDoNextComboAttack, VisibleAnywhere, BlueprintReadOnly)
	bool bCanDoNextComboAttack;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentComboAttackIndex, VisibleAnywhere, BlueprintReadOnly)
	int32 CurrentComboAttackIndex;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AGS_DrakharProjectile> Projectile;

	//[Draconic Fury Variables]
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AGS_DrakharProjectile> DraconicProjectile;

	//[Input Binding Function]
	virtual void Ctrl() override;

	virtual void CtrlStop() override;

	virtual void LeftMouse() override;
	
	virtual void RightMouse() override;

	//[combo attack]
	UFUNCTION(Server, Reliable)
	void ServerRPCComboAttack();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCPlayComboAttackMontage();

	UFUNCTION(Server, Reliable)
	void ServerRPCComboAttackCheck();

	UFUNCTION(Server, Reliable)
	void ServerRPCComboAttackEnd();

	UFUNCTION()
	void OnRep_IsComboAttacking();

	UFUNCTION()
	void OnRep_CurrentComboAttackIndex();

	UFUNCTION()
	void OnRep_CanDoNextComboAttack();

	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	//[Dash Skill]
	UFUNCTION(Server, Reliable)
	void ServerRPCDoDash(float DeltaTime);
	
	UFUNCTION(Server, Reliable)
	void ServerRPCEndDash();

	UFUNCTION(Server, Reliable)
	void ServerRPCCalculateDashLocation();
	
	UFUNCTION()
	void DashAttackCheck();

	//[Earthquake Skill]
	UFUNCTION(Server, Reliable)
	void ServerRPCEarthquakeAttackCheck();

	//[DraconicFury Skill]
	UFUNCTION(Server, Reliable)
	void ServerRPCSpawnDraconicFury();

private:
	//[combo attack]
	int32 MaxComboAttackIndex;
	int32 ClientComboAttackIndex;
	bool ClientComboAttacking;
	bool ClientNextComboAttack;

	//[dash skill]
	UPROPERTY()
	TSet<AGS_Character*> DamagedCharacters;

	FVector DashStartLocation;
	FVector DashEndLocation;
	float DashPower;
	float DashInterpAlpha;
	float DashDuration;

	//[earthquake]
	float EarthquakePower;
	float EarthquakeRadius;

	//[DraconicFury]
	FTimerHandle FlyingTimerHandle;
	FTimerHandle DraconicAttackTimer;
	float FlyingPersistenceTime;
	float DraconicAttackPersistenceTime;

	//[Draconic Fury]
	TArray<FTransform> DraconicFuryTargetArray;
	void GetRandomDraconicFuryTarget();
};
