#pragma once

#include "CoreMinimal.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "GS_Drakhar.generated.h"


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
	UPROPERTY(ReplicatedUsing = OnRep_IsDashing)
	bool bIsDashing;

	UPROPERTY(ReplicatedUsing = OnRep_IsEarthquaking)
	bool bIsEarthquaking;

	//action binding function
	virtual void ComboAttack() override;

	virtual void Skill1() override;

	virtual void Skill2() override;

	virtual void UltimateSkill() override;

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

	UFUNCTION()
	void OnRep_IsEarthquaking();

protected:

private:
	//[combo attack] 
	
	//[dash skill]
	UPROPERTY()
	TSet<AGS_Character*> DamagedCharacters;

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	TObjectPtr<UAnimMontage> DashMontage;*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess = true))
	TObjectPtr<UCapsuleComponent> DashAttackCapsule;

	FVector DashStartLocation;
	FVector DashEndLocation;

	float DashPower;
	float DashInterpAlpha;
	float DashDuration;

	//[earthquake]
	UPROPERTY()
	TObjectPtr<UAnimMontage> EarthquakeMontage;
};
