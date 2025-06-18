#pragma once

#include "CoreMinimal.h"
#include "Props/Trap/TrapMotion/GS_TrapMotionCompBase.h"
#include "GS_LoopingTrapMotionComp.generated.h"

UENUM(BlueprintType)
enum class ELoopingTrapType : uint8
{
	SwingBladeTrap UMETA(DisplayName = "SwingBladeTrap"),
	SawTrap		   UMETA(DisplayName = "SawTrap")
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAS_API UGS_LoopingTrapMotionComp : public UGS_TrapMotionCompBase
{
	GENERATED_BODY()
	
public:
	UGS_LoopingTrapMotionComp();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

	//FTimerHandle LoopTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TrapMotion")
	float MotionInterval = 0.01f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrapMotion")
	float RotationSpeed = 30.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrapMotion")
	float MoveDistance = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrapMotion")
	float MoveSpeed = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrapMotion")
	ELoopingTrapType TrapType = ELoopingTrapType::SwingBladeTrap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrapMotion")
	USceneComponent* TargetComponent;

	FVector InitialLocation;
	FRotator InitialRotation;
	float CurrentDirection = 1.0f;
	float AccumulatedTime = 0.0f;

	//보간용
	FVector CurrentLoc;
	FVector TargetLoc;

	FRotator CurrentRot;
	FRotator TargetRot;

	float InterpAlpha = 0.f;
	float InterpStep = 0.1f;



	//void TimerMotion();
	void DoSwingBladeMotion();
	void DoSawMotion();

public: 
	virtual void BeginTrapMotion() override;
	//virtual void EndTrapMotion() override;
	virtual void ClientLerpUpdate() override;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UpdateTransform(FVector Loc, FRotator Rot);
	void Multicast_UpdateTransform_Implementation(FVector Loc, FRotator Rot);



};
