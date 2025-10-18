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

void UGS_DrakharAudioComponent::PlayFeverModeEndSound()
{
	if(OwnerDrakhar)
	{
		PlaySoundEvent(OwnerDrakhar->FeverModeEndSoundEvent, OwnerDrakhar->GetActorLocation());
	}
}

void UGS_DrakharAudioComponent::PlayFeverModeStateSound()
{
	if (!OwnerDrakhar || !OwnerDrakhar->FeverModeStateSoundEvent)
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
		AudioDevice->StopPlayingID(FeverModeStateSoundPlayingID, FeverModeStateFadeOutDuration);
		FeverModeStateSoundPlayingID = AK_INVALID_PLAYING_ID;
	}
}

void UGS_DrakharAudioComponent::PlayHurtSound()
{
	if (!bHurtSoundPlayed && OwnerDrakhar)
	{
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

void UGS_DrakharAudioComponent::PlayComboFinisherSound()
{
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
	// 언리얼이 자동으로 생명주기 관리
	if (!IsValid(this)) return;

	bDraconicFurySoundPlayed = false;
}

void UGS_DrakharAudioComponent::ResetHurtSoundCooldown()
{
	// 언리얼이 자동으로 생명주기 관리
	if (!IsValid(this)) return;

	bHurtSoundPlayed = false;
}

// === Wwise 헬퍼 함수 구현 ===
void UGS_DrakharAudioComponent::PlaySoundEvent(UAkAudioEvent* SoundEvent, const FVector& Location)
{
	// 기본 체크
	if (!OwnerDrakhar || !SoundEvent)
	{
		return;
	}

	// 오디오 시스템 검증
	if (!IsAudioSystemValid())
	{
		return;
	}

	// 위치 기반 사운드 재생
	if (Location != FVector::ZeroVector)
	{
		UAkGameplayStatics::PostEventAtLocation(SoundEvent, Location, FRotator::ZeroRotator, GetWorld());
	}
	else
	{
		// 부모 클래스의 GetOrCreateAkComponent 사용
		UAkComponent* AkComp = GetOrCreateAkComponent();
		if (AkComp)
		{
			AkComp->PostAkEvent(SoundEvent);
		}
	}
} 