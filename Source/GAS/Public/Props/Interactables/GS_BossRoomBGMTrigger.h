#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GS_BossRoomBGMTrigger.generated.h"

class UAkAudioEvent;
class UBoxComponent;

UCLASS()
class GAS_API AGS_BossRoomBGMTrigger : public AActor
{
	GENERATED_BODY()

public:
	AGS_BossRoomBGMTrigger();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trigger")
	USceneComponent* RootSceneComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	UBoxComponent* TriggerBoxComp;

	UPROPERTY(EditAnywhere, Category = "Trigger|Boss BGM")
	UAkAudioEvent* BossMusicStartEvent;

	UPROPERTY(EditAnywhere, Category = "Trigger|Boss BGM")
	UAkAudioEvent* BossMusicStopEvent;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void TriggerBossRoomBGMForLocalPlayer(AActor* TargetActor, UAkAudioEvent* StartEvent, UAkAudioEvent* StopEvent);
	void EndBossRoomBGMForLocalPlayer(AActor* TargetActor);
};
