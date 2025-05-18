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
	USceneComponent* RootSceneComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trap")
	USceneComponent* RotationSceneComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trap")
	USceneComponent* MeshParentSceneComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	UBoxComponent* DamageBoxComp;

	UPROPERTY(EditDefaultsOnly, Category = "Trap")
	UDataTable* TrapDataTable;

	FTrapData TrapData;

	
	void LoadTrapData();
	
	
	//Damage Box에 오버랩 되었을 때
	UFUNCTION()
	void OnDamageBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION(BlueprintNativeEvent)
	void DamageBoxEffect(AActor* OtherActor);
	void DamageBoxEffect_Implementation(AActor* OtherActor);

	//데미지 박스에 오버랩된 플레이어에게 데미지 주는 함수
	void HandleTrapDamage(AActor* OtherActor);
	//범위 내의 여러 플레이어에게 한 번에 데미지 주는 함수
	virtual void HandleTrapAreaDamage(const TArray<AActor*>& AffectedActors);

protected:

	virtual void BeginPlay() override;
	


};
