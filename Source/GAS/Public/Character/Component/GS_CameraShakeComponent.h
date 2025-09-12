#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/Component/GS_CameraShakeTypes.h"
#include "GS_CameraShakeComponent.generated.h"

class UCameraShakeBase;

/**
 * @brief 재사용 가능한 카메라 쉐이크 액터 컴포넌트.
 * 멀티플레이어 환경을 지원하며, 거리 감쇠 및 전파 지연 효과 포함.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAS_API UGS_CameraShakeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UGS_CameraShakeComponent();

	/**
	 * @brief 설정된 정보로 카메라 쉐이크를 재생. 서버에서만 호출.
	 * @param ShakeInfo 재생할 쉐이크 정보.
	 */
	UFUNCTION(BlueprintCallable, Category="Camera Shake")
	void PlayCameraShake(const FGS_CameraShakeInfo& ShakeInfo);

	/**
	 * @brief 특정 위치에서 카메라 쉐이크를 재생. 서버에서만 호출.
	 * @param ShakeInfo 재생할 쉐이크 정보.
	 * @param EpicenterLocation 쉐이크 진원지 위치.
	 */
	UFUNCTION(BlueprintCallable, Category="Camera Shake")
	void PlayCameraShakeAtLocation(const FGS_CameraShakeInfo& ShakeInfo, const FVector& EpicenterLocation);

protected:
	/**
	 * @brief 모든 클라이언트에게 카메라 쉐이크를 재생하도록 요청하는 멀티캐스트 함수.
	 * @param ShakeInfo 재생할 쉐이크 정보.
	 * @param EpicenterLocation 쉐이크의 진원지 위치.
	 */
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayCameraShake(const FGS_CameraShakeInfo& ShakeInfo, const FVector& EpicenterLocation);

private:
	/**
	 * @brief 진원지로부터의 거리에 따라 쉐이크 강도를 계산.
	 * @param ShakeInfo 쉐이크 정보.
	 * @param Distance 진원지로부터의 거리.
	 * @return 계산된 쉐이크 강도.
	 */
	float CalculateShakeIntensityByDistance(const FGS_CameraShakeInfo& ShakeInfo, float Distance) const;
}; 