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
	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	//[combo attack variables]
	UPROPERTY(ReplicatedUsing = OnRep_IsComboAttacking, VisibleAnywhere, BlueprintReadOnly)
	bool bIsComboAttacking;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	bool bCanDoNextComboAttack;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentComboAttackIndex, VisibleAnywhere, BlueprintReadOnly)
	int32 CurrentComboAttackIndex;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AGS_DrakharProjectile> Projectile;

	//[dash attack variables]
	UPROPERTY(ReplicatedUsing = OnRep_IsDashing)
	bool bIsDashing;

	//[earthquake variables]
	UPROPERTY(ReplicatedUsing = OnRep_IsEarthquaking)
	bool bIsEarthquaking;

	//[draconic fury variables]
	UPROPERTY(ReplicatedUsing = OnRep_IsFlying)
	bool bIsFlying;

	UPROPERTY(ReplicatedUsing = OnRep_CanDraconicFuryAttack)
	bool bCanDraconicFuryAttack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AActor> TargetActor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AGS_DrakharProjectile> DraconicProjectile;

	//action binding function
	virtual void ComboAttack() override;

	virtual void Skill1() override;

	virtual void Skill2() override;

	virtual void UltimateSkill() override;

	virtual void Ctrl() override;

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
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	//[dash skill]
	UFUNCTION(Server, Reliable)
	void ServerRPCDashCharacter();

	UFUNCTION()
	void DashAttackCheck();

	UFUNCTION()
	void EndDash();

	UFUNCTION()
	void OnRep_IsDashing();

	//[earthquake skill]
	UFUNCTION(Server, Reliable)
	void ServerRPCEarthquake();

	UFUNCTION(Server, Reliable)
	void ServerRPCEarthquakeEnd();

	UFUNCTION(Server, Reliable)
	void ServerRPCEarthquakeAttackCheck();

	UFUNCTION()
	void OnRep_IsEarthquaking();
	
	//[draconic fury skill]
	UFUNCTION(Server, Reliable)
	void ServerRPCDraconicFuryFly();

	UFUNCTION(Server, Reliable)
	void ServerRPCDraconicFuryAttack();


	UFUNCTION()
	void EndFlying();

	UFUNCTION()
	void EndDraconicAttack();

	UFUNCTION()
	void OnRep_IsFlying();

	UFUNCTION()
	void OnRep_CanDraconicFuryAttack();

protected:

private:
	//[combo attack] 
	//int32 CurrentComboAttackIndex;
	int32 MaxComboAttackIndex;
	int32 ClientComboAttackIndex;
	bool ClientComboAttacking;

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
	/*FTimerHandle SpawnDraconicFuryTimer;
	int32 SpawnCount = 0;*/

	void GetRandomDraconicFuryTarget();
	//void SpawnDraconicFury();
};
