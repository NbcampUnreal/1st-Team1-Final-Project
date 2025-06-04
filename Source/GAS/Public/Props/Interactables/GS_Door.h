#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "GS_Door.generated.h"

UCLASS()
class GAS_API AGS_Door : public AActor
{
	GENERATED_BODY()
	
public:	
	AGS_Door();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	USceneComponent* RootSceneComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	UBoxComponent* TriggerBoxComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	UStaticMeshComponent* DoorFrameMeshComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	UStaticMeshComponent* DoorMeshComp;
	
	bool bIsOpen = false;

	FTimerHandle DoorCloseTimerHandle;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintNativeEvent, Category="Door")
	void DoorOpen();
	virtual void DoorOpen_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Door")
	void DoorClose();
	virtual void DoorClose_Implementation();

	void CheckForPlayerInTrigger();

	UFUNCTION(Server, Reliable)
	void Server_DoorOpen(AActor* TargetActor);
	void Server_DoorOpen_Implementation(AActor* TargetActor);


protected:
	virtual void BeginPlay() override;
};
