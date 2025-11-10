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
	// 컴포넌트 유효성 검증
	if (!IsValid(this))
	{
		return;
	}

	// 월드 컨텍스트 유효성 검증
	UWorld* World = GetWorld();
	if (!World || !World->IsValidLowLevel() || World->bIsTearingDown)
	{
		return;
	}

	// 오너 유효성 검증
	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !Owner->HasAuthority())
	{
		return;
	}

	if (!CanSendRPC())
	{
		return;
	}

	LastMulticastTime = World->GetTimeSeconds();
	Multicast_PlayComboAttackSound();
}

void UGS_DrakharAudioComponent::Multicast_PlayComboAttackSound_Implementation()
{
	if (ShouldSkipListenServerRPC())
	{
		return;
	}

	// 통합 체크 및 Distance Scaling 설정 (보스는 항상 재생, bSkipViewFrustumCheck = true)
	if (!PrepareMulticastSound(OwnerDrakhar, true))
	{
		return;
	}

	PlaySoundEvent(OwnerDrakhar->ComboAttackSoundEvent, OwnerDrakhar->GetActorLocation());
}

void UGS_DrakharAudioComponent::PlayDashSkillSound()
{
	// 컴포넌트 유효성 검증
	if (!IsValid(this))
	{
		return;
	}

	// 월드 컨텍스트 유효성 검증
	UWorld* World = GetWorld();
	if (!World || !World->IsValidLowLevel() || World->bIsTearingDown)
	{
		return;
	}

	// 오너 유효성 검증
	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !Owner->HasAuthority())
	{
		return;
	}

	if (!CanSendRPC())
	{
		return;
	}

	LastMulticastTime = World->GetTimeSeconds();
	Multicast_PlayDashSkillSound();
}

void UGS_DrakharAudioComponent::Multicast_PlayDashSkillSound_Implementation()
{
	if (ShouldSkipListenServerRPC())
	{
		return;
	}

	// 통합 체크 및 Distance Scaling 설정 (보스는 항상 재생, bSkipViewFrustumCheck = true)
	if (!PrepareMulticastSound(OwnerDrakhar, true))
	{
		return;
	}

	PlaySoundEvent(OwnerDrakhar->DashSkillSoundEvent, OwnerDrakhar->GetActorLocation());
}

void UGS_DrakharAudioComponent::PlayEarthquakeSkillSound()
{
	// 컴포넌트 유효성 검증
	if (!IsValid(this))
	{
		return;
	}

	// 월드 컨텍스트 유효성 검증
	UWorld* World = GetWorld();
	if (!World || !World->IsValidLowLevel() || World->bIsTearingDown)
	{
		return;
	}

	// 오너 유효성 검증
	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !Owner->HasAuthority())
	{
		return;
	}

	if (!CanSendRPC())
	{
		return;
	}

	LastMulticastTime = World->GetTimeSeconds();
	Multicast_PlayEarthquakeSkillSound();
}

void UGS_DrakharAudioComponent::Multicast_PlayEarthquakeSkillSound_Implementation()
{
	if (ShouldSkipListenServerRPC())
	{
		return;
	}

	// 통합 체크 및 Distance Scaling 설정 (보스는 항상 재생, bSkipViewFrustumCheck = true)
	if (!PrepareMulticastSound(OwnerDrakhar, true))
	{
		return;
	}

	PlaySoundEvent(OwnerDrakhar->EarthquakeSkillSoundEvent, OwnerDrakhar->GetActorLocation());
}

void UGS_DrakharAudioComponent::PlayDraconicFurySkillSound()
{
	// 컴포넌트 유효성 검증
	if (!IsValid(this))
	{
		return;
	}

	// 월드 컨텍스트 유효성 검증
	UWorld* World = GetWorld();
	if (!World || !World->IsValidLowLevel() || World->bIsTearingDown)
	{
		return;
	}

	// 오너 유효성 검증
	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !Owner->HasAuthority())
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

	LastMulticastTime = World->GetTimeSeconds();
	Multicast_PlayDraconicFurySkillSound();
}

void UGS_DrakharAudioComponent::Multicast_PlayDraconicFurySkillSound_Implementation()
{
	if (ShouldSkipListenServerRPC())
	{
		return;
	}

	// 통합 체크 및 Distance Scaling 설정 (보스는 항상 재생, bSkipViewFrustumCheck = true)
	if (!PrepareMulticastSound(OwnerDrakhar, true))
	{
		return;
	}

	if (bDraconicFurySoundPlayed)
	{
		return;
	}

	PlaySoundEvent(OwnerDrakhar->DraconicFurySkillSoundEvent, OwnerDrakhar->GetActorLocation());
	bDraconicFurySoundPlayed = true;

	// 타이머 설정 (PrepareMulticastSound가 World 검증을 완료했으므로 안전)
	UWorld* World = GetWorld();
	World->GetTimerManager().SetTimer(
		DraconicFurySoundCooldownTimer,
		this,
		&UGS_DrakharAudioComponent::ResetDraconicFurySoundCooldown,
		DraconicFurySoundCooldown,
		false
	);
}

