#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "System/GS_PlayerRole.h"
#include "GS_SpawnSlot.generated.h"

UCLASS()
class GAS_API AGS_SpawnSlot : public AActor
{
	GENERATED_BODY()
	
public:
	AGS_SpawnSlot();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Slot")
	EPlayerRole ForRole;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn Slot")
	int32 SlotIndex;

public:	
	EPlayerRole GetRole() const { return ForRole; }
	int32 GetSlotIndex() const { return SlotIndex; }
	
};
