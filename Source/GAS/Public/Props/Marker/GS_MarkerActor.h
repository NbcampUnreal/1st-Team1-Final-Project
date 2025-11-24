#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Types/GS_MarkerTypes.h"
#include "GS_MarkerActor.generated.h"

class UDecalComponent;

UCLASS()
class GAS_API AGS_MarkerActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AGS_MarkerActor();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Marker")
	UDecalComponent* DecalComponent;

	UPROPERTY(ReplicatedUsing = OnRep_MarkerType, VisibleAnywhere, BlueprintReadOnly, Category = "Marker")
	EMarkerType MarkerType;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Marker")
	APlayerState* OwnerPlayerState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Marker")
	TMap<EMarkerType, UMaterialInterface*> MarkerMaterials;

	UFUNCTION()
	void OnRep_MarkerType();

	void UpdateDecalMaterial();
};

