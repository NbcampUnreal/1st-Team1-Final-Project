#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GS_BridgePiece.generated.h"

UCLASS()
class GAS_API AGS_BridgePiece : public AActor
{
	GENERATED_BODY()

public:
	AGS_BridgePiece();
	void SetBridgeMesh(UStaticMesh* InMesh, UMaterialInterface* InMaterial, float InValue);
	void BrokeBridge(float InDamage);
	UFUNCTION()
	void OnRep_BridgeMaterial();
	
protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	void StopSimulate();

private:
	UPROPERTY()
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(ReplicatedUsing=OnRep_BridgeMaterial)
	UMaterialInterface* BridgeMaterial;

	float MaxHealth;
	float CurrentHealth;
	bool bIsDestroyed;
};
