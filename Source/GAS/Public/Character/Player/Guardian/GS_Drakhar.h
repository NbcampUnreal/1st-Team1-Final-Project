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

	//variables
	UPROPERTY(ReplicatedUsing = OnRep_IsComboAttacking, VisibleAnywhere, BlueprintReadOnly)
	bool bIsComboAttacking;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	bool bCanDoNextComboAttack;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentComboAttackIndex, VisibleAnywhere, BlueprintReadOnly)
	int32 CurrentComboAttackIndex;

	UPROPERTY(ReplicatedUsing = OnRep_IsDashing)
	bool bIsDashing;

	UPROPERTY(ReplicatedUsing = OnRep_IsEarthquaking)
	bool bIsEarthquaking;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AGS_DrakharProjectile> Projectile;

	//action binding function
	virtual void ComboAttack() override;

	virtual void Skill1() override;

	virtual void Skill2() override;

	virtual void UltimateSkill() override;

	//[combo attack]

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FVector StartLocation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FRotator StartRotation;

	UFUNCTION(Server, Reliable)
	void ServerRPCComboAttack();

	UFUNCTION(Server, Reliable)
	void ServerRPCComboAttackCheck();

	UFUNCTION(Server, Reliable)
	void ServerRPCComboReset();

	UFUNCTION(Server, Reliable)
	void ServerRPCMovementSetting();

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

protected:

private:
	//[combo attack] 
	//int32 CurrentComboAttackIndex;
	int32 MaxComboAttackIndex;
	int32 ClientComboAttackIndex;

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
};
