#include "UI/Component/GS_WidgetShakeHelper.h"
#include "Components/Widget.h"
#include "Engine/World.h"

UGS_WidgetShakeHelper::UGS_WidgetShakeHelper()
{
}

void UGS_WidgetShakeHelper::Initialize(UWorld* InWorld)
{
	CachedWorld = InWorld;
}

void UGS_WidgetShakeHelper::Cleanup()
{
	if (CachedWorld.IsValid())
	{
		CachedWorld->GetTimerManager().ClearTimer(ShakeTimerHandle);
	}
	
	StopShakeEffect();
}

void UGS_WidgetShakeHelper::SetTargetWidget(UWidget* InTargetWidget)
{
	TargetWidget = InTargetWidget;
	
	// 타겟 위젯이 설정되면 원본 위치 저장
	if (TargetWidget.IsValid())
	{
		OriginalPosition = TargetWidget->GetRenderTransform().Translation;
	}
}

void UGS_WidgetShakeHelper::StartShakeEffect(float DamageRatio, float ActualDamage)
{
	if (bIsShaking || !TargetWidget.IsValid() || !CachedWorld.IsValid()) 
	{
		return;
	}
	
	// 매우 작은 피해(1 데미지 미만)만 제외
	if (ActualDamage < 1.0f)
	{
		return;
	}
	
	bIsShaking = true;
	ShakeTimer = 0.0f;
	CurrentDamageRatio = DamageRatio;
	CurrentActualDamage = ActualDamage;
	
	// 타이머를 사용해서 흔들림 효과 업데이트
	CachedWorld->GetTimerManager().SetTimer(ShakeTimerHandle, this, &UGS_WidgetShakeHelper::UpdateShakeEffect, 0.016f, true);
}

float UGS_WidgetShakeHelper::CalculateShakeIntensity(float DamageRatio, float ActualDamage)
{
	// 1. HP 비율 기반 강도 계산
	float RatioBasedIntensity = 0.0f;
	if (DamageRatio >= HeavyDamageThreshold) 
		RatioBasedIntensity = HeavyShakeIntensity;
	else if (DamageRatio >= ModerateDamageThreshold) 
		RatioBasedIntensity = FMath::Lerp(ModerateShakeIntensity, HeavyShakeIntensity, (DamageRatio - ModerateDamageThreshold) / (HeavyDamageThreshold - ModerateDamageThreshold));
	else if (DamageRatio >= LightDamageThreshold) 
		RatioBasedIntensity = FMath::Lerp(LightShakeIntensity, ModerateShakeIntensity, (DamageRatio - LightDamageThreshold) / (ModerateDamageThreshold - LightDamageThreshold));
	else if (DamageRatio >= MinorDamageThreshold) 
		RatioBasedIntensity = FMath::Lerp(MinorShakeIntensity, LightShakeIntensity, (DamageRatio - MinorDamageThreshold) / (LightDamageThreshold - MinorDamageThreshold));

	// 2. 실제 데미지 기반 최소 강도 보장
	float DamageBasedIntensity = 0.0f;
	if (ActualDamage >= 100.0f) 
		DamageBasedIntensity = ModerateShakeIntensity;
	else if (ActualDamage >= 70.0f) 
		DamageBasedIntensity = LightShakeIntensity;
	else if (ActualDamage >= 30.0f) 
		DamageBasedIntensity = MinorShakeIntensity;
	else if (ActualDamage >= 10.0f) 
		DamageBasedIntensity = MinorShakeIntensity * 0.7f;
	else if (ActualDamage >= 1.0f) 
		DamageBasedIntensity = MinorShakeIntensity * 0.4f;

	// 3. 두 강도 중 더 큰 값을 기본 강도로 사용
	float BaseIntensity = FMath::Max(RatioBasedIntensity, DamageBasedIntensity);

	// 4. 실제 데미지량에 따른 동적 배율 적용
	float DamageMultiplier = 1.0f;
	if (ActualDamage >= DamageScalingStart)
	{
		float Progress = FMath::Clamp((ActualDamage - DamageScalingStart) / (DamageScalingEnd - DamageScalingStart), 0.0f, 1.0f);
		DamageMultiplier = FMath::Lerp(MinDamageMultiplier, MaxDamageMultiplier, Progress);
	}
	else
	{
		DamageMultiplier = MinDamageMultiplier;
	}

	// 최종 강도 계산
	float FinalIntensity = BaseIntensity * DamageMultiplier;
	return FMath::Clamp(FinalIntensity, 0.1f, 30.0f);
}

void UGS_WidgetShakeHelper::UpdateShakeEffect()
{
	if (!bIsShaking || !TargetWidget.IsValid()) 
	{
		StopShakeEffect();
		return;
	}
	
	ShakeTimer += 0.016f; // 16ms 간격
	
	if (ShakeTimer >= ShakeDuration)
	{
		StopShakeEffect();
		return;
	}
	
	// 사인파를 사용한 흔들림 효과
	float ShakeProgress = ShakeTimer / ShakeDuration;
	float ShakeDecay = 1.0f - ShakeProgress; // 시간이 지날수록 흔들림 감소
	
	// 저장된 피해 비율과 실제 피해량을 사용해서 흔들림 강도 계산
	float CurrentShakeIntensity = CalculateShakeIntensity(CurrentDamageRatio, CurrentActualDamage);
	
	float ShakeX = FMath::Sin(ShakeTimer * ShakeFrequency) * CurrentShakeIntensity * ShakeDecay;
	float ShakeY = FMath::Cos(ShakeTimer * ShakeFrequency * 0.7f) * CurrentShakeIntensity * 0.3f * ShakeDecay;
	
	FVector2D NewPosition = OriginalPosition + FVector2D(ShakeX, ShakeY);
	
	// 위젯 위치 업데이트
	FWidgetTransform Transform = TargetWidget->GetRenderTransform();
	Transform.Translation = NewPosition;
	TargetWidget->SetRenderTransform(Transform);
}

void UGS_WidgetShakeHelper::StopShakeEffect()
{
	bIsShaking = false;
	
	// 흔들림 타이머 정리
	if (CachedWorld.IsValid())
	{
		CachedWorld->GetTimerManager().ClearTimer(ShakeTimerHandle);
	}
	
	// 원래 위치로 복원
	if (TargetWidget.IsValid())
	{
		FWidgetTransform Transform = TargetWidget->GetRenderTransform();
		Transform.Translation = OriginalPosition;
		TargetWidget->SetRenderTransform(Transform);
	}
}
