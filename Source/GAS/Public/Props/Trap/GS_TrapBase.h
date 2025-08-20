#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "GS_TrapData.h"
#include "Character/GS_Character.h"
#include "Character/Component/GS_DebuffComp.h"
#include "Props/Trap/GS_TrapManager.h"
#include "Components/SphereComponent.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	USceneComponent* RootSceneComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	USceneComponent* RotationSceneComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	USceneComponent* MeshParentSceneComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	UBoxComponent* DamageBoxComp;

	//플레이어가 해당 SphereComp 오버랩 시, 함정 활성화
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Trap")
	USphereComponent* ActivateSphereComp;

	

	//함정 데이터 테이블
	UPROPERTY(EditDefaultsOnly, Category = "Trap")
	UDataTable* TrapDataTable;

	FTrapData TrapData;
	

	FTimerHandle CheckOverlapTimerHandle;

	bool bIsActivated = false;

	//함정 활성화
	UFUNCTION()
	void OnActivSCompBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(Server, Reliable)
	void Server_ActivateTrap(AActor* TargetActor);
	void Server_ActivateTrap_Implementation(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ActivateTrap(AActor* TargetActor);
	void ActivateTrap_Implementation(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void DeActivateTrap();
	void DeActivateTrap_Implementation();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EnableOptimizedCollision();
	void Multicast_EnableOptimizedCollision_Implementation();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_DisableOptimizedCollision();
	void Multicast_DisableOptimizedCollision_Implementation();



	void StartDeactivateTrapCheck();

	void CheckOverlappingSeeker();









	//Damage Box에 오버랩 되었을 때
	UFUNCTION()
	virtual void OnDamageBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
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


	//Damage 관련 함수
	UFUNCTION(Server, Reliable)
	void Server_HandleTrapDamage(AActor* TargetActor);
	void Server_HandleTrapDamage_Implementation(AActor* TargetActor);

	//UFUNCTION()
	//void ApplyDotDamage(AActor* DamagedActor);

	//데미지 박스에 오버랩된 플레이어에게 데미지 주는 함수
	virtual void HandleTrapDamage(AActor* OtherActor);
	//범위 내의 여러 플레이어에게 한 번에 데미지 주는 함수
	virtual void HandleTrapAreaDamage(const TArray<AActor*>& AffectedActors);
	//HitReactType 결정하는 함수
	virtual EHitReactType GetHitReactType() const;

	//Trap Motion
	class UGS_TrapMotionCompBase* GetValidMotionComponent() const;
	virtual bool CanStartMotion() const;
	AGS_TrapManager* GetTrapManager() const;


	void LoadTrapData();


	bool IsBlockedInDirection(const FVector& Start, const FVector& Direction, float Distance, AGS_Character* CharacterToIgnore);

	//void ClearDotTimerForActor(AActor* Actor);





protected:
	//TMap<AActor*, FTimerHandle> ActiveDoTTimers;

	virtual void BeginPlay() override;
	

};
