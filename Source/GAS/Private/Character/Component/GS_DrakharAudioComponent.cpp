#include "Character/Component/GS_DrakharAudioComponent.h"
#include "Character/Player/Guardian/GS_Drakhar.h"
#include "AkGameplayStatics.h"
#include "AkAudioEvent.h"
#include "AkComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AkAudioDevice.h"

UGS_DrakharAudioComponent::UGS_DrakharAudioComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bDraconicFurySoundPlayed = false;
	bHurtSoundPlayed = false;
	FeverModeStateSoundPlayingID = AK_INVALID_PLAYING_ID;
}

void UGS_DrakharAudioComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerDrakhar = Cast<AGS_Drakhar>(GetOwner());
}

void UGS_DrakharAudioComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 모든 타이머 정리 (레벨 전환 시 크래시 방지)
	if (UWorld* World = GetWorld())
	{
		if (World->IsValidLowLevel() && !World->bIsTearingDown)
		{
			FTimerManager& TimerManager = World->GetTimerManager();

			if (DraconicFurySoundCooldownTimer.IsValid())
			{
				TimerManager.ClearTimer(DraconicFurySoundCooldownTimer);
				DraconicFurySoundCooldownTimer.Invalidate();
			}

			if (HurtSoundCooldownTimer.IsValid())
			{
				TimerManager.ClearTimer(HurtSoundCooldownTimer);
				HurtSoundCooldownTimer.Invalidate();
			}
		}
	}

	Super::EndPlay(EndPlayReason);
}

// === 사운드 재생 함수 구현 ===
void UGS_DrakharAudioComponent::PlayComboAttackSound()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (!CanSendRPC())
	{
		return;
	}

	LastMulticastTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	Multicast_PlayComboAttackSound();
}

void UGS_DrakharAudioComponent::Multicast_PlayComboAttackSound_Implementation()
{
	if (ShouldSkipListenServerRPC())
	{
		return;
	}

	if (!OwnerDrakhar)
	{
		return;
	}

	PlaySoundEvent(OwnerDrakhar->ComboAttackSoundEvent, OwnerDrakhar->GetActorLocation());
}

void UGS_DrakharAudioComponent::PlayDashSkillSound()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (!CanSendRPC())
	{
		return;
	}

	LastMulticastTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	Multicast_PlayDashSkillSound();
}

void UGS_DrakharAudioComponent::Multicast_PlayDashSkillSound_Implementation()
{
	if (ShouldSkipListenServerRPC())
	{
		return;
	}

	if (!OwnerDrakhar)
	{
		return;
	}

	PlaySoundEvent(OwnerDrakhar->DashSkillSoundEvent, OwnerDrakhar->GetActorLocation());
}

void UGS_DrakharAudioComponent::PlayEarthquakeSkillSound()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (!CanSendRPC())
	{
		return;
	}

	LastMulticastTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	Multicast_PlayEarthquakeSkillSound();
}

void UGS_DrakharAudioComponent::Multicast_PlayEarthquakeSkillSound_Implementation()
{
	if (ShouldSkipListenServerRPC())
	{
		return;
	}

	if (!OwnerDrakhar)
	{
		return;
	}

	PlaySoundEvent(OwnerDrakhar->EarthquakeSkillSoundEvent, OwnerDrakhar->GetActorLocation());
}

void UGS_DrakharAudioComponent::PlayDraconicFurySkillSound()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (bDraconicFurySoundPlayed)
	{
		return;
	}

	if (!CanSendRPC())
	{
		return;
	}

	LastMulticastTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	Multicast_PlayDraconicFurySkillSound();
}

void UGS_DrakharAudioComponent::Multicast_PlayDraconicFurySkillSound_Implementation()
{
	if (ShouldSkipListenServerRPC())
	{
		return;
	}

	if (!OwnerDrakhar || bDraconicFurySoundPlayed)
	{
		return;
	}

	PlaySoundEvent(OwnerDrakhar->DraconicFurySkillSoundEvent, OwnerDrakhar->GetActorLocation());
	bDraconicFurySoundPlayed = true;

	// 타이머 설정
	if (UWorld* World = GetWorld())
	{
		if (World->IsValidLowLevel() && !World->bIsTearingDown)
		{
			World->GetTimerManager().SetTimer(
				DraconicFurySoundCooldownTimer,
				this,
				&UGS_DrakharAudioComponent::ResetDraconicFurySoundCooldown,
				DraconicFurySoundCooldown,
				false
			);
		}
	}
}

