#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GS_TrapVisualProjectile.generated.h"

UCLASS()
class GAS_API AGS_TrapVisualProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AGS_TrapVisualProjectile();

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	void SetProjectileMesh(UStaticMesh* Mesh);
};
