// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile/GS_WeaponProjectile.h"
#include "Engine/Engine.h" 
#include "Components/PrimitiveComponent.h"
#include "Engine/HitResult.h"
#include "AkAudioEvent.h"
#include "NiagaraSystem.h"
#include "GS_SeekerMerciArrow.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnArrowHitEnemy, AActor*, HitActor);

class AGS_ArrowVisualActor;

UENUM(BlueprintType)
enum class ETargetType : uint8
{
	Guardian		UMETA(DisplayName = "Guardian"),
	DungeonMonster	UMETA(DisplayName = "DungeonMonster"),
	Seeker			UMETA(DisplayName = "Seeker"),
	Structure		UMETA(DisplayName = "Structure"),
	Skill			UMETA(DisplayName = "Skill")
};

UCLASS()
class GAS_API AGS_SeekerMerciArrow : public AGS_WeaponProjectile
{
	GENERATED_BODY()

public:
	AGS_SeekerMerciArrow();
	UPROPERTY()
	TSet<AActor*> HitActors;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_InitHomingTarget(AActor* Target);
	//AGS_Player* OwnerPlayer;

	//Crosshair
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnArrowHitEnemy OnArrowHitEnemy;
protected:
	virtual void BeginPlay() override;
	void StickWithVisualOnly(const FHitResult& Hit);

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AGS_ArrowVisualActor> VisualArrowClass;

	// Wwise 히트 사운드 이벤트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	UAkAudioEvent* HitPawnSoundEvent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	UAkAudioEvent* HitStructureSoundEvent;

	// 히트 VFX 에셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	UNiagaraSystem* HitPawnVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	UNiagaraSystem* HitStructureVFX;

	// 유도 화살
	UPROPERTY()
	AActor* HomingTarget = nullptr;

	

	UFUNCTION()
	virtual void OnBeginOverlap(
		UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
	virtual ETargetType DetermineTargetType(AActor* OtherActor) const;
	virtual void HandleTargetTypeGeneric(ETargetType TargetType, const FHitResult& SweepResult);
	virtual void PlayHitSound(ETargetType TargetType, const FHitResult& SweepResult);
	virtual void PlayHitVFX(ETargetType TargetType, const FHitResult& SweepResult);

	// 멀티캐스트 함수들
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayHitSound(ETargetType TargetType, const FHitResult& SweepResult);
	bool Multicast_PlayHitSound_Validate(ETargetType TargetType, const FHitResult& SweepResult);
	void Multicast_PlayHitSound_Implementation(ETargetType TargetType, const FHitResult& SweepResult);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayHitVFX(ETargetType TargetType, const FHitResult& SweepResult);
	bool Multicast_PlayHitVFX_Validate(ETargetType TargetType, const FHitResult& SweepResult);
	void Multicast_PlayHitVFX_Implementation(ETargetType TargetType, const FHitResult& SweepResult);
};
