#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "GS_Door.generated.h"

// Forward declarations
class UAkAudioEvent;
class UAkComponent;

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

	/** 문 사운드용 AkComponent */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door|Audio")
	UAkComponent* DoorAkComponent;

	/** 오디오 앵커 컴포넌트 (사운드 위치 고정용) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door|Audio")
	TObjectPtr<USceneComponent> AudioAnchorComponent;

	/** Door 오디오 앵커의 상대 위치 (RootSceneComp 기준) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Audio")
	FVector AudioAnchorRelativeLocation = FVector(0.0f, 0.0f, 120.0f);

	/** 문 열림 사운드 (TPS 모드용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Audio")
	UAkAudioEvent* OpenSound_TPS;

	/** 문 열림 사운드 (RTS 모드용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Audio")
	UAkAudioEvent* OpenSound_RTS;

	/** 문 닫힘 사운드 (TPS 모드용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Audio")
	UAkAudioEvent* CloseSound_TPS;

	/** 문 닫힘 사운드 (RTS 모드용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Audio")
	UAkAudioEvent* CloseSound_RTS;

	/** 문 사운드 최대 거리 (기본값: 3000.0f = 30m) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Audio")
	float DoorSoundMaxDistance = 3000.0f;

	/** 오디오 앵커 사용 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Audio")
	bool bUseAudioAnchor = true;

	bool bIsOpen = false;

	FTimerHandle DoorCloseTimerHandle;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintImplementableEvent, Category="Door")
	void InitDoor();

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

	// ===================
	// Audio Functions
	// ===================

	/** 현재 RTS 모드인지 확인 */
	UFUNCTION(BlueprintPure, Category = "Door|Audio")
	bool IsRTSMode() const;

	/** 모드에 맞는 사운드 이벤트 선택 */
	UFUNCTION(BlueprintPure, Category = "Door|Audio")
	UAkAudioEvent* SelectSoundEventByMode(UAkAudioEvent* TPSSound, UAkAudioEvent* RTSSound) const;

	/** 문 사운드 재생 최적화를 위한 거리/시야 체크 */
	UFUNCTION(BlueprintPure, Category = "Door|Audio")
	bool ShouldPlayDoorSoundAtLocation(const FVector& DoorLocation) const;

	/** DoorAkComponent를 에디터에서 설정하는 헬퍼 함수 */
	UFUNCTION(BlueprintCallable, Category = "Door|Audio", CallInEditor)
	void SetDoorAkComponent(UAkComponent* NewAkComponent);

	/** 문 열림 사운드 재생 */
	UFUNCTION(BlueprintCallable, Category = "Door|Audio")
	void PlayOpenSound();

	/** 문 닫힘 사운드 재생 */
	UFUNCTION(BlueprintCallable, Category = "Door|Audio")
	void PlayCloseSound();

	/** 서버에서 멀티캐스트로 사운드 재생 */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayOpenSound();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayCloseSound();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	void RefreshDoorAudioSetup(bool bForceFindComponent = false);
	void AttachDoorAkComponentToAnchor();

	/** 안전한 타이머 정리 함수 (레벨 전환 안정성) */
	void SafeClearTimer(FTimerHandle& TimerHandle);

	/** 월드 컨텍스트 검증 함수 */
	bool IsWorldContextValid() const;
};
