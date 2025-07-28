// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GS_CharacterAudioComponent.generated.h"

class UAkAudioEvent;
class UAkComponent;

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

	// 스킬 충돌 사운드 재생 (궁극기용)
	UFUNCTION(BlueprintCallable, Category = "Sound|Skill")
	void PlaySkillCollisionSoundFromDataTable(ESkillSlot SkillSlot, uint8 CollisionType);

	// Event-Driven 오디오 시스템
	UFUNCTION(BlueprintCallable, Category = "Sound|EventDriven")
	void RequestSkillAudio(ESkillSlot SkillSlot, int32 AudioEventType, FVector Location = FVector::ZeroVector);

	// 스킬셋 데이터 기반 사운드 재생 헬퍼 함수
	UFUNCTION(BlueprintCallable, Category = "Sound|Character")
	void PlaySkillSoundFromSkillInfo(bool bIsSkillStart, UAkAudioEvent* SkillStartSound, UAkAudioEvent* SkillEndSound);

protected:
	// DT_SkillSet에서 스킬 정보 조회
	const struct FSkillInfo* GetSkillInfoFromDataTable(ESkillSlot SkillSlot) const;

private:
	UPROPERTY()
	UAkComponent* CachedAkComponent;
};
