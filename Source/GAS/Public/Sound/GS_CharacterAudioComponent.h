// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GS_CharacterAudioComponent.generated.h"

class UAkAudioEvent;
class UAkComponent;
class UGS_StatComp;
class AGS_Character;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAS_API UGS_CharacterAudioComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UGS_CharacterAudioComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 기본 스킬 이벤트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Character")
	UAkAudioEvent* SkillEvent;

	int32 SkillEventID;

	UFUNCTION(BlueprintCallable, Category = "Sound|Character")
	void PlaySkill();
	
	UFUNCTION(BlueprintCallable, Category = "Sound|Character")
	void StopSkill();
	
	// AkComponent 헬퍼 함수
	UFUNCTION(BlueprintCallable, Category = "Sound|Character")
	UAkComponent* GetOrCreateAkComponent();

	// 위치 기반 사운드 재생
	UFUNCTION(BlueprintCallable, Category = "Sound|Character")
	void PlaySoundAtLocation(UAkAudioEvent* SoundEvent, const FVector& Location);

	// 스킬 사운드 재생
	UFUNCTION(BlueprintCallable, Category = "Sound|Skill")
	void PlaySkillSoundFromDataTable(ESkillSlot SkillSlot, bool bIsSkillStart = true);

	// 스킬 루프 사운드 재생/정지 (궁극기용)
	UFUNCTION(BlueprintCallable, Category = "Sound|Skill")
	void PlaySkillLoopSoundFromDataTable(ESkillSlot SkillSlot);

	UFUNCTION(BlueprintCallable, Category = "Sound|Skill")
	void StopSkillLoopSoundFromDataTable(ESkillSlot SkillSlot);

	// 스킬 충돌 사운드 재생 (궁극기용)
	UFUNCTION(BlueprintCallable, Category = "Sound|Skill")
	void PlaySkillCollisionSoundFromDataTable(ESkillSlot SkillSlot, uint8 CollisionType);

	// Event-Driven 오디오 시스템
	UFUNCTION(BlueprintCallable, Category = "Sound|EventDriven")
	void RequestSkillAudio(ESkillSlot SkillSlot, int32 AudioEventType, FVector Location = FVector::ZeroVector);

	// 스킬셋 데이터 기반 사운드 재생 헬퍼 함수
	UFUNCTION(BlueprintCallable, Category = "Sound|Character")
	void PlaySkillSoundFromSkillInfo(bool bIsSkillStart, UAkAudioEvent* SkillStartSound, UAkAudioEvent* SkillEndSound);

public:
	// 콤보 공격 사운드 재생 (근접 공격 시커)
	UFUNCTION(BlueprintCallable, Category = "Sound|Combo")
	void PlayComboAttackSound(UAkAudioEvent* SwingSound, UAkAudioEvent* VoiceSound, UAkAudioEvent* StopEvent, float ResetTime);
	
	// 콤보 인덱스별 공격 사운드 재생 (개선된 버전)
	UFUNCTION(BlueprintCallable, Category = "Sound|Combo")
	void PlayComboAttackSoundByIndex(int32 ComboIndex, const TArray<UAkAudioEvent*>& SwingSounds, const TArray<UAkAudioEvent*>& VoiceSounds, UAkAudioEvent* StopEvent, float ResetTime);

	// 콤보 인덱스별 공격 사운드 재생 (추가 사운드 포함)
	UFUNCTION(BlueprintCallable, Category = "Sound|Combo")
	void PlayComboAttackSoundByIndexWithExtra(int32 ComboIndex, const TArray<UAkAudioEvent*>& SwingSounds, const TArray<UAkAudioEvent*>& VoiceSounds, const TArray<UAkAudioEvent*>& ExtraSounds, UAkAudioEvent* StopEvent, float ResetTime);
	
	// 콤보 마지막 타격 특별 사운드 (근접 공격 시커)
	UFUNCTION(BlueprintCallable, Category = "Sound|Combo")
	void PlayFinalAttackSound(UAkAudioEvent* ExtraSound);

	// 단일 사운드 재생 (원거리 공격 시커)
	UFUNCTION(BlueprintCallable, Category = "Sound|Generic")
	void PlaySound(UAkAudioEvent* SoundToPlay, bool bPlayOnLocalOnly = false);

	// 피격 사운드 재생
	UFUNCTION(BlueprintCallable, Category = "Sound|Hit")
	void PlayHitSound();

public:
	// 피격 사운드 이벤트
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound|Hit")
	UAkAudioEvent* HitSoundEvent;
	
	// 히트 사운드 쿨다운 시간
	UPROPERTY(EditDefaultsOnly, Category = "Sound|Hit", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float HitSoundCooldownTime = 1.75f;

private:
	// 콤보 공격 사운드 중지 콜백
	void ResetAttackSoundSequence();

	// 콤보 사운드 중지 이벤트 (내부 사용)
	UPROPERTY()
	UAkAudioEvent* CurrentStopEvent;

	// 마지막 히트 사운드 재생 시간 (인스턴스별)
	float LastHitSoundTime = 0.0f;

protected:
	// DT_SkillSet에서 스킬 정보 조회
	const struct FSkillInfo* GetSkillInfoFromDataTable(ESkillSlot SkillSlot) const;

private:
	UPROPERTY()
	UAkComponent* CachedAkComponent;

	FTimerHandle AttackSoundResetTimerHandle;
};
