#pragma once

#include "CoreMinimal.h"
#include "Props/Trap/GS_TrapBase.h"
#include "Components/SphereComponent.h"
#include "GS_NonTrigTrapBase.generated.h"


UCLASS()
class GAS_API AGS_NonTrigTrapBase : public AGS_TrapBase
{
	GENERATED_BODY()

public:
	AGS_NonTrigTrapBase();

protected:
	//virtual void BeginPlay() override;


public:
	/*UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Trap")
	USphereComponent* ActivateSphereComp;

	FTimerHandle CheckOverlapTimerHandle;

	bool bIsActivated = false;

	UFUNCTION(Server, Reliable)
	void Server_ActivateTrap(AActor* TargetActor);
	void Server_ActivateTrap_Implementation(AActor* TargetActor);

	UFUNCTION(BlueprintNativeEvent)
	void ActivateTrap(AActor* TargetActor);
	void ActivateTrap_Implementation(AActor* TargetActor);

	UFUNCTION(BlueprintNativeEvent)
	void DeActivateTrap();
	void DeActivateTrap_Implementation();

	UFUNCTION()
	void OnActivSCompBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
									UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
									bool bFromSweep, const FHitResult& SweepResult);

	void StartDeactivateTrapCheck();

	void CheckOverlappingSeeker();*/

	//TrapMotion
	//virtual bool CanStartMotion() const override;
	//virtual bool CanStopMotion() const override;

};
