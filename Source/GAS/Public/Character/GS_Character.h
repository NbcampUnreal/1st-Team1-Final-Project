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

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	//variable
	float MaxSpeed;

	//getter
	FORCEINLINE UGS_StatComp* GetStatComp() const { return StatComp; }
	FORCEINLINE UGS_SkillComp* GetSkillComp() const { return SkillComp; }
	FORCEINLINE UGS_DebuffComp* GetDebuffComp() const { return DebuffComp; }

	//serverRPC
	UFUNCTION(Server, Reliable)
	void ServerRPCMeleeAttack(AGS_Character* InDamagedCharacter);


protected:
	//override function

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	//component
	UPROPERTY()
	TObjectPtr<UGS_StatComp> StatComp;

	UPROPERTY()
	TObjectPtr<UGS_SkillComp> SkillComp;

	UPROPERTY()
	TObjectPtr<UGS_DebuffComp> DebuffComp;
};