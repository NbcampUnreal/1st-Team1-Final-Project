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
	virtual void LeftMouse();
	
	virtual void Ctrl();

	virtual void CtrlStop();

	virtual void RightMouse();	

	//[test function]
	UFUNCTION()
	void MeleeAttackCheck();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCDrawDebugLine();
	

	UPROPERTY()
	TObjectPtr<UGS_DrakharAnimInstance> GuardianAnim;

private:
	//for fever mode
	float FeverTime;
	float FeverGage;
};
