#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GS_BuildManager.generated.h"

UCLASS()
class GAS_API AGS_BuildManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AGS_BuildManager();
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components")
	TObjectPtr<UBillboardComponent> BillboardCompo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components")
	TObjectPtr<UStaticMeshComponent> StaticMeshCompo;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Setting")
	TArray<FDataTableRowHandle> BuildingsRows;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Setting")
	bool bTraceComplex;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Setting|Grid")
	FIntPoint GridSize;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Setting|Grid")
	float CellSize;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Setting|Grid")
	float StartTraceHeight;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Setting|Grid")
	float EndTraceHeight;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Setting|Grid")
	TObjectPtr<UMaterialInterface> GridMaterial;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Setting|Grid")
	TObjectPtr<UTexture> GridTexture;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Setting|Grid")
	FLinearColor GridColor;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Setting|Grid")
	float GridOpacity;
	
protected:
	virtual void BeginPlay() override;

private:
	virtual void OnConstruction(const FTransform& Transform) override;
	
	int CellsCount;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> MIDGrid;
	UFUNCTION()
	void InitGrid();
};
