#pragma once

#include "CoreMinimal.h"
#include "Data/GS_PlaceableObjectsRow.h"
#include "GameFramework/Actor.h"
#include "GS_BuildManager.generated.h"

struct FGS_PlaceableObjectsRow;
class UBillboardComponent;

UCLASS()
class GAS_API AGS_BuildManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AGS_BuildManager();
	virtual void Tick(float DeltaTime) override;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components")
	TObjectPtr<UBillboardComponent> BillboardCompo;
#endif
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components")
	TObjectPtr<UStaticMeshComponent> StaticMeshCompo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Components")
	TObjectPtr<UDecalComponent> DecalCompo;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DecalMaterialInstance;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Setting")
	TArray<FDataTableRowHandle> BuildingsRows;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Setting")
	bool bTraceComplex;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Setting|Grid")
	FIntPoint GridSize;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Setting|Grid")
	float CellSize;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Setting|Grid")
	float StartTraceHeight;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Setting|Grid")
	float EndTraceHeight;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Setting|Grid")
	TObjectPtr<UMaterialInterface> GridMaterial;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Setting|Grid")
	TObjectPtr<UTexture> GridTexture;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Setting|Grid")
	FLinearColor GridColor;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Setting|Grid")
	float GridOpacity;
	
	void ChangeDecalType(FDataTableRowHandle* Data);
	void ClearDecalType();
	
	TArray<FDataTableRowHandle>* GetBuildingsRows() { return &BuildingsRows; }
	
	float GetGridCellSize() { return CellSize; }
	bool IsCellUnderSursorChanged(){ return bIsCellUnderCursorChanged; }
	void SetCellUnderCursorChanged(bool InChanged) { bIsCellUnderCursorChanged = InChanged; }
	FIntPoint GetCellUnderCursor() { return CellUnderCursor; }
	FVector GetLocationUnderCursorCamera() { return LocationUnderCursorCamera; }
	
	FVector GetCellLocation(FIntPoint InCellUnderCurosr);
	FVector2d GetCenterOfRectArea(FIntPoint InAreaCenterCell, FIntPoint AreaSize, float RotateDegree = 0.0f);
	void GetCellsInRectArea(TArray<FIntPoint>& InIntPointArray, FIntPoint InCenterAreaCell, FIntPoint InAreaSize, float RotateDegree = 0.0f);

	// 나중에 배치한 애들의 타입을 넣어주어야 할 때 이용하면 괜찮을 것 같다.
	void SetOccupancyData(FIntPoint InCellPoint, EDEditorCellType InTargetType, EObjectType InObjectType, AActor* InActor, bool InIsRoom = false, bool InDeleteMode = false);
	bool CheckOccupancyData(FIntPoint InCellPoint, EDEditorCellType InTargetType);
	void ConvertFindOccupancyData(EDEditorCellType InTargetType, EDEditorCellType& InOutFindType);
	EDEditorCellType GetTargetCellType(EObjectType InObjectType, ETrapPlacement InTrapType = ETrapPlacement::Floor);
	
	// 나중에 BuildableBoundaryEnabled bool 변수 추가할 경우 함수 구현해야 함.
	// bool CheckCellInBuildableBoundary(FIntPoint Cell);
	void EnableBuildingTool(FDataTableRowHandle* Data);
	void ChangeObjectForPlacement(FDataTableRowHandle* Data);

	void PressedLMB();
	void ReleasedLMB();
	void SelectPlaceableObject();
	void PressedRMB();
	void ReleasedRMB();
	void RotateProp();
	void PressedDel();

	void ResetDungeonData();
	
	void SaveDungeonData();
	void LoadDungeonData();
	
protected:
	virtual void BeginPlay() override;

	// 저장할 슬롯 이름을 지정합니다.
	// 추후에 배열형태로 변경해 프리셋으로 사용해주어야 할 것 같다.
	FString CurrentSaveSlotName;

private:
	virtual void OnConstruction(const FTransform& Transform) override;
	
	int CellsCount;
	
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> MIDGrid;
	UFUNCTION()
	void InitGrid();
	void UpdateBuildingManagerValue();

	bool bBuildToolEnabled;
	bool bDemolitionToolEnable;
	
	FIntPoint CellUnderCursor;
	FIntPoint LastCellUnderCursor;
	bool bIsCellUnderCursorChanged;
	FVector2D GetCellCenter(FIntPoint CellIdx);
	FIntPoint GetCellFromWorldLocation(FVector InLocation);
	FVector LocationUnderCursorVisibility;
	FVector LocationUnderCursorCamera;
	UPROPERTY()
	TObjectPtr<AActor> ActorUnderCursor;

	TMap<FIntPoint, FDEOccupancyData> OccupancyData;

	FDataTableRowHandle* ObjectForPlacement;
	FGS_PlaceableObjectsRow PlaceableObjectData;
	bool bIsPlacementSelected;
	UPROPERTY()
	TObjectPtr<AGS_PlacerBase> ActivePlacer;

	bool bInteractStarted;
	UPROPERTY()
	TObjectPtr<AGS_PlacerBase> PlaceableObjectUnderCursor;
	UPROPERTY()
	TObjectPtr<AGS_PlacerBase> SelectedPlacableObject;
	bool bPlaceableObjectSelected;
	FVector StartLocationUnderCursor;
};