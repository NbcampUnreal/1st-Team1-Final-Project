#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GS_PlacerBase.generated.h"

UCLASS()
class GAS_API AGS_PlacerBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AGS_PlacerBase();
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components")
	TObjectPtr<UStaticMeshComponent> StaticMeshCompo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Material")
	FDataTableRowHandle ObjectNameInTable;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Material")
	TObjectPtr<UMaterialInterface> PlaceAcceptedMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Material")
	TObjectPtr<UMaterialInterface> PlaceRejectedMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Material")
	TObjectPtr<UMaterialInterface> BuildAcceptedMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Material")
	TObjectPtr<UMaterialInterface> BuildRejectedMaterial;

	virtual void BuildObject() PURE_VIRTUAL(AGS_PlacerBase::BuildObject,);
	
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	FIntPoint ObjectSize;
	// UPROPERTY()
	// TObjectPtr<PlaceableObjectClass> PlaceableObjectClass;

	void SetupObjectPlacer();
};
