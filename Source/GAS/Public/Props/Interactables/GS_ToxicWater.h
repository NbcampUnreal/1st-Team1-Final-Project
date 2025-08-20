#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Character/Component/GS_DebuffComp.h"
#include "GS_ToxicWater.generated.h"

class UBoxComponent;
UCLASS()
class GAS_API AGS_ToxicWater : public AActor
{
	GENERATED_BODY()

public:
	AGS_ToxicWater();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, Category = "Boss")
	USceneComponent* RootSceneComp;

	UPROPERTY(VisibleAnywhere, Category = "Boss")
	UBoxComponent* DamageDebuffBoxComp;

	UPROPERTY(VisibleAnywhere, Category = "Boss")
	UStaticMeshComponent* ToxicWaterMeshComp;


	UFUNCTION()
	void OnTWaterBeginOverlap(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnTWaterEndOverlap(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

};
