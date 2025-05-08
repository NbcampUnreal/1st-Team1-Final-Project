#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "GS_TrapData.h"
#include "GS_TrapBase.generated.h"

class UBoxComponent;
UCLASS()
class GAS_API AGS_TrapBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AGS_TrapBase();


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trap")
	FName TrapID;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trap")
	USceneComponent* RootScene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trap")
	UStaticMeshComponent* TrapStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	UBoxComponent* DamageBox;

	UPROPERTY(EditDefaultsOnly, Category = "Trap")
	UDataTable* TrapDataTable;

	FTrapData TrapData;

	
	void LoadTrapData();

	virtual void ApplyTrapEffect(AActor* TargetActor);

	//Tigger Box에 오버랩 되었을 때
	void OnDamageBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	void HandleTrapDamage(AActor* OtherActor);

protected:

	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

};
