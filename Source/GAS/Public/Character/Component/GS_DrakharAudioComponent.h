#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GS_DrakharAudioComponent.generated.h"

class UAkAudioEvent;
class UAkComponent;
class AGS_Drakhar;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAS_API UGS_DrakharAudioComponent : public UActorComponent
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
	void HandleDraconicProjectileImpact(const FVector& ImpactLocation, bool bHitCharacter);

private:
	UPROPERTY()
	TObjectPtr<AGS_Drakhar> OwnerDrakhar;

	// 사운드 중복 재생 방지
	bool bDraconicFurySoundPlayed;
	bool bHurtSoundPlayed;

	// === Wwise 관련 헬퍼 함수 ===
	void PlaySoundEvent(UAkAudioEvent* SoundEvent, const FVector& Location = FVector::ZeroVector);
	UAkComponent* GetOrCreateAkComponent();
}; 