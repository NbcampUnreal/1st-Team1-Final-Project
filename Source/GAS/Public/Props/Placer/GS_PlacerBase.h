#pragma once

#include "CoreMinimal.h"
#include "DungeonEditor/Data/GS_PlaceableObjectsRow.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "GS_PlacerBase.generated.h"

class UNiagaraSystem;
class AGS_BuildManager;

UCLASS()
class GAS_API AGS_PlacerBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AGS_PlacerBase();
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Components")
	TObjectPtr<UStaticMeshComponent> StaticMeshCompo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Ref")
	FDataTableRowHandle ObjectNameInTable;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Setting|Ref")
	TObjectPtr<AGS_BuildManager> BuildManagerRef;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Setting|Data")
	FGS_PlaceableObjectsRow ObjectData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Material")
	TObjectPtr<UMaterialInterface> PlaceAcceptedMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Material")
	TObjectPtr<UMaterialInterface> PlaceRejectedMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Material")
	TObjectPtr<UMaterialInterface> BuildAcceptedMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Material")
	TObjectPtr<UMaterialInterface> BuildRejectedMaterial;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Sound")
	TObjectPtr<USoundBase> PlaceSuccessSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Sound")
	TObjectPtr<USoundBase> PlaceFailSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Sound")
	TObjectPtr<USoundBase> PlaceDragSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setting|Sound")
	float DragSoundCooldown = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Components")
	TObjectPtr<UNiagaraSystem> DustEffectTemplate;
	
	// virtual void BuildObject() PURE_VIRTUAL(AGS_PlacerBase::BuildObject,);
	void BuildObject();
	void ActiveObjectPlacer();

	void SetObjectSelectedState(bool InState);

	void RotatePlacer();
	
protected:
	virtual void BeginPlay() override;

	// 하부 표시기 배열
	UPROPERTY()
	TArray<UStaticMeshComponent*> PlaceIndicators;
	UStaticMeshComponent* CreateIndicatorMesh();
	
private:
	UPROPERTY()
	FIntPoint ObjectSize;
	UPROPERTY()
	UStaticMesh* PlaneMesh;
	// UPROPERTY()
	// TObjectPtr<PlaceableObjectClass> PlaceableObjectClass;
	INT CurPlaceMeshIndex;
	bool bUpdatePlaceIndicators;
	bool bCanBuild;
	EPlacerDirectionType Direction;

	void SetupObjectPlacer();
	void DrawPlacementIndicators();

	void CalCellsInRectArea(TArray<FIntPoint>& InIntPointArray);

	// EDEditorCellType GetTargetCellType();
	EDEditorCellType GetRoomCellInfo(int InIdx);

	int RotateYaw;
	bool bObjectSelected;
	
	// 드래그 사운드 관련
	FIntPoint LastCellPosition;
	float LastDragSoundTime;
};
