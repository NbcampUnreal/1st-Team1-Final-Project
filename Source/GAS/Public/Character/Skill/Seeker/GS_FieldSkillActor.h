// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GS_FieldSkillActor.generated.h"

class USphereComponent;
class AGS_Character;
class AGS_Monster;
class AGS_Guardian;

UCLASS()
class GAS_API AGS_FieldSkillActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGS_FieldSkillActor();

	void SetCaster(AGS_Character* InCaster) { Caster = InCaster; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Component")
	USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect")
	UParticleSystemComponent* ParticleSystemComp;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	AGS_Character* Caster;

	float Duration = 20.f;

	FTimerHandle DestroyTimerHandle;

	void ApplyFieldEffect();
	void DestroySelf();

	virtual void ApplyFieldEffectToMonster(AGS_Monster* Target);
	virtual void RemoveFieldEffectFromMonster(AGS_Monster* Target);
	virtual void ApplyFieldEffectToGuardian(AGS_Guardian* Target);
	virtual void RemoveFieldEffectFromGuardian(AGS_Guardian* Target);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_DrawDebugSphere();

	UPROPERTY(EditAnywhere)
	float Radius = 300.0f;

};
