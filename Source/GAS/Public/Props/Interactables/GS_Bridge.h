#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GS_Bridge.generated.h"

UCLASS()
class GAS_API AGS_Bridge : public AActor
{
	GENERATED_BODY()

public:
	AGS_Bridge();

	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> NotBrokenPieces1;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> NotBrokenPieces2;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> NotBrokenPieces3;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> NotBrokenPieces4;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> NotBrokenPieces5;
	
protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	void SetUpBridge();

private:
	UPROPERTY()
	TArray<UChildActorComponent*> BrokenPieces;

	
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess))
	TArray<UStaticMesh*> BridgeMeshAssets;

	int32 HalfBridgeMeshSize;
};
