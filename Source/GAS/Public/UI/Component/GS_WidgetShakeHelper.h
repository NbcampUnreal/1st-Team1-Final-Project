#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GS_WidgetShakeHelper.generated.h"

class UWidget;

UCLASS(BlueprintType)
class GAS_API UGS_WidgetShakeHelper : public UObject
{
	GENERATED_BODY()

public:
	UGS_WidgetShakeHelper();

	// 흔들림 효과 시작
	UFUNCTION(BlueprintCallable, Category = "Widget Shake")
	void StartShakeEffect(float DamageRatio, float ActualDamage);

	// 타겟 위젯 설정
	UFUNCTION(BlueprintCallable, Category = "Widget Shake")
	void SetTargetWidget(UWidget* InTargetWidget);

	// 초기화 및 정리
	void Initialize(UWorld* InWorld);
	void Cleanup();

protected:
	// 피해 효과 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Effect")
	float ShakeDuration = 0.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Effect")
	float ShakeFrequency = 60.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Effect|Intensity")
	float MinorShakeIntensity = 10.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Effect|Intensity")
	float LightShakeIntensity = 10.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Effect|Intensity")
	float ModerateShakeIntensity = 20.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Effect|Intensity")
	float HeavyShakeIntensity = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Effect|Thresholds")
	float MinorDamageThreshold = 0.05f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Effect|Thresholds")
	float LightDamageThreshold = 0.15f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Effect|Thresholds")
	float ModerateDamageThreshold = 0.50f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Effect|Thresholds")
	float HeavyDamageThreshold = 0.70f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Effect|Scaling")
	float DamageScalingStart = 10.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Effect|Scaling")
	float DamageScalingEnd = 200.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Effect|Scaling")
	float MinDamageMultiplier = 0.9f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Effect|Scaling")
	float MaxDamageMultiplier = 1.5f;

private:
	UPROPERTY()
	TWeakObjectPtr<UWidget> TargetWidget;

	TWeakObjectPtr<UWorld> CachedWorld;

	// 흔들림 상태 변수들
	bool bIsShaking = false;
	float ShakeTimer = 0.0f;
	FVector2D OriginalPosition;
	FTimerHandle ShakeTimerHandle;
	float CurrentDamageRatio = 0.0f;
	float CurrentActualDamage = 0.0f;
	
	// 흔들림 함수들
	void UpdateShakeEffect();
	void StopShakeEffect();
	float CalculateShakeIntensity(float DamageRatio, float ActualDamage);
};
