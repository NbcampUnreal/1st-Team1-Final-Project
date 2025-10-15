// Copyright Epic Games, Inc. All Rights Reserved.


#include "VFX/GS_VFX_FunctionLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void UGS_VFX_FunctionLibrary::PlayBloodEffect(UObject* WorldContextObject, UNiagaraSystem* BloodEffectSystem, const FVector& Location, const FRotator& Rotation, float Scale)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World || World->GetNetMode() == NM_DedicatedServer)
    {
        return;
    }

	UNiagaraSystem* EffectToPlay = BloodEffectSystem;
	
	// BloodEffectSystem이 없으면 기본 혈흔 이펙트 사용
	if (!EffectToPlay)
	{
		EffectToPlay = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/RealisticBlood/Burst/Niagara/NS_BloodBurst_High.NS_BloodBurst_High"));
	}
	
	if (EffectToPlay)
	{
		// 스케일 적용
		FVector BloodScale = FVector(Scale);
		
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			EffectToPlay,
			Location,
			Rotation,
			BloodScale,
			true,
			true
		);
	}
	else 
	{
		UE_LOG(LogTemp, Warning, TEXT("BloodEffect could not be loaded or spawned"));
	}
}