void UGS_DrakharAudioComponent::PlayDraconicProjectileSound(const FVector& Location)
{
	// 컴포넌트 유효성 검증
	if (!IsValid(this))
	{
		return;
	}

	// 월드 컨텍스트 유효성 검증
	UWorld* World = GetWorld();
	if (!World || !World->IsValidLowLevel() || World->bIsTearingDown)
	{
		return;
	}

	// 오너 유효성 검증
	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !Owner->HasAuthority())
	{
		return;
	}

	if (!CanSendRPC())
	{
		return;
	}

	LastMulticastTime = World->GetTimeSeconds();
	Multicast_PlayDraconicProjectileSound(Location);
}

void UGS_DrakharAudioComponent::Multicast_PlayDraconicProjectileSound_Implementation(const FVector& Location)
{
	if (ShouldSkipListenServerRPC())
	{
		return;
	}

	// 통합 체크 및 Distance Scaling 설정 (보스는 항상 재생, bSkipViewFrustumCheck = true)
	if (!PrepareMulticastSound(OwnerDrakhar, true))
	{
		return;
	}

	PlaySoundEvent(OwnerDrakhar->DraconicProjectileSoundEvent, Location);
}

void UGS_DrakharAudioComponent::PlayAttackHitSound()
{
	// 컴포넌트 유효성 검증
	if (!IsValid(this))
	{
		return;
	}

	// 월드 컨텍스트 유효성 검증
	UWorld* World = GetWorld();
	if (!World || !World->IsValidLowLevel() || World->bIsTearingDown)
	{
		return;
	}

	// 오너 유효성 검증
	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !Owner->HasAuthority())
	{
		return;
	}

	if (!CanSendRPC())
	{
		return;
	}

	LastMulticastTime = World->GetTimeSeconds();
	Multicast_PlayAttackHitSound();
}

void UGS_DrakharAudioComponent::Multicast_PlayAttackHitSound_Implementation()
{
	if (ShouldSkipListenServerRPC())
	{
		return;
	}

	// 통합 체크 및 Distance Scaling 설정 (보스는 항상 재생, bSkipViewFrustumCheck = true)
	if (!PrepareMulticastSound(OwnerDrakhar, true))
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
	// 컴포넌트 유효성 검증
	if (!IsValid(this))
	{
		return;
	}

	// 월드 컨텍스트 유효성 검증
	UWorld* World = GetWorld();
	if (!World || !World->IsValidLowLevel() || World->bIsTearingDown)
	{
		return;
	}

	// 오너 유효성 검증
	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !Owner->HasAuthority())
	{
		return;
	}

	if (!CanSendRPC())
	{
		return;
	}

	LastMulticastTime = World->GetTimeSeconds();
	Multicast_PlayFeverModeStartSound();
}

void UGS_DrakharAudioComponent::Multicast_PlayFeverModeStartSound_Implementation()
{
	if (ShouldSkipListenServerRPC())
	{
		return;
	}

	// 통합 체크 및 Distance Scaling 설정 (보스는 항상 재생, bSkipViewFrustumCheck = true)
	if (!PrepareMulticastSound(OwnerDrakhar, true))
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
	// 컴포넌트 유효성 검증
	if (!IsValid(this))
	{
		return;
	}

	// 월드 컨텍스트 유효성 검증
	UWorld* World = GetWorld();
	if (!World || !World->IsValidLowLevel() || World->bIsTearingDown)
	{
		return;
	}

	// 오너 유효성 검증
	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !Owner->HasAuthority())
	{
		return;
	}

	if (!CanSendRPC())
	{
		return;
	}

	LastMulticastTime = World->GetTimeSeconds();
	Multicast_PlayFeverModeEndSound();
}

void UGS_DrakharAudioComponent::Multicast_PlayFeverModeEndSound_Implementation()
{
	if (ShouldSkipListenServerRPC())
	{
		return;
	}

	// 통합 체크 및 Distance Scaling 설정 (보스는 항상 재생, bSkipViewFrustumCheck = true)
	if (!PrepareMulticastSound(OwnerDrakhar, true))
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
	// 컴포넌트 유효성 검증
	if (!IsValid(this))
	{
		return;
	}

	// 월드 컨텍스트 유효성 검증
	UWorld* World = GetWorld();
	if (!World || !World->IsValidLowLevel() || World->bIsTearingDown)
	{
		return;
	}

	// 오너 유효성 검증
	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !Owner->HasAuthority())
	{
		return;
	}

	if (!CanSendRPC())
	{
		return;
	}

	LastMulticastTime = World->GetTimeSeconds();
	Multicast_PlayFeverModeStateSound();
}

