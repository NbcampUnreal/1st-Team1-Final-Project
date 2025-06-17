#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GS_TrapMotionCompBase.generated.h"


UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAS_API UGS_TrapMotionCompBase : public UActorComponent
{
	GENERATED_BODY()

public:	
	UGS_TrapMotionCompBase();

protected:
	virtual void BeginPlay() override;

public:	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "TrapMotion")
	bool bIsMoving = false;
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "TrapMotion")
	bool bReplicateMotion = true;

	UFUNCTION(BlueprintCallable, Category = "TrapMotion")
	virtual void StartMotion();
	UFUNCTION(BlueprintCallable, Category = "TrapMotion")
	virtual void StopMotion();
	UFUNCTION()
	virtual void ClientLerpUpdate();



	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UFUNCTION()
	virtual void BeginTrapMotion();
	UFUNCTION()
	virtual void EndTrapMotion();
	
};
