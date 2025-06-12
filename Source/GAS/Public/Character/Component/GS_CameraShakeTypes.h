#pragma once

#include "CoreMinimal.h"
#include "GS_CameraShakeTypes.generated.h"

class UCameraShakeBase;

/**
 * @brief 카메라 쉐이크의 상세 정보를 담는 구조체.
 */
USTRUCT(BlueprintType)
struct GAS_API FGS_CameraShakeInfo
{
	GENERATED_BODY()

	/** 재생할 카메라 쉐이크 블루프린트 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Shake")
	TSubclassOf<UCameraShakeBase> ShakeClass;

	/** 쉐이크 기본 강도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Shake", meta=(ClampMin="0.1", ClampMax="10.0"))
	float Intensity = 1.0f;

	/** 쉐이크가 영향을 미치는 최대 거리 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Shake", meta=(ClampMin="100.0", ClampMax="5000.0"))
	float MaxDistance = 2500.0f;

	/** 이 거리 안에서는 쉐이크 강도가 최대치 적용 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Shake", meta=(ClampMin="0.0", ClampMax="1000.0"))
	float MinDistance = 300.0f;

	/** 쉐이크 효과의 전파 속도 (단위: cm/s) - 0으로 하면 즉시 적용 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Shake", meta=(DisplayName="Propagation Speed (cm/s)", ClampMin="0.0"))
	float PropagationSpeed = 300000.0f;

	/** 쉐이크 감쇠 사용 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Shake")
	bool bUseFalloff = true;
}; 