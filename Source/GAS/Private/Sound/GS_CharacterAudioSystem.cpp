// Fill out your copyright notice in the Description page of Project Settings.


#include "Sound/GS_CharacterAudioSystem.h"
#include "Sound/GS_AudioManager.h"

UGS_CharacterAudioSystem::UGS_CharacterAudioSystem()
{
    // 생성자에서 필요한 초기화 작업을 수행합니다.
}

void UGS_CharacterAudioSystem::PlayFootstepSound()
{

}

void UGS_CharacterAudioSystem::PlayCharacterCombatSound(FName CombatEventName, AActor* Context)
{
    
}

void UGS_CharacterAudioSystem::PlayCombatMusic(AActor* Context)
{
    if (!Context)
    {
        return;
    }

    // 전투 음악 이벤트 재생
    if (CombatMusicEvent)
    {
        UAkGameplayStatics::PostEvent(CombatMusicEvent, Context, 0, FOnAkPostEventCallback());
    }
}

void UGS_CharacterAudioSystem::StopCombatMusic(AActor* Context)
{
    if (!Context)
    {
        return;
    }

    // 전투 음악 중지
    UAkGameplayStatics::StopActor(Context);
}

