#include "Character/Component/GS_CameraShakeComponent.h"
#include "Camera/CameraShakeBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

UGS_CameraShakeComponent::UGS_CameraShakeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UGS_CameraShakeComponent::PlayCameraShake(const FGS_CameraShakeInfo& ShakeInfo)
{
	AActor* Owner = GetOwner();
	if (Owner && Owner->HasAuthority())
	{
		Multicast_PlayCameraShake(ShakeInfo, Owner->GetActorLocation());
	}
}

void UGS_CameraShakeComponent::PlayCameraShakeAtLocation(const FGS_CameraShakeInfo& ShakeInfo, const FVector& EpicenterLocation)
{
	AActor* Owner = GetOwner();
	if (Owner && Owner->HasAuthority())
	{
		Multicast_PlayCameraShake(ShakeInfo, EpicenterLocation);
	}
}

void UGS_CameraShakeComponent::Multicast_PlayCameraShake_Implementation(const FGS_CameraShakeInfo& ShakeInfo, const FVector& EpicenterLocation)
{
	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_DedicatedServer || !ShakeInfo.ShakeClass)
	{
		return;
	}

	if (ShakeInfo.bUseFalloff)
	{
		// 거리 감쇠를 사용하는 경우
		float InnerRadius = ShakeInfo.MinDistance;
		float OuterRadius = ShakeInfo.MaxDistance;

		UGameplayStatics::PlayWorldCameraShake(
			World,
			ShakeInfo.ShakeClass,
			EpicenterLocation,
			InnerRadius,
			OuterRadius,
			ShakeInfo.Intensity,
			false  // bOrientShakeTowardsEpicenter
		);
	}
	else
	{
		// 거리 감쇠 없이 모든 플레이어에게 동일한 강도로 적용
		for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PC = Iterator->Get();
			if (PC && PC->IsLocalController())
			{
				APawn* ControlledPawn = PC->GetPawn();
				if (!ControlledPawn) continue;

				float Distance = FVector::Dist(ControlledPawn->GetActorLocation(), EpicenterLocation);
				
				if (Distance <= ShakeInfo.MaxDistance)
				{
					float AdjustedIntensity = CalculateShakeIntensityByDistance(ShakeInfo, Distance);
					
					if (AdjustedIntensity > 0.01f)
					{
						// 전파 지연 효과 적용
						if (ShakeInfo.PropagationSpeed > 0.0f)
						{
							float Delay = Distance / ShakeInfo.PropagationSpeed;
							
							FTimerHandle ShakeTimerHandle;
							FTimerDelegate ShakeDelegate = FTimerDelegate::CreateWeakLambda(PC, [PC, ShakeClass = ShakeInfo.ShakeClass, AdjustedIntensity]()
							{
								if (PC)
								{
									PC->ClientStartCameraShake(ShakeClass, AdjustedIntensity);
								}
							});
							World->GetTimerManager().SetTimer(ShakeTimerHandle, ShakeDelegate, Delay, false);
						}
						else
						{
							PC->ClientStartCameraShake(ShakeInfo.ShakeClass, AdjustedIntensity);
						}
					}
				}
			}
		}
	}
}

float UGS_CameraShakeComponent::CalculateShakeIntensityByDistance(const FGS_CameraShakeInfo& ShakeInfo, float Distance) const
{
	if (Distance <= ShakeInfo.MinDistance)
	{
		return ShakeInfo.Intensity;
	}
	if (Distance >= ShakeInfo.MaxDistance)
	{
		return 0.0f;
	}

	// 거리에 따라 부드럽게 감소하는 곡선 (제곱근 사용)
	const float DistanceRatio = (Distance - ShakeInfo.MinDistance) / (ShakeInfo.MaxDistance - ShakeInfo.MinDistance);
	return ShakeInfo.Intensity * (1.0f - FMath::Sqrt(DistanceRatio));
} 