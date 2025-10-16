#pragma once

#include "CoreMinimal.h"
#include "Sound/GS_AudioComponentBase.h"
#include "GS_DrakharAudioComponent.generated.h"

class UAkAudioEvent;
class AGS_Drakhar;

/**
 * 드라카르(가디언) 전용 오디오 컴포넌트
 * GS_AudioComponentBase를 상속받아 공통 기능 활용
 */
UCLASS( ClassGroup=(Audio), meta=(BlueprintSpawnableComponent) )
class GAS_API UGS_DrakharAudioComponent : public UGS_AudioComponentBase
{
	GENERATED_BODY()

public:	
	UGS_DrakharAudioComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// === Wwise 사운드 재생 함수 ===
	void PlayComboAttackSound();
	void PlayDashSkillSound();
	void PlayEarthquakeSkillSound();
	void PlayDraconicFurySkillSound();
	void PlayDraconicProjectileSound(const FVector& Location);
	void PlayAttackHitSound();
	void PlayFeverModeStartSound();
	void PlayFeverModeEndSound();
	void PlayFeverModeStateSound();
	void StopFeverModeStateSound();
	void PlayHurtSound();
	void PlayComboFinisherSound();
	void HandleDraconicProjectileImpact(const FVector& ImpactLocation, bool bHitCharacter);

private:
	UPROPERTY()
	TObjectPtr<AGS_Drakhar> OwnerDrakhar;

	// 사운드 중복 재생 방지
	bool bDraconicFurySoundPlayed;
	bool bHurtSoundPlayed;

	// 피버모드 스테이트 사운드 Playing ID 저장
	int32 FeverModeStateSoundPlayingID;

	// 쿨다운 값
	UPROPERTY(EditDefaultsOnly, Category = "Audio|Cooldown", meta=(ClampMin="0.1"))
	float DraconicFurySoundCooldown = 7.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Audio|Cooldown", meta=(ClampMin="0.1"))
	float HurtSoundCooldown = 1.0f;

	// 페이드아웃 시간 (ms)
	UPROPERTY(EditDefaultsOnly, Category = "Audio|FadeOut", meta=(ClampMin="0", ClampMax="5000"))
	int32 FeverModeStateFadeOutDuration = 500;

	// === 타이머 핸들 ===
	FTimerHandle DraconicFurySoundCooldownTimer;
	FTimerHandle HurtSoundCooldownTimer;

	// === 타이머 콜백 함수 ===
	UFUNCTION()
	void ResetDraconicFurySoundCooldown();

	UFUNCTION()
	void ResetHurtSoundCooldown();

	// === Wwise 관련 헬퍼 함수 ===
	void PlaySoundEvent(UAkAudioEvent* SoundEvent, const FVector& Location = FVector::ZeroVector);
}; 