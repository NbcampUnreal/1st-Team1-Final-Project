#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GS_RoomBase.generated.h"

UCLASS()
class GAS_API AGS_RoomBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AGS_RoomBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Floor;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Ceiling;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Wall;
	
	void HideCeiling();
	void ShowCeiling();

	void UseDepthStencil();
	
protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};
