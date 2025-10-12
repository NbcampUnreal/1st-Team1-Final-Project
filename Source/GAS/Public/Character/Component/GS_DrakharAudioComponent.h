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

public:
	// === Wwise 사운드 재생 함수 ===
	void PlayComboAttackSound();
	void PlayDashSkillSound();
	void PlayEarthquakeSkillSound();
	void PlayDraconicFurySkillSound();
	void PlayDraconicProjectileSound(const FVector& Location);
	void PlayAttackHitSound();
	void PlayFeverModeStartSound();
	void PlayHurtSound();
	void PlayComboFinisherSound();
	void HandleDraconicProjectileImpact(const FVector& ImpactLocation, bool bHitCharacter);

private:
	UPROPERTY()
	TObjectPtr<AGS_Drakhar> OwnerDrakhar;

	// 사운드 중복 재생 방지
	bool bDraconicFurySoundPlayed;
	bool bHurtSoundPlayed;

	// 쿨다운 값
	UPROPERTY(EditDefaultsOnly, Category = "Audio|Cooldown", meta=(ClampMin="0.1"))
	float DraconicFurySoundCooldown = 7.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Audio|Cooldown", meta=(ClampMin="0.1"))
	float HurtSoundCooldown = 1.0f;

	// === Wwise 관련 헬퍼 함수 ===
	void PlaySoundEvent(UAkAudioEvent* SoundEvent, const FVector& Location = FVector::ZeroVector);
}; 