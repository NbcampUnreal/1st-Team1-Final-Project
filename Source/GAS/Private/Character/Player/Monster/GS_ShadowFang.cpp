// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Monster/GS_ShadowFang.h"
#include "Character/Skill/Monster/GS_MonsterSkillComp.h"


AGS_ShadowFang::AGS_ShadowFang()
{
}

void AGS_ShadowFang::BeginPlay()
{
	Super::BeginPlay();
	
	// ShadowFang 전용 몬스터 오디오 설정 (컴포넌트 사용)
	if (MonsterAudioComponent)
	{
		MonsterAudioComponent->MonsterSoundVariant = 3; // ShadowFang = 3

		// 큰 몬스터 특성: 더 먼 거리에서 경계, 긴 최대 거리
		MonsterAudioComponent->AudioConfig.AlertDistance = 1000.0f;
		MonsterAudioComponent->AudioConfig.MaxAudioDistance = 4000.0f;

		// 사운드 재생 간격 (큰 몬스터이므로 느리게 울음)
		MonsterAudioComponent->IdleSoundInterval = 8.0f;
		MonsterAudioComponent->CombatSoundInterval = 5.0f;
	}
}

void AGS_ShadowFang::UseSkill()
{
	if (MonsterSkillComp)
	{
		MonsterSkillComp->TryActivateSkill();
	}
}
