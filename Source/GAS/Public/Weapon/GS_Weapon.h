#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GS_Weapon.generated.h"

UCLASS()
class GAS_API AGS_Weapon : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGS_Weapon();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	USkeletalMeshComponent* WeaponMeshComponent;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
