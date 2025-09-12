#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/Component/GS_CameraShakeTypes.h"
#include "GS_DrakharVFXComponent.generated.h"

class AGS_Drakhar;
class UNiagaraSystem;
class UNiagaraComponent;
class UArrowComponent;
class UGS_CameraShakeComponent;
class UGS_FootManagerComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAS_API UGS_DrakharVFXComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UGS_DrakharVFXComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UPROPERTY(BlueprintReadOnly, Category = "VFX|Drakhar")
	UNiagaraComponent* ActiveWingRushVFXComponent;
	
	UPROPERTY(BlueprintReadOnly, Category = "VFX|Drakhar")
	UNiagaraComponent* ActiveDustVFXComponent;

	UPROPERTY(BlueprintReadOnly, Category = "VFX|Earthquake")
	UNiagaraComponent* ActiveGroundCrackVFXComponent;

	UPROPERTY(BlueprintReadOnly, Category = "VFX|Earthquake")
	UNiagaraComponent* ActiveDustCloudVFXComponent;
	
	UPROPERTY(BlueprintReadOnly, Category = "VFX|Drakhar")
	UNiagaraComponent* ActiveFlyingDustVFXComponent;
	
	void OnFlyStart();
	void OnFlyEnd();

	void OnUltimateStart();
	void OnEarthquakeStart();

	void StartWingRushVFX();
	void StopWingRushVFX();

	void StartDustVFX();
	void StopDustVFX();
	
	void StartGroundCrackVFX();
	void StopGroundCrackVFX();

	void StartDustCloudVFX();
	void StopDustCloudVFX();
	
	void PlayAttackHitVFX(FVector ImpactPoint);
	void PlayEarthquakeImpactVFX(const FVector& ImpactLocation);
	void PlayFeverEarthquakeImpactVFX(const FVector& ImpactLocation);

	void HandleDraconicProjectileImpact(const FVector& ImpactLocation, const FVector& ImpactNormal, bool bHitCharacter);

	void OnFeverModeChanged(bool bIsFeverMode);

	void ApplyFeverModeOverlay();
	void RemoveFeverModeOverlay();
	
protected:
	UPROPERTY()
	TObjectPtr<UGS_FootManagerComponent> FootManagerComponent;
	
	UPROPERTY()
	TObjectPtr<UArrowComponent> WingRushVFXSpawnPoint;
	
	UPROPERTY()
	TObjectPtr<UArrowComponent> EarthquakeVFXSpawnPoint;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> FeverModeOverlayMID;

private:
	UPROPERTY()
	AGS_Drakhar* OwnerDrakhar;
};
