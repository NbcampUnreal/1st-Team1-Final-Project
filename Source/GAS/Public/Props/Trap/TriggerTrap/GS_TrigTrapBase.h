#pragma once

#include "CoreMinimal.h"
#include "Props/Trap/GS_TrapBase.h"
#include "Components/BoxComponent.h"
#include "AkGameplayTypes.h"
#include "GS_TrigTrapBase.generated.h"

class UBoxComponent;
class UAkComponent;
class UAkRtpc;
UCLASS()
class GAS_API AGS_TrigTrapBase : public AGS_TrapBase
{
	GENERATED_BODY()
	
public:
	AGS_TrigTrapBase();


protected:
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, Replicated, Category="Trap")
	bool bIsTriggered = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	UBoxComponent* TriggerBoxComp;

	//트리거 후 발동까지 딜레이
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	float TriggerDelay = 0.0f;

	UPROPERTY()
	FTimerHandle DelayHandle;


	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
							UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


	//BeginOverlap with TriggerBoxComp
	
	// - 딜레이 부여 
	void DelayTrapEffect(AActor* TargetActor);

	UFUNCTION(Server, Reliable)
	void Server_DelayTrapEffect(AActor* TargetActor);
	void Server_DelayTrapEffect_Implementation(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category="Trap|Sound")
	void CallTrapAlertSound(AActor* TargetActor);

	UFUNCTION(BlueprintImplementableEvent, Category="Trap|Sound")
	void OnTrapAlertSoundPlayed(AActor* TargetActor);

	/** 서버에서 멀티캐스트로 경고 사운드 재생 */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayTrapAlertSound(AActor* TargetActor);


	// - 함정 동작 적용
	UFUNCTION(BlueprintCallable, Category="Trap")
	void TrapEffectComplete();

	UFUNCTION(BlueprintNativeEvent)
	void ApplyTrapEffect(AActor* TargetActor);
	void ApplyTrapEffect_Implementation(AActor* TargetActor);

	

	//EndOverlap with TriggerBoxComp
	UFUNCTION(Server, Reliable)
	void Server_EndTrapEffect(AActor* TargetActor);
	void Server_EndTrapEffect_Implementation(AActor* TargetActor);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EndTrapEffect(AActor* TargetActor);
	void Multicast_EndTrapEffect_Implementation(AActor* TargetActor);

	UFUNCTION(BlueprintNativeEvent)
	void EndTrapEffect(AActor* TargetActor);
	void EndTrapEffect_Implementation(AActor* TargetActor);

	// === Overrides ===
	void ActivateTrap_Implementation(AActor* TargetActor);
	void DeActivateTrap_Implementation();
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UAkComponent* GetOrCreateTrapAkComponent();
};
