// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Character/GS_MonsterAnimInstance.h"
#include "Character/Player/Monster/GS_NeedleFang.h"

void UGS_MonsterAnimInstance::AnimNotify_SpawnProjectile()
{
	AGS_NeedleFang* NeedleFang = Cast<AGS_NeedleFang>(GetOwningActor());
	{
		if (NeedleFang->HasAuthority())
		{
			NeedleFang->Server_SpawnProjectile();
		}
	}
}
