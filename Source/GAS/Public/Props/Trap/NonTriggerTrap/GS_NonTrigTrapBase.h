#pragma once

#include "CoreMinimal.h"
#include "Props/Trap/GS_TrapBase.h"
#include "Components/SphereComponent.h"
#include "GS_NonTrigTrapBase.generated.h"


UCLASS()
class GAS_API AGS_NonTrigTrapBase : public AGS_TrapBase
{
	GENERATED_BODY()

public:
	AGS_NonTrigTrapBase();

protected:
	virtual void BeginPlay() override;

	/** 부모 클래스의 오버랩 콜리전 함수 오버라이드 */
	virtual void OnActivSCompBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);


// ===================
// Non-Trigger Trap Specific Functions
// ===================

/** 논트리거 함정 전용 활성화 로직 */
UFUNCTION(BlueprintCallable, Category = "Trap|NonTrigger")
void ActivateNonTriggerTrap(AActor* TargetActor);

/** 논트리거 함정 전용 비활성화 로직 */
UFUNCTION(BlueprintCallable, Category = "Trap|NonTrigger")
void DeactivateNonTriggerTrap();

	// ===================
	// Trap Motion Functions
	// ===================

	virtual bool CanStartMotion() const override;

	/** 논트리거 함정의 모션 정지 가능 여부 */
	virtual bool CanStopMotion() const;

};
