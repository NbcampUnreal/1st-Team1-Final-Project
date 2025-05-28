#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "GS_TrapData.h"
#include "Character/GS_Character.h"
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


	bool IsBlockedInDirection(const FVector& Start, const FVector& Direction, float Distance, AGS_Character* CharacterToIgnore);
	

	//데미지 박스에 오버랩된 플레이어에게 데미지 주는 함수
	void HandleTrapDamage(AActor* OtherActor);
	//범위 내의 여러 플레이어에게 한 번에 데미지 주는 함수
	virtual void HandleTrapAreaDamage(const TArray<AActor*>& AffectedActors);
	
	UFUNCTION(Server, Reliable)
	void Server_HandleTrapDamage(AActor* TargetActor);
	void Server_HandleTrapDamage_Implementation(AActor* TargetActor);


	//Damage Box에 오버랩 되었을 때
	UFUNCTION()
	void OnDamageBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);


	UFUNCTION(BlueprintNativeEvent)
	void DamageBoxEffect(AActor* OtherActor);
	void DamageBoxEffect_Implementation(AActor* OtherActor);

	UFUNCTION(Server, Reliable)
	void Server_DamageBoxEffect(AActor* TargetActor);
	void Server_DamageBoxEffect_Implementation(AActor* TargetActor);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_DamageBoxEffect(AActor* TargetActor);
	void Multicast_DamageBoxEffect_Implementation(AActor* TargetActor);

	//서버에서 실행되어야 하는 트리거 함정 효과
	UFUNCTION(Server, Reliable)
	void Server_CustomTrapEffect(AActor* TargetActor);
	void Server_CustomTrapEffect_Implementation(AActor* TargetActor);

	UFUNCTION(BlueprintNativeEvent)
	void CustomTrapEffect(AActor* TargetActor);
	void CustomTrapEffect_Implementation(AActor* TargetActor);


	UFUNCTION(BlueprintCallable, Category="Trap")
	void PushCharacterInBox(UBoxComponent* CollisionBox, float PushPower = 600.0f);


	
protected:

	virtual void BeginPlay() override;
	
	

};
