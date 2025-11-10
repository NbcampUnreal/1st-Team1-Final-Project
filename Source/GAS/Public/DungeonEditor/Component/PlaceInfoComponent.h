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
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category="CellInfo")
	TArray<FIntPoint> CellCoord;
	UPROPERTY(ReplicatedUsing = OnRep_UpdateObjectType, VisibleAnywhere, BlueprintReadOnly, Category="CellInfo")
	EObjectType ObjectType;
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category="CellInfo")
	ETrapPlacement TrapPlacement;
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "CellInfo")
	float ConstructionCost;
	UPROPERTY(VisibleAnywhere, Category = "CellInfo")
	bool Is_ObjectTypeSynchronization;

	UPlaceInfoComponent();

	void SetCellInfo(const EObjectType InObjectType, const ETrapPlacement InTrapPlacement, const TArray<FIntPoint>& InCellCoord, float InConstructionCost) { ObjectType = InObjectType; TrapPlacement = InTrapPlacement; CellCoord = InCellCoord; ConstructionCost = InConstructionCost; }
	EObjectType GetObjectType() { return ObjectType; }
	ETrapPlacement GetTrapPlacement() { return TrapPlacement; }
	TArray<FIntPoint>& GetCellCoord() { return CellCoord; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void OnRep_UpdateObjectType();
		
};
