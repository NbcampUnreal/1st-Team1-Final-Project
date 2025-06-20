#pragma once

#include "CoreMinimal.h"
#include "Character/Player/GS_Player.h"
#include "CollisionShape.h"
#include "Character/Component/GS_CameraShakeTypes.h"
#include "GS_Guardian.generated.h"

class UGS_DrakharAnimInstance;
class UGS_DebuffVFXComponent;
class UGS_CameraShakeComponent;

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
	
	// 디버프 VFX 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
	UGS_DebuffVFXComponent* DebuffVFXComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGS_CameraShakeComponent> CameraShakeComponent;
	
	FORCEINLINE UGS_CameraShakeComponent* GetCameraShakeComponent() const { return CameraShakeComponent; }
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "HitStop Camera Shake Info"))
	FGS_CameraShakeInfo HitStopShakeInfo;
	
	virtual void LeftMouse();
	virtual void Ctrl();
	virtual void CtrlStop();
	virtual void RightMouse();
	
	UFUNCTION()
	void OnRep_MoveSpeed();
	
	//[attck check function]
	UFUNCTION()
	void MeleeAttackCheck();
	
	//check player in attack range
	TSet<AGS_Character*> DetectPlayerInRange(const FVector& Start, float SkillRange, float Radius);
	
	//damage player in TSet
	void ApplyDamageToDetectedPlayer(const TSet<AGS_Character*>& DamagedCharacters, float PlusDamge);
	
	UFUNCTION()
	void OnRep_GuardianState();

	//[quit skill - server logic]
	UFUNCTION(BlueprintCallable)
	void QuitGuardianSkill();

	//for debugging
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCDrawDebugSphere(bool bIsOverlap, const FVector& Location, float CapsuleRadius);
	
protected:
	UPROPERTY()
	EGuardianState ClientGuardianState;
	
	float NormalMoveSpeed;
	float SpeedUpMoveSpeed;
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCApplyHitStop(AGS_Character* InDamagedCharacter);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCEndHitStop(AGS_Character* InDamagedCharacter);


private:
	//hit stop duration
	UPROPERTY()
	float HitStopDurtaion = 0.2f;

	//set world time (default -> 1.f)
	UPROPERTY()
	float HitStopTimeDilation = 0.1f;
	
	UPROPERTY()
	TArray<AGS_Character*> EndHitStopDamagedCharacters;
};