void UGS_DrakharAudioComponent::PlayDraconicProjectileSound(const FVector& Location)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (!CanSendRPC())
	{
		return;
	}

	LastMulticastTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	Multicast_PlayDraconicProjectileSound(Location);
}

void UGS_DrakharAudioComponent::Multicast_PlayDraconicProjectileSound_Implementation(const FVector& Location)
{
	if (ShouldSkipListenServerRPC())
	{
		return;
	}

	if (!OwnerDrakhar)
	{
		return;
	}

	PlaySoundEvent(OwnerDrakhar->DraconicProjectileSoundEvent, Location);
}

void UGS_DrakharAudioComponent::PlayAttackHitSound()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (!CanSendRPC())
	{
		return;
	}

	LastMulticastTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	Multicast_PlayAttackHitSound();
}

void UGS_DrakharAudioComponent::Multicast_PlayAttackHitSound_Implementation()
{
	if (ShouldSkipListenServerRPC())
	{
		return;
	}

	if (!OwnerDrakhar || !OwnerDrakhar->AttackHitSoundEvent)
	{
		return;
	}

	PlaySoundEvent(OwnerDrakhar->AttackHitSoundEvent, OwnerDrakhar->GetActorLocation());
}

void UGS_DrakharAudioComponent::PlayFeverModeStartSound()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (!CanSendRPC())
	{
		return;
	}

	LastMulticastTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	Multicast_PlayFeverModeStartSound();
}

void UGS_DrakharAudioComponent::Multicast_PlayFeverModeStartSound_Implementation()
{
	if (ShouldSkipListenServerRPC())
	{
		return;
	}

	if (!OwnerDrakhar)
	{
		return;
	}

	if (!OwnerDrakhar->FeverModeStartSoundEvent)
	{
		return;
	}

	PlaySoundEvent(OwnerDrakhar->FeverModeStartSoundEvent, OwnerDrakhar->GetActorLocation());
}

void UGS_DrakharAudioComponent::PlayFeverModeEndSound()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (!CanSendRPC())
	{
		return;
	}

	LastMulticastTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	Multicast_PlayFeverModeEndSound();
}

void UGS_DrakharAudioComponent::Multicast_PlayFeverModeEndSound_Implementation()
{
	if (ShouldSkipListenServerRPC())
	{
		return;
	}

	if (!OwnerDrakhar)
	{
		return;
	}

	if (!OwnerDrakhar->FeverModeEndSoundEvent)
	{
		return;
	}

	PlaySoundEvent(OwnerDrakhar->FeverModeEndSoundEvent, OwnerDrakhar->GetActorLocation());
}

void UGS_DrakharAudioComponent::PlayFeverModeStateSound()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (!CanSendRPC())
	{
		return;
	}

	LastMulticastTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	Multicast_PlayFeverModeStateSound();
}

void UGS_DrakharAudioComponent::Multicast_PlayFeverModeStateSound_Implementation()
{
	if (ShouldSkipListenServerRPC())
	{
		return;
	}

	if (!OwnerDrakhar)
	{
		return;
	}

	if (!OwnerDrakhar->FeverModeStateSoundEvent)
	{
		return;
	}

	// 오디오 시스템 검증
	if (!IsAudioSystemValid())
	{
		return;
	}

	// 피버모드 스테이트 사운드 재생 및 Playing ID 저장
	FeverModeStateSoundPlayingID = UAkGameplayStatics::PostEvent(
		OwnerDrakhar->FeverModeStateSoundEvent,
		OwnerDrakhar,
		0,
		FOnAkPostEventCallback()
	);

}

void UGS_DrakharAudioComponent::StopFeverModeStateSound()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (!CanSendRPC())
	{
		return;
	}

	LastMulticastTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	Multicast_StopFeverModeStateSound();
}

void UGS_DrakharAudioComponent::Multicast_StopFeverModeStateSound_Implementation()
{
	if (ShouldSkipListenServerRPC())
	{
		return;
	}

	if (!OwnerDrakhar)
	{
		return;
	}

	// 오디오 시스템 검증
	if (!IsAudioSystemValid())
	{
		return;
	}

	// Playing ID가 유효하면 FAkAudioDevice를 통해 중지
	if (FeverModeStateSoundPlayingID != AK_INVALID_PLAYING_ID)
	{

		// FAkAudioDevice를 통해 StopPlayingID 호출
		FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
		if (AudioDevice != nullptr)
		{
			AudioDevice->StopPlayingID(FeverModeStateSoundPlayingID, FeverModeStateFadeOutDuration);
			FeverModeStateSoundPlayingID = AK_INVALID_PLAYING_ID;
		}
	}
}