void UGS_DrakharAudioComponent::Multicast_PlayFeverModeStateSound_Implementation()
{
	if (ShouldSkipListenServerRPC())
	{
		return;
	}

	// 통합 체크 및 Distance Scaling 설정 (보스는 항상 재생, bSkipViewFrustumCheck = true)
	if (!PrepareMulticastSound(OwnerDrakhar, true))
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
	// 컴포넌트 유효성 검증
	if (!IsValid(this))
	{
		return;
	}

	// 월드 컨텍스트 유효성 검증
	UWorld* World = GetWorld();
	if (!World || !World->IsValidLowLevel() || World->bIsTearingDown)
	{
		return;
	}

	// 오너 유효성 검증
	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !Owner->HasAuthority())
	{
		return;
	}

	if (!CanSendRPC())
	{
		return;
	}

	LastMulticastTime = World->GetTimeSeconds();
	Multicast_StopFeverModeStateSound();
}

void UGS_DrakharAudioComponent::Multicast_StopFeverModeStateSound_Implementation()
{
	if (ShouldSkipListenServerRPC())
	{
		return;
	}

	// 통합 체크 및 Distance Scaling 설정 (Stop은 거리 체크 불필요, bSkipViewFrustumCheck = true)
	if (!PrepareMulticastSound(OwnerDrakhar, true))
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

// 로컬 전용 Hurt 사운드 재생 (RPC 없음 - RepNotify에서 호출)
void UGS_DrakharAudioComponent::PlayHurtSoundLocal()
{
	// 통합 체크 및 Distance Scaling 설정 (보스는 항상 재생, bSkipViewFrustumCheck = true)
	if (!PrepareMulticastSound(OwnerDrakhar, true))
	{
		return;
	}

	if (bHurtSoundPlayed)
	{
		return;
	}

	PlaySoundEvent(OwnerDrakhar->HurtSoundEvent, OwnerDrakhar->GetActorLocation());
	bHurtSoundPlayed = true;

	// 타이머 설정 (PrepareMulticastSound가 World 검증을 완료했으므로 안전)
	UWorld* World = GetWorld();
	World->GetTimerManager().SetTimer(
		HurtSoundCooldownTimer,
		this,
		&UGS_DrakharAudioComponent::ResetHurtSoundCooldown,
		HurtSoundCooldown,
		false
	);
}

// 로컬 전용 Death 사운드 재생 (RPC 없음 - RepNotify에서 호출)
void UGS_DrakharAudioComponent::PlayDeathSoundLocal()
{
	// 통합 체크 및 Distance Scaling 설정
	if (!PrepareMulticastSound(OwnerDrakhar, true))
	{
		return;
	}

	// Death Sound 재생
	if (IsValid(OwnerDrakhar->DeathSoundEvent))
	{
		PlaySoundEvent(OwnerDrakhar->DeathSoundEvent, OwnerDrakhar->GetActorLocation());
	}
}

void UGS_DrakharAudioComponent::PlayDraconicProjectileImpactSoundLocal(const FVector& ImpactLocation, bool bHitCharacter)
{
	// 컴포넌트 유효성 검증
	if (!IsValid(this))
	{
		return;
	}

	// 월드 컨텍스트 유효성 검증
	UWorld* World = GetWorld();
	if (!World || !World->IsValidLowLevel() || World->bIsTearingDown)
	{
		return;
	}

	// 통합 체크 및 Distance Scaling 설정 (보스는 항상 재생, bSkipViewFrustumCheck = true)
	if (!PrepareMulticastSound(OwnerDrakhar, true))
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
	// 컴포넌트 유효성 검증
	if (!IsValid(this))
	{
		return;
	}

	// 월드 컨텍스트 유효성 검증
	UWorld* World = GetWorld();
	if (!World || !World->IsValidLowLevel() || World->bIsTearingDown)
	{
		return;
	}

	// 오너 유효성 검증
	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !Owner->HasAuthority())
	{
		return;
	}

	if (!CanSendRPC())
	{
		return;
	}

	LastMulticastTime = World->GetTimeSeconds();
	Multicast_PlayComboFinisherSound();
}

void UGS_DrakharAudioComponent::Multicast_PlayComboFinisherSound_Implementation()
{
	if (ShouldSkipListenServerRPC())
	{
		return;
	}

	// 통합 체크 및 Distance Scaling 설정 (보스는 항상 재생, bSkipViewFrustumCheck = true)
	if (!PrepareMulticastSound(OwnerDrakhar, true))
	{
		return;
	}

	if (!OwnerDrakhar->ComboFinisherSoundEvent)
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