#pragma once

#include "CoreMinimal.h"

#include "GS_EarthquakeEffect.generated.h"

//struct FChaosBreakEvent;
class UGeometryCollectionComponent;

UCLASS()
class GAS_API AGS_EarthquakeEffect : public AActor
{
	GENERATED_BODY()

public:
	AGS_EarthquakeEffect();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastTriggerDestruction(FVector ImpactLocation, float Radius, float Strength);
	
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UGeometryCollectionComponent* GeometryCollectionComponent;

};
