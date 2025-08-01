#pragma once

#include "CoreMinimal.h"
#include "Props/Trap/GS_TrapBase.h"
#include "Components/BoxComponent.h"
#include "GS_TrigTrapBase.generated.h"

class UBoxComponent;
UCLASS()
class GAS_API AGS_TrigTrapBase : public AGS_TrapBase
{
	GENERATED_BODY()
	
public:
	AGS_TrigTrapBase();


protected:
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, Replicated, Category="Trap")
	bool bIsTriggered = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	UBoxComponent* TriggerBoxComp;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
							UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


	//BeginOverlap with TriggerBoxComp
	UFUNCTION(Server, Reliable)
	void Server_ApplyTrapEffect(AActor* TargetActor);
	void Server_ApplyTrapEffect_Implementation(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category="Trap")
	void TrapEffectComplete();


	UFUNCTION(BlueprintNativeEvent)
	void ApplyTrapEffect(AActor* TargetActor);
	void ApplyTrapEffect_Implementation(AActor* TargetActor);



	//EndOverlap with TriggerBoxComp
	UFUNCTION(Server, Reliable)
	void Server_EndTrapEffect(AActor* TargetActor);
	void Server_EndTrapEffect_Implementation(AActor* TargetActor);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EndTrapEffect(AActor* TargetActor);
	void Multicast_EndTrapEffect_Implementation(AActor* TargetActor);

	UFUNCTION(BlueprintNativeEvent)
	void EndTrapEffect(AActor* TargetActor);
	void EndTrapEffect_Implementation(AActor* TargetActor);
};
