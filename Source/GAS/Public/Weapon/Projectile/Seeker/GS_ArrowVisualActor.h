// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GS_ArrowVisualActor.generated.h"

UCLASS()
class GAS_API AGS_ArrowVisualActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AGS_ArrowVisualActor();

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* ArrowMesh;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;

	UPROPERTY(ReplicatedUsing = OnRep_SkeletalMesh)
	USkeletalMesh* CurrentMesh;

	UFUNCTION()
	void OnRep_SkeletalMesh();

	float LifeTime = 5.0f; // 5초 후 소멸
	float ElapsedTime = 0.0f;
	

	void SetArrowMesh(USkeletalMesh* Mesh);
	void SetAttachedTargetActor(AActor* Target);

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY()
	AActor* AttachedTargetActor;

	UFUNCTION()
	void OnAttachedTargetDestroyed(AActor* DestroyedActor)
	{
		Destroy();
	}
};
