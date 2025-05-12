#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GS_Character.generated.h"

class UGS_StatComp;
class UGS_SkillComp;
class UGS_DebuffComp;

UCLASS()
class GAS_API AGS_Character : public ACharacter
{
	GENERATED_BODY()

public:
	AGS_Character();

	virtual void BeginPlay() override;

	//component
	UPROPERTY()
	TObjectPtr<UGS_StatComp> StatComp;

	UPROPERTY()
	TObjectPtr<UGS_SkillComp> SkillComp;

	UPROPERTY()
	TObjectPtr<UGS_DebuffComp> DebuffComp;

	//variable
	float MaxSpeed;

	//getter
	UFUNCTION()
	FORCEINLINE UGS_StatComp* GetStatComp() const { return StatComp; }

	//serverRPC
	UFUNCTION(Server, Reliable)
	void ServerRPCMeleeAttack(AGS_Character* InDamagedCharacter);


protected:
	//override function
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};