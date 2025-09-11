// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Monster/GS_IronFang.h"
#include "Weapon/Equipable/GS_WeaponSword.h"

AGS_IronFang::AGS_IronFang()
{
}

void AGS_IronFang::BeginPlay()
{
	Super::BeginPlay();

	// IronFang 전용 몬스터 오디오 설정 (컴포넌트 사용)
	if (MonsterAudioComponent)
	{
		// 중간 몬스터 특성: 기본 거리 사용
		MonsterAudioComponent->AudioConfig.AlertDistance = 800.0f;
		MonsterAudioComponent->AudioConfig.MaxAudioDistance = 3000.0f;

		// 사운드 재생 간격 (기본값)
		MonsterAudioComponent->IdleSoundInterval = 5.0f;
		MonsterAudioComponent->CombatSoundInterval = 3.0f;
	}
}
