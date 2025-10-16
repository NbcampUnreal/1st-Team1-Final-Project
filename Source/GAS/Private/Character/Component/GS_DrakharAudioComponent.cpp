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
        }, DraconicFurySoundCooldown, false);
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

	// 데디케이티드 서버에서는 오디오 처리 불필요
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	// Wwise 오디오 디바이스 초기화 상태 확인
	FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
	if (!AudioDevice || !AudioDevice->IsInitialized())
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

	// 데디케이티드 서버에서는 오디오 처리 불필요
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	// Wwise 오디오 디바이스 초기화 상태 확인
	FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
	if (!AudioDevice || !AudioDevice->IsInitialized())
	{
		return;
	}

	// Playing ID가 유효하면 FAkAudioDevice를 통해 중지
	if (FeverModeStateSoundPlayingID != AK_INVALID_PLAYING_ID)
	{
		// FAkAudioDevice를 통해 StopPlayingID 호출 - 500ms 페이드아웃
		AudioDevice->StopPlayingID(FeverModeStateSoundPlayingID, 500);
		FeverModeStateSoundPlayingID = AK_INVALID_PLAYING_ID;
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
        }, HurtSoundCooldown, false);
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
    if (!OwnerDrakhar)
    {
        return;
    }
    if (!OwnerDrakhar->ComboFinisherSoundEvent)
    {
        return;
    }
    if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer)
    {
        return;
    }

    FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
    if (!AudioDevice || !AudioDevice->IsInitialized())
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

// === Wwise 헬퍼 함수 구현 ===
void UGS_DrakharAudioComponent::PlaySoundEvent(UAkAudioEvent* SoundEvent, const FVector& Location)
{
	// 기본 체크
	if (!OwnerDrakhar || !SoundEvent)
	{
		return;
	}
    
	// 데디케이티드 서버에서는 오디오 처리 불필요
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) 
	{
		return;
	}

	// Wwise 오디오 디바이스 초기화 상태 확인
	FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
	if (!AudioDevice || !AudioDevice->IsInitialized())
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