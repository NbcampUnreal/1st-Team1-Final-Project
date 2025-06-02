// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GS_WeaponEquipable.h"
#include "GS_WeaponAxe.generated.h"

UCLASS()
class GAS_API AGS_WeaponAxe : public AGS_WeaponEquipable
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGS_WeaponAxe();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	USkeletalMeshComponent* AxeMeshComponent;

	UFUNCTION()
	void OnHit(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void EnableHit();
	
	UFUNCTION()
	void DisableHit();

	UFUNCTION(Server, Reliable)
	void ServerEnableHit();
	UFUNCTION(Server, Reliable)
	void ServerDisableHit();

	// 히트 사운드
	//UPROPERTY(EditDefaultsOnly, Category = "Sound")
	//UAkAudioEvent* HitSound;

	UPROPERTY()
	class AGS_Character* OwnerChar;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Attack")
	class UBoxComponent* HitBox;


};

