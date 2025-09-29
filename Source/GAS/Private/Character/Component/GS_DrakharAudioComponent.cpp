#include "Character/Component/GS_DrakharAudioComponent.h"
#include "Character/Player/Guardian/GS_Drakhar.h"
#include "AkGameplayStatics.h"
#include "AkAudioEvent.h"
#include "AkComponent.h"
#include "Kismet/GameplayStatics.h"

UGS_DrakharAudioComponent::UGS_DrakharAudioComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bDraconicFurySoundPlayed = false;
	bHurtSoundPlayed = false;
}

void UGS_DrakharAudioComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerDrakhar = Cast<AGS_Drakhar>(GetOwner());
}

// === 사운드 재생 함수 구현 ===
void UGS_DrakharAudioComponent::PlayComboAttackSound()
{
	if(OwnerDrakhar) 
	{
		PlaySoundEvent(OwnerDrakhar->ComboAttackSoundEvent, OwnerDrakhar->GetActorLocation());
	}
}

void UGS_DrakharAudioComponent::PlayDashSkillSound()
{
	if(OwnerDrakhar) 
	{
		PlaySoundEvent(OwnerDrakhar->DashSkillSoundEvent, OwnerDrakhar->GetActorLocation());
	}
}

void UGS_DrakharAudioComponent::PlayEarthquakeSkillSound()
{
	if(OwnerDrakhar) 
	{
		PlaySoundEvent(OwnerDrakhar->EarthquakeSkillSoundEvent, OwnerDrakhar->GetActorLocation());
	}
}

void UGS_DrakharAudioComponent::PlayDraconicFurySkillSound()
{
	if (!bDraconicFurySoundPlayed && OwnerDrakhar)
	{
		PlaySoundEvent(OwnerDrakhar->DraconicFurySkillSoundEvent, OwnerDrakhar->GetActorLocation());
		bDraconicFurySoundPlayed = true;

		FTimerHandle ResetSoundTimer;
		GetWorld()->GetTimerManager().SetTimer(ResetSoundTimer, [this]()
		{
			bDraconicFurySoundPlayed = false;
		}, 7.0f, false);
	}
}

void UGS_DrakharAudioComponent::PlayDraconicProjectileSound(const FVector& Location)
{
	if(OwnerDrakhar) PlaySoundEvent(OwnerDrakhar->DraconicProjectileSoundEvent, Location);
}

void UGS_DrakharAudioComponent::PlayAttackHitSound()
{
	if (!OwnerDrakhar)
	{
		return;
	}
	
	if (!OwnerDrakhar->AttackHitSoundEvent)
	{
		return;
	}
	
	PlaySoundEvent(OwnerDrakhar->AttackHitSoundEvent, OwnerDrakhar->GetActorLocation());
}

void UGS_DrakharAudioComponent::PlayFeverModeStartSound()
{
	if(OwnerDrakhar) 
	{
		PlaySoundEvent(OwnerDrakhar->FeverModeStartSoundEvent, OwnerDrakhar->GetActorLocation());
	}
}

void UGS_DrakharAudioComponent::PlayHurtSound()
{
	if (!bHurtSoundPlayed && OwnerDrakhar)
	{
		PlaySoundEvent(OwnerDrakhar->HurtSoundEvent, OwnerDrakhar->GetActorLocation());
		bHurtSoundPlayed = true;

		// N초 후에 다시 재생 가능하도록 설정
		FTimerHandle ResetHurtSoundTimer;
		GetWorld()->GetTimerManager().SetTimer(ResetHurtSoundTimer, [this]()
		{
			bHurtSoundPlayed = false;
		}, 1.0f, false);
	}
}

void UGS_DrakharAudioComponent::HandleDraconicProjectileImpact(const FVector& ImpactLocation, bool bHitCharacter)
{
	if (!OwnerDrakhar) return;
	
	UAkAudioEvent* SoundToPlay = bHitCharacter ? OwnerDrakhar->DraconicProjectileExplosionSoundEvent : OwnerDrakhar->DraconicProjectileImpactSoundEvent;
	if (SoundToPlay)
	{
		PlaySoundEvent(SoundToPlay, ImpactLocation);
	}
}

// === Wwise 헬퍼 함수 구현 ===
void UGS_DrakharAudioComponent::PlaySoundEvent(UAkAudioEvent* SoundEvent, const FVector& Location)
{
	// 멀티플레이어 환경에서 안전성 체크 강화
	if (!OwnerDrakhar)
	{
		return;
	}
    
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) 
	{
		return;
	}
	
	// 사운드 이벤트 유효성 검사
	if (!SoundEvent)
	{
		return;
	}

	// Wwise 오디오 디바이스 초기화 상태 확인
	FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
	if (!AudioDevice)
	{
		return;
	}

	// 오디오 디바이스가 초기화되었는지 확인
	if (!AudioDevice->IsInitialized())
	{
		return;
	}

	// 월드 컨텍스트 유효성 검사
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 위치 기반 사운드 재생
	if (Location != FVector::ZeroVector)
	{
		if (IsValid(SoundEvent) && World->IsValidLowLevel())
		{
			UAkGameplayStatics::PostEventAtLocation(SoundEvent, Location, FRotator::ZeroRotator, World);
		}
	}
	else
	{
		UAkComponent* AkComp = GetOrCreateAkComponent();
		if (AkComp && IsValid(AkComp))
		{
			AkComp->PostAkEvent(SoundEvent);
		}
	}
}

UAkComponent* UGS_DrakharAudioComponent::GetOrCreateAkComponent()
{
	if (!OwnerDrakhar) 
	{
		return nullptr;
	}

	if (!IsValid(OwnerDrakhar))
	{
		return nullptr;
	}

	UAkComponent* AkComp = OwnerDrakhar->FindComponentByClass<UAkComponent>();
	if (!AkComp)
	{
		if (OwnerDrakhar->GetRootComponent() && IsValid(OwnerDrakhar->GetRootComponent()))
		{
			AkComp = NewObject<UAkComponent>(OwnerDrakhar, TEXT("RuntimeAkAudioComponent"));
			if (AkComp && IsValid(AkComp))
			{
				AkComp->SetupAttachment(OwnerDrakhar->GetRootComponent());
				AkComp->RegisterComponent();
			}
		}
	}
	return AkComp;
} 