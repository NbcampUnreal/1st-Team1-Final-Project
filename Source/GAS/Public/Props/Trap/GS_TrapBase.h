#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "GS_TrapData.h"
#include "Character/GS_Character.h"
#include "Character/Component/GS_DebuffComp.h"
#include "Props/Trap/GS_TrapManager.h"
#include "Components/SphereComponent.h"
#include "Sound/GS_AudioComponentBase.h"
#include "GS_TrapBase.generated.h"

class UNiagaraSystem;

// Forward declarations
class UAkAudioEvent;
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

	/** 함정 사운드용 AkComponent */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
	class UAkComponent* TrapAkComponent;

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

	// ===================
	// Audio Functions
	// ===================

	/** 현재 RTS 모드인지 확인 */
	UFUNCTION(BlueprintPure, Category = "Trap|Audio")
	bool IsRTSMode() const;

	/** 모드에 맞는 사운드 이벤트 선택 */
	UFUNCTION(BlueprintPure, Category = "Trap|Audio")
	UAkAudioEvent* SelectSoundEventByMode(UAkAudioEvent* TPSSound, UAkAudioEvent* RTSSound) const;

	/** 함정 사운드 재생 최적화를 위한 거리/시야 체크 */
	UFUNCTION(BlueprintPure, Category = "Trap|Audio")
	bool ShouldPlayTrapSoundAtLocation(const FVector& TrapLocation) const;

	/** TrapAkComponent를 에디터에서 설정하는 헬퍼 함수 */
	UFUNCTION(BlueprintCallable, Category = "Audio", CallInEditor)
	void SetTrapAkComponent(class UAkComponent* NewAkComponent);



	/** 활성화 사운드 재생 */
	UFUNCTION(BlueprintCallable, Category = "Trap|Audio")
	void PlayActivationSound();

	/** 비활성화 사운드 재생 */
	UFUNCTION(BlueprintCallable, Category = "Trap|Audio")
	void PlayDeactivationSound();

	/** 함정 히트 사운드 재생 */
	UFUNCTION(BlueprintCallable, Category = "Trap|Audio")
	void PlayHitSound();

	/** 시커 외의 환경 오브젝트(바닥, 벽 등)와 충돌했을 때도 히트 사운드를 재생할지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Audio")
	bool bPlayHitSoundOnEnvironmentImpact = true;

	/** 활성화/경고 사운드는 방향 필터링을 완화할지 여부 (천장 함정 등) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Audio")
	bool bRelaxDirectionFilterForWarning = true;

	/** 활성화/경고 사운드 최대 거리 (기본값: 3000.0f = 30m, 히트 사운드보다 멈) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Audio")
	float ActivationSoundMaxDistance = 3000.0f;

	/** 오디오 위치를 일정 높이에 고정하기 위한 앵커 사용 여부 (움직이는 함정용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Audio")
	bool bUseAudioAnchor = true;

	/** Trap 오디오 앵커의 상대 위치 (RootSceneComp 기준) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap|Audio")
	FVector AudioAnchorRelativeLocation = FVector(0.0f, 0.0f, 120.0f);

	/** 오디오 앵커 컴포넌트 (편집용으로 노출) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trap|Audio")
	TObjectPtr<USceneComponent> AudioAnchorComponent;

	/** AudioAnchor 위치를 함정 배치 타입에 맞게 동적으로 조정 */
	UFUNCTION(BlueprintCallable, Category = "Trap|Audio")
	void AdjustAudioAnchorByPlacement();

	/** 서버에서 멀티캐스트로 사운드 재생 */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayActivationSound();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayDeactivationSound();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayHitSound();

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

	//Damage Box가 충돌했을 때 (바닥, 천장 등)
	UFUNCTION()
	virtual void OnDamageBoxHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

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

	// 혈흔 이펙트 재생 (멀티캐스트)
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayTrapHitBloodEffect(FVector HitLocation);
	void Multicast_PlayTrapHitBloodEffect_Implementation(FVector HitLocation);

	class UGS_TrapMotionCompBase* GetValidMotionComponent() const;
	virtual bool CanStartMotion() const;

protected:
	AGS_TrapManager* GetTrapManager() const;

	void LoadTrapData();

	bool IsBlockedInDirection(const FVector& Start, const FVector& Direction, float Distance, AGS_Character* CharacterToIgnore);

	//void ClearDotTimerForActor(AActor* Actor);

protected:
	//TMap<AActor*, FTimerHandle> ActiveDoTTimers;

	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	void RefreshTrapAudioSetup(bool bForceFindComponent = false);
	void AttachTrapAkComponentToAnchor();
	
};
