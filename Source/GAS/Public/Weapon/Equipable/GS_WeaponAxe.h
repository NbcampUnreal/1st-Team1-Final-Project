// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GS_WeaponEquipable.h"
#include "Engine/HitResult.h"
#include "AkAudioEvent.h"
#include "NiagaraSystem.h"
#include "GS_WeaponAxe.generated.h"

UENUM(BlueprintType)
enum class EAxeHitTargetType : uint8
{
	Guardian		UMETA(DisplayName = "Guardian"),
	DungeonMonster	UMETA(DisplayName = "DungeonMonster"),
	Seeker			UMETA(DisplayName = "Seeker"),
	Structure		UMETA(DisplayName = "Structure"),
	Other			UMETA(DisplayName = "Other")
};

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

	// 히트 사운드 에셋들
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	UAkAudioEvent* HitPawnSoundEvent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	UAkAudioEvent* HitStructureSoundEvent;

	// 히트 VFX 에셋들
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	UNiagaraSystem* HitPawnVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	UNiagaraSystem* HitStructureVFX;

	UPROPERTY()
	class AGS_Character* OwnerChar;

	UPROPERTY()
	TSet<AActor*> HitActors;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Attack")
	class UBoxComponent* HitBox;

	// 히트 이펙트 관련 함수들
	virtual EAxeHitTargetType DetermineTargetType(AActor* OtherActor) const;
	virtual void PlayHitSound(EAxeHitTargetType TargetType, const FHitResult& SweepResult);
	virtual void PlayHitVFX(EAxeHitTargetType TargetType, const FHitResult& SweepResult);

	// 멀티캐스트 함수들
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayHitSound(EAxeHitTargetType TargetType, const FHitResult& SweepResult);
	bool Multicast_PlayHitSound_Validate(EAxeHitTargetType TargetType, const FHitResult& SweepResult);
	void Multicast_PlayHitSound_Implementation(EAxeHitTargetType TargetType, const FHitResult& SweepResult);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayHitVFX(EAxeHitTargetType TargetType, const FHitResult& SweepResult);
	bool Multicast_PlayHitVFX_Validate(EAxeHitTargetType TargetType, const FHitResult& SweepResult);
	void Multicast_PlayHitVFX_Implementation(EAxeHitTargetType TargetType, const FHitResult& SweepResult);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySpecialHitVFX(class UNiagaraSystem* VFXToPlay, const FHitResult& HitResult);
};

