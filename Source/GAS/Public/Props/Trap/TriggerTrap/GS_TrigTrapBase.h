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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	UBoxComponent* TriggerBoxComp;

	UFUNCTION()
	void OnTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
							UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


	UFUNCTION(BlueprintNativeEvent)
	void ApplyTrapEffect(AActor* TargetActor);
	void ApplyTrapEffect_Implementation(AActor* TargetActor);

	UFUNCTION(BlueprintNativeEvent)
	void EndTrapEffect(AActor* TargetActor);
	void EndTrapEffect_Implementation(AActor* TargetActor);

};
