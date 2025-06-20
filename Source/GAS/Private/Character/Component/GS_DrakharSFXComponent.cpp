#include "Character/Component/GS_DrakharSFXComponent.h"
#include "Character/Player/Guardian/GS_Drakhar.h"
#include "AkGameplayStatics.h"
#include "AkAudioEvent.h"
#include "AkComponent.h"
#include "Kismet/GameplayStatics.h"

UGS_DrakharSFXComponent::UGS_DrakharSFXComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bDraconicFurySoundPlayed = false;
}

void UGS_DrakharSFXComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerDrakhar = Cast<AGS_Drakhar>(GetOwner());
}

// === 사운드 재생 함수 구현 ===
void UGS_DrakharSFXComponent::PlayComboAttackSound()
{
	if(OwnerDrakhar) PlaySoundEvent(OwnerDrakhar->ComboAttackSoundEvent);
}

void UGS_DrakharSFXComponent::PlayDashSkillSound()
{
	if(OwnerDrakhar) PlaySoundEvent(OwnerDrakhar->DashSkillSoundEvent);
}

void UGS_DrakharSFXComponent::PlayEarthquakeSkillSound()
{
	if(OwnerDrakhar) PlaySoundEvent(OwnerDrakhar->EarthquakeSkillSoundEvent);
}

void UGS_DrakharSFXComponent::PlayDraconicFurySkillSound()
{
	if (!bDraconicFurySoundPlayed && OwnerDrakhar)
	{
		PlaySoundEvent(OwnerDrakhar->DraconicFurySkillSoundEvent);
		bDraconicFurySoundPlayed = true;

		FTimerHandle ResetSoundTimer;
		GetWorld()->GetTimerManager().SetTimer(ResetSoundTimer, [this]()
		{
			bDraconicFurySoundPlayed = false;
		}, 7.0f, false);
	}
}

void UGS_DrakharSFXComponent::PlayDraconicProjectileSound(const FVector& Location)
{
	if(OwnerDrakhar) PlaySoundEvent(OwnerDrakhar->DraconicProjectileSoundEvent, Location);
}

void UGS_DrakharSFXComponent::PlayAttackHitSound()
{
	UE_LOG(LogTemp, Warning, TEXT("UGS_DrakharSFXComponent::PlayAttackHitSound() called"));
	
	if (!OwnerDrakhar)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayAttackHitSound: OwnerDrakhar is null"));
		return;
	}
	
	if (!OwnerDrakhar->AttackHitSoundEvent)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayAttackHitSound: AttackHitSoundEvent is null - Check Blueprint settings"));
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("PlayAttackHitSound: Playing AttackHitSoundEvent"));
	PlaySoundEvent(OwnerDrakhar->AttackHitSoundEvent);
}

void UGS_DrakharSFXComponent::PlayFeverModeStartSound()
{
	if(OwnerDrakhar) PlaySoundEvent(OwnerDrakhar->FeverModeStartSoundEvent);
}

void UGS_DrakharSFXComponent::HandleDraconicProjectileImpact(const FVector& ImpactLocation, bool bHitCharacter)
{
	if (!OwnerDrakhar) return;
	
	UAkAudioEvent* SoundToPlay = bHitCharacter ? OwnerDrakhar->DraconicProjectileExplosionSoundEvent : OwnerDrakhar->DraconicProjectileImpactSoundEvent;
	if (SoundToPlay)
	{
		PlaySoundEvent(SoundToPlay, ImpactLocation);
	}
}

// === Wwise 헬퍼 함수 구현 ===
void UGS_DrakharSFXComponent::PlaySoundEvent(UAkAudioEvent* SoundEvent, const FVector& Location)
{
	if (!OwnerDrakhar) return;
    
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) return;

	if (!SoundEvent)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGS_DrakharSFXComponent::PlaySoundEvent - SoundEvent is null"));
		return;
	}

	if (!FAkAudioDevice::Get())
	{
		UE_LOG(LogTemp, Warning, TEXT("UGS_DrakharSFXComponent::PlaySoundEvent - Wwise AudioDevice is not initialized"));
		return;
	}

	if (Location != FVector::ZeroVector)
	{
		UAkGameplayStatics::PostEventAtLocation(SoundEvent, Location, FRotator::ZeroRotator, GetWorld());
	}
	else
	{
		UAkComponent* AkComp = GetOrCreateAkComponent();
		if (AkComp)
		{
			AkComp->PostAkEvent(SoundEvent);
		}
	}
}

UAkComponent* UGS_DrakharSFXComponent::GetOrCreateAkComponent()
{
	if (!OwnerDrakhar) return nullptr;

	UAkComponent* AkComp = OwnerDrakhar->FindComponentByClass<UAkComponent>();
	if (!AkComp)
	{
		AkComp = NewObject<UAkComponent>(OwnerDrakhar, TEXT("RuntimeAkAudioComponent"));
		if (AkComp)
		{
			AkComp->SetupAttachment(OwnerDrakhar->GetRootComponent());
			AkComp->RegisterComponent();
			UE_LOG(LogTemp, Log, TEXT("UGS_DrakharSFXComponent: AkComponent created at runtime for GS_Drakhar"));
		}
	}
	return AkComp;
} 