void UGS_DrakharAudioComponent::PlayHurtSound()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (bHurtSoundPlayed)
	{
		return;
	}

	if (!CanSendRPC())
	{
		return;
	}

	LastMulticastTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	Multicast_PlayHurtSound();
}

void UGS_DrakharAudioComponent::Multicast_PlayHurtSound_Implementation()
{
	if (ShouldSkipListenServerRPC())
	{
		return;
	}

	if (!OwnerDrakhar || bHurtSoundPlayed)
	{
		return;
	}

	PlaySoundEvent(OwnerDrakhar->HurtSoundEvent, OwnerDrakhar->GetActorLocation());
	bHurtSoundPlayed = true;

	// 타이머 설정
	if (UWorld* World = GetWorld())
	{
		if (World->IsValidLowLevel() && !World->bIsTearingDown)
		{
			World->GetTimerManager().SetTimer(
				HurtSoundCooldownTimer,
				this,
				&UGS_DrakharAudioComponent::ResetHurtSoundCooldown,
				HurtSoundCooldown,
				false
			);
		}
	}
}

void UGS_DrakharAudioComponent::HandleDraconicProjectileImpact(const FVector& ImpactLocation, bool bHitCharacter)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (!CanSendRPC())
	{
		return;
	}

	LastMulticastTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	Multicast_HandleDraconicProjectileImpact(ImpactLocation, bHitCharacter);
}

void UGS_DrakharAudioComponent::Multicast_HandleDraconicProjectileImpact_Implementation(const FVector& ImpactLocation, bool bHitCharacter)
{
	if (ShouldSkipListenServerRPC())
	{
		return;
	}

	if (!OwnerDrakhar)
	{
		return;
	}

	UAkAudioEvent* SoundToPlay = bHitCharacter ? OwnerDrakhar->DraconicProjectileExplosionSoundEvent : OwnerDrakhar->DraconicProjectileImpactSoundEvent;
	if (SoundToPlay)
	{
		PlaySoundEvent(SoundToPlay, ImpactLocation);
	}
}

void UGS_DrakharAudioComponent::PlayComboFinisherSound()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (!CanSendRPC())
	{
		return;
	}

	LastMulticastTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	Multicast_PlayComboFinisherSound();
}

void UGS_DrakharAudioComponent::Multicast_PlayComboFinisherSound_Implementation()
{
	if (ShouldSkipListenServerRPC())
	{
		return;
	}

	if (!OwnerDrakhar || !OwnerDrakhar->ComboFinisherSoundEvent)
	{
		return;
	}

	// 오디오 시스템 검증
	if (!IsAudioSystemValid())
	{
		return;
	}

	UAkGameplayStatics::PostEvent(
		OwnerDrakhar->ComboFinisherSoundEvent,
		OwnerDrakhar,
		0,
		FOnAkPostEventCallback()
	);
}

// === 타이머 콜백 함수 구현 ===
void UGS_DrakharAudioComponent::ResetDraconicFurySoundCooldown()
{
	if (!IsValid(this)) return;

	bDraconicFurySoundPlayed = false;
}

void UGS_DrakharAudioComponent::ResetHurtSoundCooldown()
{
	if (!IsValid(this)) return;

	bHurtSoundPlayed = false;
}

// === Wwise 헬퍼 함수 구현 ===
void UGS_DrakharAudioComponent::PlaySoundEvent(UAkAudioEvent* SoundEvent, const FVector& Location)
{
	if (!OwnerDrakhar || !SoundEvent)
	{
		return;
	}

	if (!IsAudioSystemValid())
	{
		return;
	}

	if (Location != FVector::ZeroVector)
	{
		UAkGameplayStatics::PostEventAtLocation(SoundEvent, Location, FRotator::ZeroRotator, GetWorld());
	}
	else
	{
		UAkComponent* AkComp = GetOrCreateAkComponent();
		if (IsValid(AkComp))
		{
			AkComp->PostAkEvent(SoundEvent);
		}
	}
} 