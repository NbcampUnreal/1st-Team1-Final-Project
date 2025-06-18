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
	void SetBridgeMesh(UStaticMesh* InMesh, float InValue);
	void BrokeBridge(float InDamage);

protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY()
	UStaticMeshComponent* MeshComponent;
	
	float MaxHealth;
	float CurrentHealth;
	bool bIsDestroyed;
};
