#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlaceInfoComponent.generated.h"

enum class ETrapPlacement : uint8;
enum class EObjectType : uint8;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAS_API UPlaceInfoComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CellInfo")
	TArray<FIntPoint> CellCoord;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CellInfo")
	EObjectType ObjectType;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CellInfo")
	ETrapPlacement TrapPlacement;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CellInfo")
	float ConstructionCost;

	UPlaceInfoComponent();

	void SetCellInfo(const EObjectType InObjectType, const ETrapPlacement InTrapPlacement, const TArray<FIntPoint>& InCellCoord, float InConstructionCost) { ObjectType = InObjectType; TrapPlacement = InTrapPlacement; CellCoord = InCellCoord; ConstructionCost = InConstructionCost; }
	EObjectType GetObjectType() { return ObjectType; }
	ETrapPlacement GetTrapPlacement() { return TrapPlacement; }
	TArray<FIntPoint>& GetCellCoord() { return CellCoord; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
		
};